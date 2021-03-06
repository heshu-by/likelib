#include "core.hpp"

#include "base/log.hpp"
#include "vm/tools.hpp"

#include <algorithm>
#include <iterator>


namespace lk
{

Core::Core(const base::PropertyTree& config, const base::KeyVault& key_vault)
  : _config{ config }
  , _vault{ key_vault }
  , _this_node_address{ _vault.getPublicKey() }
  , _blockchain{ _config }
  , _network{ _config, *this }
  , _eth_adapter{ *this, _account_manager, _code_manager }
{
    [[maybe_unused]] bool result = _blockchain.tryAddBlock(getGenesisBlock());
    ASSERT(result);
    _account_manager.updateFromGenesis(getGenesisBlock());
    _is_account_manager_updated = true;

    _blockchain.load();
    for (bc::BlockDepth d = 1; d <= _blockchain.getTopBlock().getDepth(); ++d) {
        auto block = *_blockchain.findBlock(*_blockchain.findBlockHashByDepth(d));
        _account_manager.update(block);
    }
}


const bc::Block& Core::getGenesisBlock()
{
    static bc::Block genesis = [] {
        auto timestamp = base::Time(1583789617);

        bc::Block ret{ 0, base::Sha256(base::Bytes(32)), timestamp, bc::Address::null(), {} };
        bc::Address from{ bc::Address::null() };
        bc::Address to{ "28dpzpURpyqqLoWrEhnHrajndeCq" };
        bc::Balance amount{ 0xFFFFFFFF };
        bc::Balance fee{ 0 };

        ret.addTransaction({ from, to, amount, fee, timestamp, bc::Transaction::Type::MESSAGE_CALL, base::Bytes{} });
        return ret;
    }();
    return genesis;
}


void Core::run()
{
    _network.run();
}


bool Core::addPendingTransaction(const bc::Transaction& tx)
{
    if (checkTransaction(tx)) {
        {
            LOG_DEBUG << "Adding tx to pending";
            std::unique_lock lk(_pending_transactions_mutex);
            _pending_transactions.add(tx);
        }
        _event_new_pending_transaction.notify(tx);
        return true;
    }
    return false;
}


void Core::addPendingTransactionAndWait(const bc::Transaction& tx)
{
    if (!checkTransaction(tx)) {
        RAISE_ERROR(base::InvalidArgument, "invalid transaction");
    }

    std::condition_variable cv;
    std::mutex mt;
    bool is_tx_mined = false;

    auto id = _event_block_added.subscribe([&cv, &tx, &is_tx_mined](const bc::Block& block) {
        if (block.getTransactions().find(tx)) {
            is_tx_mined = true;
            cv.notify_all();
        }
    });

    addPendingTransaction(tx);

    std::unique_lock lk(mt);
    cv.wait(lk, [&is_tx_mined] { return is_tx_mined; });
    _event_block_added.unsubscribe(id);
}


base::Bytes Core::getTransactionOutput(const base::Sha256& tx)
{
    std::shared_lock lk(_tx_outputs_mutex);
    if (auto it = _tx_outputs.find(tx); it != _tx_outputs.end()) {
        return it->second;
    }
    else {
        return {};
    }
}


bool Core::tryAddBlock(const bc::Block& b)
{
    if (checkBlock(b) && _blockchain.tryAddBlock(b)) {
        {
            std::shared_lock lk(_pending_transactions_mutex);
            _pending_transactions.remove(b.getTransactions());
        }
        LOG_DEBUG << "Applying transactions from block #" << b.getDepth();
        applyBlockTransactions(b);
        _event_block_added.notify(b);
        return true;
    }
    else {
        return false;
    }
}


std::optional<bc::Block> Core::findBlock(const base::Sha256& hash) const
{
    return _blockchain.findBlock(hash);
}


std::optional<base::Sha256> Core::findBlockHash(const bc::BlockDepth& depth) const
{
    return _blockchain.findBlockHashByDepth(depth);
}


bool Core::checkBlock(const bc::Block& b) const
{
    if (_blockchain.findBlock(base::Sha256::compute(base::toBytes(b)))) {
        return false;
    }

    // FIXME: this works wrong if two transactions are both valid, but together are not
    for (const auto& tx : b.getTransactions()) {
        if (!_account_manager.checkTransaction(tx)) {
            return false;
        }
    }

    return true;
}


bool Core::checkTransaction(const bc::Transaction& tx) const
{
    if (!tx.checkSign()) {
        LOG_DEBUG << "Failed signature verification";
        return false;
    }

    if (_blockchain.findTransaction(base::Sha256::compute(base::toBytes(tx)))) {
        return false;
    }

    std::map<bc::Address, bc::Balance> current_pending_balance;
    {
        std::shared_lock lk(_pending_transactions_mutex);
        if (_pending_transactions.find(tx)) {
            return false;
        }

        current_pending_balance = bc::calcBalance(_pending_transactions);
    }

    auto pending_from_account_balance = current_pending_balance.find(tx.getFrom());
    if (pending_from_account_balance != current_pending_balance.end()) {
        auto current_from_account_balance = _account_manager.getBalance(tx.getFrom());
        return pending_from_account_balance->second + current_from_account_balance >= tx.getAmount();
    }
    else {
        return _account_manager.checkTransaction(tx);
    }
}


bc::Block Core::getBlockTemplate() const
{
    const auto& top_block = _blockchain.getTopBlock();
    bc::BlockDepth depth = top_block.getDepth() + 1;
    auto prev_hash = base::Sha256::compute(base::toBytes(top_block));
    std::shared_lock lk(_pending_transactions_mutex);
    return bc::Block{ depth, prev_hash, base::Time::now(), getThisNodeAddress(), _pending_transactions };
}


bc::Balance Core::getBalance(const bc::Address& address) const
{
    return _account_manager.getBalance(address);
}


const bc::Block& Core::getTopBlock() const
{
    return _blockchain.getTopBlock();
}


const bc::Address& Core::getThisNodeAddress() const noexcept
{
    return _this_node_address;
}


void Core::applyBlockTransactions(const bc::Block& block)
{
    if (!_is_account_manager_updated) {
        _account_manager.updateFromGenesis(block);
        _is_account_manager_updated = true;
    }
    else {
        for (const auto& tx : block.getTransactions()) {
            tryPerformTransaction(tx, block);
        }
        constexpr bc::Balance EMISSION_VALUE = 1000;
        _account_manager.getAccount(block.getCoinbase()).addBalance(EMISSION_VALUE);
    }
}


bool Core::tryPerformTransaction(const bc::Transaction& tx, const bc::Block& block_where_tx)
{
    auto hash = base::Sha256::compute(base::toBytes(tx));
    if (tx.getType() == bc::Transaction::Type::CONTRACT_CREATION) {
        try {
            _account_manager.getAccount(tx.getFrom()).subBalance(tx.getFee());
            auto [address, result, gas_left] = doContractCreation(tx, block_where_tx);
            LOG_DEBUG << "Contract created at " << address << " with output = " << base::toHex<base::Bytes>(result);
            base::SerializationOArchive oa;
            oa.serialize(true);
            oa.serialize(address);
            oa.serialize(result);
            oa.serialize(gas_left);
            {
                std::unique_lock lk(_tx_outputs_mutex);
                _tx_outputs[hash] = std::move(oa).getBytes();
            }
            _account_manager.getAccount(block_where_tx.getCoinbase()).addBalance(tx.getFee() - gas_left);
            _account_manager.getAccount(tx.getFrom()).addBalance(gas_left);
        }
        catch (const base::Error&) {
            return false;
        }
        return true;
    }
    else {
        try {
            _account_manager.getAccount(tx.getFrom()).subBalance(tx.getFee());
            auto result = doMessageCall(tx, block_where_tx);
            LOG_DEBUG << "Message call result: " << base::toHex(result.toOutputData());
            base::SerializationOArchive oa;
            oa.serialize(true);
            oa.serialize(result.toOutputData());
            oa.serialize(result.gasLeft());
            {
                std::unique_lock lk(_tx_outputs_mutex);
                _tx_outputs[hash] = std::move(oa).getBytes();
            }
            _account_manager.getAccount(block_where_tx.getCoinbase()).addBalance(tx.getFee() - result.gasLeft());
            _account_manager.getAccount(tx.getFrom()).addBalance(result.gasLeft());
            return true;
        }
        catch (const base::Error&) {
            return false;
        }
    }
}


std::tuple<bc::Address, base::Bytes, bc::Balance> Core::doContractCreation(const bc::Transaction& tx,
                                                                           const bc::Block& block_where_tx)
{
    base::SerializationIArchive ia(tx.getData());
    auto contract_data = ia.deserialize<bc::ContractInitData>();

    auto hash = base::Sha256::compute(contract_data.getCode());
    _code_manager.saveCode(contract_data.getCode());

    bc::Address contract_address = _account_manager.newContract(tx.getFrom(), hash);
    LOG_DEBUG << "Deploying smart contract at address " << contract_address;
    if (tx.getAmount() != 0) {
        if (!_account_manager.tryTransferMoney(tx.getFrom(), tx.getTo(), tx.getAmount())) {
            RAISE_ERROR(base::Error, "cannot transfer money");
        }
    }

    return _eth_adapter.createContract(contract_address, tx, block_where_tx);
}


vm::ExecutionResult Core::doMessageCall(const bc::Transaction& tx, const bc::Block& block_where_tx)
{
    auto code_hash = _account_manager.getAccount(tx.getTo()).getCodeHash();

    if (!_account_manager.tryTransferMoney(tx.getFrom(), tx.getTo(), tx.getAmount())) {
        RAISE_ERROR(base::Error, "cannot transfer money");
    }

    if (code_hash != base::Sha256::null()) {
        // if we're here -- do a call to a contract
        return _eth_adapter.call(tx, block_where_tx);
    }
    else {
        return {};
    }
}


void Core::subscribeToBlockAddition(decltype(Core::_event_block_added)::CallbackType callback)
{
    _event_block_added.subscribe(std::move(callback));
}


void Core::subscribeToNewPendingTransaction(decltype(Core::_event_new_pending_transaction)::CallbackType callback)
{
    _event_new_pending_transaction.subscribe(std::move(callback));
}


} // namespace lk

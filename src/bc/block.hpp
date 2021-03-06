#pragma once

#include "base/bytes.hpp"
#include "base/hash.hpp"
#include "base/serialization.hpp"
#include "bc/transaction.hpp"
#include "bc/transactions_set.hpp"
#include "bc/types.hpp"

#include <iosfwd>

namespace bc
{

class Block
{
  public:
    //=================
    Block(bc::BlockDepth depth,
          base::Sha256 prev_block_hash,
          base::Time timestamp,
          bc::Address coinbase,
          TransactionsSet txs);

    Block(const Block&) = default;
    Block(Block&&) = default;

    Block& operator=(const Block&) = default;
    Block& operator=(Block&&) = default;

    ~Block() = default;
    //=================
    void serialize(base::SerializationOArchive& oa) const;
    [[nodiscard]] static Block deserialize(base::SerializationIArchive& ia);
    //=================
    BlockDepth getDepth() const noexcept;
    const base::Sha256& getPrevBlockHash() const;
    const TransactionsSet& getTransactions() const;
    NonceInt getNonce() const noexcept;
    const base::Time& getTimestamp() const noexcept;
    const bc::Address& getCoinbase() const noexcept;
    //=================
    void setDepth(BlockDepth depth) noexcept;
    void setNonce(NonceInt nonce) noexcept;
    void setPrevBlockHash(const base::Sha256& prev_block_hash);
    void setTransactions(TransactionsSet txs);
    void addTransaction(const Transaction& tx);
    //=================
  private:
    //=================
    bc::BlockDepth _depth;
    NonceInt _nonce;
    base::Sha256 _prev_block_hash;
    base::Time _timestamp;
    bc::Address _coinbase;
    TransactionsSet _txs;
    //=================
};

std::ostream& operator<<(std::ostream& os, const Block& block);

bool operator==(const bc::Block& a, const bc::Block& b);
bool operator!=(const bc::Block& a, const bc::Block& b);

} // namespace bc

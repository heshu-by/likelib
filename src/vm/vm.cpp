#include "vm.hpp"
#include "error.hpp"
#include "tools.hpp"

#include "base/log.hpp"

#include <evmc/loader.h>

#include <filesystem>

namespace vm
{

namespace
{

base::Bytes getRuntimeCode(const base::Bytes& full_code)
{
    static const std::string target{ "60806040" };
    // TODO: make normal algorithm
    auto hex_code = base::toHex<base::Bytes>(full_code);
    auto index = hex_code.find(target, target.size());
    if (index == 0) {
        RAISE_ERROR(base::LogicError, "Not valid code");
    }
    auto sub = hex_code.substr(index, hex_code.size());
    return base::fromHex<base::Bytes>(sub);
}

} // namespace


SmartContract::SmartContract(const base::Bytes& contract_code)
  : _revision(EVMC_ISTANBUL)
  , _contract_code{ contract_code }
{}


SmartContractMessage SmartContract::createInitMessage(int64_t gas,
                                                      const bc::Address& source,
                                                      const bc::Address& destination,
                                                      const bc::Balance& value,
                                                      const base::Bytes& input) const
{
    SmartContractMessage message{ _revision };
    message._contract_code = _contract_code + input;
    message._message.kind = evmc_call_kind::EVMC_CALL;
    message._message.depth = 0;
    message._message.gas = gas;
    message._message.sender = toEthAddress(source);
    message._message.destination = toEthAddress(destination);
    message._message.value = toEvmcUint256(value);
    message._message.create2_salt = evmc_bytes32();
    return message;
}


SmartContractMessage SmartContract::createMessage(int64_t gas,
                                                  const bc::Address& source,
                                                  const bc::Address& destination,
                                                  const bc::Balance& value,
                                                  const base::Bytes& input) const
{
    SmartContractMessage message{ _revision };
    message._contract_code = getRuntimeCode(_contract_code);
    message._message.kind = evmc_call_kind::EVMC_CALL;
    message._message.depth = 0;
    message._message.gas = gas;
    message._message.sender = toEthAddress(source);
    message._message.destination = toEthAddress(destination);
    message._message.value = toEvmcUint256(value);
    message._message.create2_salt = evmc_bytes32();
    message._input_data = input;
    message._message.input_data = message._input_data.getData();
    message._message.input_size = message._input_data.size();
    return message;
}


const evmc_message& SmartContractMessage::getMessage() const
{
    return _message;
}


int64_t SmartContractMessage::getGas() const
{
    return _message.gas;
}


base::Bytes SmartContractMessage::getDestination() const
{
    return toBytes(_message.destination);
}


base::Bytes SmartContractMessage::getSender() const
{
    return toBytes(_message.sender);
}


base::Bytes SmartContractMessage::toInputData() const
{
    return copy(_message.input_data, _message.input_size);
}


base::Bytes SmartContractMessage::getCreate2Salt() const
{
    return toBytes(_message.create2_salt);
}


const base::Bytes& SmartContractMessage::getCode() const
{
    return _contract_code;
}


bc::Balance SmartContractMessage::getValue() const
{
    return toBalance(_message.value);
}


evmc_revision SmartContractMessage::getRevision() const
{
    return _revision;
}


SmartContractMessage::SmartContractMessage(evmc_revision revision)
  : _message{}
  , _contract_code{}
  , _revision{ revision }
{}


ExecutionResult::ExecutionResult(evmc::result&& data)
  : _data(std::move(data))
{}


bool ExecutionResult::ok() const noexcept
{
    return !_data || _data->status_code == evmc_status_code::EVMC_SUCCESS;
}


base::Bytes ExecutionResult::toOutputData() const
{
    return copy(_data->output_data, _data->output_size);
}


int64_t ExecutionResult::gasLeft() const
{
    return _data->gas_left;
}


base::Bytes ExecutionResult::createdAddress() const
{
    return toBytes(_data->create_address);
}


evmc::result ExecutionResult::getResult() noexcept
{
    return std::move(*_data);
}


namespace
{
std::filesystem::path getVmPath()
{
    static const std::filesystem::path lib_name = std::filesystem::absolute("libevmone.so.0.4");

    if (std::filesystem::exists(lib_name)) {
        if (std::filesystem::is_symlink(lib_name)) {
            std::error_code ec;
            auto result = std::filesystem::read_symlink(lib_name, ec);
            if (!ec) {
                RAISE_ERROR(base::InaccessibleFile, "Vm library was not found");
            }
            return std::filesystem::absolute(result);
        }
        else {
            return std::filesystem::absolute(lib_name);
        }
    }
    else {
        RAISE_ERROR(base::InaccessibleFile, "Vm library was not found");
    }
}
} // namespace

Vm Vm::load(evmc::Host& vm_host)
{
    evmc_loader_error_code load_error_code;

    auto vm_ptr = evmc_load_and_create(getVmPath().c_str(), &load_error_code);

    if (load_error_code != EVMC_LOADER_SUCCESS || vm_ptr == nullptr) {
        switch (load_error_code) {
            case EVMC_LOADER_SUCCESS:
                RAISE_ERROR(base::LogicError, "Error status is success but pointer is null");
            case EVMC_LOADER_CANNOT_OPEN:
                RAISE_ERROR(base::InaccessibleFile, "File can't be open");
            case EVMC_LOADER_SYMBOL_NOT_FOUND:
                RAISE_ERROR(VmError, "Vm dll has incorrect format");
            case EVMC_LOADER_ABI_VERSION_MISMATCH:
                RAISE_ERROR(VmError, "Invalid vm abi version");
            default:
                RAISE_ERROR(VmError, "Undefined error at loading vm");
        }
    }

    return { vm_ptr, vm_host };
}


ExecutionResult Vm::execute(const SmartContractMessage& msg)
{
    auto res = _vm.execute(_host, msg.getRevision(), msg.getMessage(), msg.getCode().getData(), msg.getCode().size());
    return ExecutionResult{ std::move(res) };
}


Vm::Vm(evmc_vm* vm_instance_ptr, evmc::Host& vm_host)
  : _vm{ vm_instance_ptr }
  , _host{ vm_host }
{
    LOG_INFO << "Created EVM name: " << _vm.name() << ", version:" << _vm.version();

    if (!_vm.is_abi_compatible()) {
        RAISE_ERROR(VmError, " ABI version is incompatible.");
    }

    auto vm_capabilities = _vm.get_capabilities();

    if (vm_capabilities & evmc_capabilities::EVMC_CAPABILITY_EVM1) {
        LOG_INFO << "EVM compatible with EVM1 instructions";
    }

    if (vm_capabilities & evmc_capabilities::EVMC_CAPABILITY_EWASM) {
        LOG_INFO << "EVM compatible with EWASM instructions";
    }

    if (vm_capabilities & evmc_capabilities::EVMC_CAPABILITY_PRECOMPILES) {
        LOG_INFO << "EVM compatible with precompiles instructions";
    }
}

} // namespace vm
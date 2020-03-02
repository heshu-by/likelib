#pragma once

#include "base/crypto.hpp"
#include "base/hash.hpp"
#include "base/serialization.hpp"

namespace bc
{

class Address
{
  public:
    //=============================
    static constexpr std::size_t BYTE_LENGTH = 20;
    //=============================
    explicit Address(const std::string_view& base64_address);
    explicit Address(base::Bytes raw);
    explicit Address(const base::RsaPublicKey& pub);
    Address(const Address& another) = default;
    Address(Address&& another) = default;
    Address& operator=(const Address& another) = default;
    Address& operator=(Address&& another) = default;
    ~Address() = default;
    //=============================
    [[nodiscard]] std::string toString() const;
    const base::Bytes& getBytes() const noexcept;
    //=============================
    static const Address& null();
    [[nodiscard]] bool isNull() const;
    //=============================
    bool operator==(const Address& another) const;
    bool operator!=(const Address& another) const;
    bool operator<(const Address& another) const;
    //=============================
    static Address deserialize(base::SerializationIArchive& ia);
    void serialize(base::SerializationOArchive& oa) const;
    //=============================
  private:
    base::Bytes _address;
};


std::ostream& operator<<(std::ostream& os, const Address& address);

} // namespace bc

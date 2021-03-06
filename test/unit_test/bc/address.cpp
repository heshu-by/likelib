#include <boost/test/unit_test.hpp>

#include "bc/address.hpp"

BOOST_AUTO_TEST_CASE(address_null)
{
    auto null = bc::Address::null();
    BOOST_CHECK(null.isNull());
}

BOOST_AUTO_TEST_CASE(address_constructor_fromPublicKey)
{
    bc::Address target_address(base::generateKeys().first);
}


BOOST_AUTO_TEST_CASE(address_constructor_from_one_publickey)
{
    auto [pub_key, priv_key] = base::generateKeys();
    bc::Address address1(pub_key);
    bc::Address address2(pub_key);
    BOOST_CHECK(address1 == address2);
    BOOST_CHECK(address1.toString() == address2.toString());
}


BOOST_AUTO_TEST_CASE(address_constructor_from_string)
{
    auto [pub_key, priv_key] = base::generateKeys();
    bc::Address address1(pub_key);
    bc::Address address2(address1.toString());
    BOOST_CHECK(address1 == address2);
    BOOST_CHECK(address1.toString() == address2.toString());
}


BOOST_AUTO_TEST_CASE(address_constructor_from_bytes)
{
    auto [pub_key, priv_key] = base::generateKeys();
    bc::Address address1(pub_key);
    bc::Address address2(address1.getBytes());
    BOOST_CHECK(address1 == address2);
    BOOST_CHECK(address1.toString() == address2.toString());
}


BOOST_AUTO_TEST_CASE(address_constructor_copy)
{
    auto [pub_key, priv_key] = base::generateKeys();
    bc::Address address1(pub_key);
    bc::Address address2(address1);
    BOOST_CHECK(address1 == address2);
    BOOST_CHECK(address1.toString() == address2.toString());
}


BOOST_AUTO_TEST_CASE(address_constructor_move)
{
    auto [pub_key, priv_key] = base::generateKeys();
    bc::Address address(pub_key);
    bc::Address address1(pub_key);
    bc::Address address2(std::move(address1));
    BOOST_CHECK(address2 == address);
    BOOST_CHECK(address2.toString() == address.toString());
}


BOOST_AUTO_TEST_CASE(address_operator_equal)
{
    auto [pub_key1, priv_key1] = base::generateKeys();
    auto [pub_key2, priv_key2] = base::generateKeys();
    bc::Address address1(pub_key1);
    bc::Address address2(pub_key2);
    BOOST_CHECK(address1.toString() != address2.toString());
    address2 = address1;
    BOOST_CHECK(address1 == address2);
    BOOST_CHECK(address1.toString() == address2.toString());
}


BOOST_AUTO_TEST_CASE(address_operator_move)
{
    auto [pub_key1, priv_key1] = base::generateKeys();
    auto [pub_key2, priv_key2] = base::generateKeys();
    bc::Address address(pub_key1);
    bc::Address address1(pub_key1);
    bc::Address address2(pub_key2);
    BOOST_CHECK(address1.toString() != address2.toString());
    address2 = std::move(address1);
    BOOST_CHECK(address2 == address);
    BOOST_CHECK(address2.toString() == address.toString());
}


BOOST_AUTO_TEST_CASE(address_serialization1)
{
    auto [pub_key, priv_key] = base::generateKeys();
    bc::Address address1(pub_key);
    base::SerializationOArchive oa;
    oa.serialize(address1);
    base::SerializationIArchive ia(oa.getBytes());
    bc::Address address2 = ia.deserialize<bc::Address>();
    BOOST_CHECK(address1 == address2);
    BOOST_CHECK(address1.toString() == address2.toString());
}


BOOST_AUTO_TEST_CASE(address_serialization2)
{
    auto [pub_key, priv_key] = base::generateKeys();
    bc::Address a1(pub_key);
    bc::Address a2 = base::fromBytes<bc::Address>(base::toBytes(a1));
    BOOST_CHECK(a1 == a2);
}

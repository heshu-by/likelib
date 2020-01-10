#include <boost/test/unit_test.hpp>

#include "base/hash.hpp"


BOOST_AUTO_TEST_CASE(sha256_hash)
{
    auto sha256_1 =
        base::Sha256::compute(base::Bytes{0x4c, 0x49, 0x4b, 0x45, 0x4c, 0x49, 0x42, 0x9, 0x32, 0x2e, 0x30});
    BOOST_CHECK_EQUAL(sha256_1.toHex(), "5fa56e73ead625a67cb2b6c3394664491432c7d1402d738c285a8903572c4846");
    BOOST_CHECK_EQUAL(sha256_1.getBytes().toHex(), "5fa56e73ead625a67cb2b6c3394664491432c7d1402d738c285a8903572c4846");

    auto sha256_2 = base::Sha256::compute(base::Bytes("likelib.2"));
    BOOST_CHECK_EQUAL(sha256_2.toHex(), "1242fcfab7d240b6d6538dc0fa626cb2e43fa1186febd52cf4dce0da3c55a9e5");
    BOOST_CHECK_EQUAL(sha256_2.getBytes().toHex(), "1242fcfab7d240b6d6538dc0fa626cb2e43fa1186febd52cf4dce0da3c55a9e5");

    auto sha256_3 = base::Sha256::compute(base::Bytes("it's third test"));
    BOOST_CHECK_EQUAL(sha256_3.toHex(), "2431f272555362f2d9ee255ec2ea24dffa371a03137699cf9d0b96e988346421");
    BOOST_CHECK_EQUAL(sha256_3.getBytes().toHex(), "2431f272555362f2d9ee255ec2ea24dffa371a03137699cf9d0b96e988346421");

    auto sha256_4 = base::Sha256::compute(base::Bytes(""));
    BOOST_CHECK_EQUAL(sha256_4.toHex(), "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    BOOST_CHECK_EQUAL(sha256_4.getBytes().toHex(), "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
}


BOOST_AUTO_TEST_CASE(sha256_serialization)
{
    auto target_hash =
        base::Sha256::compute(base::Bytes{0x4c, 0x49, 0x4b, 0x45, 0x4c, 0x49, 0x42, 0x9, 0x32, 0x2e, 0x30});
    auto target_hex_view = "5fa56e73ead625a67cb2b6c3394664491432c7d1402d738c285a8903572c4846";

    BOOST_CHECK_EQUAL(target_hash.toHex(), target_hex_view);
    BOOST_CHECK_EQUAL(target_hash.getBytes().toHex(), target_hex_view);

    base::SerializationOArchive out;
    base::Sha256::serialize(out, target_hash);

    auto serialized_bytes = out.getBytes();
    base::SerializationIArchive in(serialized_bytes);
    auto deserialized_hash = base::Sha256::deserialize(in);

    BOOST_CHECK_EQUAL(deserialized_hash, target_hash);
    BOOST_CHECK_EQUAL(deserialized_hash.toHex(), target_hex_view);
    BOOST_CHECK_EQUAL(deserialized_hash.getBytes().toHex(), target_hex_view);
}


BOOST_AUTO_TEST_CASE(sha256_hex)
{
    auto target_hash =
        base::Sha256::compute(base::Bytes{0x4c, 0x49, 0x4b, 0x45, 0x4c, 0x49, 0x42, 0x9, 0x32, 0x2e, 0x30});
    auto target_hex_view = "5fa56e73ead625a67cb2b6c3394664491432c7d1402d738c285a8903572c4846";

    auto hex_view = target_hash.toHex();
    auto from_hex_hash = base::Sha256::fromHex(hex_view);

    BOOST_CHECK_EQUAL(hex_view, target_hex_view);
    BOOST_CHECK_EQUAL(target_hash, from_hex_hash);
}


BOOST_AUTO_TEST_CASE(sha256_multiple_serialization)
{
    auto target_hash_1 =
        base::Sha256::compute(base::Bytes{0x4c, 0x49, 0x4b, 0x45, 0x4c, 0x49, 0x42, 0x9, 0x32, 0x2e, 0x30});
    auto target_hex_view_1 = "5fa56e73ead625a67cb2b6c3394664491432c7d1402d738c285a8903572c4846";

    BOOST_CHECK_EQUAL(target_hash_1.toHex(), target_hex_view_1);
    BOOST_CHECK_EQUAL(target_hash_1.getBytes().toHex(), target_hex_view_1);

    auto target_hash_2 = base::Sha256::compute(base::Bytes("likelib.2"));
    auto target_hex_view_2 = "1242fcfab7d240b6d6538dc0fa626cb2e43fa1186febd52cf4dce0da3c55a9e5";

    BOOST_CHECK_EQUAL(target_hash_2.toHex(), target_hex_view_2);
    BOOST_CHECK_EQUAL(target_hash_2.getBytes().toHex(), target_hex_view_2);

    base::SerializationOArchive out;
    base::Sha256::serialize(out, target_hash_1);
    base::Sha256::serialize(out, target_hash_2);

    auto serialized_bytes = out.getBytes();
    base::SerializationIArchive in(serialized_bytes);
    auto deserialized_hash_1 = base::Sha256::deserialize(in);
    auto deserialized_hash_2 = base::Sha256::deserialize(in);

    BOOST_CHECK_EQUAL(deserialized_hash_1, target_hash_1);
    BOOST_CHECK_EQUAL(deserialized_hash_1.toHex(), target_hex_view_1);
    BOOST_CHECK_EQUAL(deserialized_hash_1.getBytes().toHex(), target_hex_view_1);

    BOOST_CHECK_EQUAL(deserialized_hash_2, target_hash_2);
    BOOST_CHECK_EQUAL(deserialized_hash_2.toHex(), target_hex_view_2);
    BOOST_CHECK_EQUAL(deserialized_hash_2.getBytes().toHex(), target_hex_view_2);
}


BOOST_AUTO_TEST_CASE(sha1_hash)
{
    auto sha1_1 = base::Sha1::compute(base::Bytes{0x4c, 0x49, 0x4b, 0x45, 0x4c, 0x49, 0x42, 0x9, 0x32, 0x2e, 0x30});
    BOOST_CHECK_EQUAL(sha1_1.toHex(), "8b3b3476a984cc1c0d2bf1b3751ca366818f8b08");
    BOOST_CHECK_EQUAL(sha1_1.getBytes().toHex(), "8b3b3476a984cc1c0d2bf1b3751ca366818f8b08");

    auto sha1_2 = base::Sha1::compute(base::Bytes("likelib.2"));
    BOOST_CHECK_EQUAL(sha1_2.toHex(), "ee2f5885c39b865f83e5f91dd94ce466f3be371d");
    BOOST_CHECK_EQUAL(sha1_2.getBytes().toHex(), "ee2f5885c39b865f83e5f91dd94ce466f3be371d");

    auto sha1_3 = base::Sha1::compute(base::Bytes("it's third test"));
    BOOST_CHECK_EQUAL(sha1_3.toHex(), "3f68e91144cc3d272df2950c0676918980a35d01");
    BOOST_CHECK_EQUAL(sha1_3.getBytes().toHex(), "3f68e91144cc3d272df2950c0676918980a35d01");

    auto sha1_4 = base::Sha1::compute(base::Bytes(""));
    BOOST_CHECK_EQUAL(sha1_4.toHex(), "da39a3ee5e6b4b0d3255bfef95601890afd80709");
    BOOST_CHECK_EQUAL(sha1_4.getBytes().toHex(), "da39a3ee5e6b4b0d3255bfef95601890afd80709");
}


BOOST_AUTO_TEST_CASE(sha1_serialization)
{
    auto target_hash =
        base::Sha1::compute(base::Bytes{0x4c, 0x49, 0x4b, 0x45, 0x4c, 0x49, 0x42, 0x9, 0x32, 0x2e, 0x30});
    auto target_hex_view = "8b3b3476a984cc1c0d2bf1b3751ca366818f8b08";

    BOOST_CHECK_EQUAL(target_hash.toHex(), target_hex_view);
    BOOST_CHECK_EQUAL(target_hash.getBytes().toHex(), target_hex_view);

    base::SerializationOArchive out;
    base::Sha1::serialize(out, target_hash);

    auto serialized_bytes = out.getBytes();
    base::SerializationIArchive in(serialized_bytes);
    auto deserialized_hash = base::Sha1::deserialize(in);

    BOOST_CHECK_EQUAL(deserialized_hash, target_hash);
    BOOST_CHECK_EQUAL(deserialized_hash.toHex(), target_hex_view);
    BOOST_CHECK_EQUAL(deserialized_hash.getBytes().toHex(), target_hex_view);
}


BOOST_AUTO_TEST_CASE(sha1_hex)
{
    auto target_hash =
        base::Sha1::compute(base::Bytes{0x4c, 0x49, 0x4b, 0x45, 0x4c, 0x49, 0x42, 0x9, 0x32, 0x2e, 0x30});
    auto target_hex_view = "8b3b3476a984cc1c0d2bf1b3751ca366818f8b08";

    auto hex_view = target_hash.toHex();
    auto from_hex_hash = base::Sha1::fromHex(hex_view);

    BOOST_CHECK_EQUAL(hex_view, target_hex_view);
    BOOST_CHECK_EQUAL(target_hash, from_hex_hash);
}


BOOST_AUTO_TEST_CASE(sha1_multiple_serialization)
{
    auto target_hash_1 =
        base::Sha1::compute(base::Bytes{0x4c, 0x49, 0x4b, 0x45, 0x4c, 0x49, 0x42, 0x9, 0x32, 0x2e, 0x30});
    auto target_hex_view_1 = "8b3b3476a984cc1c0d2bf1b3751ca366818f8b08";

    BOOST_CHECK_EQUAL(target_hash_1.toHex(), target_hex_view_1);
    BOOST_CHECK_EQUAL(target_hash_1.getBytes().toHex(), target_hex_view_1);

    auto target_hash_2 = base::Sha1::compute(base::Bytes("likelib.2"));
    auto target_hex_view_2 = "ee2f5885c39b865f83e5f91dd94ce466f3be371d";

    BOOST_CHECK_EQUAL(target_hash_2.toHex(), target_hex_view_2);
    BOOST_CHECK_EQUAL(target_hash_2.getBytes().toHex(), target_hex_view_2);

    base::SerializationOArchive out;
    base::Sha1::serialize(out, target_hash_1);
    base::Sha1::serialize(out, target_hash_2);

    auto serialized_bytes = out.getBytes();
    base::SerializationIArchive in(serialized_bytes);
    auto deserialized_hash_1 = base::Sha1::deserialize(in);
    auto deserialized_hash_2 = base::Sha1::deserialize(in);

    BOOST_CHECK_EQUAL(deserialized_hash_1, target_hash_1);
    BOOST_CHECK_EQUAL(deserialized_hash_1.toHex(), target_hex_view_1);
    BOOST_CHECK_EQUAL(deserialized_hash_1.getBytes().toHex(), target_hex_view_1);

    BOOST_CHECK_EQUAL(deserialized_hash_2, target_hash_2);
    BOOST_CHECK_EQUAL(deserialized_hash_2.toHex(), target_hex_view_2);
    BOOST_CHECK_EQUAL(deserialized_hash_2.getBytes().toHex(), target_hex_view_2);
}

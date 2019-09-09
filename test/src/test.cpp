#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_FAST_COMPILE
#include <catch.hpp>

#include <libbinary_format/data_reader.hpp>
#include <libbinary_format/read_uint.hpp>

#include <list>
#include <string_view>
#include <vector>


using namespace std::literals;
using Catch::Matchers::Predicate;


TEST_CASE("read_uint is working", "[read_uint]") {
    using libbinary_format::read_uint;
    CHECK((read_uint<uint32_t>("\x74\x91\xb2\x03"sv) == 1955705347u));
    CHECK((read_uint<uint8_t>("\x74\x91\xb2\x03"sv) == 0x74u));
    CHECK((read_uint<uint8_t>("\x94\x91\xb2\x03"sv) == 0x94u));
    CHECK((read_uint<uint32_t>("\x00\x00\x00\x00"sv) == 0));
    CHECK((read_uint<uint32_t>("\x00\x00\x00\x00\x12"sv) == 0));
    REQUIRE_THROWS_AS((read_uint<uint64_t, true>("\x00\x00\x00\x00\x12\x14\x63"sv)), std::logic_error);
    REQUIRE_NOTHROW((read_uint<uint64_t, true>("\x00\x00\x00\x00\x12\x14\x63\x1e"sv)));
}


TEST_CASE("DataReader::read_bytes is working", "[DataReader]") {
    using libbinary_format::DataReader;
    {
        DataReader r(""sv);
        uint8_t buf[1];
        REQUIRE_NOTHROW(
            r.read_bytes(0, buf)
        );
        REQUIRE_THROWS_AS(
            r.read_bytes(1, buf),
            DataReader::EofError
        );
    }
    {
        DataReader r("Python is cool"sv);
        
        SECTION("Exact number of bytes is written") {
            std::string buf = "01234567890123456789";
            r.read_bytes(7, buf.begin());
            REQUIRE(buf == "Python 7890123456789");
        }
        SECTION("Different iterator types work") {
            SECTION("std::string iterator works") {
                std::string buf = "abcdef";
                r.read_bytes(6, buf.begin());
                REQUIRE(buf == "Python");
            }
            SECTION("std::vector iterator works") {
                std::vector<char> buf(6);
                r.read_bytes(6, buf.begin());
                REQUIRE((buf == std::vector<char>{'P', 'y', 't', 'h', 'o', 'n'}));
            }
            SECTION("std::list<uint8_t> iterator works") {
                std::list<uint8_t> buf = {0, 0, 0, 0, 0, 0};
                r.read_bytes(6, buf.begin());
                REQUIRE((buf == std::list<uint8_t>{'P', 'y', 't', 'h', 'o', 'n'}));
            }
            SECTION("char* works") {
                char buf[6] = {0, 0, 0, 0, 0, 0};
                r.read_bytes(6, buf);
                REQUIRE(std::string_view(buf, 6) == "Python");
            }
        }
        SECTION("Sequential reads work") {
            std::string buf = "01234567890123456789";
            r.read_bytes(7, buf.begin());
            REQUIRE(buf == "Python 7890123456789");
            r.read_bytes(3, buf.begin());
            REQUIRE(buf == "is hon 7890123456789");
            r.read_bytes(3, buf.begin());
            REQUIRE(buf == "coohon 7890123456789");
            REQUIRE_THROWS_AS(r.read_bytes(2, buf.begin()), DataReader::EofError);
            r.read_bytes(1, buf.begin());
            REQUIRE(buf == "loohon 7890123456789");
        }
    }
}

TEST_CASE("DataReader::skip is working", "[DataReader]") {
    using namespace libbinary_format;
    DataReader r("Python is cool"sv);
    std::string buf = "01234567890123456789";
    r.skip(4);
    r.read_bytes(6, buf.begin());
    REQUIRE(buf == "on is 67890123456789");
    REQUIRE_THROWS_AS(r.skip(5), DataReader::EofError);
    r.skip(4);
    REQUIRE_THROWS_AS(r.read_bytes(1, buf.begin()), DataReader::EofError);
    REQUIRE_NOTHROW(r.skip(0));
    REQUIRE_THROWS_AS(r.skip(1), DataReader::EofError);
}

TEST_CASE("DataReader::get_{offset,number_of_bytes_left} are working", "[DataReader]") {
    using namespace libbinary_format;
    DataReader r("Python is cool"sv);
    CHECK(r.get_offset() == 0);
    CHECK(r.get_number_of_bytes_left() == 14);

    std::string buf = "01234567890123456789";
    r.skip(4);
    CHECK(r.get_offset() == 4);
    CHECK(r.get_number_of_bytes_left() == 10);
    r.read_bytes(6, buf.begin());
    CHECK(r.get_offset() == 10);
    CHECK(r.get_number_of_bytes_left() == 4);
    r.skip(4);
    CHECK(r.get_offset() == 14);
    CHECK(r.get_number_of_bytes_left() == 0);
}

TEST_CASE("DataReader::read_uint is working", "[DataReader]") {
    using namespace libbinary_format;
    DataReader r("\x12\x34\x56\x78\x9A\xBC\xDE\xF0"sv);

    SECTION("8 bytes = 2 * uint32_t") {
        CHECK(r.read_uint<uint32_t>() == 0x1234'5678u);
        CHECK(r.read_uint<uint32_t>() == 0x9ABC'DEF0u);
        REQUIRE_THROWS_AS(r.read_uint<uint8_t>(), DataReader::EofError);
    }
    SECTION("8 bytes = 1 * uint64_t") {
        CHECK(r.read_uint<uint64_t>() == 0x1234'5678'9ABC'DEF0ULL);
        REQUIRE_THROWS_AS(r.read_uint<uint8_t>(), DataReader::EofError);
    }
    SECTION("8 bytes = uint32_t + uint8_t + NOT uint32_t") {
        CHECK(r.read_uint<uint32_t>() == 0x1234'5678u);
        CHECK(r.read_uint<uint8_t>() == 0x9Au);
        REQUIRE_THROWS_AS(r.read_uint<uint32_t>(), DataReader::EofError);
    }
}

#include <structify/structify.h>
#include "tokenizer-test-util.h"

#include "catch2/catch_all.hpp"

namespace cbor_tokenizer_test
{

// Helper to create CBOR data from byte initializer list
static std::vector<unsigned char> cbor(std::initializer_list<unsigned char> bytes)
{
  return std::vector<unsigned char>(bytes);
}

// -- Major type 0: Unsigned integers --

TEST_CASE("cbor_unsigned_int_inline", "[cbor][tokenizer]")
{
  // CBOR: 0x05 = unsigned int 5
  auto data = cbor({0x05});

  STFY::Tokenizer tokenizer;
  tokenizer.allowCbor(true);
  tokenizer.addData((const char *)data.data(), data.size());

  STFY::Token token;
  STFY::Error error;

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::Number);
  REQUIRE(std::string(token.value.data, token.value.size) == "5");
}

TEST_CASE("cbor_unsigned_int_zero", "[cbor][tokenizer]")
{
  auto data = cbor({0x00});

  STFY::Tokenizer tokenizer;
  tokenizer.allowCbor(true);
  tokenizer.addData((const char *)data.data(), data.size());

  STFY::Token token;
  STFY::Error error;

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::Number);
  REQUIRE(std::string(token.value.data, token.value.size) == "0");
}

TEST_CASE("cbor_unsigned_int_23", "[cbor][tokenizer]")
{
  // 0x17 = unsigned int 23 (max inline)
  auto data = cbor({0x17});

  STFY::Tokenizer tokenizer;
  tokenizer.allowCbor(true);
  tokenizer.addData((const char *)data.data(), data.size());

  STFY::Token token;
  STFY::Error error;

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::Number);
  REQUIRE(std::string(token.value.data, token.value.size) == "23");
}

TEST_CASE("cbor_unsigned_int_1byte", "[cbor][tokenizer]")
{
  // 0x18 0xFF = unsigned int 255 (1-byte argument)
  auto data = cbor({0x18, 0xFF});

  STFY::Tokenizer tokenizer;
  tokenizer.allowCbor(true);
  tokenizer.addData((const char *)data.data(), data.size());

  STFY::Token token;
  STFY::Error error;

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::Number);
  REQUIRE(std::string(token.value.data, token.value.size) == "255");
}

TEST_CASE("cbor_unsigned_int_2byte", "[cbor][tokenizer]")
{
  // 0x19 0x01 0x00 = unsigned int 256 (2-byte argument)
  auto data = cbor({0x19, 0x01, 0x00});

  STFY::Tokenizer tokenizer;
  tokenizer.allowCbor(true);
  tokenizer.addData((const char *)data.data(), data.size());

  STFY::Token token;
  STFY::Error error;

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::Number);
  REQUIRE(std::string(token.value.data, token.value.size) == "256");
}

TEST_CASE("cbor_unsigned_int_4byte", "[cbor][tokenizer]")
{
  // 0x1A 0x00 0x01 0x00 0x00 = unsigned int 65536
  auto data = cbor({0x1A, 0x00, 0x01, 0x00, 0x00});

  STFY::Tokenizer tokenizer;
  tokenizer.allowCbor(true);
  tokenizer.addData((const char *)data.data(), data.size());

  STFY::Token token;
  STFY::Error error;

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::Number);
  REQUIRE(std::string(token.value.data, token.value.size) == "65536");
}

TEST_CASE("cbor_unsigned_int_8byte", "[cbor][tokenizer]")
{
  // 0x1B 0x00 0x00 0x00 0x01 0x00 0x00 0x00 0x00 = unsigned int 4294967296
  auto data = cbor({0x1B, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00});

  STFY::Tokenizer tokenizer;
  tokenizer.allowCbor(true);
  tokenizer.addData((const char *)data.data(), data.size());

  STFY::Token token;
  STFY::Error error;

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::Number);
  REQUIRE(std::string(token.value.data, token.value.size) == "4294967296");
}

// -- Major type 1: Negative integers --

TEST_CASE("cbor_negative_int_inline", "[cbor][tokenizer]")
{
  // 0x20 = negative int -1 (= -1 - 0)
  auto data = cbor({0x20});

  STFY::Tokenizer tokenizer;
  tokenizer.allowCbor(true);
  tokenizer.addData((const char *)data.data(), data.size());

  STFY::Token token;
  STFY::Error error;

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::Number);
  REQUIRE(std::string(token.value.data, token.value.size) == "-1");
}

TEST_CASE("cbor_negative_int_minus_100", "[cbor][tokenizer]")
{
  // 0x38 0x63 = negative int -100 (= -1 - 99)
  auto data = cbor({0x38, 0x63});

  STFY::Tokenizer tokenizer;
  tokenizer.allowCbor(true);
  tokenizer.addData((const char *)data.data(), data.size());

  STFY::Token token;
  STFY::Error error;

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::Number);
  REQUIRE(std::string(token.value.data, token.value.size) == "-100");
}

// -- Major type 2: Byte strings --

TEST_CASE("cbor_byte_string_empty", "[cbor][tokenizer]")
{
  // 0x40 = byte string of length 0
  auto data = cbor({0x40});

  STFY::Tokenizer tokenizer;
  tokenizer.allowCbor(true);
  tokenizer.addData((const char *)data.data(), data.size());

  STFY::Token token;
  STFY::Error error;

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::Ascii);
  REQUIRE(std::string(token.value.data, token.value.size) == ""); // empty base64
}

TEST_CASE("cbor_byte_string_4bytes", "[cbor][tokenizer]")
{
  // 0x44 0x01 0x02 0x03 0x04 = byte string [1,2,3,4]
  auto data = cbor({0x44, 0x01, 0x02, 0x03, 0x04});

  STFY::Tokenizer tokenizer;
  tokenizer.allowCbor(true);
  tokenizer.addData((const char *)data.data(), data.size());

  STFY::Token token;
  STFY::Error error;

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::Ascii);
  REQUIRE(std::string(token.value.data, token.value.size) == "AQIDBA=="); // base64 of [1,2,3,4]
}

// -- Major type 3: Text strings --

TEST_CASE("cbor_text_string_empty", "[cbor][tokenizer]")
{
  // 0x60 = text string of length 0
  auto data = cbor({0x60});

  STFY::Tokenizer tokenizer;
  tokenizer.allowCbor(true);
  tokenizer.addData((const char *)data.data(), data.size());

  STFY::Token token;
  STFY::Error error;

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::String);
  REQUIRE(std::string(token.value.data, token.value.size) == "");
}

TEST_CASE("cbor_text_string_hello", "[cbor][tokenizer]")
{
  // 0x65 "hello" = text string "hello"
  auto data = cbor({0x65, 'h', 'e', 'l', 'l', 'o'});

  STFY::Tokenizer tokenizer;
  tokenizer.allowCbor(true);
  tokenizer.addData((const char *)data.data(), data.size());

  STFY::Token token;
  STFY::Error error;

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::String);
  REQUIRE(std::string(token.value.data, token.value.size) == "hello");
}

// -- Major type 4: Arrays --

TEST_CASE("cbor_empty_array", "[cbor][tokenizer]")
{
  // 0x80 = array of length 0
  auto data = cbor({0x80});

  STFY::Tokenizer tokenizer;
  tokenizer.allowCbor(true);
  tokenizer.addData((const char *)data.data(), data.size());

  STFY::Token token;
  STFY::Error error;

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ArrayStart);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ArrayEnd);
}

TEST_CASE("cbor_array_of_ints", "[cbor][tokenizer]")
{
  // 0x83 0x01 0x02 0x03 = [1, 2, 3]
  auto data = cbor({0x83, 0x01, 0x02, 0x03});

  STFY::Tokenizer tokenizer;
  tokenizer.allowCbor(true);
  tokenizer.addData((const char *)data.data(), data.size());

  STFY::Token token;
  STFY::Error error;

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ArrayStart);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::Number);
  REQUIRE(std::string(token.value.data, token.value.size) == "1");

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::Number);
  REQUIRE(std::string(token.value.data, token.value.size) == "2");

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::Number);
  REQUIRE(std::string(token.value.data, token.value.size) == "3");

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ArrayEnd);
}

TEST_CASE("cbor_indefinite_array", "[cbor][tokenizer]")
{
  // 0x9F 0x01 0x02 0xFF = indefinite array [1, 2]
  auto data = cbor({0x9F, 0x01, 0x02, 0xFF});

  STFY::Tokenizer tokenizer;
  tokenizer.allowCbor(true);
  tokenizer.addData((const char *)data.data(), data.size());

  STFY::Token token;
  STFY::Error error;

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ArrayStart);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::Number);
  REQUIRE(std::string(token.value.data, token.value.size) == "1");

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::Number);
  REQUIRE(std::string(token.value.data, token.value.size) == "2");

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ArrayEnd);
}

// -- Major type 5: Maps --

TEST_CASE("cbor_empty_map", "[cbor][tokenizer]")
{
  // 0xA0 = map of 0 pairs
  auto data = cbor({0xA0});

  STFY::Tokenizer tokenizer;
  tokenizer.allowCbor(true);
  tokenizer.addData((const char *)data.data(), data.size());

  STFY::Token token;
  STFY::Error error;

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectStart);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectEnd);
}

TEST_CASE("cbor_simple_map", "[cbor][tokenizer]")
{
  // {"a": 1, "b": 2}
  // 0xA2 0x61 'a' 0x01 0x61 'b' 0x02
  auto data = cbor({0xA2, 0x61, 'a', 0x01, 0x61, 'b', 0x02});

  STFY::Tokenizer tokenizer;
  tokenizer.allowCbor(true);
  tokenizer.addData((const char *)data.data(), data.size());

  STFY::Token token;
  STFY::Error error;

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectStart);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(assert_token(token, STFY::Type::Ascii, "a", STFY::Type::Number, "1") == 0);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(assert_token(token, STFY::Type::Ascii, "b", STFY::Type::Number, "2") == 0);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectEnd);
}

TEST_CASE("cbor_indefinite_map", "[cbor][tokenizer]")
{
  // indefinite {"a": 1}
  // 0xBF 0x61 'a' 0x01 0xFF
  auto data = cbor({0xBF, 0x61, 'a', 0x01, 0xFF});

  STFY::Tokenizer tokenizer;
  tokenizer.allowCbor(true);
  tokenizer.addData((const char *)data.data(), data.size());

  STFY::Token token;
  STFY::Error error;

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectStart);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(assert_token(token, STFY::Type::Ascii, "a", STFY::Type::Number, "1") == 0);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectEnd);
}

// -- Major type 6: Tags --

TEST_CASE("cbor_tag_ignored", "[cbor][tokenizer]")
{
  // Tag 1 (epoch-based date) around unsigned int 1363896240
  // 0xC1 0x1A 0x51 0x4B 0x67 0xB0
  auto data = cbor({0xC1, 0x1A, 0x51, 0x4B, 0x67, 0xB0});

  STFY::Tokenizer tokenizer;
  tokenizer.allowCbor(true);
  tokenizer.addData((const char *)data.data(), data.size());

  STFY::Token token;
  STFY::Error error;

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::Number);
  REQUIRE(std::string(token.value.data, token.value.size) == "1363896240");
}

// -- Major type 7: Simple values and floats --

TEST_CASE("cbor_false", "[cbor][tokenizer]")
{
  auto data = cbor({0xF4}); // false
  STFY::Tokenizer tokenizer;
  tokenizer.allowCbor(true);
  tokenizer.addData((const char *)data.data(), data.size());

  STFY::Token token;
  STFY::Error error;
  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::Bool);
  REQUIRE(std::string(token.value.data, token.value.size) == "false");
}

TEST_CASE("cbor_true", "[cbor][tokenizer]")
{
  auto data = cbor({0xF5}); // true
  STFY::Tokenizer tokenizer;
  tokenizer.allowCbor(true);
  tokenizer.addData((const char *)data.data(), data.size());

  STFY::Token token;
  STFY::Error error;
  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::Bool);
  REQUIRE(std::string(token.value.data, token.value.size) == "true");
}

TEST_CASE("cbor_null", "[cbor][tokenizer]")
{
  auto data = cbor({0xF6}); // null
  STFY::Tokenizer tokenizer;
  tokenizer.allowCbor(true);
  tokenizer.addData((const char *)data.data(), data.size());

  STFY::Token token;
  STFY::Error error;
  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::Null);
  REQUIRE(std::string(token.value.data, token.value.size) == "null");
}

TEST_CASE("cbor_float16_zero", "[cbor][tokenizer]")
{
  // float16 0.0 = 0xF9 0x00 0x00
  auto data = cbor({0xF9, 0x00, 0x00});
  STFY::Tokenizer tokenizer;
  tokenizer.allowCbor(true);
  tokenizer.addData((const char *)data.data(), data.size());

  STFY::Token token;
  STFY::Error error;
  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::Number);
  // 0.0 should produce some text representation
  double val;
  const char *ptr;
  ft::to_double(token.value.data, token.value.size, val, ptr);
  REQUIRE(val == 0.0);
}

TEST_CASE("cbor_float16_one", "[cbor][tokenizer]")
{
  // float16 1.0 = 0xF9 0x3C 0x00
  auto data = cbor({0xF9, 0x3C, 0x00});
  STFY::Tokenizer tokenizer;
  tokenizer.allowCbor(true);
  tokenizer.addData((const char *)data.data(), data.size());

  STFY::Token token;
  STFY::Error error;
  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::Number);
  double val;
  const char *ptr;
  ft::to_double(token.value.data, token.value.size, val, ptr);
  REQUIRE(val == Catch::Approx(1.0));
}

TEST_CASE("cbor_float32", "[cbor][tokenizer]")
{
  // float32 100000.0 = 0xFA 0x47 0xC3 0x50 0x00
  auto data = cbor({0xFA, 0x47, 0xC3, 0x50, 0x00});
  STFY::Tokenizer tokenizer;
  tokenizer.allowCbor(true);
  tokenizer.addData((const char *)data.data(), data.size());

  STFY::Token token;
  STFY::Error error;
  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::Number);
  double val;
  const char *ptr;
  ft::to_double(token.value.data, token.value.size, val, ptr);
  REQUIRE(val == Catch::Approx(100000.0));
}

TEST_CASE("cbor_float64", "[cbor][tokenizer]")
{
  // float64 1.1 = 0xFB 0x3F 0xF1 0x99 0x99 0x99 0x99 0x99 0x9A
  auto data = cbor({0xFB, 0x3F, 0xF1, 0x99, 0x99, 0x99, 0x99, 0x99, 0x9A});
  STFY::Tokenizer tokenizer;
  tokenizer.allowCbor(true);
  tokenizer.addData((const char *)data.data(), data.size());

  STFY::Token token;
  STFY::Error error;
  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::Number);
  double val;
  const char *ptr;
  ft::to_double(token.value.data, token.value.size, val, ptr);
  REQUIRE(val == Catch::Approx(1.1));
}

// -- Nested structures --

TEST_CASE("cbor_nested_map_with_array", "[cbor][tokenizer]")
{
  // {"name": "John", "scores": [10, 20, 30]}
  // A2                         -- map(2)
  //   64 6E616D65              -- text(4) "name"
  //   64 4A6F686E              -- text(4) "John"
  //   66 73636F726573          -- text(6) "scores"
  //   83                       -- array(3)
  //     0A                     -- unsigned(10)
  //     14                     -- unsigned(20)
  //     18 1E                  -- unsigned(30)
  auto data = cbor({
    0xA2,
    0x64, 'n', 'a', 'm', 'e',
    0x64, 'J', 'o', 'h', 'n',
    0x66, 's', 'c', 'o', 'r', 'e', 's',
    0x83, 0x0A, 0x14, 0x18, 0x1E
  });

  STFY::Tokenizer tokenizer;
  tokenizer.allowCbor(true);
  tokenizer.addData((const char *)data.data(), data.size());

  STFY::Token token;
  STFY::Error error;

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectStart);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(assert_token(token, STFY::Type::Ascii, "name", STFY::Type::String, "John") == 0);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ArrayStart);
  REQUIRE(std::string(token.name.data, token.name.size) == "scores");

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(std::string(token.value.data, token.value.size) == "10");

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(std::string(token.value.data, token.value.size) == "20");

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(std::string(token.value.data, token.value.size) == "30");

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ArrayEnd);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectEnd);
}

// -- Error cases --

TEST_CASE("cbor_truncated_data", "[cbor][tokenizer]")
{
  // 0x19 needs 2 bytes but we only provide 1
  auto data = cbor({0x19, 0x01});

  STFY::Tokenizer tokenizer;
  tokenizer.allowCbor(true);
  tokenizer.addData((const char *)data.data(), data.size());

  STFY::Token token;
  STFY::Error error;
  error = tokenizer.nextToken(token);
  // Should get NeedMoreData since no tokens were emitted
  REQUIRE(error == STFY::Error::NeedMoreData);
}

TEST_CASE("cbor_empty_data", "[cbor][tokenizer]")
{
  STFY::Tokenizer tokenizer;
  tokenizer.allowCbor(true);
  tokenizer.addData("", 0);

  STFY::Token token;
  STFY::Error error;
  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NeedMoreData);
}

// -- Integer key in map --

TEST_CASE("cbor_integer_key_map", "[cbor][tokenizer]")
{
  // {1: "one", 2: "two"}
  // A2 01 63 6F6E65 02 63 74776F
  auto data = cbor({0xA2, 0x01, 0x63, 'o', 'n', 'e', 0x02, 0x63, 't', 'w', 'o'});

  STFY::Tokenizer tokenizer;
  tokenizer.allowCbor(true);
  tokenizer.addData((const char *)data.data(), data.size());

  STFY::Token token;
  STFY::Error error;

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectStart);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(std::string(token.name.data, token.name.size) == "1");
  REQUIRE(token.value_type == STFY::Type::String);
  REQUIRE(std::string(token.value.data, token.value.size) == "one");

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(std::string(token.name.data, token.name.size) == "2");
  REQUIRE(token.value_type == STFY::Type::String);
  REQUIRE(std::string(token.value.data, token.value.size) == "two");

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectEnd);
}

// -- Indefinite-length text string --

TEST_CASE("cbor_indefinite_text_string", "[cbor][tokenizer]")
{
  // (_ "hel", "lo") = 0x7F 0x63 "hel" 0x62 "lo" 0xFF
  auto data = cbor({0x7F, 0x63, 'h', 'e', 'l', 0x62, 'l', 'o', 0xFF});

  STFY::Tokenizer tokenizer;
  tokenizer.allowCbor(true);
  tokenizer.addData((const char *)data.data(), data.size());

  STFY::Token token;
  STFY::Error error;

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::String);
  REQUIRE(std::string(token.value.data, token.value.size) == "hello");
}

// -- Map with string values --

TEST_CASE("cbor_map_with_string_values", "[cbor][tokenizer]")
{
  // {"key": "value"}
  // A1 63 6B6579 65 76616C7565
  auto data = cbor({0xA1, 0x63, 'k', 'e', 'y', 0x65, 'v', 'a', 'l', 'u', 'e'});

  STFY::Tokenizer tokenizer;
  tokenizer.allowCbor(true);
  tokenizer.addData((const char *)data.data(), data.size());

  STFY::Token token;
  STFY::Error error;

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectStart);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(assert_token(token, STFY::Type::Ascii, "key", STFY::Type::String, "value") == 0);

  error = tokenizer.nextToken(token);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(token.value_type == STFY::Type::ObjectEnd);
}

} // namespace cbor_tokenizer_test

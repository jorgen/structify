#include <structify/structify.h>

#include "catch2/catch_all.hpp"

namespace cbor_struct_test
{

struct SimpleStruct
{
  std::string name;
  int age;
  float score;
  STFY_OBJ(name, age, score);
};

struct NestedInner
{
  std::string street;
  std::string city;
  STFY_OBJ(street, city);
};

struct NestedOuter
{
  std::string name;
  NestedInner address;
  STFY_OBJ(name, address);
};

struct WithVector
{
  std::string name;
  std::vector<int> scores;
  STFY_OBJ(name, scores);
};

struct WithBooleans
{
  bool enabled;
  bool active;
  STFY_OBJ(enabled, active);
};

struct WithOptional
{
  std::string name;
  STFY::Optional<int> age;
  STFY::Optional<std::string> email;
  STFY_OBJ(name, age, email);
};

struct MixedStruct
{
  std::string name;
  int age;
  bool active;
  std::vector<std::string> hobbies;
  STFY_OBJ(name, age, active, hobbies);
};

struct PreInitInner
{
  std::string label;
  STFY_OBJ(label);
};

struct PreInitStruct
{
  std::string name = "original_name";
  int count = 42;
  float ratio = 3.14f;
  bool flag = true;
  PreInitInner inner;
  STFY_OBJ(name, count, ratio, flag, inner);
};

// Helper to build CBOR from byte list
static std::vector<unsigned char> cbor(std::initializer_list<unsigned char> bytes)
{
  return std::vector<unsigned char>(bytes);
}

TEST_CASE("cbor_parse_simple_struct", "[cbor][struct]")
{
  // {"name": "John", "age": 30, "score": float32(95.5)}
  // A3
  //   64 6E616D65        -- "name"
  //   64 4A6F686E        -- "John"
  //   63 616765          -- "age"
  //   18 1E              -- 30
  //   65 73636F7265      -- "score"
  //   FA 42BF0000        -- float32(95.5)
  auto data = cbor({
    0xA3,
    0x64, 'n', 'a', 'm', 'e',
    0x64, 'J', 'o', 'h', 'n',
    0x63, 'a', 'g', 'e',
    0x18, 0x1E,
    0x65, 's', 'c', 'o', 'r', 'e',
    0xFA, 0x42, 0xBF, 0x00, 0x00
  });

  STFY::ParseContext context;
  context.tokenizer.allowCbor(true);
  context.tokenizer.addData((const char *)data.data(), data.size());

  SimpleStruct s;
  (void)context.parseTo(s);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(s.name == "John");
  REQUIRE(s.age == 30);
  REQUIRE(s.score == Catch::Approx(95.5f));
}

TEST_CASE("cbor_parse_nested_struct", "[cbor][struct]")
{
  // {"name": "Alice", "address": {"street": "Main St", "city": "NYC"}}
  auto data = cbor({
    0xA2,
    // "name"
    0x64, 'n', 'a', 'm', 'e',
    // "Alice"
    0x65, 'A', 'l', 'i', 'c', 'e',
    // "address"
    0x67, 'a', 'd', 'd', 'r', 'e', 's', 's',
    // nested map(2)
    0xA2,
      // "street"
      0x66, 's', 't', 'r', 'e', 'e', 't',
      // "Main St"
      0x67, 'M', 'a', 'i', 'n', ' ', 'S', 't',
      // "city"
      0x64, 'c', 'i', 't', 'y',
      // "NYC"
      0x63, 'N', 'Y', 'C'
  });

  STFY::ParseContext context;
  context.tokenizer.allowCbor(true);
  context.tokenizer.addData((const char *)data.data(), data.size());

  NestedOuter s;
  (void)context.parseTo(s);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(s.name == "Alice");
  REQUIRE(s.address.street == "Main St");
  REQUIRE(s.address.city == "NYC");
}

TEST_CASE("cbor_parse_with_vector", "[cbor][struct]")
{
  // {"name": "Bob", "scores": [10, 20, 30]}
  auto data = cbor({
    0xA2,
    // "name"
    0x64, 'n', 'a', 'm', 'e',
    // "Bob"
    0x63, 'B', 'o', 'b',
    // "scores"
    0x66, 's', 'c', 'o', 'r', 'e', 's',
    // array(3)
    0x83, 0x0A, 0x14, 0x18, 0x1E
  });

  STFY::ParseContext context;
  context.tokenizer.allowCbor(true);
  context.tokenizer.addData((const char *)data.data(), data.size());

  WithVector s;
  (void)context.parseTo(s);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(s.name == "Bob");
  REQUIRE(s.scores.size() == 3);
  REQUIRE(s.scores[0] == 10);
  REQUIRE(s.scores[1] == 20);
  REQUIRE(s.scores[2] == 30);
}

TEST_CASE("cbor_parse_booleans", "[cbor][struct]")
{
  // {"enabled": true, "active": false}
  auto data = cbor({
    0xA2,
    0x67, 'e', 'n', 'a', 'b', 'l', 'e', 'd',
    0xF5, // true
    0x66, 'a', 'c', 't', 'i', 'v', 'e',
    0xF4  // false
  });

  STFY::ParseContext context;
  context.tokenizer.allowCbor(true);
  context.tokenizer.addData((const char *)data.data(), data.size());

  WithBooleans s;
  (void)context.parseTo(s);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(s.enabled == true);
  REQUIRE(s.active == false);
}

TEST_CASE("cbor_parse_optional_present", "[cbor][struct]")
{
  // {"name": "Eve", "age": 25, "email": "eve@example.com"}
  auto data = cbor({
    0xA3,
    0x64, 'n', 'a', 'm', 'e',
    0x63, 'E', 'v', 'e',
    0x63, 'a', 'g', 'e',
    0x18, 0x19, // 25
    0x65, 'e', 'm', 'a', 'i', 'l',
    0x6F, 'e', 'v', 'e', '@', 'e', 'x', 'a', 'm', 'p', 'l', 'e', '.', 'c', 'o', 'm'
  });

  STFY::ParseContext context;
  context.tokenizer.allowCbor(true);
  context.tokenizer.addData((const char *)data.data(), data.size());

  WithOptional s;
  (void)context.parseTo(s);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(s.name == "Eve");
  REQUIRE(s.age.data == 25);
  REQUIRE(s.email.data == "eve@example.com");
}

TEST_CASE("cbor_parse_optional_missing", "[cbor][struct]")
{
  // {"name": "Eve"}
  auto data = cbor({
    0xA1,
    0x64, 'n', 'a', 'm', 'e',
    0x63, 'E', 'v', 'e'
  });

  STFY::ParseContext context;
  context.tokenizer.allowCbor(true);
  context.tokenizer.addData((const char *)data.data(), data.size());

  WithOptional s;
  (void)context.parseTo(s);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(s.name == "Eve");
  // Optional members not in CBOR data remain default-initialized
  REQUIRE(s.age.data == 0);
  REQUIRE(s.email.data == "");
}

TEST_CASE("cbor_parse_mixed_struct", "[cbor][struct]")
{
  // {"name": "Charlie", "age": 35, "active": true, "hobbies": ["reading", "coding"]}
  auto data = cbor({
    0xA4,
    // "name"
    0x64, 'n', 'a', 'm', 'e',
    // "Charlie"
    0x67, 'C', 'h', 'a', 'r', 'l', 'i', 'e',
    // "age"
    0x63, 'a', 'g', 'e',
    // 35
    0x18, 0x23,
    // "active"
    0x66, 'a', 'c', 't', 'i', 'v', 'e',
    // true
    0xF5,
    // "hobbies"
    0x67, 'h', 'o', 'b', 'b', 'i', 'e', 's',
    // array(2)
    0x82,
      0x67, 'r', 'e', 'a', 'd', 'i', 'n', 'g',
      0x66, 'c', 'o', 'd', 'i', 'n', 'g'
  });

  STFY::ParseContext context;
  context.tokenizer.allowCbor(true);
  context.tokenizer.addData((const char *)data.data(), data.size());

  MixedStruct s;
  (void)context.parseTo(s);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(s.name == "Charlie");
  REQUIRE(s.age == 35);
  REQUIRE(s.active == true);
  REQUIRE(s.hobbies.size() == 2);
  REQUIRE(s.hobbies[0] == "reading");
  REQUIRE(s.hobbies[1] == "coding");
}

TEST_CASE("cbor_parse_negative_int", "[cbor][struct]")
{
  // {"name": "test", "age": -5, "score": float32(0.0)}
  auto data = cbor({
    0xA3,
    0x64, 'n', 'a', 'm', 'e',
    0x64, 't', 'e', 's', 't',
    0x63, 'a', 'g', 'e',
    0x24, // negative int -5 = -1-4
    0x65, 's', 'c', 'o', 'r', 'e',
    0xFA, 0x00, 0x00, 0x00, 0x00 // float32(0.0)
  });

  STFY::ParseContext context;
  context.tokenizer.allowCbor(true);
  context.tokenizer.addData((const char *)data.data(), data.size());

  SimpleStruct s;
  (void)context.parseTo(s);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(s.name == "test");
  REQUIRE(s.age == -5);
  REQUIRE(s.score == Catch::Approx(0.0f));
}

TEST_CASE("cbor_parse_float64_value", "[cbor][struct]")
{
  // {"name": "pi", "age": 0, "score": float64(3.14159)}
  // float64 3.14159 = 0x400921FA9999999A (approx)
  // Actually let's use a simpler value: float64(3.5) = 0x400C000000000000
  auto data = cbor({
    0xA3,
    0x64, 'n', 'a', 'm', 'e',
    0x62, 'p', 'i',
    0x63, 'a', 'g', 'e',
    0x00,
    0x65, 's', 'c', 'o', 'r', 'e',
    0xFB, 0x40, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // float64(3.5)
  });

  STFY::ParseContext context;
  context.tokenizer.allowCbor(true);
  context.tokenizer.addData((const char *)data.data(), data.size());

  SimpleStruct s;
  (void)context.parseTo(s);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(s.name == "pi");
  REQUIRE(s.age == 0);
  REQUIRE(s.score == Catch::Approx(3.5f));
}

TEST_CASE("cbor_parse_unmentioned_members_preserved", "[cbor][struct]")
{
  // {"name": "updated", "flag": false}
  // Deliberately omits: count, ratio, inner
  auto data = cbor({
    0xA2,
    0x64, 'n', 'a', 'm', 'e',
    0x67, 'u', 'p', 'd', 'a', 't', 'e', 'd',
    0x64, 'f', 'l', 'a', 'g',
    0xF4
  });

  STFY::ParseContext context;
  context.tokenizer.allowCbor(true);
  context.tokenizer.addData((const char *)data.data(), data.size());

  PreInitStruct s;
  s.inner.label = "original_label";
  (void)context.parseTo(s);

  REQUIRE(context.error == STFY::Error::NoError);
  // Mentioned members are updated
  REQUIRE(s.name == "updated");
  REQUIRE(s.flag == false);
  // Unmentioned members retain pre-initialized values
  REQUIRE(s.count == 42);
  REQUIRE(s.ratio == Catch::Approx(3.14f));
  REQUIRE(s.inner.label == "original_label");
}

TEST_CASE("cbor_report_missing_members", "[cbor][struct]")
{
  // {"name": "John", "age": 30, "score": float32(95.5), "extra_field": "ignored"}
  auto data = cbor({
    0xA4,
    0x64, 'n', 'a', 'm', 'e',
    0x64, 'J', 'o', 'h', 'n',
    0x63, 'a', 'g', 'e',
    0x18, 0x1E,
    0x65, 's', 'c', 'o', 'r', 'e',
    0xFA, 0x42, 0xBF, 0x00, 0x00,
    0x6B, 'e', 'x', 't', 'r', 'a', '_', 'f', 'i', 'e', 'l', 'd',
    0x67, 'i', 'g', 'n', 'o', 'r', 'e', 'd'
  });

  STFY::ParseContext context;
  context.tokenizer.allowCbor(true);
  context.tokenizer.addData((const char *)data.data(), data.size());

  SimpleStruct s;
  (void)context.parseTo(s);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(s.name == "John");
  REQUIRE(s.age == 30);
  REQUIRE(s.score == Catch::Approx(95.5f));
  REQUIRE(context.missing_members.size() == 1);
  REQUIRE(context.missing_members.front() == "extra_field");
}

TEST_CASE("cbor_report_unassigned_required_members", "[cbor][struct]")
{
  // {"name": "John", "age": 30}
  // Omits "score" which is a required member
  auto data = cbor({
    0xA2,
    0x64, 'n', 'a', 'm', 'e',
    0x64, 'J', 'o', 'h', 'n',
    0x63, 'a', 'g', 'e',
    0x18, 0x1E
  });

  STFY::ParseContext context;
  context.tokenizer.allowCbor(true);
  context.tokenizer.addData((const char *)data.data(), data.size());

  SimpleStruct s;
  (void)context.parseTo(s);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(s.name == "John");
  REQUIRE(s.age == 30);
  REQUIRE(context.unassigned_required_members.size() == 1);
  REQUIRE(context.unassigned_required_members.front() == "score");
}

} // namespace cbor_struct_test

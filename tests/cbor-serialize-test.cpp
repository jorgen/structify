#include <structify/structify.h>

#include "catch2/catch_all.hpp"

namespace cbor_serialize_test
{

// ── Helpers ─────────────────────────────────────────────────────────

static uint8_t hexCharToNibble(char c)
{
  if (c >= '0' && c <= '9')
    return static_cast<uint8_t>(c - '0');
  if (c >= 'a' && c <= 'f')
    return static_cast<uint8_t>(c - 'a' + 10);
  if (c >= 'A' && c <= 'F')
    return static_cast<uint8_t>(c - 'A' + 10);
  return 0;
}

static std::vector<uint8_t> fromHex(const char *hex)
{
  std::vector<uint8_t> bytes;
  for (size_t i = 0; hex[i] && hex[i + 1]; i += 2)
  {
    uint8_t byte = static_cast<uint8_t>((hexCharToNibble(hex[i]) << 4) | hexCharToNibble(hex[i + 1]));
    bytes.push_back(byte);
  }
  return bytes;
}

static std::vector<unsigned char> cbor(std::initializer_list<unsigned char> bytes)
{
  return std::vector<unsigned char>(bytes);
}

// ── Struct definitions ──────────────────────────────────────────────

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

struct DeepNested
{
  struct Level2
  {
    struct Level3
    {
      std::string value;
      STFY_OBJ(value);
    };
    Level3 level3;
    STFY_OBJ(level3);
  };
  Level2 level2;
  STFY_OBJ(level2);
};

struct WithDouble
{
  std::string label;
  double value;
  STFY_OBJ(label, value);
};

struct EmptyVecStruct
{
  std::vector<int> items;
  STFY_OBJ(items);
};

// K8s-like complex struct
struct EnvVar
{
  std::string name;
  std::string value;
  STFY_OBJ(name, value);
};

struct ContainerPort
{
  int containerPort;
  STFY_OBJ(containerPort);
};

struct Container
{
  std::string name;
  std::string image;
  std::vector<ContainerPort> ports;
  std::vector<EnvVar> env;
  STFY_OBJ(name, image, ports, env);
};

struct PodSpec
{
  std::vector<Container> containers;
  STFY_OBJ(containers);
};

struct Labels
{
  std::string app;
  std::string tier;
  STFY_OBJ(app, tier);
};

struct Metadata
{
  std::string name;
  Labels labels;
  STFY_OBJ(name, labels);
};

struct PodConfig
{
  std::string kind;
  Metadata metadata;
  PodSpec spec;
  STFY_OBJ(kind, metadata, spec);
};

// ── RFC 7049 Appendix A: CborWriter scalar encode tests ─────────────

TEST_CASE("cbor_encode_unsigned_integers", "[cbor][serialize][rfc7049]")
{
  SECTION("uint 0")
  {
    std::vector<uint8_t> out;
    STFY::CborWriter w(out);
    w.writeUint(0);
    REQUIRE(out == fromHex("00"));
  }
  SECTION("uint 1")
  {
    std::vector<uint8_t> out;
    STFY::CborWriter w(out);
    w.writeUint(1);
    REQUIRE(out == fromHex("01"));
  }
  SECTION("uint 10")
  {
    std::vector<uint8_t> out;
    STFY::CborWriter w(out);
    w.writeUint(10);
    REQUIRE(out == fromHex("0a"));
  }
  SECTION("uint 23")
  {
    std::vector<uint8_t> out;
    STFY::CborWriter w(out);
    w.writeUint(23);
    REQUIRE(out == fromHex("17"));
  }
  SECTION("uint 24")
  {
    std::vector<uint8_t> out;
    STFY::CborWriter w(out);
    w.writeUint(24);
    REQUIRE(out == fromHex("1818"));
  }
  SECTION("uint 25")
  {
    std::vector<uint8_t> out;
    STFY::CborWriter w(out);
    w.writeUint(25);
    REQUIRE(out == fromHex("1819"));
  }
  SECTION("uint 100")
  {
    std::vector<uint8_t> out;
    STFY::CborWriter w(out);
    w.writeUint(100);
    REQUIRE(out == fromHex("1864"));
  }
  SECTION("uint 1000")
  {
    std::vector<uint8_t> out;
    STFY::CborWriter w(out);
    w.writeUint(1000);
    REQUIRE(out == fromHex("1903e8"));
  }
  SECTION("uint 1000000")
  {
    std::vector<uint8_t> out;
    STFY::CborWriter w(out);
    w.writeUint(1000000);
    REQUIRE(out == fromHex("1a000f4240"));
  }
  SECTION("uint 1000000000000")
  {
    std::vector<uint8_t> out;
    STFY::CborWriter w(out);
    w.writeUint(1000000000000ULL);
    REQUIRE(out == fromHex("1b000000e8d4a51000"));
  }
  SECTION("uint max 64-bit: 18446744073709551615")
  {
    std::vector<uint8_t> out;
    STFY::CborWriter w(out);
    w.writeUint(18446744073709551615ULL);
    REQUIRE(out == fromHex("1bffffffffffffffff"));
  }
}

TEST_CASE("cbor_encode_negative_integers", "[cbor][serialize][rfc7049]")
{
  SECTION("neg int -1")
  {
    std::vector<uint8_t> out;
    STFY::CborWriter w(out);
    w.writeInt(-1);
    REQUIRE(out == fromHex("20"));
  }
  SECTION("neg int -10")
  {
    std::vector<uint8_t> out;
    STFY::CborWriter w(out);
    w.writeInt(-10);
    REQUIRE(out == fromHex("29"));
  }
  SECTION("neg int -100")
  {
    std::vector<uint8_t> out;
    STFY::CborWriter w(out);
    w.writeInt(-100);
    REQUIRE(out == fromHex("3863"));
  }
  SECTION("neg int -1000")
  {
    std::vector<uint8_t> out;
    STFY::CborWriter w(out);
    w.writeInt(-1000);
    REQUIRE(out == fromHex("3903e7"));
  }
}

TEST_CASE("cbor_encode_booleans", "[cbor][serialize][rfc7049]")
{
  SECTION("false")
  {
    std::vector<uint8_t> out;
    STFY::CborWriter w(out);
    w.writeBool(false);
    REQUIRE(out == fromHex("f4"));
  }
  SECTION("true")
  {
    std::vector<uint8_t> out;
    STFY::CborWriter w(out);
    w.writeBool(true);
    REQUIRE(out == fromHex("f5"));
  }
}

TEST_CASE("cbor_encode_null", "[cbor][serialize][rfc7049]")
{
  std::vector<uint8_t> out;
  STFY::CborWriter w(out);
  w.writeNull();
  REQUIRE(out == fromHex("f6"));
}

TEST_CASE("cbor_encode_float64", "[cbor][serialize][rfc7049]")
{
  SECTION("double 1.1")
  {
    std::vector<uint8_t> out;
    STFY::CborWriter w(out);
    w.writeDouble(1.1);
    REQUIRE(out == fromHex("fb3ff199999999999a"));
  }
  SECTION("double 1.0e+300")
  {
    std::vector<uint8_t> out;
    STFY::CborWriter w(out);
    w.writeDouble(1.0e+300);
    REQUIRE(out == fromHex("fb7e37e43c8800759c"));
  }
  SECTION("double -4.1")
  {
    std::vector<uint8_t> out;
    STFY::CborWriter w(out);
    w.writeDouble(-4.1);
    REQUIRE(out == fromHex("fbc010666666666666"));
  }
  SECTION("double 0.0")
  {
    std::vector<uint8_t> out;
    STFY::CborWriter w(out);
    w.writeDouble(0.0);
    REQUIRE(out == fromHex("fb0000000000000000"));
  }
  SECTION("double 1.5")
  {
    std::vector<uint8_t> out;
    STFY::CborWriter w(out);
    w.writeDouble(1.5);
    REQUIRE(out == fromHex("fb3ff8000000000000"));
  }
  SECTION("double 100000.0")
  {
    std::vector<uint8_t> out;
    STFY::CborWriter w(out);
    w.writeDouble(100000.0);
    REQUIRE(out == fromHex("fb40f86a0000000000"));
  }
  SECTION("double 3.4028234663852886e+38")
  {
    std::vector<uint8_t> out;
    STFY::CborWriter w(out);
    w.writeDouble(3.4028234663852886e+38);
    REQUIRE(out == fromHex("fb47efffffe0000000"));
  }
}

TEST_CASE("cbor_encode_float32", "[cbor][serialize][rfc7049]")
{
  SECTION("float32 95.5")
  {
    std::vector<uint8_t> out;
    STFY::CborWriter w(out);
    w.writeFloat(95.5f);
    REQUIRE(out == fromHex("fa42bf0000"));
  }
  SECTION("float32 0.0")
  {
    std::vector<uint8_t> out;
    STFY::CborWriter w(out);
    w.writeFloat(0.0f);
    REQUIRE(out == fromHex("fa00000000"));
  }
  SECTION("float32 3.14")
  {
    // float32 3.14 = 0x4048F5C3
    std::vector<uint8_t> out;
    STFY::CborWriter w(out);
    w.writeFloat(3.14f);
    auto expected = fromHex("fa4048f5c3");
    REQUIRE(out == expected);
  }
}

TEST_CASE("cbor_encode_text_strings", "[cbor][serialize][rfc7049]")
{
  SECTION("empty string")
  {
    std::vector<uint8_t> out;
    STFY::CborWriter w(out);
    w.writeKey("", 0);
    REQUIRE(out == fromHex("60"));
  }
  SECTION("string 'a'")
  {
    std::vector<uint8_t> out;
    STFY::CborWriter w(out);
    w.writeKey("a", 1);
    REQUIRE(out == fromHex("6161"));
  }
  SECTION("string 'IETF'")
  {
    std::vector<uint8_t> out;
    STFY::CborWriter w(out);
    w.writeKey("IETF", 4);
    REQUIRE(out == fromHex("6449455446"));
  }
  SECTION("string with backslash-quote")
  {
    // "\"\\" → 2 chars
    std::vector<uint8_t> out;
    STFY::CborWriter w(out);
    w.writeKey("\"\\", 2);
    REQUIRE(out == fromHex("62225c"));
  }
}

// ── writeInt dispatching test ───────────────────────────────────────

TEST_CASE("cbor_writeInt_positive_uses_uint", "[cbor][serialize]")
{
  // writeInt with positive values should produce same as writeUint
  std::vector<uint8_t> out_int, out_uint;

  STFY::CborWriter w1(out_int);
  w1.writeInt(42);

  STFY::CborWriter w2(out_uint);
  w2.writeUint(42);

  REQUIRE(out_int == out_uint);
}

TEST_CASE("cbor_writeInt_zero", "[cbor][serialize]")
{
  std::vector<uint8_t> out;
  STFY::CborWriter w(out);
  w.writeInt(0);
  REQUIRE(out == fromHex("00"));
}

// ── Struct-level serialization output tests ─────────────────────────

TEST_CASE("cbor_serialize_simple_struct_output", "[cbor][serialize]")
{
  SimpleStruct s;
  s.name = "John";
  s.age = 30;
  s.score = 95.5f;

  auto data = STFY::serializeStructCbor(s);

  // Should start with indefinite-length map marker
  REQUIRE(data.front() == 0xBF);
  // Should end with break code
  REQUIRE(data.back() == 0xFF);

  // Should contain the key "name" as CBOR text string (64 6E616D65)
  auto name_key = fromHex("646e616d65");
  bool found_name = false;
  for (size_t i = 0; i + name_key.size() <= data.size(); i++)
  {
    if (memcmp(&data[i], name_key.data(), name_key.size()) == 0)
    {
      found_name = true;
      break;
    }
  }
  REQUIRE(found_name);

  // Should contain the value "John" as CBOR text string (64 4A6F686E)
  auto john_val = fromHex("644a6f686e");
  bool found_john = false;
  for (size_t i = 0; i + john_val.size() <= data.size(); i++)
  {
    if (memcmp(&data[i], john_val.data(), john_val.size()) == 0)
    {
      found_john = true;
      break;
    }
  }
  REQUIRE(found_john);
}

TEST_CASE("cbor_serialize_contains_boolean_markers", "[cbor][serialize]")
{
  WithBooleans s;
  s.enabled = true;
  s.active = false;

  auto data = STFY::serializeStructCbor(s);

  // Should contain 0xF5 (true) and 0xF4 (false)
  bool found_true = false;
  bool found_false = false;
  for (auto b : data)
  {
    if (b == 0xF5)
      found_true = true;
    if (b == 0xF4)
      found_false = true;
  }
  REQUIRE(found_true);
  REQUIRE(found_false);
}

TEST_CASE("cbor_serialize_contains_array_markers", "[cbor][serialize]")
{
  WithVector s;
  s.name = "test";
  s.scores = {10, 20, 30};

  auto data = STFY::serializeStructCbor(s);

  // Should contain indefinite-length array start (0x9F) and break (0xFF)
  bool found_array_start = false;
  for (auto b : data)
  {
    if (b == 0x9F)
    {
      found_array_start = true;
      break;
    }
  }
  REQUIRE(found_array_start);
}

// ── Round-trip tests ────────────────────────────────────────────────

TEST_CASE("cbor_roundtrip_simple_struct", "[cbor][serialize][roundtrip]")
{
  SimpleStruct original;
  original.name = "John";
  original.age = 30;
  original.score = 95.5f;

  auto data = STFY::serializeStructCbor(original);

  STFY::ParseContext context;
  context.tokenizer.allowCbor(true);
  context.tokenizer.addData((const char *)data.data(), data.size());

  SimpleStruct parsed;
  (void)context.parseTo(parsed);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(parsed.name == original.name);
  REQUIRE(parsed.age == original.age);
  REQUIRE(parsed.score == Catch::Approx(original.score));
}

TEST_CASE("cbor_roundtrip_nested_struct", "[cbor][serialize][roundtrip]")
{
  NestedOuter original;
  original.name = "Alice";
  original.address.street = "123 Main St";
  original.address.city = "Springfield";

  auto data = STFY::serializeStructCbor(original);

  STFY::ParseContext context;
  context.tokenizer.allowCbor(true);
  context.tokenizer.addData((const char *)data.data(), data.size());

  NestedOuter parsed;
  (void)context.parseTo(parsed);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(parsed.name == original.name);
  REQUIRE(parsed.address.street == original.address.street);
  REQUIRE(parsed.address.city == original.address.city);
}

TEST_CASE("cbor_roundtrip_with_vector", "[cbor][serialize][roundtrip]")
{
  WithVector original;
  original.name = "test";
  original.scores = {100, 95, 88, 72};

  auto data = STFY::serializeStructCbor(original);

  STFY::ParseContext context;
  context.tokenizer.allowCbor(true);
  context.tokenizer.addData((const char *)data.data(), data.size());

  WithVector parsed;
  (void)context.parseTo(parsed);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(parsed.name == original.name);
  REQUIRE(parsed.scores == original.scores);
}

TEST_CASE("cbor_roundtrip_booleans", "[cbor][serialize][roundtrip]")
{
  WithBooleans original;
  original.enabled = true;
  original.active = false;

  auto data = STFY::serializeStructCbor(original);

  STFY::ParseContext context;
  context.tokenizer.allowCbor(true);
  context.tokenizer.addData((const char *)data.data(), data.size());

  WithBooleans parsed;
  (void)context.parseTo(parsed);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(parsed.enabled == original.enabled);
  REQUIRE(parsed.active == original.active);
}

TEST_CASE("cbor_roundtrip_optional_present", "[cbor][serialize][roundtrip]")
{
  WithOptional original;
  original.name = "Eve";
  original.age.data = 25;
  original.email.data = "eve@example.com";

  auto data = STFY::serializeStructCbor(original);

  STFY::ParseContext context;
  context.tokenizer.allowCbor(true);
  context.tokenizer.addData((const char *)data.data(), data.size());

  WithOptional parsed;
  (void)context.parseTo(parsed);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(parsed.name == original.name);
  REQUIRE(parsed.age.data == original.age.data);
  REQUIRE(parsed.email.data == original.email.data);
}

TEST_CASE("cbor_roundtrip_mixed_struct", "[cbor][serialize][roundtrip]")
{
  MixedStruct original;
  original.name = "Charlie";
  original.age = 35;
  original.active = true;
  original.hobbies = {"reading", "coding"};

  auto data = STFY::serializeStructCbor(original);

  STFY::ParseContext context;
  context.tokenizer.allowCbor(true);
  context.tokenizer.addData((const char *)data.data(), data.size());

  MixedStruct parsed;
  (void)context.parseTo(parsed);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(parsed.name == original.name);
  REQUIRE(parsed.age == original.age);
  REQUIRE(parsed.active == original.active);
  REQUIRE(parsed.hobbies == original.hobbies);
}

TEST_CASE("cbor_roundtrip_deep_nested", "[cbor][serialize][roundtrip]")
{
  DeepNested original;
  original.level2.level3.value = "deep_value";

  auto data = STFY::serializeStructCbor(original);

  STFY::ParseContext context;
  context.tokenizer.allowCbor(true);
  context.tokenizer.addData((const char *)data.data(), data.size());

  DeepNested parsed;
  (void)context.parseTo(parsed);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(parsed.level2.level3.value == original.level2.level3.value);
}

TEST_CASE("cbor_roundtrip_negative_numbers", "[cbor][serialize][roundtrip]")
{
  SimpleStruct original;
  original.name = "neg";
  original.age = -42;
  original.score = -3.14f;

  auto data = STFY::serializeStructCbor(original);

  STFY::ParseContext context;
  context.tokenizer.allowCbor(true);
  context.tokenizer.addData((const char *)data.data(), data.size());

  SimpleStruct parsed;
  (void)context.parseTo(parsed);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(parsed.name == original.name);
  REQUIRE(parsed.age == original.age);
  REQUIRE(parsed.score == Catch::Approx(original.score));
}

TEST_CASE("cbor_roundtrip_empty_vector", "[cbor][serialize][roundtrip]")
{
  EmptyVecStruct original;

  auto data = STFY::serializeStructCbor(original);

  STFY::ParseContext context;
  context.tokenizer.allowCbor(true);
  context.tokenizer.addData((const char *)data.data(), data.size());

  EmptyVecStruct parsed;
  (void)context.parseTo(parsed);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(parsed.items.empty());
}

TEST_CASE("cbor_roundtrip_with_double", "[cbor][serialize][roundtrip]")
{
  WithDouble original;
  original.label = "pi";
  original.value = 3.141592653589793;

  auto data = STFY::serializeStructCbor(original);

  STFY::ParseContext context;
  context.tokenizer.allowCbor(true);
  context.tokenizer.addData((const char *)data.data(), data.size());

  WithDouble parsed;
  (void)context.parseTo(parsed);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(parsed.label == original.label);
  REQUIRE(parsed.value == Catch::Approx(original.value));
}

TEST_CASE("cbor_roundtrip_complex_k8s_like", "[cbor][serialize][roundtrip]")
{
  PodConfig original;
  original.kind = "Pod";
  original.metadata.name = "my-app";
  original.metadata.labels.app = "web-server";
  original.metadata.labels.tier = "frontend";

  Container c0;
  c0.name = "nginx";
  c0.image = "nginx:1.25";
  c0.ports = {{80}, {443}};
  c0.env = {{"ENV", "production"}, {"LOG_LEVEL", "warn"}};

  Container c1;
  c1.name = "sidecar";
  c1.image = "fluent/fluentd:v1.16";
  c1.ports = {{24224}};
  c1.env = {{"FLUSH_INTERVAL", "5s"}};

  original.spec.containers = {c0, c1};

  auto data = STFY::serializeStructCbor(original);

  STFY::ParseContext context;
  context.tokenizer.allowCbor(true);
  context.tokenizer.addData((const char *)data.data(), data.size());

  PodConfig parsed;
  (void)context.parseTo(parsed);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(parsed.kind == original.kind);
  REQUIRE(parsed.metadata.name == original.metadata.name);
  REQUIRE(parsed.metadata.labels.app == original.metadata.labels.app);
  REQUIRE(parsed.metadata.labels.tier == original.metadata.labels.tier);
  REQUIRE(parsed.spec.containers.size() == 2);

  auto &pc0 = parsed.spec.containers[0];
  REQUIRE(pc0.name == "nginx");
  REQUIRE(pc0.image == "nginx:1.25");
  REQUIRE(pc0.ports.size() == 2);
  REQUIRE(pc0.ports[0].containerPort == 80);
  REQUIRE(pc0.ports[1].containerPort == 443);
  REQUIRE(pc0.env.size() == 2);
  REQUIRE(pc0.env[0].name == "ENV");
  REQUIRE(pc0.env[0].value == "production");

  auto &pc1 = parsed.spec.containers[1];
  REQUIRE(pc1.name == "sidecar");
  REQUIRE(pc1.image == "fluent/fluentd:v1.16");
  REQUIRE(pc1.ports.size() == 1);
  REQUIRE(pc1.ports[0].containerPort == 24224);
  REQUIRE(pc1.env.size() == 1);
  REQUIRE(pc1.env[0].name == "FLUSH_INTERVAL");
  REQUIRE(pc1.env[0].value == "5s");
}

// ── Parse-then-serialize-then-parse round-trip ──────────────────────

TEST_CASE("cbor_parse_serialize_reparse_simple", "[cbor][serialize][roundtrip]")
{
  // Build CBOR by hand: {"name": "John", "age": 30, "score": float32(95.5)}
  auto input = cbor({
    0xA3,
    0x64, 'n', 'a', 'm', 'e',
    0x64, 'J', 'o', 'h', 'n',
    0x63, 'a', 'g', 'e',
    0x18, 0x1E,
    0x65, 's', 'c', 'o', 'r', 'e',
    0xFA, 0x42, 0xBF, 0x00, 0x00
  });

  // Parse
  STFY::ParseContext ctx1;
  ctx1.tokenizer.allowCbor(true);
  ctx1.tokenizer.addData((const char *)input.data(), input.size());
  SimpleStruct s1;
  (void)ctx1.parseTo(s1);
  REQUIRE(ctx1.error == STFY::Error::NoError);

  // Serialize
  auto output = STFY::serializeStructCbor(s1);

  // Re-parse
  STFY::ParseContext ctx2;
  ctx2.tokenizer.allowCbor(true);
  ctx2.tokenizer.addData((const char *)output.data(), output.size());
  SimpleStruct s2;
  (void)ctx2.parseTo(s2);
  REQUIRE(ctx2.error == STFY::Error::NoError);

  // Compare data
  REQUIRE(s2.name == s1.name);
  REQUIRE(s2.age == s1.age);
  REQUIRE(s2.score == Catch::Approx(s1.score));
}

TEST_CASE("cbor_parse_serialize_reparse_mixed", "[cbor][serialize][roundtrip]")
{
  // Build CBOR: {"name": "Charlie", "age": 35, "active": true, "hobbies": ["reading", "coding"]}
  auto input = cbor({
    0xA4,
    0x64, 'n', 'a', 'm', 'e',
    0x67, 'C', 'h', 'a', 'r', 'l', 'i', 'e',
    0x63, 'a', 'g', 'e',
    0x18, 0x23,
    0x66, 'a', 'c', 't', 'i', 'v', 'e',
    0xF5,
    0x67, 'h', 'o', 'b', 'b', 'i', 'e', 's',
    0x82,
      0x67, 'r', 'e', 'a', 'd', 'i', 'n', 'g',
      0x66, 'c', 'o', 'd', 'i', 'n', 'g'
  });

  STFY::ParseContext ctx1;
  ctx1.tokenizer.allowCbor(true);
  ctx1.tokenizer.addData((const char *)input.data(), input.size());
  MixedStruct s1;
  (void)ctx1.parseTo(s1);
  REQUIRE(ctx1.error == STFY::Error::NoError);

  auto output = STFY::serializeStructCbor(s1);

  STFY::ParseContext ctx2;
  ctx2.tokenizer.allowCbor(true);
  ctx2.tokenizer.addData((const char *)output.data(), output.size());
  MixedStruct s2;
  (void)ctx2.parseTo(s2);
  REQUIRE(ctx2.error == STFY::Error::NoError);

  REQUIRE(s2.name == s1.name);
  REQUIRE(s2.age == s1.age);
  REQUIRE(s2.active == s1.active);
  REQUIRE(s2.hobbies == s1.hobbies);
}

// ── Type fidelity tests ────────────────────────────────────────────

struct FloatDoubleStruct
{
  float f_val;
  double d_val;
  STFY_OBJ(f_val, d_val);
};

TEST_CASE("cbor_serialize_float_uses_float32", "[cbor][serialize][float]")
{
  SimpleStruct s;
  s.name = "test";
  s.age = 1;
  s.score = 3.14f;

  auto data = STFY::serializeStructCbor(s);

  // Find 0xFA marker (CBOR float32) in the output
  bool found_fa = false;
  for (size_t i = 0; i < data.size(); i++)
  {
    if (data[i] == 0xFA)
    {
      found_fa = true;
      REQUIRE(i + 4 < data.size());
      uint32_t bits = ((uint32_t)data[i + 1] << 24) | ((uint32_t)data[i + 2] << 16) |
                      ((uint32_t)data[i + 3] << 8) | (uint32_t)data[i + 4];
      float parsed;
      memcpy(&parsed, &bits, sizeof(parsed));
      REQUIRE(parsed == Catch::Approx(3.14f));
      break;
    }
  }
  REQUIRE(found_fa);
}

TEST_CASE("cbor_serialize_double_uses_float64", "[cbor][serialize][float]")
{
  WithDouble s;
  s.label = "pi";
  s.value = 3.141592653589793;

  auto data = STFY::serializeStructCbor(s);

  bool found_fb = false;
  for (size_t i = 0; i < data.size(); i++)
  {
    if (data[i] == 0xFB)
    {
      found_fb = true;
      REQUIRE(i + 8 < data.size());
      uint64_t bits = ((uint64_t)data[i + 1] << 56) | ((uint64_t)data[i + 2] << 48) |
                      ((uint64_t)data[i + 3] << 40) | ((uint64_t)data[i + 4] << 32) |
                      ((uint64_t)data[i + 5] << 24) | ((uint64_t)data[i + 6] << 16) |
                      ((uint64_t)data[i + 7] << 8) | (uint64_t)data[i + 8];
      double parsed;
      memcpy(&parsed, &bits, sizeof(parsed));
      REQUIRE(parsed == Catch::Approx(3.141592653589793));
      break;
    }
  }
  REQUIRE(found_fb);
}

TEST_CASE("cbor_serialize_float_and_double_type_fidelity", "[cbor][serialize][float]")
{
  FloatDoubleStruct s;
  s.f_val = 1.5f;
  s.d_val = 1.5;

  auto data = STFY::serializeStructCbor(s);

  int fa_count = 0;
  int fb_count = 0;
  for (auto b : data)
  {
    if (b == 0xFA)
      fa_count++;
    if (b == 0xFB)
      fb_count++;
  }
  REQUIRE(fa_count == 1); // float field → float32
  REQUIRE(fb_count == 1); // double field → float64
}

TEST_CASE("cbor_roundtrip_float_exact_precision", "[cbor][serialize][roundtrip][float]")
{
  SimpleStruct original;
  original.name = "precision";
  original.age = 1;
  original.score = 3.14f;

  auto data = STFY::serializeStructCbor(original);

  STFY::ParseContext context;
  context.tokenizer.allowCbor(true);
  context.tokenizer.addData((const char *)data.data(), data.size());

  SimpleStruct parsed;
  (void)context.parseTo(parsed);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(parsed.score == original.score); // exact bit equality
}

TEST_CASE("cbor_serialize_float_edge_cases", "[cbor][serialize][float]")
{
  SECTION("positive zero")
  {
    FloatDoubleStruct s;
    s.f_val = 0.0f;
    s.d_val = 0.0;
    auto data = STFY::serializeStructCbor(s);

    STFY::ParseContext ctx;
    ctx.tokenizer.allowCbor(true);
    ctx.tokenizer.addData((const char *)data.data(), data.size());
    FloatDoubleStruct p;
    (void)ctx.parseTo(p);
    REQUIRE(ctx.error == STFY::Error::NoError);
    REQUIRE(p.f_val == 0.0f);
    REQUIRE(p.d_val == 0.0);
  }

  SECTION("negative values")
  {
    FloatDoubleStruct s;
    s.f_val = -42.5f;
    s.d_val = -42.5;
    auto data = STFY::serializeStructCbor(s);

    STFY::ParseContext ctx;
    ctx.tokenizer.allowCbor(true);
    ctx.tokenizer.addData((const char *)data.data(), data.size());
    FloatDoubleStruct p;
    (void)ctx.parseTo(p);
    REQUIRE(ctx.error == STFY::Error::NoError);
    REQUIRE(p.f_val == s.f_val);
    REQUIRE(p.d_val == s.d_val);
  }

  SECTION("large values")
  {
    FloatDoubleStruct s;
    s.f_val = 3.4028234e+38f;
    s.d_val = 1.7976931348623157e+308;
    auto data = STFY::serializeStructCbor(s);

    STFY::ParseContext ctx;
    ctx.tokenizer.allowCbor(true);
    ctx.tokenizer.addData((const char *)data.data(), data.size());
    FloatDoubleStruct p;
    (void)ctx.parseTo(p);
    REQUIRE(ctx.error == STFY::Error::NoError);
    REQUIRE(p.f_val == s.f_val);
    REQUIRE(p.d_val == s.d_val);
  }
}

} // namespace cbor_serialize_test

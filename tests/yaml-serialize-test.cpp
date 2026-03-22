#include <structify/structify.h>

#include "catch2/catch_all.hpp"

namespace yaml_serialize_test
{

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

struct WithStringVector
{
  std::vector<std::string> items;
  STFY_OBJ(items);
};

struct Person
{
  std::string name;
  int age;
  STFY_OBJ(name, age);
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

struct MixedStruct
{
  std::string name;
  int age;
  bool active;
  std::vector<std::string> hobbies;
  STFY_OBJ(name, age, active, hobbies);
};

struct EmptyVecStruct
{
  std::vector<int> items;
  STFY_OBJ(items);
};

struct WithDouble
{
  std::string label;
  double value;
  STFY_OBJ(label, value);
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

struct PodSpec
{
  std::vector<Container> containers;
  STFY_OBJ(containers);
};

struct PodConfig
{
  std::string kind;
  Metadata metadata;
  PodSpec spec;
  STFY_OBJ(kind, metadata, spec);
};

// Inheritance test structs
struct Base
{
  std::string id;
  STFY_OBJ(id);
};

struct Derived : public Base
{
  std::string extra;
  STFY_OBJECT_WITH_SUPER(STFY_SUPER_CLASSES(STFY_SUPER_CLASS(Base)), STFY_MEMBER(extra));
};

struct PeopleWrapper
{
  std::vector<Person> people;
  STFY_OBJ(people);
};

// ── Output format tests ─────────────────────────────────────────────

TEST_CASE("yaml_serialize_simple_struct", "[yaml][serialize]")
{
  SimpleStruct s;
  s.name = "John";
  s.age = 30;
  s.score = 95.5f;

  std::string yaml = STFY::serializeStructYaml(s);

  REQUIRE(yaml.find("name: John\n") != std::string::npos);
  REQUIRE(yaml.find("age: 30\n") != std::string::npos);
  REQUIRE(yaml.find("score: 95.5\n") != std::string::npos);
}

TEST_CASE("yaml_serialize_nested_struct", "[yaml][serialize]")
{
  NestedOuter s;
  s.name = "Alice";
  s.address.street = "123 Main St";
  s.address.city = "Springfield";

  std::string yaml = STFY::serializeStructYaml(s);

  REQUIRE(yaml.find("name: Alice\n") != std::string::npos);
  REQUIRE(yaml.find("address:\n") != std::string::npos);
  REQUIRE(yaml.find("  street: 123 Main St\n") != std::string::npos);
  REQUIRE(yaml.find("  city: Springfield\n") != std::string::npos);
}

TEST_CASE("yaml_serialize_deeply_nested", "[yaml][serialize]")
{
  DeepNested s;
  s.level2.level3.value = "deep";

  std::string yaml = STFY::serializeStructYaml(s);

  REQUIRE(yaml.find("level2:\n") != std::string::npos);
  REQUIRE(yaml.find("  level3:\n") != std::string::npos);
  REQUIRE(yaml.find("    value: deep\n") != std::string::npos);
}

TEST_CASE("yaml_serialize_vector_of_ints", "[yaml][serialize]")
{
  WithVector s;
  s.name = "test";
  s.scores = {100, 95, 88};

  std::string yaml = STFY::serializeStructYaml(s);

  REQUIRE(yaml.find("name: test\n") != std::string::npos);
  REQUIRE(yaml.find("scores:\n") != std::string::npos);
  REQUIRE(yaml.find("- 100\n") != std::string::npos);
  REQUIRE(yaml.find("- 95\n") != std::string::npos);
  REQUIRE(yaml.find("- 88\n") != std::string::npos);
}

TEST_CASE("yaml_serialize_vector_of_strings", "[yaml][serialize]")
{
  WithStringVector s;
  s.items = {"apple", "banana", "cherry"};

  std::string yaml = STFY::serializeStructYaml(s);

  REQUIRE(yaml.find("- apple\n") != std::string::npos);
  REQUIRE(yaml.find("- banana\n") != std::string::npos);
  REQUIRE(yaml.find("- cherry\n") != std::string::npos);
}

TEST_CASE("yaml_serialize_empty_vector", "[yaml][serialize]")
{
  EmptyVecStruct s;

  std::string yaml = STFY::serializeStructYaml(s);

  REQUIRE(yaml.find("[]\n") != std::string::npos);
}

TEST_CASE("yaml_serialize_booleans", "[yaml][serialize]")
{
  WithBooleans s;
  s.enabled = true;
  s.active = false;

  std::string yaml = STFY::serializeStructYaml(s);

  REQUIRE(yaml.find("enabled: true\n") != std::string::npos);
  REQUIRE(yaml.find("active: false\n") != std::string::npos);
}

TEST_CASE("yaml_serialize_negative_numbers", "[yaml][serialize]")
{
  SimpleStruct s;
  s.name = "test";
  s.age = -42;
  s.score = -3.14f;

  std::string yaml = STFY::serializeStructYaml(s);

  REQUIRE(yaml.find("age: -42\n") != std::string::npos);
  // Float serialization may vary, just check it's negative
  REQUIRE(yaml.find("score: -3.14") != std::string::npos);
}

TEST_CASE("yaml_serialize_zero_values", "[yaml][serialize]")
{
  SimpleStruct s;
  s.name = "";
  s.age = 0;
  s.score = 0.0f;

  std::string yaml = STFY::serializeStructYaml(s);

  REQUIRE(yaml.find("name: \"\"\n") != std::string::npos);
  REQUIRE(yaml.find("age: 0\n") != std::string::npos);
}

TEST_CASE("yaml_serialize_strings_needing_quoting", "[yaml][serialize]")
{
  // Strings that look like YAML reserved words need quoting
  WithStringVector s;
  s.items = {"true", "false", "null", "yes", "no", "on", "off"};

  std::string yaml = STFY::serializeStructYaml(s);

  // Each reserved word should be quoted
  REQUIRE(yaml.find("\"true\"") != std::string::npos);
  REQUIRE(yaml.find("\"false\"") != std::string::npos);
  REQUIRE(yaml.find("\"null\"") != std::string::npos);
  REQUIRE(yaml.find("\"yes\"") != std::string::npos);
  REQUIRE(yaml.find("\"no\"") != std::string::npos);
  REQUIRE(yaml.find("\"on\"") != std::string::npos);
  REQUIRE(yaml.find("\"off\"") != std::string::npos);
}

TEST_CASE("yaml_serialize_strings_with_special_chars", "[yaml][serialize]")
{
  WithStringVector s;
  s.items = {"has:colon", "has#hash", "has[bracket"};

  std::string yaml = STFY::serializeStructYaml(s);

  // Each should be double-quoted
  REQUIRE(yaml.find("\"has:colon\"") != std::string::npos);
  REQUIRE(yaml.find("\"has#hash\"") != std::string::npos);
  REQUIRE(yaml.find("\"has[bracket\"") != std::string::npos);
}

TEST_CASE("yaml_serialize_strings_with_escapes", "[yaml][serialize]")
{
  WithStringVector s;
  s.items = {"line1\nline2", "tab\there", "say \"hi\""};

  std::string yaml = STFY::serializeStructYaml(s);

  // Newlines and tabs should be escaped in quoted strings
  REQUIRE(yaml.find("\"line1\\nline2\"") != std::string::npos);
  REQUIRE(yaml.find("\"tab\\there\"") != std::string::npos);
  REQUIRE(yaml.find("\"say \\\"hi\\\"\"") != std::string::npos);
}

TEST_CASE("yaml_serialize_optional_present", "[yaml][serialize]")
{
  WithOptional s;
  s.name = "Eve";
  s.age.data = 25;
  s.email.data = "eve@example.com";

  std::string yaml = STFY::serializeStructYaml(s);

  REQUIRE(yaml.find("name: Eve\n") != std::string::npos);
  REQUIRE(yaml.find("age: 25\n") != std::string::npos);
  // '@' requires quoting in YAML
  REQUIRE(yaml.find("email: \"eve@example.com\"\n") != std::string::npos);
}

TEST_CASE("yaml_serialize_mixed_struct", "[yaml][serialize]")
{
  MixedStruct s;
  s.name = "Alice";
  s.age = 28;
  s.active = true;
  s.hobbies = {"reading", "gaming", "cooking"};

  std::string yaml = STFY::serializeStructYaml(s);

  REQUIRE(yaml.find("name: Alice\n") != std::string::npos);
  REQUIRE(yaml.find("age: 28\n") != std::string::npos);
  REQUIRE(yaml.find("active: true\n") != std::string::npos);
  REQUIRE(yaml.find("hobbies:\n") != std::string::npos);
  REQUIRE(yaml.find("- reading\n") != std::string::npos);
  REQUIRE(yaml.find("- gaming\n") != std::string::npos);
  REQUIRE(yaml.find("- cooking\n") != std::string::npos);
}

TEST_CASE("yaml_serialize_large_numbers", "[yaml][serialize]")
{
  WithDouble s;
  s.label = "big";
  s.value = 1000000.0;

  std::string yaml = STFY::serializeStructYaml(s);

  REQUIRE(yaml.find("label: big\n") != std::string::npos);
  // Double serialization should include the number
  REQUIRE(yaml.find("value:") != std::string::npos);
}

// ── Round-trip tests ────────────────────────────────────────────────

TEST_CASE("yaml_roundtrip_simple_struct", "[yaml][serialize][roundtrip]")
{
  SimpleStruct original;
  original.name = "John";
  original.age = 30;
  original.score = 95.5f;

  std::string yaml = STFY::serializeStructYaml(original);

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml.c_str(), yaml.size());

  SimpleStruct parsed;
  context.parseTo(parsed);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(parsed.name == original.name);
  REQUIRE(parsed.age == original.age);
  REQUIRE(parsed.score == Catch::Approx(original.score));
}

TEST_CASE("yaml_roundtrip_nested_struct", "[yaml][serialize][roundtrip]")
{
  NestedOuter original;
  original.name = "Alice";
  original.address.street = "123 Main St";
  original.address.city = "Springfield";

  std::string yaml = STFY::serializeStructYaml(original);

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml.c_str(), yaml.size());

  NestedOuter parsed;
  context.parseTo(parsed);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(parsed.name == original.name);
  REQUIRE(parsed.address.street == original.address.street);
  REQUIRE(parsed.address.city == original.address.city);
}

TEST_CASE("yaml_roundtrip_with_vector", "[yaml][serialize][roundtrip]")
{
  WithVector original;
  original.name = "test";
  original.scores = {100, 95, 88, 72};

  std::string yaml = STFY::serializeStructYaml(original);

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml.c_str(), yaml.size());

  WithVector parsed;
  context.parseTo(parsed);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(parsed.name == original.name);
  REQUIRE(parsed.scores == original.scores);
}

TEST_CASE("yaml_roundtrip_with_booleans", "[yaml][serialize][roundtrip]")
{
  WithBooleans original;
  original.enabled = true;
  original.active = false;

  std::string yaml = STFY::serializeStructYaml(original);

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml.c_str(), yaml.size());

  WithBooleans parsed;
  context.parseTo(parsed);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(parsed.enabled == original.enabled);
  REQUIRE(parsed.active == original.active);
}

TEST_CASE("yaml_roundtrip_with_optional", "[yaml][serialize][roundtrip]")
{
  WithOptional original;
  original.name = "Eve";
  original.age.data = 25;
  original.email.data = "eve@example.com";

  std::string yaml = STFY::serializeStructYaml(original);

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml.c_str(), yaml.size());

  WithOptional parsed;
  context.parseTo(parsed);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(parsed.name == original.name);
  REQUIRE(parsed.age.data == original.age.data);
  REQUIRE(parsed.email.data == original.email.data);
}

TEST_CASE("yaml_roundtrip_deep_nested", "[yaml][serialize][roundtrip]")
{
  DeepNested original;
  original.level2.level3.value = "deep_value";

  std::string yaml = STFY::serializeStructYaml(original);

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml.c_str(), yaml.size());

  DeepNested parsed;
  context.parseTo(parsed);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(parsed.level2.level3.value == original.level2.level3.value);
}

TEST_CASE("yaml_roundtrip_mixed_struct", "[yaml][serialize][roundtrip]")
{
  MixedStruct original;
  original.name = "Alice";
  original.age = 28;
  original.active = true;
  original.hobbies = {"reading", "gaming", "cooking"};

  std::string yaml = STFY::serializeStructYaml(original);

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml.c_str(), yaml.size());

  MixedStruct parsed;
  context.parseTo(parsed);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(parsed.name == original.name);
  REQUIRE(parsed.age == original.age);
  REQUIRE(parsed.active == original.active);
  REQUIRE(parsed.hobbies == original.hobbies);
}

TEST_CASE("yaml_roundtrip_complex_k8s_like", "[yaml][serialize][roundtrip]")
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

  std::string yaml = STFY::serializeStructYaml(original);

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml.c_str(), yaml.size());

  PodConfig parsed;
  context.parseTo(parsed);

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

TEST_CASE("yaml_roundtrip_negative_numbers", "[yaml][serialize][roundtrip]")
{
  SimpleStruct original;
  original.name = "neg";
  original.age = -42;
  original.score = -3.14f;

  std::string yaml = STFY::serializeStructYaml(original);

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml.c_str(), yaml.size());

  SimpleStruct parsed;
  context.parseTo(parsed);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(parsed.name == original.name);
  REQUIRE(parsed.age == original.age);
  REQUIRE(parsed.score == Catch::Approx(original.score));
}

TEST_CASE("yaml_roundtrip_empty_vector", "[yaml][serialize][roundtrip]")
{
  EmptyVecStruct original;

  std::string yaml = STFY::serializeStructYaml(original);

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml.c_str(), yaml.size());

  EmptyVecStruct parsed;
  context.parseTo(parsed);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(parsed.items.empty());
}

TEST_CASE("yaml_roundtrip_string_vector", "[yaml][serialize][roundtrip]")
{
  WithStringVector original;
  original.items = {"apple", "banana", "cherry"};

  std::string yaml = STFY::serializeStructYaml(original);

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml.c_str(), yaml.size());

  WithStringVector parsed;
  context.parseTo(parsed);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(parsed.items == original.items);
}

TEST_CASE("yaml_roundtrip_inheritance", "[yaml][serialize][roundtrip]")
{
  Derived original;
  original.id = "base-123";
  original.extra = "derived-data";

  std::string yaml = STFY::serializeStructYaml(original);

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml.c_str(), yaml.size());

  Derived parsed;
  context.parseTo(parsed);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(parsed.id == original.id);
  REQUIRE(parsed.extra == original.extra);
}

TEST_CASE("yaml_roundtrip_vector_of_structs", "[yaml][serialize][roundtrip]")
{
  std::vector<Person> original = {{"John", 30}, {"Jane", 25}, {"Bob", 35}};

  PeopleWrapper wrapper;
  wrapper.people = original;

  std::string yaml = STFY::serializeStructYaml(wrapper);

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml.c_str(), yaml.size());

  PeopleWrapper parsed;
  context.parseTo(parsed);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(parsed.people.size() == 3);
  REQUIRE(parsed.people[0].name == "John");
  REQUIRE(parsed.people[0].age == 30);
  REQUIRE(parsed.people[1].name == "Jane");
  REQUIRE(parsed.people[1].age == 25);
  REQUIRE(parsed.people[2].name == "Bob");
  REQUIRE(parsed.people[2].age == 35);
}

// ── Parse-then-serialize-then-parse round-trip ──────────────────────

TEST_CASE("yaml_parse_serialize_reparse_simple", "[yaml][serialize][roundtrip]")
{
  const char yaml_input[] = R"(name: John
age: 30
score: 95.5
)";

  // Parse
  STFY::ParseContext ctx1;
  ctx1.tokenizer.allowYaml(true);
  ctx1.tokenizer.addData(yaml_input, sizeof(yaml_input) - 1);
  SimpleStruct s1;
  ctx1.parseTo(s1);
  REQUIRE(ctx1.error == STFY::Error::NoError);

  // Serialize
  std::string yaml_out = STFY::serializeStructYaml(s1);

  // Re-parse
  STFY::ParseContext ctx2;
  ctx2.tokenizer.allowYaml(true);
  ctx2.tokenizer.addData(yaml_out.c_str(), yaml_out.size());
  SimpleStruct s2;
  ctx2.parseTo(s2);
  REQUIRE(ctx2.error == STFY::Error::NoError);

  // Compare data
  REQUIRE(s2.name == s1.name);
  REQUIRE(s2.age == s1.age);
  REQUIRE(s2.score == Catch::Approx(s1.score));
}

TEST_CASE("yaml_parse_serialize_reparse_nested", "[yaml][serialize][roundtrip]")
{
  const char yaml_input[] = R"(name: Alice
address:
  street: 123 Main St
  city: Springfield
)";

  STFY::ParseContext ctx1;
  ctx1.tokenizer.allowYaml(true);
  ctx1.tokenizer.addData(yaml_input, sizeof(yaml_input) - 1);
  NestedOuter s1;
  ctx1.parseTo(s1);
  REQUIRE(ctx1.error == STFY::Error::NoError);

  std::string yaml_out = STFY::serializeStructYaml(s1);

  STFY::ParseContext ctx2;
  ctx2.tokenizer.allowYaml(true);
  ctx2.tokenizer.addData(yaml_out.c_str(), yaml_out.size());
  NestedOuter s2;
  ctx2.parseTo(s2);
  REQUIRE(ctx2.error == STFY::Error::NoError);

  REQUIRE(s2.name == s1.name);
  REQUIRE(s2.address.street == s1.address.street);
  REQUIRE(s2.address.city == s1.address.city);
}

TEST_CASE("yaml_parse_serialize_reparse_mixed", "[yaml][serialize][roundtrip]")
{
  const char yaml_input[] = R"(name: Alice
age: 28
active: true
hobbies:
  - reading
  - gaming
  - cooking
)";

  STFY::ParseContext ctx1;
  ctx1.tokenizer.allowYaml(true);
  ctx1.tokenizer.addData(yaml_input, sizeof(yaml_input) - 1);
  MixedStruct s1;
  ctx1.parseTo(s1);
  REQUIRE(ctx1.error == STFY::Error::NoError);

  std::string yaml_out = STFY::serializeStructYaml(s1);

  STFY::ParseContext ctx2;
  ctx2.tokenizer.allowYaml(true);
  ctx2.tokenizer.addData(yaml_out.c_str(), yaml_out.size());
  MixedStruct s2;
  ctx2.parseTo(s2);
  REQUIRE(ctx2.error == STFY::Error::NoError);

  REQUIRE(s2.name == s1.name);
  REQUIRE(s2.age == s1.age);
  REQUIRE(s2.active == s1.active);
  REQUIRE(s2.hobbies == s1.hobbies);
}

} // namespace yaml_serialize_test

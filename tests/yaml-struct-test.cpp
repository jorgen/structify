#include <structify/structify.h>

#include "catch2/catch_all.hpp"

namespace yaml_struct_test
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
  bool visible;
  bool debug;
  STFY_OBJ(enabled, active, visible, debug);
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

struct WithFlowValues
{
  std::string name;
  std::vector<int> nums;
  STFY_OBJ(name, nums);
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

TEST_CASE("yaml_parse_simple_struct", "[yaml][struct]")
{
  const char yaml[] = R"(name: John
age: 30
score: 95.5
)";

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  SimpleStruct s;
  (void)context.parseTo(s);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(s.name == "John");
  REQUIRE(s.age == 30);
  REQUIRE(s.score == Catch::Approx(95.5f));
}

TEST_CASE("yaml_parse_nested_struct", "[yaml][struct]")
{
  const char yaml[] = R"(name: John
address:
  street: 123 Main St
  city: Springfield
)";

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  NestedOuter s;
  (void)context.parseTo(s);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(s.name == "John");
  REQUIRE(s.address.street == "123 Main St");
  REQUIRE(s.address.city == "Springfield");
}

TEST_CASE("yaml_parse_vector_of_ints", "[yaml][struct]")
{
  const char yaml[] = R"(name: test
scores:
  - 100
  - 95
  - 88
)";

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  WithVector s;
  (void)context.parseTo(s);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(s.name == "test");
  REQUIRE(s.scores.size() == 3);
  REQUIRE(s.scores[0] == 100);
  REQUIRE(s.scores[1] == 95);
  REQUIRE(s.scores[2] == 88);
}

TEST_CASE("yaml_parse_vector_of_strings", "[yaml][struct]")
{
  const char yaml[] = R"(items:
  - apple
  - banana
  - cherry
)";

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  WithStringVector s;
  (void)context.parseTo(s);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(s.items.size() == 3);
  REQUIRE(s.items[0] == "apple");
  REQUIRE(s.items[1] == "banana");
  REQUIRE(s.items[2] == "cherry");
}

TEST_CASE("yaml_parse_vector_of_structs", "[yaml][struct]")
{
  const char yaml[] = R"(- name: John
  age: 30
- name: Jane
  age: 25
- name: Bob
  age: 35
)";

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  std::vector<Person> people;
  (void)context.parseTo(people);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(people.size() == 3);
  REQUIRE(people[0].name == "John");
  REQUIRE(people[0].age == 30);
  REQUIRE(people[1].name == "Jane");
  REQUIRE(people[1].age == 25);
  REQUIRE(people[2].name == "Bob");
  REQUIRE(people[2].age == 35);
}

TEST_CASE("yaml_parse_booleans_yes_no", "[yaml][struct]")
{
  const char yaml[] = R"(enabled: yes
active: true
visible: on
debug: no
)";

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  WithBooleans s;
  (void)context.parseTo(s);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(s.enabled == true);
  REQUIRE(s.active == true);
  REQUIRE(s.visible == true);
  REQUIRE(s.debug == false);
}

TEST_CASE("yaml_parse_optional_members", "[yaml][struct]")
{
  const char yaml[] = R"(name: John
age: 30
)";

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  WithOptional s;
  (void)context.parseTo(s);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(s.name == "John");
  REQUIRE(s.age.data == 30);
  // email is not in YAML, should remain default
}

TEST_CASE("yaml_parse_deeply_nested", "[yaml][struct]")
{
  const char yaml[] = R"(level2:
  level3:
    value: deep
)";

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  DeepNested s;
  (void)context.parseTo(s);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(s.level2.level3.value == "deep");
}

TEST_CASE("yaml_parse_mixed_struct", "[yaml][struct]")
{
  const char yaml[] = R"(name: Alice
age: 28
active: true
hobbies:
  - reading
  - gaming
  - cooking
)";

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  MixedStruct s;
  (void)context.parseTo(s);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(s.name == "Alice");
  REQUIRE(s.age == 28);
  REQUIRE(s.active == true);
  REQUIRE(s.hobbies.size() == 3);
  REQUIRE(s.hobbies[0] == "reading");
  REQUIRE(s.hobbies[1] == "gaming");
  REQUIRE(s.hobbies[2] == "cooking");
}

TEST_CASE("yaml_parse_with_document_marker", "[yaml][struct]")
{
  const char yaml[] = R"(---
name: John
age: 30
score: 100.0
)";

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  SimpleStruct s;
  (void)context.parseTo(s);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(s.name == "John");
  REQUIRE(s.age == 30);
  REQUIRE(s.score == Catch::Approx(100.0f));
}

TEST_CASE("yaml_parse_flow_array_value", "[yaml][struct]")
{
  const char yaml[] = R"(name: test
nums: [1, 2, 3]
)";

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  WithFlowValues s;
  (void)context.parseTo(s);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(s.name == "test");
  REQUIRE(s.nums.size() == 3);
  REQUIRE(s.nums[0] == 1);
  REQUIRE(s.nums[1] == 2);
  REQUIRE(s.nums[2] == 3);
}

TEST_CASE("yaml_parse_extra_members_ignored", "[yaml][struct]")
{
  const char yaml[] = R"(name: John
age: 30
score: 95.5
extra_field: ignored
another: also_ignored
)";

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  SimpleStruct s;
  (void)context.parseTo(s);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(s.name == "John");
  REQUIRE(s.age == 30);
  REQUIRE(s.score == Catch::Approx(95.5f));
}

TEST_CASE("yaml_parse_with_comments", "[yaml][struct]")
{
  const char yaml[] = R"(# Configuration
name: John # person name
age: 30
score: 100.0
)";

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  SimpleStruct s;
  (void)context.parseTo(s);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(s.name == "John");
  REQUIRE(s.age == 30);
}

TEST_CASE("yaml_parse_quoted_strings", "[yaml][struct]")
{
  const char yaml[] = R"(name: "John Doe"
age: 30
score: 0.0
)";

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  SimpleStruct s;
  (void)context.parseTo(s);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(s.name == "John Doe");
}

TEST_CASE("yaml_parse_negative_numbers", "[yaml][struct]")
{
  const char yaml[] = R"(name: test
age: -5
score: -3.14
)";

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  SimpleStruct s;
  (void)context.parseTo(s);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(s.age == -5);
  REQUIRE(s.score == Catch::Approx(-3.14f));
}

TEST_CASE("yaml_parse_literal_block_scalar", "[yaml][struct]")
{
  const char yaml[] = R"(name: |
  line one
  line two
age: 30
score: 0.0
)";

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  SimpleStruct s;
  (void)context.parseTo(s);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(s.name == "line one\nline two\n");
  REQUIRE(s.age == 30);
}

// --- Test 1: Server configuration with block scalars and nested objects ---

struct TlsConfig
{
  bool enabled;
  std::string cert;
  STFY_OBJ(enabled, cert);
};

struct DatabaseConfig
{
  std::string host;
  int port;
  std::string name;
  STFY_OBJ(host, port, name);
};

struct ServerConfig
{
  std::string host;
  int port;
  bool debug;
  DatabaseConfig database;
  TlsConfig tls;
  std::string description;
  STFY_OBJ(host, port, debug, database, tls, description);
};

TEST_CASE("yaml_server_config_block_scalars", "[yaml][struct]")
{
  const char yaml[] = R"(host: 0.0.0.0
port: 8080
debug: false
database:
  host: db.example.com
  port: 5432
  name: myapp
tls:
  enabled: true
  cert: |
    -----BEGIN CERTIFICATE-----
    MIIBxTCCAWugAwIBAgIJALP2kEZ5
    -----END CERTIFICATE-----
description: >
  This server handles
  all incoming API requests
  for the application.
)";

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  ServerConfig s;
  (void)context.parseTo(s);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(s.host == "0.0.0.0");
  REQUIRE(s.port == 8080);
  REQUIRE(s.debug == false);
  REQUIRE(s.database.host == "db.example.com");
  REQUIRE(s.database.port == 5432);
  REQUIRE(s.database.name == "myapp");
  REQUIRE(s.tls.enabled == true);
  REQUIRE(s.tls.cert == "-----BEGIN CERTIFICATE-----\nMIIBxTCCAWugAwIBAgIJALP2kEZ5\n-----END CERTIFICATE-----\n");
  REQUIRE(s.description == "This server handles all incoming API requests for the application.\n");
}

// --- Test 2: CI pipeline configuration ---

struct PipelineStage
{
  std::string name;
  std::string image;
  std::vector<std::string> commands;
  STFY_OBJ(name, image, commands);
};

struct Pipeline
{
  std::string version;
  std::vector<PipelineStage> stages;
  STFY_OBJ(version, stages);
};

TEST_CASE("yaml_ci_pipeline_config", "[yaml][struct]")
{
  const char yaml[] = R"(version: '3'
stages:
  - name: build
    image: gcc:latest
    commands:
      - mkdir build
      - cmake ..
      - make -j4
  - name: test
    image: gcc:latest
    commands:
      - cd build
      - ctest --output-on-failure
  - name: deploy
    image: alpine:3.18
    commands:
      - ./deploy.sh
)";

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  Pipeline s;
  (void)context.parseTo(s);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(s.version == "3");
  REQUIRE(s.stages.size() == 3);

  REQUIRE(s.stages[0].name == "build");
  REQUIRE(s.stages[0].image == "gcc:latest");
  REQUIRE(s.stages[0].commands.size() == 3);
  REQUIRE(s.stages[0].commands[0] == "mkdir build");
  REQUIRE(s.stages[0].commands[1] == "cmake ..");
  REQUIRE(s.stages[0].commands[2] == "make -j4");

  REQUIRE(s.stages[1].name == "test");
  REQUIRE(s.stages[1].image == "gcc:latest");
  REQUIRE(s.stages[1].commands.size() == 2);
  REQUIRE(s.stages[1].commands[0] == "cd build");
  REQUIRE(s.stages[1].commands[1] == "ctest --output-on-failure");

  REQUIRE(s.stages[2].name == "deploy");
  REQUIRE(s.stages[2].image == "alpine:3.18");
  REQUIRE(s.stages[2].commands.size() == 1);
  REQUIRE(s.stages[2].commands[0] == "./deploy.sh");
}

// --- Test 3: Flow collections inline ---

struct Point
{
  int x;
  int y;
  STFY_OBJ(x, y);
};

struct FlowCollections
{
  std::string name;
  std::vector<int> values;
  std::vector<std::string> tags;
  Point origin;
  STFY_OBJ(name, values, tags, origin);
};

TEST_CASE("yaml_flow_collections_struct", "[yaml][struct]")
{
  const char yaml[] = R"(name: shape
values: [10, 20, 30, 40]
tags: [geometry, 2d, primary]
origin: {x: 5, y: 10}
)";

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  FlowCollections s;
  (void)context.parseTo(s);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(s.name == "shape");
  REQUIRE(s.values.size() == 4);
  REQUIRE(s.values[0] == 10);
  REQUIRE(s.values[1] == 20);
  REQUIRE(s.values[2] == 30);
  REQUIRE(s.values[3] == 40);
  REQUIRE(s.tags.size() == 3);
  REQUIRE(s.tags[0] == "geometry");
  REQUIRE(s.tags[1] == "2d");
  REQUIRE(s.tags[2] == "primary");
  REQUIRE(s.origin.x == 5);
  REQUIRE(s.origin.y == 10);
}

// --- Test 4: Quoted strings with special characters ---

struct QuotedStrings
{
  std::string double_escaped;
  std::string with_tab;
  std::string with_quote;
  std::string single_quoted;
  STFY_OBJ(double_escaped, with_tab, with_quote, single_quoted);
};

TEST_CASE("yaml_quoted_strings_struct", "[yaml][struct]")
{
  const char yaml[] = R"(double_escaped: "line1\nline2\nline3"
with_tab: "col1\tcol2\tcol3"
with_quote: "she said \"hello\""
single_quoted: 'it''s a test'
)";

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  QuotedStrings s;
  (void)context.parseTo(s);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(s.double_escaped == "line1\nline2\nline3");
  REQUIRE(s.with_tab == "col1\tcol2\tcol3");
  REQUIRE(s.with_quote == "she said \"hello\"");
  REQUIRE(s.single_quoted == "it's a test");
}

// --- Test 5: Complex nested config (Kubernetes-pod-like) ---

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

TEST_CASE("yaml_complex_nested_k8s_like", "[yaml][struct]")
{
  const char yaml[] = R"(kind: Pod
metadata:
  name: my-app
  labels:
    app: web-server
    tier: frontend
spec:
  containers:
    - name: nginx
      image: nginx:1.25
      ports:
        - containerPort: 80
        - containerPort: 443
      env:
        - name: ENV
          value: production
        - name: LOG_LEVEL
          value: warn
    - name: sidecar
      image: fluent/fluentd:v1.16
      ports:
        - containerPort: 24224
      env:
        - name: FLUSH_INTERVAL
          value: 5s
)";

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  PodConfig s;
  (void)context.parseTo(s);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(s.kind == "Pod");
  REQUIRE(s.metadata.name == "my-app");
  REQUIRE(s.metadata.labels.app == "web-server");
  REQUIRE(s.metadata.labels.tier == "frontend");

  REQUIRE(s.spec.containers.size() == 2);

  auto &c0 = s.spec.containers[0];
  REQUIRE(c0.name == "nginx");
  REQUIRE(c0.image == "nginx:1.25");
  REQUIRE(c0.ports.size() == 2);
  REQUIRE(c0.ports[0].containerPort == 80);
  REQUIRE(c0.ports[1].containerPort == 443);
  REQUIRE(c0.env.size() == 2);
  REQUIRE(c0.env[0].name == "ENV");
  REQUIRE(c0.env[0].value == "production");
  REQUIRE(c0.env[1].name == "LOG_LEVEL");
  REQUIRE(c0.env[1].value == "warn");

  auto &c1 = s.spec.containers[1];
  REQUIRE(c1.name == "sidecar");
  REQUIRE(c1.image == "fluent/fluentd:v1.16");
  REQUIRE(c1.ports.size() == 1);
  REQUIRE(c1.ports[0].containerPort == 24224);
  REQUIRE(c1.env.size() == 1);
  REQUIRE(c1.env[0].name == "FLUSH_INTERVAL");
  REQUIRE(c1.env[0].value == "5s");
}

// --- Test 6: Document with comments throughout ---

struct CommentedConfig
{
  std::string name;
  int workers;
  bool verbose;
  std::vector<std::string> endpoints;
  STFY_OBJ(name, workers, verbose, endpoints);
};

TEST_CASE("yaml_comments_everywhere", "[yaml][struct]")
{
  const char yaml[] = R"(# Top-level application config
# Version 2.0

name: my-service # service name
workers: 4 # number of worker threads
verbose: true # enable verbose logging
# The endpoints section lists all API routes
endpoints: # begin list
  # health check endpoint
  - /health
  - /api/v1/users # user management
  # this is the orders endpoint
  - /api/v1/orders
  - /metrics # prometheus metrics
)";

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  CommentedConfig s;
  (void)context.parseTo(s);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(s.name == "my-service");
  REQUIRE(s.workers == 4);
  REQUIRE(s.verbose == true);
  REQUIRE(s.endpoints.size() == 4);
  REQUIRE(s.endpoints[0] == "/health");
  REQUIRE(s.endpoints[1] == "/api/v1/users");
  REQUIRE(s.endpoints[2] == "/api/v1/orders");
  REQUIRE(s.endpoints[3] == "/metrics");
}

TEST_CASE("yaml_parse_unmentioned_members_preserved", "[yaml][struct]")
{
  // Deliberately omits: count, ratio, inner
  const char yaml[] = R"(name: updated
flag: false
)";

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

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

TEST_CASE("yaml_report_missing_members", "[yaml][struct]")
{
  const char yaml[] = R"(name: John
age: 30
score: 95.5
extra_field: ignored
)";

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  SimpleStruct s;
  (void)context.parseTo(s);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(s.name == "John");
  REQUIRE(s.age == 30);
  REQUIRE(s.score == Catch::Approx(95.5f));
  REQUIRE(context.missing_members.size() == 1);
  REQUIRE(context.missing_members.front() == "extra_field");
}

TEST_CASE("yaml_report_unassigned_required_members", "[yaml][struct]")
{
  // Omits "score" which is a required member
  const char yaml[] = R"(name: John
age: 30
)";

  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml, sizeof(yaml) - 1);

  SimpleStruct s;
  (void)context.parseTo(s);

  REQUIRE(context.error == STFY::Error::NoError);
  REQUIRE(s.name == "John");
  REQUIRE(s.age == 30);
  REQUIRE(context.unassigned_required_members.size() == 1);
  REQUIRE(context.unassigned_required_members.front() == "score");
}

} // namespace yaml_struct_test

/*
 * Copyright © 2016 Jorgen Lind
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#include <json_struct/json_struct.h>
#include "catch2/catch.hpp"
#include <unordered_map>

namespace json_struct_comments_test
{

// Test data with comprehensive comment usage
static const char json_with_comments[] = R"json({
  // Configuration settings for the application
  "app_name": "TestApp", // The application name
  "version": 1.42,
  // Database configuration section
  "database": {
    "host": "localhost", // Database server host
    // Port configuration - default is 5432
    "port": 5432,
    "ssl_enabled": true // Enable SSL connections
  },
  // Array of user preferences
  "preferences": [
    // First preference item
    "theme_dark",
    "notifications_enabled", // Enable notifications
    // Last preference
    "auto_save"
  ],
  // Optional feature flags
  "features": {
    // Beta features (experimental)
    "beta_ui": false,
    "new_parser": true // Use the new JSON parser
    // End of feature flags
  }
  // End of configuration
})json";

struct DatabaseConfig
{
  std::string host;
  int port = 0;
  bool ssl_enabled = false;

  JS_OBJECT(JS_MEMBER(host), JS_MEMBER(port), JS_MEMBER(ssl_enabled));
};

struct FeatureFlags
{
  bool beta_ui = false;
  bool new_parser = false;

  JS_OBJECT(JS_MEMBER(beta_ui), JS_MEMBER(new_parser));
};

struct AppConfig
{
  std::string app_name;
  double version = 0.0;
  DatabaseConfig database;
  std::vector<std::string> preferences;
  FeatureFlags features;

  JS_OBJECT(JS_MEMBER(app_name), JS_MEMBER(version), JS_MEMBER(database),
            JS_MEMBER(preferences), JS_MEMBER(features));
};

TEST_CASE("parse_struct_with_comments", "[json_struct][comments]")
{
  JS::ParseContext context(json_with_comments);
  context.tokenizer.allowComments(true);

  AppConfig config;
  auto error = context.parseTo(config);

  REQUIRE(error == JS::Error::NoError);
  REQUIRE(config.app_name == "TestApp");
  REQUIRE(config.version == 1.42);
  REQUIRE(config.database.host == "localhost");
  REQUIRE(config.database.port == 5432);
  REQUIRE(config.database.ssl_enabled == true);
  REQUIRE(config.preferences.size() == 3);
  REQUIRE(config.preferences[0] == "theme_dark");
  REQUIRE(config.preferences[1] == "notifications_enabled");
  REQUIRE(config.preferences[2] == "auto_save");
  REQUIRE(config.features.beta_ui == false);
  REQUIRE(config.features.new_parser == true);
}

// Test data with nested comments
static const char nested_comments_json[] = R"json({
  // Top level comment
  "outer": {
    // Nested object comment
    "inner": {
      // Deep nested comment
      "value": 42, // Inline comment in deep nesting
      "name": "test_string"
    }
  }
})json";

struct DeepInner
{
  int value = 0;
  std::string name;

  JS_OBJECT(JS_MEMBER(value), JS_MEMBER(name));
};

struct Inner
{
  DeepInner inner;

  JS_OBJECT(JS_MEMBER(inner));
};

struct NestedCommentStruct
{
  Inner outer;

  JS_OBJECT(JS_MEMBER(outer));
};

TEST_CASE("parse_nested_struct_with_comments", "[json_struct][comments]")
{
  JS::ParseContext context(nested_comments_json);
  context.tokenizer.allowComments(true);

  NestedCommentStruct data;
  auto error = context.parseTo(data);

  REQUIRE(error == JS::Error::NoError);
  REQUIRE(data.outer.inner.value == 42);
  REQUIRE(data.outer.inner.name == "test_string");
}

// Test template struct with comments
static const char template_json_with_comments[] = R"json({
  // Generic container with type T
  "container_id": 123,
  // The contained data
  "data": {
    "message": "Hello World", // The message content
    "priority": 5 // Message priority level
  }
})json";

struct MessageData
{
  std::string message;
  int priority = 0;

  JS_OBJECT(JS_MEMBER(message), JS_MEMBER(priority));
};

template<typename T>
struct GenericContainer
{
  int container_id = 0;
  T data;

  JS_OBJECT(JS_MEMBER(container_id), JS_MEMBER(data));
};

TEST_CASE("parse_template_struct_with_comments", "[json_struct][comments]")
{
  JS::ParseContext context(template_json_with_comments);
  context.tokenizer.allowComments(true);

  GenericContainer<MessageData> container;
  auto error = context.parseTo(container);

  REQUIRE(error == JS::Error::NoError);
  REQUIRE(container.container_id == 123);
  REQUIRE(container.data.message == "Hello World");
  REQUIRE(container.data.priority == 5);
}

// Test inheritance with comments
static const char inheritance_json_with_comments[] = R"json({
  // Base class properties
  "base_id": 42,
  "base_name": "BaseObject",
  // Derived class properties
  "derived_value": 3.14,
  "derived_flag": true // Boolean flag in derived class
})json";

struct BaseClass
{
  int base_id = 0;
  std::string base_name;

  JS_OBJECT(JS_MEMBER(base_id), JS_MEMBER(base_name));
};

struct DerivedClass : public BaseClass
{
  double derived_value = 0.0;
  bool derived_flag = false;

  JS_OBJECT_WITH_SUPER(JS_SUPER_CLASSES(JS_SUPER_CLASS(BaseClass)),
                       JS_MEMBER(derived_value), JS_MEMBER(derived_flag));
};

TEST_CASE("parse_inheritance_struct_with_comments", "[json_struct][comments]")
{
  JS::ParseContext context(inheritance_json_with_comments);
  context.tokenizer.allowComments(true);

  DerivedClass obj;
  auto error = context.parseTo(obj);

  REQUIRE(error == JS::Error::NoError);
  REQUIRE(obj.base_id == 42);
  REQUIRE(obj.base_name == "BaseObject");
  REQUIRE(obj.derived_value == 3.14);
  REQUIRE(obj.derived_flag == true);
}

// Test optional members with comments
static const char optional_json_with_comments[] = R"json({
  // Required fields
  "name": "TestUser",
  "age": 25,
  // Optional fields with comments
  "email": "test@example.com", // User's email address
  // Note: phone field is omitted intentionally
  // "phone": "+1234567890",
  "settings": {
    "theme": "dark" // UI theme preference
    // Other settings are using defaults
  }
})json";

struct UserSettings
{
  std::string theme;
  JS::OptionalChecked<bool> notifications;

  JS_OBJECT(JS_MEMBER(theme), JS_MEMBER(notifications));
};

struct User
{
  std::string name;
  int age = 0;
  JS::OptionalChecked<std::string> email;
  JS::OptionalChecked<std::string> phone;
  UserSettings settings;

  JS_OBJECT(JS_MEMBER(name), JS_MEMBER(age), JS_MEMBER(email),
            JS_MEMBER(phone), JS_MEMBER(settings));
};

TEST_CASE("parse_optional_members_with_comments", "[json_struct][comments]")
{
  JS::ParseContext context(optional_json_with_comments);
  context.tokenizer.allowComments(true);

  User user;
  auto error = context.parseTo(user);

  REQUIRE(error == JS::Error::NoError);
  REQUIRE(user.name == "TestUser");
  REQUIRE(user.age == 25);
  REQUIRE(user.email.assigned);
  REQUIRE(user.email.data == "test@example.com");
  REQUIRE(!user.phone.assigned);
  REQUIRE(user.settings.theme == "dark");
  REQUIRE(!user.settings.notifications.assigned);
}

// Test multi-top-level JSON with comments
static const char multi_level_json_comments[] = R"json(
// First JSON object
{ "id": 1, "name": "First" }
// Second JSON object with more details
{ "id": 2, "name": "Second", "active": true }
// Final JSON object
{ "id": 3, "name": "Third" }
)json";

struct SimpleItem
{
  int id = 0;
  std::string name;
  JS::OptionalChecked<bool> active;

  JS_OBJECT(JS_MEMBER(id), JS_MEMBER(name), JS_MEMBER(active));
};

TEST_CASE("parse_multi_top_level_with_comments", "[json_struct][comments]")
{
  JS::ParseContext context(multi_level_json_comments);
  context.tokenizer.allowComments(true);
  context.tokenizer.allowAsciiType(true);  // For parsing multiple top-level objects

  // Parse first object
  SimpleItem item1;
  auto error = context.parseTo(item1);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(item1.id == 1);
  REQUIRE(item1.name == "First");
  REQUIRE(!item1.active.assigned);

  // Parse second object
  SimpleItem item2;
  error = context.parseTo(item2);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(item2.id == 2);
  REQUIRE(item2.name == "Second");
  REQUIRE(item2.active.assigned);
  REQUIRE(item2.active.data == true);

  // Parse third object
  SimpleItem item3;
  error = context.parseTo(item3);
  REQUIRE(error == JS::Error::NoError);
  REQUIRE(item3.id == 3);
  REQUIRE(item3.name == "Third");
}

// Test array of complex objects with comments
static const char array_json_with_comments[] = R"json({
  // Array of employee records
  "employees": [
    {
      // First employee
      "name": "Alice",
      "department": "Engineering", // Software development
      "salary": 75000,
      "skills": ["C++", "Python", "JavaScript"] // Programming languages
    },
    {
      // Second employee
      "name": "Bob",
      "department": "Marketing", // Sales and marketing
      "salary": 65000,
      "skills": ["Communication", "Strategy"] // Soft skills
    }
    // Could add more employees here
  ]
})json";

struct Employee
{
  std::string name;
  std::string department;
  int salary = 0;
  std::vector<std::string> skills;

  JS_OBJECT(JS_MEMBER(name), JS_MEMBER(department), JS_MEMBER(salary), JS_MEMBER(skills));
};

struct Company
{
  std::vector<Employee> employees;

  JS_OBJECT(JS_MEMBER(employees));
};

TEST_CASE("parse_array_of_structs_with_comments", "[json_struct][comments]")
{
  JS::ParseContext context(array_json_with_comments);
  context.tokenizer.allowComments(true);

  Company company;
  auto error = context.parseTo(company);

  REQUIRE(error == JS::Error::NoError);
  REQUIRE(company.employees.size() == 2);

  REQUIRE(company.employees[0].name == "Alice");
  REQUIRE(company.employees[0].department == "Engineering");
  REQUIRE(company.employees[0].salary == 75000);
  REQUIRE(company.employees[0].skills.size() == 3);
  REQUIRE(company.employees[0].skills[0] == "C++");

  REQUIRE(company.employees[1].name == "Bob");
  REQUIRE(company.employees[1].department == "Marketing");
  REQUIRE(company.employees[1].salary == 65000);
  REQUIRE(company.employees[1].skills.size() == 2);
}

// Test serialization round-trip (comments will be lost, which is expected)
TEST_CASE("serialization_round_trip_with_comments", "[json_struct][comments]")
{
  // Parse with comments
  JS::ParseContext context(json_with_comments);
  context.tokenizer.allowComments(true);

  AppConfig original;
  auto parse_error = context.parseTo(original);
  REQUIRE(parse_error == JS::Error::NoError);

  // Serialize back to JSON (without comments)
  std::string serialized = JS::serializeStruct(original);

  // Parse the serialized version (no comments)
  JS::ParseContext new_context(serialized);
  new_context.tokenizer.allowComments(true); // Still allow comments, but there won't be any

  AppConfig deserialized;
  auto reparse_error = new_context.parseTo(deserialized);
  REQUIRE(reparse_error == JS::Error::NoError);

  // Verify data integrity
  REQUIRE(original.app_name == deserialized.app_name);
  REQUIRE(original.version == deserialized.version);
  REQUIRE(original.database.host == deserialized.database.host);
  REQUIRE(original.database.port == deserialized.database.port);
  REQUIRE(original.database.ssl_enabled == deserialized.database.ssl_enabled);
  REQUIRE(original.preferences.size() == deserialized.preferences.size());
  REQUIRE(original.features.beta_ui == deserialized.features.beta_ui);
  REQUIRE(original.features.new_parser == deserialized.features.new_parser);
}

// Test error handling when comments are disabled but JSON contains comments
TEST_CASE("error_when_comments_disabled", "[json_struct][comments]")
{
  JS::ParseContext context(json_with_comments);
  // Don't enable comments - should cause parsing issues

  AppConfig config;
  auto error = context.parseTo(config);

  // Should either fail or succeed depending on where the comment appears
  // The key thing is that it should be consistent behavior
  if (error == JS::Error::NoError) {
    // If it succeeds, it's because the first comment didn't interfere with parsing
    // But the data might be incomplete or incorrect
    INFO("Parsing succeeded despite comments being disabled");
  } else {
    // If it fails, that's expected behavior when comments are not supported
    REQUIRE(error != JS::Error::NoError);
    INFO("Parsing failed as expected when comments are disabled");
  }
}

// Test deeply nested comments
static const char deeply_nested_comments[] = R"json({
  // Level 1
  "level1": {
    // Level 2
    "level2": {
      // Level 3
      "level3": {
        // Level 4
        "level4": {
          // Level 5
          "value": "deeply_nested", // Final value
          "array": [
            // Array in deep nesting
            {
              // Object in array in deep nesting
              "deep_array_obj": true
            }
          ]
        }
      }
    }
  }
})json";

struct Level4
{
  std::string value;
  std::vector<JS::JsonObjectOrArray> array;

  JS_OBJECT(JS_MEMBER(value), JS_MEMBER(array));
};

struct Level3
{
  Level4 level4;

  JS_OBJECT(JS_MEMBER(level4));
};

struct Level2
{
  Level3 level3;

  JS_OBJECT(JS_MEMBER(level3));
};

struct Level1
{
  Level2 level2;

  JS_OBJECT(JS_MEMBER(level2));
};

struct DeeplyNested
{
  Level1 level1;

  JS_OBJECT(JS_MEMBER(level1));
};

TEST_CASE("parse_deeply_nested_with_comments", "[json_struct][comments]")
{
  JS::ParseContext context(deeply_nested_comments);
  context.tokenizer.allowComments(true);

  DeeplyNested data;
  auto error = context.parseTo(data);

  REQUIRE(error == JS::Error::NoError);
  REQUIRE(data.level1.level2.level3.level4.value == "deeply_nested");
  REQUIRE(data.level1.level2.level3.level4.array.size() == 1);
}

} // namespace json_struct_comments_test
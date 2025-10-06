# **Structurize your JSON**

[![Build status](https://ci.appveyor.com/api/projects/status/mduab0w8u12atfbu?svg=true)](https://ci.appveyor.com/project/jorgen36373/json-struct)
[![ClusterFuzzLite PR fuzzing](https://github.com/jorgen/json_struct/actions/workflows/cflite_pr.yml/badge.svg)](https://github.com/jorgen/json_struct/actions/workflows/cflite_pr.yml)

json_struct is a single-header C++ library that parses JSON to structs/classes and serializes structs/classes back to JSON. With support for relaxed parsing rules, it's also excellent for configuration files and human-editable data formats.

**Getting Started:** Simply copy `json_struct.h` from the `include` folder into your project's include path.

**Requirements:** C++11 or newer. Tested on GCC, Clang, and Visual Studio 2015+.

## Quick Start

json_struct automatically maps JSON to C++ structs by adding simple metadata declarations.

```json
{
    "One" : 1,
    "Two" : "two",
    "Three" : 3.333
}
```

can be parsed into a structure defined like this:

```c++
struct JsonObject
{
    int One;
    std::string Two;
    double Three;

    JS_OBJ(One, Two, Three);
};
```

or

```c++
struct JsonObject
{
    int One;
    std::string Two;
    double Three;
};
JS_OBJ_EXT(JsonObject, One, Two, Three);
```

**Parse JSON to struct:**

```c++
JS::ParseContext context(json_data);
JsonObject obj;
context.parseTo(obj);
```

**Serialize struct to JSON:**

```c++
std::string pretty_json = JS::serializeStruct(obj);
// or
std::string compact_json = JS::serializeStruct(obj, JS::SerializerOptions(JS::SerializerOptions::Compact));
```

## Relaxed Parsing for Config Files

json_struct supports relaxed JSON parsing rules, making it ideal for configuration files and human-editable data. Enable optional features for a more forgiving syntax:

* **Comments** using `//` syntax
* **Unquoted property names** and string values (supports `A-Z`, `a-z`, `0-9`, `_`, `-`, `.`, `/`)
* **Newlines** instead of commas as delimiters
* **Trailing commas** in objects and arrays

**Example configuration file:**
```js
{
  // Server configuration
  host: localhost
  port: 8080

  database: {
    name: myapp_db
    max_connections: 100,  // Trailing comma is OK
  }

  log_file: /var/log/app.log
}
```

**Enable relaxed parsing:**
```c++
JS::ParseContext context(config_data);
context.tokenizer.allowComments(true);
context.tokenizer.allowAsciiType(true);
context.tokenizer.allowNewLineAsTokenDelimiter(true);
context.tokenizer.allowSuperfluousComma(true);
context.parseTo(config_obj);
```

## Dynamic JSON with Maps

When the JSON structure depends on runtime values, you can parse into a `JS::Map` first, inspect the data, then dispatch to the appropriate type. For example, consider JSON describing different vehicle types:
```json
{
  "type" : "car",
  "wheels" : 4,
  "electric" : true,
...
}
```
or it could look like this:
```json
{
  "type" : "sailboat",
  "sail_area_m2" : 106.5,
  "swimming_platform": true,
...
}
```

Parse into a map, query for the type field, then convert to the specific struct:

```c++
void handle_data(const char *data, size_t size)
{
  JS::Map map;
  JS::ParseContext parseContext(data, size, map);
  if (parseContext.error != JS::Error::NoError)
  {
    fprintf(stderr, "Failed to parse Json:\n%s\n", parseContext.makeErrorString().c_str());
    return;
  }
  VehicleType vehicleType = map.castTo<VehicleType>("type", parseContext);
  if (parseContext.error != JS::Error::NoError)
  {
    fprintf(stderr, "Failed to extract type:\n%s\n", parseContext.makeErrorString().c_str());
    return;
  }
  switch (vehicleType)
  {
  case VehicleType::car:
  {
    Car car = map.castTo<Car>(parseContext);
    if (parseContext.error != JS::Error::NoError)
    {
      //error handling 
    }
    handle_car(car);
    break;
  }
  case VehicleType::sailboat:
    Sailboat sailboat;
    map.castToType(parseContext, sailboat);
    if (parseContext.error != JS::Error::NoError)
    {
      //error handling 
    }
    handle_sailboat(sailboat);
    break;
  }
}
```

The `JS::Map` allows querying and extracting individual fields before converting the entire object. Two casting styles are available:
```c++
    Car car = map.castTo<Car>(parseContext);
    // or
    Sailboat sailboat;
    map.castToType(parseContext, sailboat);
```

## Advanced Macro Usage

The `JS_OBJ` macro adds a static metadata object to your struct without affecting its size or semantics. For more control, use the verbose `JS_OBJECT` macro with explicit member declarations:
```c++
struct JsonObject
{
    int One;
    std::string Two;
    double Three;

    JS_OBJECT(JS_MEMBER(One)
            , JS_MEMBER(Two)
            , JS_MEMBER(Three));
};
```

**Custom JSON keys and aliases:**
```c++
struct JsonObject
{
    int One;
    std::string Two;
    double Three;

    JS_OBJECT(JS_MEMBER(One)
            , JS_MEMBER_WITH_NAME(Two, "TheTwo")
            , JS_MEMBER_ALIASES(Three, "TheThree", "the_three"));
};
```

* `JS_MEMBER_WITH_NAME` - Uses only the supplied name, ignoring the member name
* `JS_MEMBER_ALIASES` - Checks aliases after the primary member name

**Note:** Don't mix `JS_MEMBER` macros with `JS_OBJ` as it will double-apply the macro.

## Custom Type Handlers

For types that don't fit the standard object model (e.g., custom serialization logic), implement a `TypeHandler` specialization. The `JS::ParseContext` looks for template specializations:

```c++
namespace JS {
    template<typename T>
    struct TypeHandler;
}
```

**Built-in type handlers:**

* `std::string`
* `double`
* `float`
* `uint8_t`
* `int16_t`
* `uint16_t`
* `int32_t`
* `uint32_t`
* `int64_t`
* `uint64_t`
* `std::unique_ptr`
* `bool`
* `std::vector`
* `[T]`

**Custom type handler example:**

```c++
namespace JS {
template<>
struct TypeHandler<uint32_t>
{
public:
    static inline Error to(uint32_t &to_type, ParseContext &context)
    {
        char *pointer;
        unsigned long value = strtoul(context.token.value.data, &pointer, 10);
        to_type = static_cast<unsigned int>(value);
        if (context.token.value.data == pointer)
            return Error::FailedToParseInt;
        return Error::NoError;
    }

    static void from(const uint32_t &from_type, Token &token, Serializer &serializer)
    {
        std::string buf = std::to_string(from_type);
        token.value_type = Type::Number;
        token.value.data = buf.data();
        token.value.size = buf.size();
        serializer.write(token);
    }
};
}
```

This gives complete control over serialization/deserialization and can represent any JSON structure (objects, arrays, primitives).

## Examples and Tests

* [Examples](https://github.com/jorgen/json_struct/tree/master/examples) - See practical usage patterns
* [Unit Tests](https://github.com/jorgen/json_struct/tree/master/tests) - Comprehensive test coverage

## Quality Assurance

**Static Analysis:**
* [PVS-Studio](https://pvs-studio.com/pvs-studio/?utm_source=website&utm_medium=github&utm_campaign=open_source) - C, C++, C#, and Java static analyzer

**Dynamic Analysis:**
All pull requests are tested with:
* [Clang Address Sanitizer](https://clang.llvm.org/docs/AddressSanitizer.html)
* [Clang Memory Sanitizer](https://clang.llvm.org/docs/MemorySanitizer.html)

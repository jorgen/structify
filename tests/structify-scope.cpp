#include "catch2/catch_all.hpp"
#include <structify/structify.h>
#include <stdio.h>

namespace
{

const char json[] = R"json({
  "func1": {
    "arg1": "hello",
    "arg2": "world"
  },
  "func2": {
    "one": [ 1, 2, 3, 4 ],
    "two": true
  },
  "func3": {
    "first":  {
      "advanced": true
    },
    "second": false
  }
})json";

struct Func1Arg
{
  int arg1;
  std::string arg2;
  STFY_OBJECT(STFY_MEMBER(arg1), STFY_MEMBER(arg2));
};

struct Func2Arg
{
  int one[4];
  bool two;
  STFY_OBJECT(STFY_MEMBER(one), STFY_MEMBER(two));
};

struct Func3Arg
{
  int first;
  double second;
  STFY_OBJECT(STFY_MEMBER(first), STFY_MEMBER(second));
};

struct FunctionCont
{
  void func1(const Func1Arg &)
  {
    func1_called = true;
  }

  void func2(const Func2Arg &arg)
  {
    STFY_UNUSED(arg);
    func2_called = true;
  }

  void func3(const Func3Arg &arg)
  {
    STFY_UNUSED(arg);
    func3_called = true;
  }

  bool func1_called = false;
  bool func2_called = false;
  bool func3_called = false;
  STFY_FUNCTION_CONTAINER(STFY_FUNCTION(func1), STFY_FUNCTION(func2), STFY_FUNCTION(func3));
};

TEST_CASE("json_function_scope_test", "[function]")
{
  FunctionCont cont;
  std::string json_out;
  STFY::DefaultCallFunctionContext context(json, json_out);
  context.callFunctions(cont);
  REQUIRE(context.error_context.getLatestError() == STFY::Error::NoError);
}
} // namespace

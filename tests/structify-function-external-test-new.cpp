/*
 * Copyright © 2020 Jorgen Lind
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

#include <structify/structify.h>

#include "catch2/catch_all.hpp"

namespace
{

const char json[] = "{"
                    "  \"execute_one\": {\n"
                    "    \"number\": 45,\n"
                    "    \"valid\": \"false\"\n"
                    "  },"
                    "  \"execute_two\": 99,\n"
                    "  \"execute_three\": [\n"
                    "    4,\n"
                    "    6,\n"
                    "    8\n"
                    "  ]\n"
                    "}\n";

struct SimpleData
{
  float number;
  bool valid;

  STFY_OBJECT(STFY_MEMBER(number), STFY_MEMBER(valid));
};
struct CallFunction
{
  virtual void execute_one(const SimpleData &data)
  {
    STFY_UNUSED(data);
    called_one = true;
  }

  int execute_two(const double &data, STFY::CallFunctionContext &context)
  {
    STFY_UNUSED(data);
    STFY_UNUSED(context);
    called_two = true;
    return 2;
  }

  void execute_three(const std::vector<double> &data, STFY::CallFunctionContext &context)
  {
    STFY_UNUSED(context);
    for (auto x : data)
      STFY_UNUSED(x);
    called_three = true;
  }
  bool called_one = false;
  bool called_two = false;
  bool called_three = false;
};
} // namespace

STFY_FUNC_OBJ_EXTERNAL(CallFunction, execute_one, execute_two, execute_three)

namespace
{
TEST_CASE("simpleFunctionTestExternalShort", "[function]")
{
  CallFunction cont;
  std::string json_out;
  STFY::DefaultCallFunctionContext context(json, sizeof(json), json_out);
  context.callFunctions(cont);

  REQUIRE(cont.called_one);
  REQUIRE(cont.called_two);
  REQUIRE(cont.called_three);

  if (context.parse_context.error != STFY::Error::NoError)
  REQUIRE(context.parse_context.error == STFY::Error::NoError);
}

struct CallFunctionSuperSuper
{
  void execute_one(const SimpleData &data)
  {
    STFY_UNUSED(data);
    called_one = true;
  }

  bool called_one = false;
};
} // namespace
STFY_FUNC_OBJ_EXTERNAL(CallFunctionSuperSuper, execute_one)

namespace
{
struct CallFunctionSuper
{
  void execute_two(const double &data)
  {
    STFY_UNUSED(data);
    called_two = true;
  }

  bool called_two = false;
};
} // namespace
STFY_FUNC_OBJ_EXTERNAL(CallFunctionSuper, execute_two)

namespace
{
struct ExecuteThreeReturn
{
  bool valid = true;
  int error_code = 0;
  std::string data = "hello world";
  STFY_OBJECT(STFY_MEMBER(valid), STFY_MEMBER(error_code), STFY_MEMBER(data));
};
struct CallFunctionSub : public CallFunctionSuperSuper, public CallFunctionSuper
{
  ExecuteThreeReturn execute_three(const std::vector<double> &data)
  {
    for (auto x : data)
      STFY_UNUSED(x);
    called_three = true;
    return ExecuteThreeReturn();
  }

  bool called_three = false;
};
} // namespace
STFY_FUNC_OBJ_EXTERNAL_SUPER(CallFunctionSub, STFY_SUPER(CallFunctionSuperSuper, CallFunctionSuper), execute_three)

namespace
{
TEST_CASE("inheritanceTestExternalShort", "[function]")
{
  CallFunctionSub cont;
  std::string json_out;
  STFY::DefaultCallFunctionContext context(json, json_out);
  context.callFunctions(cont);

  REQUIRE(cont.called_one);
  REQUIRE(cont.called_two);
  REQUIRE(cont.called_three);

  if (context.parse_context.error != STFY::Error::NoError)
  REQUIRE(context.parse_context.error == STFY::Error::NoError);
}

struct CallFunctionVirtualOverload : public CallFunction
{
  virtual void execute_one(const SimpleData &data) override
  {
    STFY_UNUSED(data);
    override_called = true;
  }
  bool override_called = false;
};
// STFY_FUNC_OBJ_EXTERNAL_SUPER(CallFunctionVirtualOverload, STFY_SUPER(CallFunction));

// void virtualFunctionTest()
//{
//    std::string json_out;
//    json_out.reserve(512);
//    CallFunctionVirtualOverload cont;
//    STFY::DefaultCallFunctionContext context(json,json_out);
//    context.callFunctions(cont);
//
//    REQUIRE(cont.override_called);
//    REQUIRE(!cont.called_one);
//    REQUIRE(cont.called_two);
//    REQUIRE(cont.called_three);
//
//    if (context.parse_context.error != STFY::Error::NoError)
//    REQUIRE(context.parse_context.error == STFY::Error::NoError);
//}

const char json_two[] = R"json(
{
  "execute_one": {
    "number": 45,
    "valid": false,
    "more_data": "string data",
    "super_data": "hello"
  }
}
)json";

struct SuperParamOne
{
  int number;
  STFY_OBJECT(STFY_MEMBER(number));
};

struct SuperParamTwo
{
  bool valid;
  STFY_OBJECT(STFY_MEMBER(valid));
};

struct SuperParamTwoOne : public SuperParamTwo
{
  std::string more_data;
  STFY_OBJECT_WITH_SUPER(STFY_SUPER_CLASSES(STFY_SUPER_CLASS(SuperParamTwo)), STFY_MEMBER(more_data));
};
struct Param : public SuperParamOne, public SuperParamTwoOne
{
  std::string super_data;
  STFY_OBJECT_WITH_SUPER(STFY_SUPER_CLASSES(STFY_SUPER_CLASS(SuperParamOne), STFY_SUPER_CLASS(SuperParamTwoOne)),
                       STFY_MEMBER(super_data));
};

struct SuperParamCallable
{
  void execute_one(const Param &param)
  {
    STFY_UNUSED(param);
    execute_one_executed = true;
  }
  bool execute_one_executed = false;
};
} // namespace
STFY_FUNC_OBJ_EXTERNAL(SuperParamCallable, execute_one)

namespace
{
TEST_CASE("super_class_param_testExternalShort", "[function]")
{
  SuperParamCallable cont;
  std::string json_out;
  STFY::DefaultCallFunctionContext context(json_two, json_out);
  context.callFunctions(cont);

  REQUIRE(cont.execute_one_executed);
  if (context.parse_context.error != STFY::Error::NoError)
  REQUIRE(context.parse_context.error == STFY::Error::NoError);
}

const char call_void_json[] = R"json(
{
  "call_void": [],
  "call_void_context": null,
  "call_int_void": {},
  "call_int_void_context": {},
  "call_void_with_value": 4,
  "call_void_error": {}
}
)json";

struct CallVoidStruct
{
  void call_void()
  {
    executed_1 = true;
  }

  void call_void_context(STFY::CallFunctionContext &context)
  {
    STFY_UNUSED(context);
    executed_2 = true;
  }

  int call_int_void()
  {
    executed_3 = true;
    return 3;
  }

  int call_int_void_context(STFY::CallFunctionContext &context)
  {
    STFY_UNUSED(context);
    executed_4 = true;
    return 7;
  }

  void call_void_error(STFY::CallFunctionErrorContext &error)
  {
    STFY_UNUSED(error);
    executed_6 = true;
  }

  void call_void_with_value()
  {
    executed_5 = true;
  }

  bool executed_1 = false;
  bool executed_2 = false;
  bool executed_3 = false;
  bool executed_4 = false;
  bool executed_5 = false;
  bool executed_6 = false;
};
} // namespace
STFY_FUNC_OBJ_EXTERNAL(CallVoidStruct, call_void, call_void_context, call_int_void, call_int_void_context,
                     call_void_with_value, call_void_error)

namespace
{
TEST_CASE("call_void_testExternalShort", "[function]")
{
  CallVoidStruct voidStruct;
  std::string json_out;
  STFY::DefaultCallFunctionContext context(call_void_json, json_out);
  context.callFunctions(voidStruct);

  if (context.error_context.getLatestError() != STFY::Error::NoError)
  REQUIRE(context.error_context.getLatestError() == STFY::Error::NoError);
  REQUIRE(voidStruct.executed_1);
  REQUIRE(voidStruct.executed_2);
  REQUIRE(voidStruct.executed_3);
  REQUIRE(voidStruct.executed_4);
  REQUIRE(voidStruct.executed_5);
  REQUIRE(voidStruct.executed_6);
  REQUIRE(context.execution_list.size() == 6);
}

const char call_error_check_json[] = R"json(
{
  "call_void": [],
  "call_with_int": 5,
  "call_another_void": {},
  "call_with_object": { "x": 9 }
}
)json";

struct CallErrorCheckArg
{
  int x;
  STFY_OBJECT(STFY_MEMBER(x));
};
struct CallErrorCheck
{
  void call_void()
  {
    executed1 = true;
  }

  void call_with_int(int x, STFY::CallFunctionErrorContext &context)
  {
    STFY_UNUSED(x);
    executed2 = true;
    context.setError(STFY::Error::UserDefinedErrors, "CallWithIntCustomError problem with number");
  }

  void call_another_void()
  {
    executed3 = true;
  }

  std::string call_with_object(const CallErrorCheckArg &arg, STFY::CallFunctionErrorContext &context)
  {
    STFY_UNUSED(arg);
    executed4 = true;
    context.setError(STFY::Error::UserDefinedErrors, "This functions should not serialize the string");
    return std::string("THIS SHOULD NOT BE SERIALIZED");
  }

  bool executed1 = false;
  bool executed2 = false;
  bool executed3 = false;
  bool executed4 = false;
};
} // namespace
STFY_FUNC_OBJ_EXTERNAL(CallErrorCheck, call_void, call_with_int, call_another_void, call_with_object)

namespace
{
TEST_CASE("call_error_checkExternalShort", "[function]")
{
  CallErrorCheck errorCheck;
  std::string json_out;
  STFY::DefaultCallFunctionContext context(call_error_check_json, json_out);
  context.stop_execute_on_fail = false;
  STFY::Error error = context.callFunctions(errorCheck);

  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(errorCheck.executed1);
  REQUIRE(errorCheck.executed2);
  REQUIRE(errorCheck.executed3);
  REQUIRE(errorCheck.executed4);
  REQUIRE(json_out.size() == 3);
}

const char json_alias[] = R"json(
{
  "execute_one": 4,
  "execute_two": 5,
  "execute_three": 6
}
)json";

struct JsonAlias
{
  bool executeOne = false;
  bool executeTwo = false;
  bool executeThree = false;

  void execute_one(int x)
  {
    REQUIRE(x == 4);
    executeOne = true;
  }

  void execute_two_primary(int x)
  {
    REQUIRE(x == 5);
    executeTwo = true;
  }

  void execute_three(int x)
  {
    REQUIRE(x == 6);
    executeThree = true;
  }
};
} // namespace
STFY_FUNCTION_CONTAINER_EXTERNAL(JsonAlias, STFY_FUNCTION(execute_one),
                               STFY_FUNCTION_ALIASES(execute_two_primary, "execute_two"), STFY_FUNCTION(execute_three))

namespace
{
TEST_CASE("call_json_aliasExternalShort", "[function]")
{
  JsonAlias cont;
  std::string json_out;
  STFY::DefaultCallFunctionContext context(json_alias, sizeof(json_alias), json_out);
  context.callFunctions(cont);

  REQUIRE(cont.executeOne);
  REQUIRE(cont.executeTwo);
  REQUIRE(cont.executeThree);

  if (context.parse_context.error != STFY::Error::NoError)
  REQUIRE(context.parse_context.error == STFY::Error::NoError);
}

const char json_wrong_arg_type[] = R"json(
{
  "execute_one": { "some_function_object": 1 },
  "execute_two": { "more_members": false },
  "execute_three": { "last_member": 44.50 }
}
)json";

struct JsonWrongArgTypeExecThree
{
  double last_member;
  STFY_OBJECT(STFY_MEMBER(last_member));
};

struct JsonWrongArgType
{
  bool executeOne = false;
  bool executeTwo = false;
  bool executeThree = false;

  void execute_one(const std::string &foo)
  {
    STFY_UNUSED(foo);
    executeOne = true;
  }

  void execute_two(const std::string &bar)
  {
    STFY_UNUSED(bar);
    executeTwo = true;
  }

  void execute_three(const JsonWrongArgTypeExecThree &arg)
  {
    REQUIRE(arg.last_member == 44.50);
    executeThree = true;
  }
};
} // namespace
STFY_FUNC_OBJ_EXTERNAL(JsonWrongArgType, execute_one, execute_two, execute_three)

namespace
{
TEST_CASE("call_json_wrong_arg_typeExternalShort", "[function]")
{
  JsonWrongArgType cont;
  std::string json_out;
  STFY::DefaultCallFunctionContext context(json_wrong_arg_type, sizeof(json_wrong_arg_type), json_out);
  context.callFunctions(cont);

  REQUIRE(cont.executeOne);
  REQUIRE(cont.executeTwo);
  REQUIRE(cont.executeThree);

  if (context.parse_context.error != STFY::Error::NoError)
  REQUIRE(context.parse_context.error == STFY::Error::NoError);
}

} // namespace

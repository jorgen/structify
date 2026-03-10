/*
 * Copyright © 2017 Jorgen Lind
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

namespace structify_verify
{

static const char json_data1[] = "{\n"
                                 "\"StringNode\": \"Some test data\",\n"
                                 "\"NumberNode\": 4676\n"
                                 "}\n";

struct ContainsStringNode
{
  std::string StringNode;

  STFY_OBJECT(STFY_MEMBER(StringNode));
};

struct SubStructVerify : public ContainsStringNode
{
  int NumberNode;

  STFY_OBJECT_WITH_SUPER(STFY_SUPER_CLASSES(STFY_SUPER_CLASS(ContainsStringNode)), STFY_MEMBER(NumberNode));
};

TEST_CASE("testSimpleOneMember", "[structify][structify_verify]")
{
  STFY::ParseContext context(json_data1);
  SubStructVerify substruct;
  auto error = context.parseTo(substruct);
  REQUIRE(error == STFY::Error::NoError);

  REQUIRE(substruct.StringNode == "Some test data");
  REQUIRE(substruct.NumberNode == 4676);
}

static const char json_data2[] = "{\n"
                                 "\"ThisWillBeUnassigned\": \"Some data\",\n"
                                 "\"StringNode\": \"Some test data\"\n"
                                 "}\n";

TEST_CASE("testSimpleVerifyMissingMemberInStruct", "[structify][structify_verify]")
{
  STFY::ParseContext context(json_data2);
  ContainsStringNode containsString;
  auto error = context.parseTo(containsString);
  REQUIRE(error == STFY::Error::NoError);

  REQUIRE(containsString.StringNode == "Some test data");
  REQUIRE(context.missing_members.size() == 1);
  REQUIRE(context.missing_members.front() == "ThisWillBeUnassigned");
}

static const char json_data3[] = "{\n"
                                 "\"Field1\": 1\n,"
                                 "\"Field3\": 3\n"
                                 "}\n";

struct RequiredMemberStruct
{
  int Field1;
  int Field2;
  int Field3;

  STFY_OBJECT(STFY_MEMBER(Field1), STFY_MEMBER(Field2), STFY_MEMBER(Field3));
};

TEST_CASE("testSimpleVerifyMissingRequiredMemberInStruct", "[structify][json_strut_verify]")
{
  STFY::ParseContext context(json_data3);
  RequiredMemberStruct requiredMemberStruct;
  auto error = context.parseTo(requiredMemberStruct);
  REQUIRE(error == STFY::Error::NoError);

  REQUIRE(requiredMemberStruct.Field3 == 3);
  REQUIRE(context.unassigned_required_members.size() == 1);
  REQUIRE(context.unassigned_required_members.front() == "Field2");
}

static const char json_data4[] = "{\n"
                                 "\"StringNode\": \"Some test data\",\n"
                                 "\"NumberNode\": 342,\n"
                                 "\"SubNode\": \"This should be in subclass\"\n"
                                 "}\n";

struct SuperClass
{
  std::string StringNode;
  int NumberNode;
  STFY_OBJECT(STFY_MEMBER(StringNode), STFY_MEMBER(NumberNode));
};

struct SubClass : public SuperClass
{
  std::string SubNode;
  int SubNode2;
  STFY_OBJECT_WITH_SUPER(STFY_SUPER_CLASSES(STFY_SUPER_CLASS(SuperClass)), STFY_MEMBER(SubNode), STFY_MEMBER(SubNode2));
};

TEST_CASE("testClassHirarchyVerifyMissingMemberInStruct", "[structify][json_strut_verify]")
{
  STFY::ParseContext context(json_data4);
  SubClass subClass;
  auto error = context.parseTo(subClass);
  REQUIRE(error == STFY::Error::NoError);

  REQUIRE(subClass.NumberNode == 342);
  REQUIRE(subClass.SubNode == "This should be in subclass");
  REQUIRE(context.unassigned_required_members.size() == 1);
  REQUIRE(context.unassigned_required_members.front() == "SubNode2");
}

struct SuperSuperClass
{
  int SuperSuper;
  STFY_OBJECT(STFY_MEMBER(SuperSuper));
};

struct SuperClass2 : public SuperSuperClass
{
  std::string Super;
  STFY_OBJECT_WITH_SUPER(STFY_SUPER_CLASSES(STFY_SUPER_CLASS(SuperSuperClass)), STFY_MEMBER(Super));
};

struct RegularClass : public SuperClass2
{
  int Regular;
  STFY_OBJECT_WITH_SUPER(STFY_SUPER_CLASSES(STFY_SUPER_CLASS(SuperClass2)), STFY_MEMBER(Regular));
};

static const char json_data5[] = "{\n"
                                 "\"SuperSuper\": 5,\n"
                                 "\"Regular\": 42\n"
                                 "}\n";

TEST_CASE("testClassHIrarchyVerifyMissingDataForStruct", "[structify][json_strut_verify]")
{
  STFY::ParseContext context(json_data5);
  RegularClass regular;
  auto error = context.parseTo(regular);
  REQUIRE(error == STFY::Error::NoError);

  REQUIRE(context.unassigned_required_members.size() == 1);
  REQUIRE(context.unassigned_required_members.front() == "SuperClass2::Super");
}

static const char json_data6[] = "{\n"
                                 "\"SuperSuper\": 5,\n"
                                 "\"Super\": \"This is super\",\n"
                                 "\"SuperSuperSuper\": 42,\n"
                                 "\"Regular\": 42\n"
                                 "}\n";

TEST_CASE("testClassHirarchyVerifyMissingMemberInStruct2", "[structify][json_strut_verify]")
{
  STFY::ParseContext context(json_data6);
  RegularClass regular;
  auto error = context.parseTo(regular);
  REQUIRE(error == STFY::Error::NoError);

  REQUIRE(context.missing_members.size() == 1);
  REQUIRE(context.missing_members.front() == "SuperSuperSuper");
}

struct A
{
  int a;
  STFY_OBJECT(STFY_MEMBER(a));
};

struct B : public A
{
  float b;
  STFY_OBJECT_WITH_SUPER(STFY_SUPER_CLASSES(STFY_SUPER_CLASS(A)), STFY_MEMBER(b));
};

struct D
{
  int d;
  STFY_OBJECT(STFY_MEMBER(d));
};

struct E : public D
{
  double e;
  STFY_OBJECT_WITH_SUPER(STFY_SUPER_CLASSES(STFY_SUPER_CLASS(D)), STFY_MEMBER(e));
};

struct F : public E
{
  int f;
  STFY_OBJECT_WITH_SUPER(STFY_SUPER_CLASSES(STFY_SUPER_CLASS(E)), STFY_MEMBER(f));
};
struct G
{
  std::string g;
  STFY_OBJECT(STFY_MEMBER(g));
};

struct Subclass : public B, public F, public G
{
  int h;
  STFY_OBJECT_WITH_SUPER(STFY_SUPER_CLASSES(STFY_SUPER_CLASS(B), STFY_SUPER_CLASS(F), STFY_SUPER_CLASS(G)), STFY_MEMBER(h));
};

static const char json_data7[] = "{\n"
                                 "\"a\": 4,\n"
                                 "\"b\": 5.5,\n"
                                 "\"d\": 127,\n"
                                 "\"f\": 345,\n"
                                 "\"g\": \"a\",\n"
                                 "\"h\": 987\n"
                                 "}\n";

TEST_CASE("testClassHirarchyVerifyMissingDataForStructDeep", "[structify][json_strut_verify]")
{
  STFY::ParseContext context(json_data7);
  Subclass subclass;
  auto error = context.parseTo(subclass);
  REQUIRE(error == STFY::Error::NoError);

  REQUIRE(context.unassigned_required_members.size() == 1);
  REQUIRE(context.unassigned_required_members.front() == "E::e");
}

} // namespace structify_verify

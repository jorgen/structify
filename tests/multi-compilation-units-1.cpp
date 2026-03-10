/*
 * Copyright © 2020 Øystein Myrmo
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

#include "catch2/catch_all.hpp"
#include <structify/structify.h>
#include <string>

namespace multiple_compilation_units
{
struct Json1
{
  float num1;
  double num2;

  STFY_OBJECT(STFY_MEMBER(num1), STFY_MEMBER(num2));
};

std::string serialize_json1(float num1, double num2)
{
  Json1 json1{num1, num2};
  return STFY::serializeStruct(json1);
}

bool deserialize_json1(const std::string &json)
{
  STFY::ParseContext pc(json);
  Json1 json1;
  auto error = pc.parseTo(json1);
  REQUIRE(error == STFY::Error::NoError);
  return pc.error == STFY::Error::NoError;
}
} // namespace multiple_compilation_units

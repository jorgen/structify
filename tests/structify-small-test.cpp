#include <structify/structify.h>
#include "assert.h"

struct SmallStruct
{
  int a;
  float b;

  STFY_OBJECT(
    STFY_MEMBER(a),
    STFY_MEMBER(b)
  );
};

const char json[] = R"json(
{
  "a": 1,
  "b": 2.2
}
)json";

int main()
{
  STFY::ParseContext context(json);
    SmallStruct data;
    context.parseTo(data);
  STFY_ASSERT(data.a == 1);
  STFY_ASSERT(data.b > 2.199 && data.b < 2.201);
  return 0;
}

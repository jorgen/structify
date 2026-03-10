#include <string>
#include <structify/structify.h>

const char json[] = R"json(
{
  "vec" : [
    { "key" : 4, "value": 1.0 },
    { "key" : 5, "value": 2.0 },
    { "key" : 6, "value": 3.0 }
  ]
}
)json";

struct VecMember
{
  std::string key;
  double value = 0.0;

  STFY_OBJ(key, value);
};


struct ModuleList
{
  enum
  {
    ReservedSize = 16
  };
  VecMember modules[ReservedSize];
  int size = 0;
};

namespace STFY
{
template <>
struct TypeHandler<ModuleList>
{
  static inline Error to(ModuleList &to_type, ParseContext &context)
  {
    if (context.token.value_type != Type::ArrayStart)
      return STFY::Error::ExpectedArrayStart;

    context.nextToken();
    for (int i = 0; i < int(ModuleList::ReservedSize); i++)
    {
      if (context.error != STFY::Error::NoError)
        return context.error;
      if (context.token.value_type == Type::ArrayEnd)
      {
        to_type.size = i;
        break;
      }
      context.error = TypeHandler<VecMember>::to(to_type.modules[i], context);
      if (context.error != STFY::Error::NoError)
        return context.error;

      context.nextToken();
    }

    if (context.token.value_type != Type::ArrayEnd)
      return STFY::Error::ExpectedArrayEnd;
    return context.error;
  }

  static inline void from(const ModuleList &from_type, Token &token, Serializer &serializer)
  {
    token.value_type = Type::ArrayStart;
    token.value = DataRef("[");
    serializer.write(token);

    token.name = DataRef("");
    for (int i = 0; i < from_type.size; i++)
      TypeHandler<VecMember>::from(from_type.modules[i], token, serializer);

    token.name = DataRef("");
    token.value_type = Type::ArrayEnd;
    token.value = DataRef("]");
    serializer.write(token);
  }
};
} // namespace STFY

struct JsonObject
{
  ModuleList vec;
  STFY_OBJ(vec);
};


int main()
{
    JsonObject obj;
    STFY::ParseContext parseContext(json);
    if (parseContext.parseTo(obj) != STFY::Error::NoError)
    {
        std::string errorStr = parseContext.makeErrorString();
        fprintf(stderr, "Error parsing struct %s\n", errorStr.c_str());
        return -1;
    }

    fprintf(stdout, "Vec has size %d\n", obj.vec.size);

    return 0;
}


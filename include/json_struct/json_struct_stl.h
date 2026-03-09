// No #pragma once - this file is designed to be included multiple times
// with different JS_STL_* macros defined. Each section has its own include guard.
#include "json_struct_type_handlers.h"

#if defined(JS_STL_MAP) && !defined(JS_STL_MAP_INCLUDE)
#define JS_STL_MAP_INCLUDE
#include <map>
namespace JS
{
template <typename Key, typename Value>
struct TypeHandler<std::map<Key, Value>> : TypeHandlerMap<Key, Value, std::map<Key, Value>>
{
};
} // namespace JS
#endif

#if defined(JS_STL_SET) && !defined(JS_STL_SET_INCLUDE)
#define JS_STL_SET_INCLUDE
#include <set>
namespace JS
{
template <typename Key>
struct TypeHandler<std::set<Key>> : TypeHandlerSet<Key, std::set<Key>>
{
};
} // namespace JS
#endif

#if defined(JS_STL_UNORDERED_SET) && !defined(JS_STL_UNORDERED_SET_INCLUDE)
#define JS_STL_UNORDERED_SET_INCLUDE
#include <unordered_set>
namespace JS
{
template <typename Key>
struct TypeHandler<std::unordered_set<Key>> : TypeHandlerSet<Key, std::unordered_set<Key>>
{
};
} // namespace JS
#endif

#if defined(JS_STL_ARRAY) && !defined(JS_STL_ARRAY_INCLUDE)
#define JS_STL_ARRAY_INCLUDE
#include <array>
namespace JS
{
template <typename T, size_t N>
struct TypeHandler<std::array<T,N>>
{
public:
  static inline Error to(std::array<T,N> &to_type, ParseContext &context)
  {
    if (context.token.value_type != Type::ArrayStart)
      return JS::Error::ExpectedArrayStart;

    context.nextToken();
    for (size_t i = 0; i < N; i++)
    {
      if (context.error != JS::Error::NoError)
        return context.error;
      context.error = TypeHandler<T>::to(to_type[i], context);
      if (context.error != JS::Error::NoError)
        return context.error;

      context.nextToken();
    }

    if (context.token.value_type != Type::ArrayEnd)
      return JS::Error::ExpectedArrayEnd;
    return context.error;
  }
  static void from(const std::array<T,N> &from, Token &token, Serializer &serializer)
  {
    token.value_type = Type::ArrayStart;
    token.value = DataRef("[");
    serializer.write(token);

    token.name = DataRef("");
    for (size_t i = 0; i < N; i++)
      TypeHandler<T>::from(from[i], token, serializer);

    token.name = DataRef("");
    token.value_type = Type::ArrayEnd;
    token.value = DataRef("]");
    serializer.write(token);
  }
};
} // namespace JS
#endif

#if defined(JS_INT_128) && !defined(JS_INT_128_INCLUDE)
#define JS_INT_128_INCLUDE 1
// Compiler support check
#if defined(__SIZEOF_INT128__) && !defined(JS_NO_INT128_TYPEDEF)
namespace JS
{
__extension__ using js_int128_t = __int128;
__extension__ using js_uint128_t = unsigned __int128;
} // namespace JS
#endif

namespace JS
{
/// \private
template <>
struct TypeHandler<js_int128_t> : TypeHandlerIntType<js_int128_t>
{
};

/// \private
template <>
struct TypeHandler<js_uint128_t> : TypeHandlerIntType<js_uint128_t>
{
};
} // namespace JS
#endif

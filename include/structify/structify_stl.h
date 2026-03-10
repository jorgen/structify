// No #pragma once - this file is designed to be included multiple times
// with different STFY_STL_* macros defined. Each section has its own include guard.
#include "structify_type_handlers.h"

#if defined(STFY_STL_MAP) && !defined(STFY_STL_MAP_INCLUDE)
#define STFY_STL_MAP_INCLUDE
#include <map>
namespace STFY
{
template <typename Key, typename Value>
struct TypeHandler<std::map<Key, Value>> : TypeHandlerMap<Key, Value, std::map<Key, Value>>
{
};
} // namespace STFY
#endif

#if defined(STFY_STL_SET) && !defined(STFY_STL_SET_INCLUDE)
#define STFY_STL_SET_INCLUDE
#include <set>
namespace STFY
{
template <typename Key>
struct TypeHandler<std::set<Key>> : TypeHandlerSet<Key, std::set<Key>>
{
};
} // namespace STFY
#endif

#if defined(STFY_STL_UNORDERED_SET) && !defined(STFY_STL_UNORDERED_SET_INCLUDE)
#define STFY_STL_UNORDERED_SET_INCLUDE
#include <unordered_set>
namespace STFY
{
template <typename Key>
struct TypeHandler<std::unordered_set<Key>> : TypeHandlerSet<Key, std::unordered_set<Key>>
{
};
} // namespace STFY
#endif

#if defined(STFY_STL_ARRAY) && !defined(STFY_STL_ARRAY_INCLUDE)
#define STFY_STL_ARRAY_INCLUDE
#include <array>
namespace STFY
{
template <typename T, size_t N>
struct TypeHandler<std::array<T,N>>
{
public:
  static inline Error to(std::array<T,N> &to_type, ParseContext &context)
  {
    if (context.token.value_type != Type::ArrayStart)
      return STFY::Error::ExpectedArrayStart;

    context.nextToken();
    for (size_t i = 0; i < N; i++)
    {
      if (context.error != STFY::Error::NoError)
        return context.error;
      context.error = TypeHandler<T>::to(to_type[i], context);
      if (context.error != STFY::Error::NoError)
        return context.error;

      context.nextToken();
    }

    if (context.token.value_type != Type::ArrayEnd)
      return STFY::Error::ExpectedArrayEnd;
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
} // namespace STFY
#endif

#if defined(STFY_INT_128) && !defined(STFY_INT_128_INCLUDE)
#define STFY_INT_128_INCLUDE 1
// Compiler support check
#if defined(__SIZEOF_INT128__) && !defined(STFY_NO_INT128_TYPEDEF)
namespace STFY
{
__extension__ using stfy_int128_t = __int128;
__extension__ using stfy_uint128_t = unsigned __int128;
} // namespace STFY
#endif

namespace STFY
{
/// \private
template <>
struct TypeHandler<stfy_int128_t> : TypeHandlerIntType<stfy_int128_t>
{
};

/// \private
template <>
struct TypeHandler<stfy_uint128_t> : TypeHandlerIntType<stfy_uint128_t>
{
};
} // namespace STFY
#endif

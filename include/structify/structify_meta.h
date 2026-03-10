#pragma once
#include "structify_tokenizer.h"

namespace STFY
{
template <typename T>
struct Nullable
{
  Nullable()
    : data()
  {
  }
  Nullable(const T &t)
    : data(t)
  {
  }
  Nullable(T &&t)
    : data(std::move(t))
  {
  }

  Nullable(Nullable<T> &&t)
    : data(std::move(t.data))
  {
  }
  Nullable(const Nullable<T> &t)
    : data(t.data)
  {
  }

  Nullable<T> &operator=(const T &other)
  {
    data = other;
    return *this;
  }
  Nullable<T> &operator=(T &&other)
  {
    data = std::move(other);
    return *this;
  }

  Nullable<T> &operator=(const Nullable<T> &other)
  {
    data = other.data;
    return *this;
  }
  Nullable<T> &operator=(Nullable<T> &&other)
  {
    data = std::move(other.data);
    return *this;
  }

  T data;
  T &operator()()
  {
    return data;
  }
  const T &operator()() const
  {
    return data;
  }
};

template <typename T>
struct NullableChecked
{
  NullableChecked()
    : data()
    , null(true)
  {
  }
  NullableChecked(const T &t)
    : data(t)
    , null(false)
  {
  }
  NullableChecked(T &&t)
    : data(std::move(t))
    , null(false)
  {
  }
  NullableChecked(const NullableChecked<T> &t)
    : data(t.data)
    , null(t.null)
  {
  }
  NullableChecked(NullableChecked<T> &&t)
    : data(std::move(t.data))
    , null(t.null)
  {
  }
  NullableChecked<T> &operator=(const T &other)
  {
    data = other;
    null = false;
    return *this;
  }
  NullableChecked<T> &operator=(T &&other)
  {
    data = std::move(other);
    null = false;
    return *this;
  }

  NullableChecked<T> &operator=(const NullableChecked<T> &other)
  {
    data = other.data;
    null = other.null;
    return *this;
  }
  NullableChecked<T> &operator=(NullableChecked<T> &&other)
  {
    data = std::move(other.data);
    null = other.null;
    return *this;
  }

  T &operator()()
  {
    return data;
  }
  const T &operator()() const
  {
    return data;
  }
  T data;
  bool null;
};

template <typename T>
struct Optional
{
  Optional()
    : data()
  {
  }
  Optional(const T &t)
    : data(t)
  {
  }
  Optional(T &&t)
    : data(std::move(t))
  {
  }

  Optional(const Optional<T> &t)
    : data(t.data)
  {
  }
  Optional(Optional<T> &&t)
    : data(std::move(t.data))
  {
  }
  Optional<T> &operator=(const T &other)
  {
    data = other;
    return *this;
  }

  Optional<T> &operator=(T &&other)
  {
    data = std::move(other);
    return *this;
  }

  Optional<T> &operator=(const Optional<T> &other)
  {
    data = other.data;
    return *this;
  }

  Optional<T> &operator=(Optional<T> &&other)
  {
    data = std::move(other.data);
    return *this;
  }

  T data;
  T &operator()()
  {
    return data;
  }
  const T &operator()() const
  {
    return data;
  }
  typedef bool IsOptionalType;
};

template <typename T>
struct OptionalChecked
{
  OptionalChecked()
    : data()
    , assigned(false)
  {
  }
  OptionalChecked(const T &t)
    : data(t)
    , assigned(true)
  {
  }
  OptionalChecked(T &&t)
    : data(std::move(t))
    , assigned(true)
  {
  }
  OptionalChecked(const OptionalChecked<T> &t)
    : data(t.data)
    , assigned(t.assigned)
  {
  }
  OptionalChecked(OptionalChecked<T> &&t)
    : data(std::move(t.data))
    , assigned(t.assigned)
  {
  }
  OptionalChecked<T> &operator=(const T &other)
  {
    data = other;
    assigned = true;
    return *this;
  }
  OptionalChecked<T> &operator=(T &&other)
  {
    data = std::move(other);
    assigned = true;
    return *this;
  }
  OptionalChecked<T> &operator=(const OptionalChecked<T> &other)
  {
    data = other.data;
    assigned = other.assigned;
    return *this;
  }
  OptionalChecked<T> &operator=(OptionalChecked<T> &&other)
  {
    data = std::move(other.data);
    assigned = other.assigned;
    return *this;
  }

  T &operator()()
  {
    return data;
  }
  const T &operator()() const
  {
    return data;
  }
#ifdef STFY_STD_OPTIONAL
  std::optional<T> opt() const
  {
    return assigned ? std::optional<T>(data) : std::nullopt;
  }
#endif
  T data;
  bool assigned;
  typedef bool IsOptionalType;
};

struct SilentString
{
  std::string data;
  typedef bool IsOptionalType;
};

template <typename T, typename A = std::allocator<T>>
struct SilentVector
{
  std::vector<T, A> data;
  typedef bool IsOptionalType;
};

template <typename T, typename Deleter = std::default_delete<T>>
struct SilentUniquePtr
{
  std::unique_ptr<T, Deleter> data;
  typedef bool IsOptionalType;
};

struct JsonObjectRef
{
  DataRef ref;
};

struct JsonObject
{
  std::string data;
};

struct JsonArrayRef
{
  DataRef ref;
};

struct JsonArray
{
  std::string data;
};

struct JsonObjectOrArrayRef
{
  DataRef ref;
};

struct JsonObjectOrArray
{
  std::string data;
};

struct JsonTokens
{
  std::vector<STFY::Token> data;
};

struct JsonMeta
{
  JsonMeta(size_t pos, bool is_array)
    : position(pos)
    , size(1)
    , skip(1)
    , children(0)
    , complex_children(0)
    , is_array(is_array)
    , has_data(false)
  {
  }

  size_t position;
  uint32_t size;
  uint32_t skip;
  uint32_t children;
  uint32_t complex_children;
  bool is_array : 1;
  bool has_data : 1;
};

static inline std::vector<JsonMeta> metaForTokens(const JsonTokens &tokens)
{
  std::vector<JsonMeta> meta;
  meta.reserve(tokens.data.size() / 4);
  std::vector<size_t> parent;
  for (size_t i = 0; i < tokens.data.size(); i++)
  {
    for (size_t parent_index : parent)
    {
      meta[parent_index].size++;
    }
    const STFY::Token &token = tokens.data.at(i);
    if (token.value_type == Type::ArrayEnd || token.value_type == Type::ObjectEnd)
    {
      assert(parent.size());
      assert(meta[parent.back()].is_array == (token.value_type == Type::ArrayEnd));
      parent.pop_back();
    }
    else
    {
      if (parent.size())
        meta[parent.back()].children++;
    }

    if (token.value_type == Type::ArrayStart || token.value_type == Type::ObjectStart)
    {
      if (parent.size())
        meta[parent.back()].complex_children++;
      for (size_t parent_index : parent)
      {
        meta[parent_index].skip++;
      }
      meta.push_back(JsonMeta(i, token.value_type == Type::ArrayStart));
      parent.push_back(meta.size() - 1);
    }
    else if (token.value_type != STFY::Type::ArrayEnd && token.value_type != STFY::Type::ObjectEnd)
    {
      for (size_t parent_index : parent)
      {
        meta[parent_index].has_data = true;
      }
    }
  }
  assert(!parent.size()); // This assert may be triggered when JSON is invalid (e.g. when creating a DiffContext).
  return meta;
}

namespace Internal
{
static inline size_t findFirstChildWithData(const std::vector<JsonMeta> &meta_vec, size_t start_index)
{
  const JsonMeta &meta = meta_vec[start_index];
  if (!meta.has_data)
    return size_t(-1);

  size_t skip_size = 0;
  for (uint32_t i = 0; i < meta.complex_children; i++)
  {
    auto &current_child = meta_vec[start_index + skip_size + 1];
    skip_size += current_child.skip;
    if (current_child.has_data)
      return i;
  }
  return size_t(-1);
}
} // namespace Internal

template <typename T>
struct IsOptionalType
{
  typedef char yes[1];
  typedef char no[2];

  template <typename C>
  static constexpr yes &test_in_optional(typename C::IsOptionalType *);

  template <typename>
  static constexpr no &test_in_optional(...);

  static constexpr const bool value = sizeof(test_in_optional<T>(0)) == sizeof(yes);
};

/// \private
template <typename T>
struct IsOptionalType<std::unique_ptr<T>>
{
  static constexpr const bool value = true;
};

#ifdef STFY_STD_OPTIONAL
/// \private
template <typename T>
struct IsOptionalType<std::optional<T>>
{
  static constexpr const bool value = true;
};
#endif

struct ParseContext
{
  ParseContext()
  {
  }
  explicit ParseContext(const char *data, size_t size)
  {
    tokenizer.addData(data, size);
  }

  explicit ParseContext(const char *data)
  {
    size_t size = strlen(data);
    tokenizer.addData(data, size);
  }

  explicit ParseContext(const std::string &data)
  {
    tokenizer.addData(&data[0], data.size());
  }

  template <typename T>
  explicit ParseContext(const char *data, size_t size, T &to_type)
  {
    tokenizer.addData(data, size);
    auto this_error = parseTo(to_type);
    (void)this_error;
  }
  template <size_t SIZE>
  explicit ParseContext(const char (&data)[SIZE])
  {
    tokenizer.addData(data);
  }

  template <typename T>
  Error parseTo(T &to_type);

  Error nextToken()
  {
    error = tokenizer.nextTokens(&token, 1).second;
    return error;
  }

  std::string makeErrorString() const
  {
    if (error == Error::MissingPropertyMember)
    {
      if (missing_members.size() == 0)
      {
        return "";
      }
      else if (missing_members.size() == 1)
      {
        return std::string("JSON Object contained member not found in C++ struct/class. JSON Object member is: ") +
               missing_members.front();
      }
      std::string member_string = missing_members.front();
      for (int i = 1; i < int(missing_members.size()); i++)
        member_string += std::string(", ") + missing_members[i];
      return std::string("JSON Object contained members not found in C++ struct/class. JSON Object members are: ") +
             member_string;
    }
    else if (error == Error::UnassignedRequiredMember)
    {
      if (unassigned_required_members.size() == 0)
      {
        return "";
      }
      else if (unassigned_required_members.size() == 1)
      {
        return std::string("C++ struct/class has a required member that is not present in input JSON. The unassigned "
                           "C++ member is: ") +
               unassigned_required_members.front();
      }
      std::string required_string = unassigned_required_members.front();
      for (int i = 1; i < int(unassigned_required_members.size()); i++)
        required_string += std::string(", ") + unassigned_required_members[i];
      return std::string("C++ struct/class has required members that are not present in the input JSON. The unassigned "
                         "C++ members are: ") +
             required_string;
    }
    if (tokenizer.errorContext().error == Error::NoError && error != Error::NoError)
    {
      std::string retString("Error:");
      if (error <= Error::UserDefinedErrors)
        retString += Internal::error_strings[int(error)];
      else
        retString += "Unknown error";
      return retString;
    }
    return tokenizer.makeErrorString();
  }

  Tokenizer tokenizer;
  Token token;
  Error error = Error::NoError;
  std::vector<std::string> missing_members;
  std::vector<std::string> unassigned_required_members;
  bool allow_missing_members = true;
  bool allow_unasigned_required_members = true;
  bool track_member_assignement_state = true;
  void *user_data = nullptr;
};

/*! \def STFY_MEMBER
 *
 * Create meta information of the member with the same name as
 * the member.
 */
/*! \def STFY_MEMBER_ALIASES
 *
 * Create meta information where the primary name is the same as the member and
 * the subsequent names are aliases.
 */
/*! \def STFY_MEMBER_WITH_NAME
 *
 * Create meta information where the primary name is argument name, and the subsequent
 * names are aliases.
 */
/*! \def STFY_MEMBER_WITH_NAME_AND_ALIASES
 *
 * Creates meta information where the primary name is argument name, a
 * and subsequent names are aliases
 */

/*! \def STFY_SUPER_CLASS
 *
 * Creates superclass meta data which is used inside the STFY_SUPER_CLASSES macro
 */

/*! \def STFY_SUPER_CLASSES
 *
 * Macro to contain the super class definitions
 */

namespace Internal
{
template <typename T>
struct HasStructifyBase
{
  typedef char yes[1];
  typedef char no[2];

  template <typename C>
  static constexpr yes &test_in_base(typename C::template StructifyBase<C> *);

  template <typename>
  static constexpr no &test_in_base(...);
};

template <typename STFY_BASE_STRUCT_T, typename STFY_OBJECT_T>
struct StructifyBaseDummy
{
  static_assert(sizeof(HasStructifyBase<STFY_OBJECT_T>::template test_in_base<STFY_OBJECT_T>(nullptr)) ==
                  sizeof(typename HasStructifyBase<STFY_OBJECT_T>::yes),
                "Missing STFY_OBJECT STFY_OBJECT_EXTERNAL or TypeHandler specialisation\n");
  using TT = decltype(STFY_OBJECT_T::template StructifyBase<STFY_OBJECT_T>::stfy_static_meta_data_info());
  using ST = decltype(STFY_OBJECT_T::template StructifyBase<STFY_OBJECT_T>::stfy_static_meta_super_info());
  static inline constexpr const TT stfy_static_meta_data_info()
  {
    return STFY_OBJECT_T::template StructifyBase<STFY_OBJECT_T>::stfy_static_meta_data_info();
  }

  static inline constexpr const ST stfy_static_meta_super_info()
  {
    return STFY_OBJECT_T::template StructifyBase<STFY_OBJECT_T>::stfy_static_meta_super_info();
  }
};
} // namespace Internal

#define STFY_INTERNAL_EXPAND(x) x
#define STFY_INTERNAL_FIRST_(a, ...) a
#define STFY_INTERNAL_SECOND_(a, b, ...) b
#define STFY_INTERNAL_FIRST(...) STFY_INTERNAL_EXPAND(STFY_INTERNAL_FIRST_(__VA_ARGS__))
#define STFY_INTERNAL_SECOND(...) STFY_INTERNAL_EXPAND(STFY_INTERNAL_SECOND_(__VA_ARGS__))
#define STFY_INTERNAL_EMPTY()
#define STFY_INTERNAL_EVAL(...) STFY_INTERNAL_EVAL1024(__VA_ARGS__)
#define STFY_INTERNAL_EVAL1024(...) STFY_INTERNAL_EVAL512(STFY_INTERNAL_EVAL512(__VA_ARGS__))
#define STFY_INTERNAL_EVAL512(...) STFY_INTERNAL_EVAL256(STFY_INTERNAL_EVAL256(__VA_ARGS__))
#define STFY_INTERNAL_EVAL256(...) STFY_INTERNAL_EVAL128(STFY_INTERNAL_EVAL128(__VA_ARGS__))
#define STFY_INTERNAL_EVAL128(...) STFY_INTERNAL_EVAL64(STFY_INTERNAL_EVAL64(__VA_ARGS__))
#define STFY_INTERNAL_EVAL64(...) STFY_INTERNAL_EVAL32(STFY_INTERNAL_EVAL32(__VA_ARGS__))
#define STFY_INTERNAL_EVAL32(...) STFY_INTERNAL_EVAL16(STFY_INTERNAL_EVAL16(__VA_ARGS__))
#define STFY_INTERNAL_EVAL16(...) STFY_INTERNAL_EVAL8(STFY_INTERNAL_EVAL8(__VA_ARGS__))
#define STFY_INTERNAL_EVAL8(...) STFY_INTERNAL_EVAL4(STFY_INTERNAL_EVAL4(__VA_ARGS__))
#define STFY_INTERNAL_EVAL4(...) STFY_INTERNAL_EVAL2(STFY_INTERNAL_EVAL2(__VA_ARGS__))
#define STFY_INTERNAL_EVAL2(...) STFY_INTERNAL_EVAL1(STFY_INTERNAL_EVAL1(__VA_ARGS__))
#define STFY_INTERNAL_EVAL1(...) __VA_ARGS__

#define STFY_INTERNAL_DEFER1(m) m STFY_INTERNAL_EMPTY()
#define STFY_INTERNAL_DEFER2(m) m STFY_INTERNAL_EMPTY STFY_INTERNAL_EMPTY()()

#define STFY_INTERNAL_IS_PROBE(...) STFY_INTERNAL_SECOND(__VA_ARGS__, 0, 0)
#define STFY_INTERNAL_PROBE() ~, 1

#define STFY_INTERNAL_CAT(a, b) a##b

#define STFY_INTERNAL_NOT(x) STFY_INTERNAL_IS_PROBE(STFY_INTERNAL_CAT(STFY_INTERNAL__NOT_, x))
#define STFY_INTERNAL__NOT_0 STFY_INTERNAL_PROBE()

#define STFY_INTERNAL_BOOL(x) STFY_INTERNAL_NOT(STFY_INTERNAL_NOT(x))

#define STFY_INTERNAL_IF_ELSE(condition) STFY_INTERNAL__IF_ELSE(STFY_INTERNAL_BOOL(condition))
#define STFY_INTERNAL__IF_ELSE(condition) STFY_INTERNAL_CAT(STFY_INTERNAL__IF_, condition)

#define STFY_INTERNAL__IF_1(...) __VA_ARGS__ STFY_INTERNAL__IF_1_ELSE
#define STFY_INTERNAL__IF_0(...) STFY_INTERNAL__IF_0_ELSE

#define STFY_INTERNAL__IF_1_ELSE(...)
#define STFY_INTERNAL__IF_0_ELSE(...) __VA_ARGS__

#define STFY_INTERNAL_HAS_MORE_THAN_ONE_ARGS(...)                                                                        \
  STFY_INTERNAL_BOOL(STFY_INTERNAL_SECOND(STFY_INTERNAL__END_OF_ARGUMENTS_ __VA_ARGS__, 0, 0)())
#define STFY_INTERNAL__END_OF_ARGUMENTS_() 0

#define STFY_MEMBER(member) STFY::makeMemberInfo(#member, &STFY_OBJECT_T::member)
#define STFY_MEMBER_ALIASES(member, ...)                                                                                 \
  STFY_INTERNAL_EXPAND(STFY::makeMemberInfo(#member, &STFY_OBJECT_T::member, __VA_ARGS__))
#define STFY_MEMBER_WITH_NAME(member, name) STFY::makeMemberInfo(name, &STFY_OBJECT_T::member)
#define STFY_MEMBER_WITH_NAME_AND_ALIASES(member, name, ...) STFY::makeMemberInfo(name, &STFY_OBJECT_T::member, __VA_ARGS__)

#define STFY_SUPER_CLASS(super) STFY::makeSuperInfo<super>(#super)

#define STFY_SUPER_CLASSES(...) STFY::makeTuple(__VA_ARGS__)
#define STFY_INTERNAL__MAP_MEMBER() STFY_INTERNAL_MAP_MEMBER

#define STFY_INTERNAL_MAKE_MEMBERS(...)                                                                                  \
  STFY_INTERNAL_IF_ELSE(STFY_INTERNAL_HAS_MORE_THAN_ONE_ARGS(__VA_ARGS__))                                                 \
  (STFY_INTERNAL_EXPAND(STFY_INTERNAL_EVAL(STFY_INTERNAL_MAP_MEMBER(STFY::makeMemberInfo, __VA_ARGS__))))(                     \
    STFY_INTERNAL_MAP_APPLY_MEMBER(STFY::makeMemberInfo, __VA_ARGS__))

#define STFY_INTERNAL_MAP_APPLY_MEMBER(m, first) m(#first, &STFY_OBJECT_T::first)

#define STFY_INTERNAL_MAP_MEMBER(m, first, ...)                                                                          \
  STFY_INTERNAL_MAP_APPLY_MEMBER(m, first)                                                                               \
  STFY_INTERNAL_IF_ELSE(STFY_INTERNAL_HAS_MORE_THAN_ONE_ARGS(__VA_ARGS__))                                                 \
  (, STFY_INTERNAL_DEFER2(STFY_INTERNAL__MAP_MEMBER)()(m, __VA_ARGS__))(, STFY_INTERNAL_MAP_APPLY_MEMBER(m, __VA_ARGS__))

#define STFY_INTERNAL_MAP_APPLY_SUPER(m, first) m<first>(#first)

#define STFY_INTERNAL_MAP_SUPER(m, first, ...)                                                                           \
  STFY_INTERNAL_MAP_APPLY_SUPER(m, first)                                                                                \
  STFY_INTERNAL_IF_ELSE(STFY_INTERNAL_HAS_MORE_THAN_ONE_ARGS(__VA_ARGS__))                                                 \
  (, STFY_INTERNAL_DEFER2(STFY_INTERNAL__MAP_SUPER)()(m, __VA_ARGS__))(, STFY_INTERNAL_MAP_APPLY_SUPER(m, __VA_ARGS__))

#define STFY_INTERNAL__MAP_SUPER() STFY_INTERNAL_MAP_SUPER

#define STFY_INTERNAL_MAKE_SUPER_CLASSES(...)                                                                            \
  STFY_INTERNAL_IF_ELSE(STFY_INTERNAL_HAS_MORE_THAN_ONE_ARGS(__VA_ARGS__))                                                 \
  (STFY_INTERNAL_EXPAND(STFY_INTERNAL_EVAL(STFY_INTERNAL_MAP_SUPER(STFY::makeSuperInfo, __VA_ARGS__))))(                       \
    STFY_INTERNAL_MAP_APPLY_SUPER(STFY::makeSuperInfo, __VA_ARGS__))

#define STFY_SUPER(...) STFY::makeTuple(STFY_INTERNAL_EXPAND(STFY_INTERNAL_MAKE_SUPER_CLASSES(__VA_ARGS__)))

#define STFY_OBJECT_INTERNAL_IMPL(super_list, member_list)                                                               \
  template <typename STFY_OBJECT_T>                                                                                      \
  struct StructifyBase                                                                                                \
  {                                                                                                                    \
    using TT = decltype(member_list);                                                                                  \
    static inline constexpr const TT stfy_static_meta_data_info()                                                        \
    {                                                                                                                  \
      return member_list;                                                                                              \
    }                                                                                                                  \
    static inline constexpr const decltype(super_list) stfy_static_meta_super_info()                                     \
    {                                                                                                                  \
      return super_list;                                                                                               \
    }                                                                                                                  \
  }

#define STFY_OBJECT_EXTERNAL_INTERNAL_IMPL(Type, super_list, member_list)                                                \
  namespace STFY                                                                                                         \
  {                                                                                                                    \
  namespace Internal                                                                                                   \
  {                                                                                                                    \
  template <typename STFY_OBJECT_T>                                                                                      \
  struct StructifyBaseDummy<Type, STFY_OBJECT_T>                                                                        \
  {                                                                                                                    \
    using TT = decltype(member_list);                                                                                  \
    static constexpr const TT stfy_static_meta_data_info()                                                               \
    {                                                                                                                  \
      return member_list;                                                                                              \
    }                                                                                                                  \
    static constexpr const decltype(super_list) stfy_static_meta_super_info()                                            \
    {                                                                                                                  \
      return super_list;                                                                                               \
    }                                                                                                                  \
  };                                                                                                                   \
  }                                                                                                                    \
  }

#define STFY_OBJECT(...) STFY_OBJECT_INTERNAL_IMPL(STFY::makeTuple(), STFY::makeTuple(__VA_ARGS__))
#define STFY_OBJECT_WITH_SUPER(super_list, ...) STFY_OBJECT_INTERNAL_IMPL(super_list, STFY::makeTuple(__VA_ARGS__))

#define STFY_OBJECT_EXTERNAL(Type, ...)                                                                                  \
  STFY_OBJECT_EXTERNAL_INTERNAL_IMPL(Type, STFY::makeTuple(), STFY::makeTuple(__VA_ARGS__))
#define STFY_OBJECT_EXTERNAL_WITH_SUPER(Type, super_list, ...)                                                           \
  STFY_OBJECT_EXTERNAL_INTERNAL_IMPL(Type, super_list, STFY::makeTuple(__VA_ARGS__))

#define STFY_OBJ(...) STFY_OBJECT_INTERNAL_IMPL(STFY::makeTuple(), STFY::makeTuple(STFY_INTERNAL_MAKE_MEMBERS(__VA_ARGS__)))
#define STFY_OBJ_SUPER(super_list, ...)                                                                                  \
  STFY_OBJECT_INTERNAL_IMPL(super_list, STFY::makeTuple(STFY_INTERNAL_MAKE_MEMBERS(__VA_ARGS__)))

#define STFY_OBJ_EXT(Type, ...)                                                                                          \
  STFY_OBJECT_EXTERNAL_INTERNAL_IMPL(Type, STFY::makeTuple(), STFY::makeTuple(STFY_INTERNAL_MAKE_MEMBERS(__VA_ARGS__)))
#define STFY_OBJ_EXT_SUPER(Type, super_list, ...)                                                                        \
  STFY_OBJECT_EXTERNAL_INTERNAL_IMPL(Type, super_list, STFY::makeTuple(STFY_INTERNAL_MAKE_MEMBERS(__VA_ARGS__)))

/*!
 * \private
 */
template <typename T, typename U, typename NAMETUPLE>
struct MI
{
  NAMETUPLE names;
  T U::*member;
  typedef T type;
};

namespace Internal
{
template <typename T, typename U, typename NAMETUPLE>
using MemberInfo = MI<T, U, NAMETUPLE>;

template <typename T>
struct SuperInfo
{
  constexpr explicit SuperInfo()
    : name()
  {
  }
  constexpr explicit SuperInfo(const DataRef &name)
    : name(name)
  {
  }
  const DataRef name;
  typedef T type;
};
} // namespace Internal

template <typename T, typename U, size_t NAME_SIZE, typename... Aliases>
constexpr auto makeMemberInfo(const char (&name)[NAME_SIZE], T U::*member, Aliases &...aliases)
  -> MI<T, U, decltype(makeTuple(STFY::Internal::makeStringLiteral(name), STFY::Internal::makeStringLiteral(aliases)...))>
{
  return {makeTuple(STFY::Internal::makeStringLiteral(name), STFY::Internal::makeStringLiteral(aliases)...), member};
}

template <typename T, size_t NAME_SIZE>
constexpr const Internal::SuperInfo<T> makeSuperInfo(const char (&name)[NAME_SIZE])
{
  return Internal::SuperInfo<T>(DataRef(name));
}

template <typename T, typename Enable = void>
struct TypeHandler
{
  static inline Error to(T &to_type, ParseContext &context);
  static inline void from(const T &from_type, Token &token, Serializer &serializer);
};

namespace Internal
{
template <size_t STRINGSIZE>
inline bool compareDataRefWithStringLiteral(const StringLiteral<STRINGSIZE> &memberName, const DataRef &jsonName)
{
  return jsonName.size == STRINGSIZE && memcmp(memberName.data, jsonName.data, STRINGSIZE) == 0;
}

template <typename NameTuple, size_t index>
struct NameChecker
{
  static bool compare(const NameTuple &tuple, const DataRef &name)
  {

    STFY_IF_CONSTEXPR(index != NameTuple::size)
    {
      auto &stringLiteral = tuple.template get<NameTuple::size - index>();
      if (compareDataRefWithStringLiteral(stringLiteral, name))
        return true;
    }
    return NameChecker<NameTuple, index - 1>::compare(tuple, name);
  }
};
template <typename NameTuple>
struct NameChecker<NameTuple, 0>
{
  static bool compare(const NameTuple &tuple, const DataRef &name)
  {
    STFY_UNUSED(tuple);
    STFY_UNUSED(name);
    return false;
  }
};

template <typename T, typename MI_T, typename MI_M, typename MI_NC>
inline Error unpackMember(T &to_type, const MemberInfo<MI_T, MI_M, MI_NC> &memberInfo, ParseContext &context,
                          size_t index, bool primary, bool *assigned_members)
{
  if (primary)
  {
    if (compareDataRefWithStringLiteral(memberInfo.names.template get<0>(), context.token.name))
    {
      assigned_members[index] = true;
      return TypeHandler<MI_T>::to(to_type.*memberInfo.member, context);
    }
  }
  else
  {
    if (NameChecker<MI_NC, MI_NC::size>::compare(memberInfo.names, context.token.name))
    {
      assigned_members[index] = true;
      return TypeHandler<MI_T>::to(to_type.*memberInfo.member, context);
    }
  }
  return Error::MissingPropertyMember;
}

template <typename MI_T, typename MI_M, typename MI_NC>
inline Error verifyMember(const MemberInfo<MI_T, MI_M, MI_NC> &memberInfo, size_t index, bool *assigned_members,
                          bool track_missing_members, std::vector<std::string> &missing_members, const char *super_name)
{
  if (assigned_members[index])
    return Error::NoError;
  if (IsOptionalType<MI_T>::value)
    return Error::NoError;

  if (track_missing_members)
  {
    std::string to_push = strlen(super_name) ? std::string(super_name) + "::" : std::string();
    to_push += std::string(memberInfo.names.template get<0>().data, memberInfo.names.template get<0>().size);
    missing_members.push_back(to_push);
  }
  return Error::UnassignedRequiredMember;
}

template <typename T, typename MI_T, typename MI_M, typename MI_NC>
inline void serializeMember(const T &from_type, const MemberInfo<MI_T, MI_M, MI_NC> &memberInfo, Token &token,
                            Serializer &serializer, const char *super_name)
{
  STFY_UNUSED(super_name);
  token.name.data = memberInfo.names.template get<0>().data;
  token.name.size = memberInfo.names.template get<0>().size;
  token.name_type = Type::Ascii;

  TypeHandler<MI_T>::from(from_type.*memberInfo.member, token, serializer);
}

template <typename T, size_t PAGE, size_t INDEX>
struct SuperClassHandler
{
  static Error handleSuperClasses(T &to_type, ParseContext &context, bool primary, bool *assigned_members);
  static Error verifyMembers(bool *assigned_members, bool track_missing_members,
                             std::vector<std::string> &missing_members);
  static constexpr size_t membersInSuperClasses();
  static void serializeMembers(const T &from_type, Token &token, Serializer &serializer);
};

template <typename T, size_t PAGE, size_t SIZE>
struct StartSuperRecursion
{
  static Error start(T &to_type, ParseContext &context, bool primary, bool *assigned)
  {
    return SuperClassHandler<T, PAGE, SIZE - 1>::handleSuperClasses(to_type, context, primary, assigned);
  }

  static Error verifyMembers(bool *assigned_members, bool track_missing_members,
                             std::vector<std::string> &missing_members)
  {
    return SuperClassHandler<T, PAGE, SIZE - 1>::verifyMembers(assigned_members, track_missing_members,
                                                               missing_members);
  }

  static constexpr size_t membersInSuperClasses()
  {
    return SuperClassHandler<T, PAGE, SIZE - 1>::membersInSuperClasses();
  }

  static void serializeMembers(const T &from_type, Token &token, Serializer &serializer)
  {
    return SuperClassHandler<T, PAGE, SIZE - 1>::serializeMembers(from_type, token, serializer);
  }
};

template <typename T, size_t PAGE>
constexpr size_t memberCount()
{
  using Members = decltype(Internal::template StructifyBaseDummy<T, T>::stfy_static_meta_data_info());
  using SuperMeta = decltype(Internal::template StructifyBaseDummy<T, T>::stfy_static_meta_super_info());
  return Members::size + StartSuperRecursion<T, PAGE + Members::size, SuperMeta::size>::membersInSuperClasses();
}

template <typename T, size_t PAGE>
struct StartSuperRecursion<T, PAGE, 0>
{
  static Error start(T &to_type, ParseContext &context, bool primary, bool *assigned)
  {
    STFY_UNUSED(to_type);
    STFY_UNUSED(context);
    STFY_UNUSED(primary);
    STFY_UNUSED(assigned);
    return Error::MissingPropertyMember;
  }

  static Error verifyMembers(bool *assigned_members, bool track_missing_members,
                             std::vector<std::string> &missing_members)
  {
    STFY_UNUSED(assigned_members);
    STFY_UNUSED(track_missing_members);
    STFY_UNUSED(missing_members);
    return Error::NoError;
  }

  static constexpr size_t membersInSuperClasses()
  {
    return 0;
  }

  static void serializeMembers(const T &from_type, Token &token, Serializer &serializer)
  {
    STFY_UNUSED(from_type);
    STFY_UNUSED(token);
    STFY_UNUSED(serializer);
  }
};

template <typename T, typename Members, size_t PAGE, size_t INDEX>
struct MemberChecker
{
  inline static Error unpackMembers(T &to_type, const Members &members, ParseContext &context, bool primary,
                                    bool *assigned_members)
  {
    Error error =
      unpackMember(to_type, members.template get<INDEX>(), context, PAGE + INDEX, primary, assigned_members);
    if (error != Error::MissingPropertyMember)
      return error;

    return MemberChecker<T, Members, PAGE, INDEX - 1>::unpackMembers(to_type, members, context, primary,
                                                                     assigned_members);
  }

  inline static Error verifyMembers(const Members &members, bool *assigned_members, bool track_missing_members,
                                    std::vector<std::string> &missing_members, const char *super_name)
  {
    Error memberError = verifyMember(members.template get<INDEX>(), PAGE + INDEX, assigned_members,
                                     track_missing_members, missing_members, super_name);
    Error error = MemberChecker<T, Members, PAGE, INDEX - 1>::verifyMembers(
      members, assigned_members, track_missing_members, missing_members, super_name);
    if (memberError != Error::NoError)
      return memberError;
    return error;
  }
  inline static void serializeMembers(const T &from_type, const Members &members, Token &token, Serializer &serializer,
                                      const char *super_name)
  {
    serializeMember(from_type, members.template get<Members::size - INDEX - 1>(), token, serializer, super_name);
    MemberChecker<T, Members, PAGE, INDEX - 1>::serializeMembers(from_type, members, token, serializer, super_name);
  }
};

template <typename T, typename Members, size_t PAGE>
struct MemberChecker<T, Members, PAGE, 0>
{
  inline static Error unpackMembers(T &to_type, const Members &members, ParseContext &context, bool primary,
                                    bool *assigned_members)
  {
    Error error = unpackMember(to_type, members.template get<0>(), context, PAGE, primary, assigned_members);
    if (error != Error::MissingPropertyMember)
      return error;

    using Super = decltype(Internal::template StructifyBaseDummy<T, T>::stfy_static_meta_super_info());
    return StartSuperRecursion<T, PAGE + Members::size, Super::size>::start(to_type, context, primary,
                                                                            assigned_members);
  }

  inline static Error verifyMembers(const Members &members, bool *assigned_members, bool track_missing_members,
                                    std::vector<std::string> &missing_members, const char *super_name)
  {
    Error memberError = verifyMember(members.template get<0>(), PAGE, assigned_members, track_missing_members,
                                     missing_members, super_name);
    using Super = decltype(Internal::template StructifyBaseDummy<T, T>::stfy_static_meta_super_info());
    Error superError = StartSuperRecursion<T, PAGE + Members::size, Super::size>::verifyMembers(
      assigned_members, track_missing_members, missing_members);
    if (memberError != Error::NoError)//-V1051 memberError is correct, but we have to allways call supers verifyMembers first
      return memberError;
    return superError;
  }

  inline static void serializeMembers(const T &from_type, const Members &members, Token &token, Serializer &serializer,
                                      const char *super_name)
  {
    serializeMember(from_type, members.template get<Members::size - 1>(), token, serializer, super_name);
    using Super = decltype(Internal::template StructifyBaseDummy<T, T>::stfy_static_meta_super_info());
    StartSuperRecursion<T, PAGE + Members::size, Super::size>::serializeMembers(from_type, token, serializer);
  }
};

template <typename T, size_t PAGE, size_t INDEX>
Error SuperClassHandler<T, PAGE, INDEX>::handleSuperClasses(T &to_type, ParseContext &context, bool primary,
                                                            bool *assigned_members)
{
  using SuperMeta = decltype(Internal::template StructifyBaseDummy<T, T>::stfy_static_meta_super_info());
  using Super = typename STFY::TypeAt<INDEX, SuperMeta>::type::type;
  using Members = decltype(Internal::template StructifyBaseDummy<Super, Super>::stfy_static_meta_data_info());
  auto members = Internal::template StructifyBaseDummy<Super, Super>::stfy_static_meta_data_info();
  Error error = MemberChecker<Super, Members, PAGE, Members::size - 1>::unpackMembers(
    static_cast<Super &>(to_type), members, context, primary, assigned_members);
  if (error != Error::MissingPropertyMember)
    return error;
  return SuperClassHandler<T, PAGE + memberCount<Super, 0>(), INDEX - 1>::handleSuperClasses(to_type, context, primary,
                                                                                             assigned_members);
}

template <typename T, size_t PAGE, size_t INDEX>
Error SuperClassHandler<T, PAGE, INDEX>::verifyMembers(bool *assigned_members, bool track_missing_members,
                                                       std::vector<std::string> &missing_members)
{
  using SuperMeta = decltype(Internal::template StructifyBaseDummy<T, T>::stfy_static_meta_super_info());
  using Super = typename TypeAt<INDEX, SuperMeta>::type::type;
  using Members = decltype(Internal::template StructifyBaseDummy<Super, Super>::stfy_static_meta_data_info());
  auto members = Internal::template StructifyBaseDummy<Super, Super>::stfy_static_meta_data_info();
  const char *super_name =
    Internal::template StructifyBaseDummy<T, T>::stfy_static_meta_super_info().template get<INDEX>().name.data;
  Error error = MemberChecker<Super, Members, PAGE, Members::size - 1>::verifyMembers(
    members, assigned_members, track_missing_members, missing_members, super_name);
  Error superError = SuperClassHandler<T, PAGE + memberCount<Super, 0>(), INDEX - 1>::verifyMembers(
    assigned_members, track_missing_members, missing_members);
  if (error != Error::NoError)
    return error;
  return superError;
}

template <typename T, size_t PAGE, size_t INDEX>
size_t constexpr SuperClassHandler<T, PAGE, INDEX>::membersInSuperClasses()
{
  using SuperMeta = decltype(Internal::template StructifyBaseDummy<T, T>::stfy_static_meta_super_info());
  using Super = typename TypeAt<INDEX, SuperMeta>::type::type;
  return memberCount<Super, PAGE>() +
         SuperClassHandler<T, PAGE + memberCount<Super, PAGE>(), INDEX - 1>::membersInSuperClasses();
}

template <typename T, size_t PAGE, size_t INDEX>
void SuperClassHandler<T, PAGE, INDEX>::serializeMembers(const T &from_type, Token &token, Serializer &serializer)
{
  using SuperMeta = decltype(Internal::template StructifyBaseDummy<T, T>::stfy_static_meta_super_info());
  using Super = typename TypeAt<INDEX, SuperMeta>::type::type;
  using Members = decltype(Internal::template StructifyBaseDummy<Super, Super>::stfy_static_meta_data_info());
  auto members = Internal::template StructifyBaseDummy<Super, Super>::stfy_static_meta_data_info();
  MemberChecker<Super, Members, PAGE, Members::size - 1>::serializeMembers(from_type, members, token, serializer, "");
  SuperClassHandler<T, PAGE + memberCount<Super, 0>(), INDEX - 1>::serializeMembers(from_type, token, serializer);
}

template <typename T, size_t PAGE>
struct SuperClassHandler<T, PAGE, 0>
{
  static Error handleSuperClasses(T &to_type, ParseContext &context, bool primary, bool *assigned_members)
  {
    using Meta = decltype(Internal::template StructifyBaseDummy<T, T>::stfy_static_meta_super_info());
    using Super = typename TypeAt<0, Meta>::type::type;
    using Members = decltype(Internal::template StructifyBaseDummy<Super, Super>::stfy_static_meta_data_info());
    auto members = Internal::template StructifyBaseDummy<Super, Super>::stfy_static_meta_data_info();
    return MemberChecker<Super, Members, PAGE, Members::size - 1>::unpackMembers(static_cast<Super &>(to_type), members,
                                                                                 context, primary, assigned_members);
  }
  static Error verifyMembers(bool *assigned_members, bool track_missing_members,
                             std::vector<std::string> &missing_members)
  {
    using SuperMeta = decltype(Internal::template StructifyBaseDummy<T, T>::stfy_static_meta_super_info());
    using Super = typename TypeAt<0, SuperMeta>::type::type;
    using Members = decltype(Internal::template StructifyBaseDummy<Super, Super>::stfy_static_meta_data_info());
    auto members = Internal::template StructifyBaseDummy<Super, Super>::stfy_static_meta_data_info();
    const char *super_name =
      Internal::template StructifyBaseDummy<T, T>::stfy_static_meta_super_info().template get<0>().name.data;
    return MemberChecker<Super, Members, PAGE, Members::size - 1>::verifyMembers(
      members, assigned_members, track_missing_members, missing_members, super_name);
  }
  constexpr static size_t membersInSuperClasses()
  {
    using SuperMeta = decltype(Internal::template StructifyBaseDummy<T, T>::stfy_static_meta_super_info());
    using Super = typename TypeAt<0, SuperMeta>::type::type;
    return memberCount<Super, PAGE>();
  }
  static void serializeMembers(const T &from_type, Token &token, Serializer &serializer)
  {
    using SuperMeta = decltype(Internal::template StructifyBaseDummy<T, T>::stfy_static_meta_super_info());
    using Super = typename TypeAt<0, SuperMeta>::type::type;
    using Members = decltype(Internal::template StructifyBaseDummy<Super, Super>::stfy_static_meta_data_info());
    auto members = Internal::StructifyBaseDummy<Super, Super>::stfy_static_meta_data_info();
    MemberChecker<Super, Members, PAGE, Members::size - 1>::serializeMembers(from_type, members, token, serializer, "");
  }
};

static bool skipArrayOrObject(ParseContext &context)
{
  assert(context.error == Error::NoError);
  Type start_type = context.token.value_type;
  Type end_type;
  if (context.token.value_type == Type::ObjectStart)
  {
    end_type = Type::ObjectEnd;
  }
  else if (context.token.value_type == Type::ArrayStart)
  {
    end_type = Type::ArrayEnd;
  }
  else
  {
    return false;
  }

  int depth = 1;
  while (depth > 0)
  {
    context.nextToken();
    if (context.error != Error::NoError)
    {
      return false;
    }
    if (context.token.value_type == start_type)
    {
      depth++;
    }
    else if (context.token.value_type == end_type)
    {
      depth--;
    }
  }

  return context.token.value_type == end_type && context.error == Error::NoError;
}
} // namespace Internal

template <typename T>
STFY_NODISCARD inline Error ParseContext::parseTo(T &to_type)
{
  missing_members.reserve(10);
  unassigned_required_members.reserve(10);
  error = tokenizer.nextTokens(&token, 1).second;
  if (error != STFY::Error::NoError)
    return error;
  error = TypeHandler<T>::to(to_type, *this);
  if (error != STFY::Error::NoError && tokenizer.errorContext().error == STFY::Error::NoError)
  {
    tokenizer.updateErrorContext(error);
  }
  return error;
}

struct SerializerContext
{
  SerializerContext(std::string &json_out_p)
    : serializer()
    , json_out(json_out_p)
    , last_pos(0)
  {
    if (json_out.empty())
      json_out.resize(4096);
    serializer.setBuffer(&json_out[0], json_out.size());
    serializer.setRequestBufferCallback([this](Serializer &serializer_p) {
      size_t end = this->json_out.size();
      this->json_out.resize(end * 2);
      serializer_p.setBuffer(&(this->json_out[0]) + end, end);
      this->last_pos = end;
    });
  }

  ~SerializerContext()
  {
    flush();
  }

  template <typename T>
  void serialize(const T &type)
  {
    STFY::Token token;
    STFY::TypeHandler<T>::from(type, token, serializer);
    flush();
  }

  void flush()
  {
    json_out.resize(last_pos + serializer.currentBuffer().used);
  }

  Serializer serializer;
  std::string &json_out;
  size_t last_pos;
};

template <typename T>
STFY_NODISCARD std::string serializeStruct(const T &from_type)
{
  std::string ret_string;
  SerializerContext serializeContext(ret_string);
  Token token;
  TypeHandler<T>::from(from_type, token, serializeContext.serializer);
  serializeContext.flush();
  return ret_string;
}

template <typename T>
STFY_NODISCARD std::string serializeStruct(const T &from_type, const SerializerOptions &options)
{
  std::string ret_string;
  SerializerContext serializeContext(ret_string);
  serializeContext.serializer.setOptions(options);
  Token token;
  TypeHandler<T>::from(from_type, token, serializeContext.serializer);
  serializeContext.flush();
  return ret_string;
}

template <>
struct TypeHandler<Error>
{
  static inline Error to(Error &to_type, ParseContext &context)
  {
    (void)to_type;
    (void)context;
    //		if (context.token.value_type == STFY::Type::Number) {
    //			int x;
    //			Error error = TypeHandler<int>::to(x, context);
    //			for (int i = 0; i < )
    //		}

    //        size_t level = 1;
    //        Error error = Error::NoError;
    //        while (error == STFY::Error::NoError && level) {
    //            error = context.nextToken();
    //            if (context.token.value_type == Type::ObjectStart)
    //                level++;
    //            else if (context.token.value_type == Type::ObjectEnd)
    //                level--;
    //        }

    //        context.tokenizer.copyIncludingValue(context.token, to_type.data);

    return Error::NoError;
  }

  static inline void from(const Error &from_type, Token &token, Serializer &serializer)
  {
    token.value_type = STFY::Type::String;
    if (from_type < STFY::Error::UserDefinedErrors)
    {
      token.value = DataRef(Internal::error_strings[(int)from_type]);
    }
    else
    {
      token.value = DataRef("UserDefinedError");
    }
    serializer.write(token);
  }
};

struct CallFunctionExecutionState
{
  explicit CallFunctionExecutionState(const std::string &name)
    : name(name)
    , error(Error::NoError)
  {
  }
  std::string name;
  SilentString context;
  Error error;
  SilentString error_string;
  SilentVector<std::string> missing_members;
  SilentVector<std::string> unassigned_required_members;
  SilentVector<CallFunctionExecutionState> child_states;
  STFY_OBJECT(STFY_MEMBER(name), STFY_MEMBER(context), STFY_MEMBER(error), STFY_MEMBER(error_string), STFY_MEMBER(missing_members),
            STFY_MEMBER(unassigned_required_members), STFY_MEMBER(child_states));
};

struct CallFunctionContext;

struct CallFunctionErrorContext
{
  CallFunctionErrorContext(CallFunctionContext &context)
    : context(context)
  {
  }

  Error setError(Error error, const std::string &error_string);
  Error setError(const std::string &error_string)
  {
    return setError(Error::UserDefinedErrors, error_string);
  }
  Error getLatestError() const;

private:
  CallFunctionContext &context;
};

struct CallFunctionContext
{
  CallFunctionContext(ParseContext &parser_context, Serializer &return_serializer)
    : parse_context(parser_context)
    , return_serializer(return_serializer)
    , error_context(*this)
  {
  }

  virtual ~CallFunctionContext()
  {
  }
  template <typename T>
  Error callFunctions(T &container);

  ParseContext &parse_context;
  Serializer &return_serializer;
  CallFunctionErrorContext error_context;
  std::vector<CallFunctionExecutionState> execution_list;
  std::string user_context;
  bool allow_missing = false;
  bool stop_execute_on_fail = false;
  void *user_handle = nullptr;

protected:
  virtual void beforeCallFunctions()
  {
  }
  virtual void afterCallFunctions()
  {
  }
};

inline Error CallFunctionErrorContext::setError(Error error, const std::string &errorString)
{
  context.parse_context.error = error;
  if (context.execution_list.size())
  {
    context.execution_list.back().error = error;
    context.execution_list.back().error_string.data = context.parse_context.tokenizer.makeErrorString();
  }
  context.parse_context.tokenizer.updateErrorContext(error, errorString);
  return error;
}

inline Error CallFunctionErrorContext::getLatestError() const
{
  return context.parse_context.error;
}

template <typename T, typename Ret, typename Arg, size_t NAME_COUNT, size_t TAKES_CONTEXT>
struct FunctionInfo
{
  typedef Ret (T::*Function)(Arg);
  typedef Ret returnType;
  DataRef name[NAME_COUNT];
  Function function;
};

/// \private
template <typename T, typename Ret, typename Arg, size_t NAME_COUNT>
struct FunctionInfo<T, Ret, Arg, NAME_COUNT, 1>
{
  typedef Ret (T::*Function)(Arg, CallFunctionErrorContext &);
  typedef Ret returnType;
  DataRef name[NAME_COUNT];
  Function function;
};

/// \private
template <typename T, typename Ret, typename Arg, size_t NAME_COUNT>
struct FunctionInfo<T, Ret, Arg, NAME_COUNT, 2>
{
  typedef Ret (T::*Function)(Arg, CallFunctionContext &);
  typedef Ret returnType;
  DataRef name[NAME_COUNT];
  Function function;
};

/// \private
template <typename T, typename Ret, size_t NAME_COUNT, size_t TAKES_CONTEXT>
struct FunctionInfo<T, Ret, void, NAME_COUNT, TAKES_CONTEXT>
{
  typedef Ret (T::*Function)(void);
  typedef Ret returnType;
  DataRef name[NAME_COUNT];
  Function function;
};

/// \private
template <typename T, typename Ret, size_t NAME_COUNT>
struct FunctionInfo<T, Ret, void, NAME_COUNT, 1>
{
  typedef Ret (T::*Function)(CallFunctionErrorContext &);
  typedef Ret returnType;
  DataRef name[NAME_COUNT];
  Function function;
};

/// \private
template <typename T, typename Ret, size_t NAME_COUNT>
struct FunctionInfo<T, Ret, void, NAME_COUNT, 2>
{
  typedef Ret (T::*Function)(CallFunctionContext &);
  typedef Ret returnType;
  DataRef name[NAME_COUNT];
  Function function;
};

/// \private
template <typename T, typename Ret, typename Arg, size_t NAME_SIZE, typename... Aliases>
constexpr FunctionInfo<T, Ret, Arg, sizeof...(Aliases) + 1, 0> makeFunctionInfo(const char (&name)[NAME_SIZE],
                                                                                Ret (T::*function)(Arg),
                                                                                Aliases... aliases)
{
  return {{DataRef(name), DataRef(aliases)...}, function};
}

/// \private
template <typename T, typename Ret, typename Arg, size_t NAME_SIZE, typename... Aliases>
constexpr FunctionInfo<T, Ret, Arg, sizeof...(Aliases) + 1, 1> makeFunctionInfo(
  const char (&name)[NAME_SIZE], Ret (T::*function)(Arg, CallFunctionErrorContext &), Aliases... aliases)
{
  return {{DataRef(name), DataRef(aliases)...}, function};
}

/// \private
template <typename T, typename Ret, typename Arg, size_t NAME_SIZE, typename... Aliases>
constexpr FunctionInfo<T, Ret, Arg, sizeof...(Aliases) + 1, 2> makeFunctionInfo(
  const char (&name)[NAME_SIZE], Ret (T::*function)(Arg, CallFunctionContext &), Aliases... aliases)
{
  return {{DataRef(name), DataRef(aliases)...}, function};
}

/// \private
template <typename T, typename Ret, size_t NAME_SIZE, typename... Aliases>
constexpr FunctionInfo<T, Ret, void, sizeof...(Aliases) + 1, 0> makeFunctionInfo(const char (&name)[NAME_SIZE],
                                                                                 Ret (T::*function)(void),
                                                                                 Aliases... aliases)
{
  return {{DataRef(name), DataRef(aliases)...}, function};
}

/// \private
template <typename T, typename Ret, size_t NAME_SIZE, typename... Aliases>
constexpr FunctionInfo<T, Ret, void, sizeof...(Aliases) + 1, 1> makeFunctionInfo(
  const char (&name)[NAME_SIZE], Ret (T::*function)(CallFunctionErrorContext &), Aliases... aliases)
{
  return {{DataRef(name), DataRef(aliases)...}, function};
}

/// \private
template <typename T, typename Ret, size_t NAME_SIZE, typename... Aliases>
constexpr FunctionInfo<T, Ret, void, sizeof...(Aliases) + 1, 2> makeFunctionInfo(
  const char (&name)[NAME_SIZE], Ret (T::*function)(CallFunctionContext &), Aliases... aliases)
{
  return {{DataRef(name), DataRef(aliases)...}, function};
}

namespace Internal
{
template <typename T>
struct HasStructifyFunctionContainer
{
  typedef char yes[1];
  typedef char no[2];

  template <typename C>
  static constexpr yes &test_in_base(typename C::template StructifyFunctionContainer<C> *);

  template <typename>
  static constexpr no &test_in_base(...);
};

template <typename STFY_BASE_CONTAINER_STRUCT_T, typename STFY_CONTAINER_STRUCT_T>
struct StructifyFunctionContainerDummy
{
  using TT = decltype(STFY_CONTAINER_STRUCT_T::template StructifyFunctionContainer<
                      STFY_CONTAINER_STRUCT_T>::stfy_static_meta_functions_info());
  using ST = decltype(
    STFY_CONTAINER_STRUCT_T::template StructifyFunctionContainer<STFY_CONTAINER_STRUCT_T>::stfy_static_meta_super_info());
  static const TT &stfy_static_meta_functions_info()
  {
    return STFY_CONTAINER_STRUCT_T::template StructifyFunctionContainer<
      STFY_CONTAINER_STRUCT_T>::stfy_static_meta_functions_info();
  }

  static const ST stfy_static_meta_super_info()
  {
    return STFY_CONTAINER_STRUCT_T::template StructifyFunctionContainer<
      STFY_CONTAINER_STRUCT_T>::stfy_static_meta_super_info();
  }
};

} // namespace Internal

#define STFY_FUNCTION(name) STFY::makeFunctionInfo(#name, &STFY_CONTAINER_STRUCT_T::name)
#define STFY_FUNCTION_ALIASES(name, ...) STFY::makeFunctionInfo(#name, &STFY_CONTAINER_STRUCT_T::name, __VA_ARGS__)
#define STFY_FUNCTION_WITH_NAME(member, name) STFY::makeFunctionInfo(name, &STFY_CONTAINER_STRUCT_T::member)
#define STFY_FUNCTION_WITH_NAME_ALIASES(member, name, ...)                                                               \
  STFY::makeFunctionInfo(name, &STFY_CONTAINER_STRUCT_T::member, __VA_ARGS__)

#define STFY_INTERNAL_MAP_APPLY_FUNCTION(m, first) m(#first, &STFY_CONTAINER_STRUCT_T::first)

#define STFY_INTERNAL_MAP_FUNCTION(m, first, ...)                                                                        \
  STFY_INTERNAL_MAP_APPLY_FUNCTION(m, first)                                                                             \
  STFY_INTERNAL_IF_ELSE(STFY_INTERNAL_HAS_MORE_THAN_ONE_ARGS(__VA_ARGS__))                                                 \
  (, STFY_INTERNAL_DEFER2(STFY_INTERNAL__MAP_FUNCTION)()(m, __VA_ARGS__))(, STFY_INTERNAL_MAP_APPLY_FUNCTION(m, __VA_ARGS__))

#define STFY_INTERNAL__MAP_FUNCTION() STFY_INTERNAL_MAP_FUNCTION

#define STFY_INTERNAL_MAKE_FUNCTIONS(...)                                                                                \
  STFY_INTERNAL_IF_ELSE(STFY_INTERNAL_HAS_MORE_THAN_ONE_ARGS(__VA_ARGS__))                                                 \
  (STFY_INTERNAL_EXPAND(STFY_INTERNAL_EVAL(STFY_INTERNAL_MAP_FUNCTION(STFY::makeFunctionInfo, __VA_ARGS__))))(                 \
    STFY_INTERNAL_MAP_APPLY_FUNCTION(STFY::makeFunctionInfo, __VA_ARGS__))

#define STFY_FUNCTION_CONTAINER_INTERNAL_IMPL(super_list, function_list)                                                 \
  template <typename STFY_CONTAINER_STRUCT_T>                                                                            \
  struct StructifyFunctionContainer                                                                                   \
  {                                                                                                                    \
    using TT = decltype(function_list);                                                                                \
    static const TT &stfy_static_meta_functions_info()                                                                   \
    {                                                                                                                  \
      static auto ret = function_list;                                                                                 \
      return ret;                                                                                                      \
    }                                                                                                                  \
    static const decltype(super_list) stfy_static_meta_super_info()                                                      \
    {                                                                                                                  \
      return super_list;                                                                                               \
    }                                                                                                                  \
  }

#define STFY_FUNCTION_CONTAINER_EXTERNAL_INTERNAL_IMPL(Type, super_list, function_list)                                  \
  namespace STFY                                                                                                         \
  {                                                                                                                    \
  namespace Internal                                                                                                   \
  {                                                                                                                    \
  template <typename STFY_CONTAINER_STRUCT_T>                                                                            \
  struct StructifyFunctionContainerDummy<Type, STFY_CONTAINER_STRUCT_T>                                                 \
  {                                                                                                                    \
    using TT = decltype(function_list);                                                                                \
    static const TT &stfy_static_meta_functions_info()                                                                   \
    {                                                                                                                  \
      static auto ret = function_list;                                                                                 \
      return ret;                                                                                                      \
    }                                                                                                                  \
    static const decltype(super_list) stfy_static_meta_super_info()                                                      \
    {                                                                                                                  \
      return super_list;                                                                                               \
    }                                                                                                                  \
  };                                                                                                                   \
  }                                                                                                                    \
  }

#define STFY_FUNC_OBJ(...)                                                                                               \
  STFY_FUNCTION_CONTAINER_INTERNAL_IMPL(STFY::makeTuple(), STFY::makeTuple(STFY_INTERNAL_MAKE_FUNCTIONS(__VA_ARGS__)))
#define STFY_FUNCTION_CONTAINER(...) STFY_FUNCTION_CONTAINER_INTERNAL_IMPL(STFY::makeTuple(), STFY::makeTuple(__VA_ARGS__))
#define STFY_FUNC_OBJ_SUPER(super_list, ...)                                                                             \
  STFY_FUNCTION_CONTAINER_INTERNAL_IMPL(super_list, STFY::makeTuple(STFY_INTERNAL_MAKE_FUNCTIONS(__VA_ARGS__)))
#define STFY_FUNCTION_CONTAINER_WITH_SUPER(super_list, ...)                                                              \
  STFY_FUNCTION_CONTAINER_INTERNAL_IMPL(super_list, STFY::makeTuple(__VA_ARGS__))
#define STFY_FUNCTION_CONTAINER_WITH_SUPER_WITHOUT_MEMBERS(super_list)                                                   \
  STFY_FUNCTION_CONTAINER_INTERNAL_IMPL(super_list, STFY::makeTuple())

#define STFY_FUNC_OBJ_EXTERNAL(Type, ...)                                                                                \
  STFY_FUNCTION_CONTAINER_EXTERNAL_INTERNAL_IMPL(Type, STFY::makeTuple(),                                                  \
                                               STFY::makeTuple(STFY_INTERNAL_MAKE_FUNCTIONS(__VA_ARGS__)))
#define STFY_FUNCTION_CONTAINER_EXTERNAL(Type, ...)                                                                      \
  STFY_FUNCTION_CONTAINER_EXTERNAL_INTERNAL_IMPL(Type, STFY::makeTuple(), STFY::makeTuple(__VA_ARGS__))
#define STFY_FUNC_OBJ_EXTERNAL_SUPER(Type, super_list, ...)                                                              \
  STFY_FUNCTION_CONTAINER_EXTERNAL_INTERNAL_IMPL(Type, super_list, STFY::makeTuple(STFY_INTERNAL_MAKE_FUNCTIONS(__VA_ARGS__)))
#define STFY_FUNCTION_CONTAINER_EXTERNAL_WITH_SUPER(Type, super_list, ...)                                               \
  STFY_FUNCTION_CONTAINER_EXTERNAL_INTERNAL_IMPL(Type, super_list, STFY::makeTuple(__VA_ARGS__))

#define STFY_FUNCTION_CONTAINER_EXTERNAL_WITH_SUPER_WITHOUT_MEMBERS(Type, super_list)                                    \
  STFY_FUNCTION_CONTAINER_EXTERNAL_INTERNAL_IMPL(Type, super_list, STFY::makeTuple())

#if !defined(__clang__) && defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif

namespace Internal
{
template <typename T, typename U, typename Ret, typename Arg, size_t NAME_COUNT, size_t TAKES_CONTEXT>
struct FunctionCaller
{
  static Error callFunctionAndSerializeReturn(T &container,
                                              FunctionInfo<U, Ret, Arg, NAME_COUNT, TAKES_CONTEXT> &functionInfo,
                                              CallFunctionContext &context)
  {
    typedef typename std::remove_reference<Arg>::type NonRefArg;
    typedef typename std::remove_cv<NonRefArg>::type PureArg;
    PureArg arg;
    context.parse_context.error = TypeHandler<PureArg>::to(arg, context.parse_context);
    if (context.parse_context.error != Error::NoError)
      return context.parse_context.error;

    Token token;
    TypeHandler<Ret>::from((container.*functionInfo.function)(arg), token, context.return_serializer);
    return Error::NoError;
  }
};

template <typename T, typename U, typename Ret, typename Arg, size_t NAME_COUNT>
struct FunctionCaller<T, U, Ret, Arg, NAME_COUNT, 1>
{
  static Error callFunctionAndSerializeReturn(T &container, FunctionInfo<U, Ret, Arg, NAME_COUNT, 1> &functionInfo,
                                              CallFunctionContext &context)
  {
    typedef typename std::remove_reference<Arg>::type NonRefArg;
    typedef typename std::remove_cv<NonRefArg>::type PureArg;
    PureArg arg;
    context.parse_context.error = TypeHandler<PureArg>::to(arg, context.parse_context);
    if (context.parse_context.error != Error::NoError)
      return context.parse_context.error;

    Token token;
    Ret ret = (container.*functionInfo.function)(arg, context.error_context);
    if (context.execution_list.back().error == Error::NoError)
      TypeHandler<Ret>::from(ret, token, context.return_serializer);
    return context.execution_list.back().error;
  }
};

template <typename T, typename U, typename Ret, typename Arg, size_t NAME_COUNT>
struct FunctionCaller<T, U, Ret, Arg, NAME_COUNT, 2>
{
  static Error callFunctionAndSerializeReturn(T &container, FunctionInfo<U, Ret, Arg, NAME_COUNT, 2> &functionInfo,
                                              CallFunctionContext &context)
  {
    typedef typename std::remove_reference<Arg>::type NonRefArg;
    typedef typename std::remove_cv<NonRefArg>::type PureArg;
    PureArg arg;
    context.parse_context.error = TypeHandler<PureArg>::to(arg, context.parse_context);
    if (context.parse_context.error != Error::NoError)
      return context.parse_context.error;

    Token token;
    Ret ret = (container.*functionInfo.function)(arg, context);
    if (context.execution_list.back().error == Error::NoError)
      TypeHandler<Ret>::from(ret, token, context.return_serializer);
    return context.execution_list.back().error;
  }
};

template <typename T, typename U, typename Arg, size_t NAME_COUNT, size_t TAKES_CONTEXT>
struct FunctionCaller<T, U, void, Arg, NAME_COUNT, TAKES_CONTEXT>
{
  static Error callFunctionAndSerializeReturn(T &container,
                                              FunctionInfo<U, void, Arg, NAME_COUNT, TAKES_CONTEXT> &functionInfo,
                                              CallFunctionContext &context)
  {
    typedef typename std::remove_reference<Arg>::type NonRefArg;
    typedef typename std::remove_cv<NonRefArg>::type PureArg;
    PureArg arg;
    context.parse_context.error = TypeHandler<PureArg>::to(arg, context.parse_context);
    if (context.parse_context.error != Error::NoError)
      return context.parse_context.error;

    (container.*functionInfo.function)(arg);
    return Error::NoError;
  }
};

template <typename T, typename U, typename Arg, size_t NAME_COUNT>
struct FunctionCaller<T, U, void, Arg, NAME_COUNT, 1>
{
  static Error callFunctionAndSerializeReturn(T &container, FunctionInfo<U, void, Arg, NAME_COUNT, 1> &functionInfo,
                                              CallFunctionContext &context)
  {
    typedef typename std::remove_reference<Arg>::type NonRefArg;
    typedef typename std::remove_cv<NonRefArg>::type PureArg;
    PureArg arg;
    context.parse_context.error = TypeHandler<PureArg>::to(arg, context.parse_context);
    if (context.parse_context.error != Error::NoError)
      return context.parse_context.error;

    (container.*functionInfo.function)(arg, context.error_context);
    return context.execution_list.back().error;
  }
};

template <typename T, typename U, typename Arg, size_t NAME_COUNT>
struct FunctionCaller<T, U, void, Arg, NAME_COUNT, 2>
{
  static Error callFunctionAndSerializeReturn(T &container, FunctionInfo<U, void, Arg, NAME_COUNT, 2> &functionInfo,
                                              CallFunctionContext &context)
  {
    typedef typename std::remove_reference<Arg>::type NonRefArg;
    typedef typename std::remove_cv<NonRefArg>::type PureArg;
    PureArg arg;
    context.parse_context.error = TypeHandler<PureArg>::to(arg, context.parse_context);
    if (context.parse_context.error != Error::NoError)
      return context.parse_context.error;

    (container.*functionInfo.function)(arg, context);
    return context.execution_list.back().error;
  }
};

static inline void checkValidVoidParameter(CallFunctionContext &context)
{
  if (context.parse_context.token.value_type != Type::Null &&
      context.parse_context.token.value_type != Type::ArrayStart &&
      context.parse_context.token.value_type != Type::ObjectStart &&
      context.parse_context.token.value_type != Type::Bool)
  {
    // what to do
    fprintf(stderr, "Passing data arguments to a void function\n");
  }
  skipArrayOrObject(context.parse_context);
}

template <typename T, typename U, typename Ret, size_t NAME_COUNT, size_t TAKES_CONTEXT>
struct FunctionCaller<T, U, Ret, void, NAME_COUNT, TAKES_CONTEXT>
{
  static Error callFunctionAndSerializeReturn(T &container,
                                              FunctionInfo<U, Ret, void, NAME_COUNT, TAKES_CONTEXT> &functionInfo,
                                              CallFunctionContext &context)
  {
    checkValidVoidParameter(context);
    if (context.parse_context.error != Error::NoError)
      return context.parse_context.error;
    Token token;
    TypeHandler<Ret>::from((container.*functionInfo.function)(), token, context.return_serializer);
    return Error::NoError;
  }
};

template <typename T, typename U, typename Ret, size_t NAME_COUNT>
struct FunctionCaller<T, U, Ret, void, NAME_COUNT, 1>
{
  static Error callFunctionAndSerializeReturn(T &container, FunctionInfo<U, Ret, void, NAME_COUNT, 1> &functionInfo,
                                              CallFunctionContext &context)
  {
    checkValidVoidParameter(context);
    if (context.parse_context.error != Error::NoError)
      return context.parse_context.error;

    Token token;
    Ret ret = (container.*functionInfo.function)(context.error_context);
    if (context.execution_list.back().error == Error::NoError)
      TypeHandler<Ret>::from(ret, token, context.return_serializer);
    return context.execution_list.back().error;
  }
};

template <typename T, typename U, typename Ret, size_t NAME_COUNT>
struct FunctionCaller<T, U, Ret, void, NAME_COUNT, 2>
{
  static Error callFunctionAndSerializeReturn(T &container, FunctionInfo<U, Ret, void, NAME_COUNT, 2> &functionInfo,
                                              CallFunctionContext &context)
  {
    checkValidVoidParameter(context);
    if (context.parse_context.error != Error::NoError)
      return context.parse_context.error;

    Token token;
    Ret ret = (container.*functionInfo.function)(context);
    if (context.execution_list.back().error == Error::NoError)
      TypeHandler<Ret>::from(ret, token, context.return_serializer);
    return context.execution_list.back().error;
  }
};

template <typename T, typename U, size_t NAME_COUNT, size_t TAKES_CONTEXT>
struct FunctionCaller<T, U, void, void, NAME_COUNT, TAKES_CONTEXT>
{
  static Error callFunctionAndSerializeReturn(T &container,
                                              FunctionInfo<U, void, void, NAME_COUNT, TAKES_CONTEXT> &functionInfo,
                                              CallFunctionContext &context)
  {
    checkValidVoidParameter(context);
    if (context.parse_context.error != Error::NoError)
      return context.parse_context.error;

    (container.*functionInfo.function)();
    return Error::NoError;
  }
};

template <typename T, typename U, size_t NAME_COUNT>
struct FunctionCaller<T, U, void, void, NAME_COUNT, 1>
{
  static Error callFunctionAndSerializeReturn(T &container, FunctionInfo<U, void, void, NAME_COUNT, 1> &functionInfo,
                                              CallFunctionContext &context)
  {
    checkValidVoidParameter(context);
    if (context.parse_context.error != Error::NoError)
      return context.parse_context.error;

    (container.*functionInfo.function)(context.error_context);
    return context.execution_list.back().error;
  }
};

template <typename T, typename U, size_t NAME_COUNT>
struct FunctionCaller<T, U, void, void, NAME_COUNT, 2>
{
  static Error callFunctionAndSerializeReturn(T &container, FunctionInfo<U, void, void, NAME_COUNT, 2> &functionInfo,
                                              CallFunctionContext &context)
  {
    checkValidVoidParameter(context);
    if (context.parse_context.error != Error::NoError)
      return context.parse_context.error;

    (container.*functionInfo.function)(context);
    return context.execution_list.back().error;
  }
};
} // namespace Internal

#if !defined(__clang__) && defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

template <typename T, typename U, typename Ret, typename Arg, size_t NAME_COUNT, size_t TAKES_CONTEXT>
Error matchAndCallFunction(T &container, CallFunctionContext &context,
                           FunctionInfo<U, Ret, Arg, NAME_COUNT, TAKES_CONTEXT> &functionInfo, bool primary)
{
  if (primary && context.parse_context.token.name.size == functionInfo.name[0].size &&
      memcmp(functionInfo.name[0].data, context.parse_context.token.name.data, functionInfo.name[0].size) == 0)
  {
    return Internal::FunctionCaller<T, U, Ret, Arg, NAME_COUNT, TAKES_CONTEXT>::callFunctionAndSerializeReturn(
      container, functionInfo, context);
  }
  else if (!primary)
  {
    for (size_t i = 1; i < NAME_COUNT; i++)
    {
      if (context.parse_context.token.name.size == functionInfo.name[i].size &&
          memcmp(functionInfo.name[i].data, context.parse_context.token.name.data, functionInfo.name[i].size) == 0)
      {
        return Internal::FunctionCaller<T, U, Ret, Arg, NAME_COUNT, TAKES_CONTEXT>::callFunctionAndSerializeReturn(
          container, functionInfo, context);
      }
    }
  }
  return Error::MissingFunction;
}

namespace Internal
{
template <typename T, size_t INDEX>
struct FunctionalSuperRecursion
{
  static Error callFunction(T &container, CallFunctionContext &context, bool primary);
};

template <typename T, size_t SIZE>
struct StartFunctionalSuperRecursion
{
  static Error callFunction(T &container, CallFunctionContext &context, bool primary)
  {
    return FunctionalSuperRecursion<T, SIZE - 1>::callFunction(container, context, primary);
  }
};
template <typename T>
struct StartFunctionalSuperRecursion<T, 0>
{
  static Error callFunction(T &container, CallFunctionContext &context, bool primary)
  {
    STFY_UNUSED(container);
    STFY_UNUSED(context);
    STFY_UNUSED(primary);
    return Error::MissingFunction;
  }
};

template <typename T, typename Functions, size_t INDEX>
struct FunctionObjectTraverser
{
  static Error call(T &container, CallFunctionContext &context, Functions &functions, bool primary)
  {
    auto function = functions.template get<INDEX>();
    Error error = matchAndCallFunction(container, context, function, primary);
    if (error == Error::NoError)
      return Error::NoError;
    if (error != Error::MissingFunction)
      return context.parse_context.error;
    return FunctionObjectTraverser<T, Functions, INDEX - 1>::call(container, context, functions, primary);
  }
};

template <typename T, typename Functions>
struct FunctionObjectTraverser<T, Functions, 0>
{
  static Error call(T &container, CallFunctionContext &context, Functions &functions, bool primary)
  {
    auto function = functions.template get<0>();
    Error error = matchAndCallFunction(container, context, function, primary);
    if (error == Error::NoError)
      return Error::NoError;
    if (error != Error::MissingFunction)
      return error;
    using SuperMeta = decltype(Internal::template StructifyFunctionContainerDummy<T, T>::stfy_static_meta_super_info());
    return StartFunctionalSuperRecursion<T, SuperMeta::size>::callFunction(container, context, primary);
  }
};

template <typename T, typename Functions>
struct FunctionObjectTraverser<T, Functions, size_t(-1)>
{
  static Error call(T &container, CallFunctionContext &context, Functions &, bool primary)
  {
    using SuperMeta = decltype(Internal::template StructifyFunctionContainerDummy<T, T>::stfy_static_meta_super_info());
    return StartFunctionalSuperRecursion<T, SuperMeta::size>::callFunction(container, context, primary);
  }
};

static inline void add_error(CallFunctionExecutionState &executionState, ParseContext &context)
{
  executionState.error = context.error;
  if (context.error != Error::NoError)
  {
    if (context.tokenizer.errorContext().custom_message.empty())
      context.tokenizer.updateErrorContext(context.error);
    executionState.error_string.data = context.tokenizer.makeErrorString();
  }
  if (context.missing_members.size())
    std::swap(executionState.missing_members.data, context.missing_members);
  if (context.unassigned_required_members.size())
    std::swap(executionState.unassigned_required_members.data, context.unassigned_required_members);
}
} // namespace Internal

namespace Internal
{
typedef void (CallFunctionContext::*AfterCallFunction)();

struct RAICallFunctionOnExit
{
  RAICallFunctionOnExit(CallFunctionContext &context, AfterCallFunction after)
    : context(context)
    , after(after)
  {
  }
  ~RAICallFunctionOnExit()
  {
    (context.*after)();
  }
  CallFunctionContext &context;
  AfterCallFunction after;
};
} // namespace Internal

namespace Internal
{
struct ArrayEndWriter
{
  ArrayEndWriter(Serializer &serializer, Token &token)
    : serializer(serializer)
    , token(token)
  {
  }

  ~ArrayEndWriter()
  {
    token.value_type = Type::ArrayEnd;
    token.value = DataRef("]");
    serializer.write(token);
  }

  Serializer &serializer;
  Token &token;
};
} // namespace Internal

template <typename T>
inline Error CallFunctionContext::callFunctions(T &container)
{
  beforeCallFunctions();
  Internal::RAICallFunctionOnExit callOnExit(*this, &CallFunctionContext::afterCallFunctions);
  STFY::Error error = parse_context.nextToken();
  if (error != STFY::Error::NoError)
    return error;
  if (parse_context.token.value_type != STFY::Type::ObjectStart)
  {
    return error_context.setError(Error::ExpectedObjectStart, "Can only call functions on objects with members");
  }
  error = parse_context.nextToken();
  if (error != STFY::Error::NoError)
    return error;
  Token token;
  token.value_type = Type::ArrayStart;
  token.value = DataRef("[");
  Internal::ArrayEndWriter endWriter(return_serializer, token);
  return_serializer.write(token);
  auto &functions = Internal::StructifyFunctionContainerDummy<T, T>::stfy_static_meta_functions_info();
  using FunctionsType = typename std::remove_reference<decltype(functions)>::type;
  while (parse_context.token.value_type != STFY::Type::ObjectEnd)
  {
    parse_context.tokenizer.pushScope(parse_context.token.value_type);
    execution_list.push_back(
      CallFunctionExecutionState(std::string(parse_context.token.name.data, parse_context.token.name.size)));
    execution_list.back().context.data = user_context;
    error = Internal::FunctionObjectTraverser<T, FunctionsType, FunctionsType::size - 1>::call(container, *this,
                                                                                               functions, true);
    if (error == Error::MissingFunction)
      error = Internal::FunctionObjectTraverser<T, FunctionsType, FunctionsType::size - 1>::call(container, *this,
                                                                                                 functions, false);
    if (error != Error::NoError)
    {
      assert(error == parse_context.error || parse_context.error == Error::NoError);
      parse_context.error = error;
    }
    Internal::add_error(execution_list.back(), parse_context);
    parse_context.tokenizer.goToEndOfScope(parse_context.token);
    parse_context.tokenizer.popScope();
    if (error == Error::MissingFunction && allow_missing)
      error = Error::NoError;
    if (stop_execute_on_fail && error != Error::NoError)
      return error;

    error = parse_context.nextToken();
    if (error != STFY::Error::NoError)
      return error;
  }

  return Error::NoError;
}

struct DefaultCallFunctionContext : public CallFunctionContext
{
  DefaultCallFunctionContext(std::string &json_out)
    : CallFunctionContext(p_context, s_context.serializer)//-V1050 The super class only store the reference so we don't mind its not initialized
    , s_context(json_out)
  {
  }

  DefaultCallFunctionContext(const char *data, size_t size, std::string &json_out)
    : CallFunctionContext(p_context, s_context.serializer)//-V1050 The super class only store the reference so we don't mind its not initialized
    , p_context(data, size)
    , s_context(json_out)
  {
  }

  template <size_t SIZE>
  DefaultCallFunctionContext(const char (&data)[SIZE], std::string &json_out)
    : CallFunctionContext(p_context, s_context.serializer)
    , p_context(data)
    , s_context(json_out)
  {
  }

  ParseContext p_context;
  SerializerContext s_context;

protected:
  void afterCallFunctions()
  {
    s_context.flush();
  }
};
namespace Internal
{
template <typename T, size_t INDEX>
Error FunctionalSuperRecursion<T, INDEX>::callFunction(T &container, CallFunctionContext &context, bool primary)
{
  using SuperMeta = decltype(Internal::template StructifyFunctionContainerDummy<T, T>::stfy_static_meta_super_info());
  using Super = typename TypeAt<INDEX, SuperMeta>::type::type;
  auto &functions = Internal::template StructifyFunctionContainerDummy<Super, Super>::stfy_static_meta_functions_info();
  using FunctionsType = typename std::remove_reference<decltype(functions)>::type;
  Error error = FunctionObjectTraverser<Super, FunctionsType, FunctionsType::size - 1>::call(container, context,
                                                                                             functions, primary);
  if (error != Error::MissingFunction)
    return error;

  return FunctionalSuperRecursion<T, INDEX - 1>::callFunction(container, context, primary);
}

template <typename T>
struct FunctionalSuperRecursion<T, 0>
{
  static Error callFunction(T &container, CallFunctionContext &context, bool primary)
  {
    using SuperMeta = decltype(Internal::template StructifyFunctionContainerDummy<T, T>::stfy_static_meta_super_info());
    using Super = typename TypeAt<0, SuperMeta>::type::type;
    auto &functions =
      Internal::template StructifyFunctionContainerDummy<Super, Super>::stfy_static_meta_functions_info();
    using FunctionsType = typename std::remove_reference<decltype(functions)>::type;
    return FunctionObjectTraverser<Super, FunctionsType, FunctionsType::size - 1>::call(container, context, functions,
                                                                                        primary);
  }
};
} // namespace Internal
namespace Internal
{
enum class ParseEnumStringState
{
  FindingNameStart,
  FindingNameEnd,
  FindingSeperator
};
template <size_t N>
void populateEnumNames(std::vector<DataRef> &names, const char (&data)[N])
{
  size_t name_starts_at = 0;
  ParseEnumStringState state = ParseEnumStringState::FindingNameStart;
  for (size_t i = 0; i < N; i++)
  {
    char c = data[i];
    assert(c != '=');
    switch (state)
    {
    case ParseEnumStringState::FindingNameStart:
      if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
      {
        name_starts_at = i;
        state = ParseEnumStringState::FindingNameEnd;
      }
      break;
    case ParseEnumStringState::FindingNameEnd:
      if (c == '\0' || c == '\t' || c == '\n' || c == '\r' || c == ' ' || c == ',')
      {
        names.push_back(DataRef(data + name_starts_at, i - name_starts_at));
        state = c == ',' ? ParseEnumStringState::FindingNameStart : ParseEnumStringState::FindingSeperator;
      }
      break;
    case ParseEnumStringState::FindingSeperator:
      if (c == ',')
        state = ParseEnumStringState::FindingNameStart;
      break;
    }
  }
}
} // namespace Internal
} // namespace STFY


#define STFY_ENUM(name, ...)                                                                                             \
  enum class name                                                                                                      \
  {                                                                                                                    \
    __VA_ARGS__                                                                                                        \
  };                                                                                                                   \
  struct js_##name##_string_struct                                                                                     \
  {                                                                                                                    \
    template <size_t N>                                                                                                \
    explicit js_##name##_string_struct(const char (&data)[N])                                                          \
    {                                                                                                                  \
      STFY::Internal::populateEnumNames(_strings, data);                                                                 \
    }                                                                                                                  \
    std::vector<STFY::DataRef> _strings;                                                                                 \
                                                                                                                       \
    static const std::vector<STFY::DataRef> &strings()                                                                   \
    {                                                                                                                  \
      static js_##name##_string_struct ret(#__VA_ARGS__);                                                              \
      return ret._strings;                                                                                             \
    }                                                                                                                  \
  };

#define STFY_ENUM_DECLARE_STRING_PARSER(name)                                                                            \
  namespace STFY                                                                                                         \
  {                                                                                                                    \
  template <>                                                                                                          \
  struct TypeHandler<name>                                                                                             \
  {                                                                                                                    \
    static inline Error to(name &to_type, ParseContext &context)                                                       \
    {                                                                                                                  \
      return Internal::EnumHandler<name, js_##name##_string_struct>::to(to_type, context);                             \
    }                                                                                                                  \
    static inline void from(const name &from_type, Token &token, Serializer &serializer)                               \
    {                                                                                                                  \
      return Internal::EnumHandler<name, js_##name##_string_struct>::from(from_type, token, serializer);               \
    }                                                                                                                  \
  };                                                                                                                   \
  }

#define STFY_ENUM_NAMESPACE_DECLARE_STRING_PARSER(ns, name)                                                              \
  namespace STFY                                                                                                         \
  {                                                                                                                    \
  template <>                                                                                                          \
  struct TypeHandler<ns::name>                                                                                         \
  {                                                                                                                    \
    static inline Error to(ns::name &to_type, ParseContext &context)                                                   \
    {                                                                                                                  \
      return Internal::EnumHandler<ns::name, ns::js_##name##_string_struct>::to(to_type, context);                     \
    }                                                                                                                  \
    static inline void from(const ns::name &from_type, Token &token, Serializer &serializer)                           \
    {                                                                                                                  \
      return Internal::EnumHandler<ns::name, ns::js_##name##_string_struct>::from(from_type, token, serializer);       \
    }                                                                                                                  \
  };                                                                                                                   \
  }

#define STFY_ENUM_DECLARE_VALUE_PARSER(name)                                                                             \
  namespace STFY                                                                                                         \
  {                                                                                                                    \
  template <>                                                                                                          \
  struct TypeHandler<name>                                                                                             \
  {                                                                                                                    \
    typedef std::underlying_type<name>::type utype;                                                                    \
    static inline Error to(name &to_type, ParseContext &context)                                                       \
    {                                                                                                                  \
      utype to_value;                                                                                                  \
      STFY::Error result = TypeHandler<utype>::to(to_value, context);                                                    \
      if (result == STFY::Error::NoError)                                                                                \
        to_type = static_cast<name>(to_value);                                                                         \
      return result;                                                                                                   \
    }                                                                                                                  \
    static inline void from(const name &from_type, Token &token, Serializer &serializer)                               \
    {                                                                                                                  \
        const utype from_value = static_cast<utype>(from_type);                                                        \
        TypeHandler<utype>::from(from_value, token, serializer);                                                       \
    }                                                                                                                  \
  };                                                                                                                   \
  }

#define STFY_ENUM_NAMESPACE_DECLARE_VALUE_PARSER(ns, name)                                                               \
  namespace STFY                                                                                                         \
  {                                                                                                                    \
  template <>                                                                                                          \
  struct TypeHandler<ns::name>                                                                                         \
  {                                                                                                                    \
    typedef std::underlying_type<ns::name>::type utype;                                                                \
    static inline Error to(ns::name &to_type, ParseContext &context)                                                   \
    {                                                                                                                  \
      utype to_value;                                                                                                  \
      STFY::Error result = TypeHandler<utype>::to(to_value, context);                                                    \
      if (result == STFY::Error::NoError)                                                                                \
        to_type = static_cast<ns::name>(to_value);                                                                     \
      return result;                                                                                                   \
    }                                                                                                                  \
    static inline void from(const ns::name &from_type, Token &token, Serializer &serializer)                           \
    {                                                                                                                  \
        const utype from_value = static_cast<utype>(from_type);                                                        \
        TypeHandler<utype>::from(from_value, token, serializer);                                                       \
    }                                                                                                                  \
  };                                                                                                                   \
  }


namespace STFY
{
template <typename T, typename Enable>
inline Error TypeHandler<T, Enable>::to(T &to_type, ParseContext &context)
{
  if (context.token.value_type != STFY::Type::ObjectStart)
    return Error::ExpectedObjectStart;
  Error error = context.tokenizer.nextTokens(&context.token, 1).second;
  if (error != STFY::Error::NoError)
    return error;
  auto members = Internal::StructifyBaseDummy<T, T>::stfy_static_meta_data_info();
  using MembersType = decltype(members);
  bool assigned_members[Internal::memberCount<T, 0>()];
  memset(assigned_members, 0, sizeof(assigned_members));
  while (context.token.value_type != STFY::Type::ObjectEnd)

  {
    DataRef token_name = context.token.name;
    error = Internal::MemberChecker<T, MembersType, 0, MembersType::size - 1>::unpackMembers(to_type, members, context,
                                                                                             true, assigned_members);
    if (error == Error::MissingPropertyMember)
      error = Internal::MemberChecker<T, MembersType, 0, MembersType::size - 1>::unpackMembers(
        to_type, members, context, false, assigned_members);
    if (error == Error::MissingPropertyMember)
    {

      if (context.track_member_assignement_state)
        context.missing_members.emplace_back(token_name.data, token_name.data + token_name.size);
      if (context.allow_missing_members)
      {
        Internal::skipArrayOrObject(context);
        if (context.error != Error::NoError)
          return context.error;
      }
      else
      {
        return error;
      }
    }
    else if (error != Error::NoError)
    {
      return error;
    }
    context.nextToken();
    if (context.error != Error::NoError)
      return context.error;
  }
  std::vector<std::string> unassigned_required_members;
  error = Internal::MemberChecker<T, MembersType, 0, MembersType::size - 1>::verifyMembers(
    members, assigned_members, context.track_member_assignement_state, unassigned_required_members, "");
  if (error == Error::UnassignedRequiredMember)
  {
    if (context.track_member_assignement_state)
      context.unassigned_required_members.insert(context.unassigned_required_members.end(),
                                                 unassigned_required_members.begin(),
                                                 unassigned_required_members.end());
    if (context.allow_unasigned_required_members)
      error = Error::NoError;
  }
  return error;
}

template <typename T, typename Enable>
void TypeHandler<T, Enable>::from(const T &from_type, Token &token, Serializer &serializer)
{
  static const char objectStart[] = "{";
  static const char objectEnd[] = "}";
  token.value_type = Type::ObjectStart;
  token.value = DataRef(objectStart);
  serializer.write(token);
  auto members = Internal::StructifyBaseDummy<T, T>::stfy_static_meta_data_info();
  using MembersType = decltype(members);
  Internal::MemberChecker<T, MembersType, 0, MembersType::size - 1>::serializeMembers(from_type, members, token,
                                                                                      serializer, "");
  token.name.size = 0;
  token.name.data = "";
  token.name_type = Type::String;
  token.value_type = Type::ObjectEnd;
  token.value = DataRef(objectEnd);
  serializer.write(token);
}

namespace Internal
{
template <typename T, typename F>
struct EnumHandler
{
  static inline Error to(T &to_type, ParseContext &context)
  {
    if (context.token.value_type == Type::String)
    {
      auto &strings = F::strings();
      for (size_t i = 0; i < strings.size(); i++)
      {
        const DataRef &ref = strings[i];
        if (ref.size == context.token.value.size)
        {
          if (memcmp(ref.data, context.token.value.data, ref.size) == 0)
          {
            to_type = static_cast<T>(i);
            return Error::NoError;
          }
        }
      }
    }
    else if (context.token.value_type == Type::Number)
    {
      using enum_int_t = typename std::underlying_type<T>::type;
      enum_int_t tmp;
      auto err = TypeHandler<enum_int_t>::to(tmp, context);
      if (err != Error::NoError)
        return err;
      to_type = static_cast<T>(tmp);
      return Error::NoError;
    }

    return Error::IllegalDataValue;
  }

  static inline void from(const T &from_type, Token &token, Serializer &serializer)
  {
    size_t i = static_cast<size_t>(from_type);
    token.value = F::strings()[i];
    token.value_type = Type::String;
    serializer.write(token);
  }
};
} // namespace Internal

namespace Internal
{
static void push_back_escape(char current_char, std::string &to_type)
{
  static const char escaped_table[] = {'b', 'f', 'n', 'r', 't', '\"', '\\', '/'};
  static const char replace_table[] = {'\b', '\f', '\n', '\r', '\t', '\"', '\\', '/'};
  static_assert(sizeof(escaped_table) == sizeof(replace_table), "Static tables have to be the same.");
  const char *it = static_cast<const char *>(memchr(escaped_table, current_char, sizeof(escaped_table)));
  if (it)
  {
    to_type.push_back(replace_table[(it - escaped_table)]);
  }
  else
  {
    to_type.push_back('\\');
    to_type.push_back(current_char);
  }
}

static void handle_json_escapes_in(const DataRef &ref, std::string &to_type)
{
  to_type.reserve(ref.size);
  const char *it = ref.data;
  size_t size = ref.size;
  while (size)
  {
    const char *next_it = static_cast<const char *>(memchr(it, '\\', size));
    if (!next_it)
    {
      to_type.insert(to_type.end(), it, it + size);
      break;
    }
    to_type.insert(to_type.end(), it, next_it);
    size -= next_it - it;
    if (!size)
    {
      break;
    }
    size -= 2;
    const char current_char = *(next_it + 1);
    // we assume utf-8 encoding when this notation is used and parsing into std::string
    if (current_char == 'u') // hexadecimal escaped unicode character
    {
      // first convert hex ascii digits to values between 0 and 15, then create
      // UTF-8 bit patterns according to https://en.wikipedia.org/wiki/UTF-8
      bool ok = (size >= 4);
      unsigned char hex[4];
      for (int k = 0; ok && k < 4; k++)
      {
        const char d = *(next_it + k + 2);
        if (d >= '0' && d <= '9')
          hex[k] = (d - '0');
        else if (d >= 'A' && d <= 'F')
          hex[k] = (d - 'A') + 10;
        else if (d >= 'a' && d <= 'f')
          hex[k] = (d - 'a') + 10;
        else
          ok = false; // stop parsing and revert to fallback
      }
      if (ok)
      {
        if (hex[0] || hex[1] & 0x08)
        {
          // code points: 0x0800 .. 0xffff
          to_type.push_back(0xd0 | hex[0]);
          to_type.push_back(0x80 | (hex[1] << 2) | ((hex[2] & 0x0c) >> 2));
          to_type.push_back(0x80 | ((hex[2] & 0x03) << 4) | hex[3]);
        }
        else if (hex[1] || hex[2] & 0x08)
        {
          // code points: 0x0080 .. 0x07ff
          to_type.push_back(0xc0 | (hex[1] << 2) | ((hex[2] & 0x0c) >> 2));
          to_type.push_back(0x80 | ((hex[2] & 0x03) << 4) | hex[3]);
        }
        else
        {
          // code points: 0x0000 .. 0x007f
          to_type.push_back((hex[2] << 4) | hex[3]);
        }
        it = next_it + 6; // advance past hex digits
        size -= 4;
      }
      else
      {
        // fallback is to simply push characters as is
        to_type.push_back('\\');
        to_type.push_back(current_char);
        it = next_it + 2;
      }
    }
    else
    {
      push_back_escape(current_char, to_type);
      it = next_it + 2;
    }
    if (!size)
      break;
  }
}

static DataRef handle_json_escapes_out(const std::string &data, std::string &buffer)
{
  int start_index = 0;
  for (size_t i = 0; i < data.size(); i++)
  {
    const char cur = data[i];
    if (static_cast<uint8_t>(cur) <= uint8_t('\r') || cur == '\"' || cur == '\\')
    {
      if (buffer.empty())
      {
        buffer.reserve(data.size() + 10);
      }
      size_t diff = i - start_index;
      if (diff > 0)
      {
        buffer.insert(buffer.end(), data.data() + start_index, data.data() + start_index + diff);
      }
      start_index = int(i) + 1;

      switch (cur)
      {
      case '\b':
        buffer += std::string("\\b");
        break;
      case '\t':
        buffer += std::string("\\t");
        break;
      case '\n':
        buffer += std::string("\\n");
        break;
      case '\f':
        buffer += std::string("\\f");
        break;
      case '\r':
        buffer += std::string("\\r");
        break;
      case '\"':
        buffer += std::string("\\\"");
        break;
      case '\\':
        buffer += std::string("\\\\");
        break;
      default:
        buffer.push_back(cur);
        break;
      }
    }
  }
  if (buffer.size())
  {
    size_t diff = data.size() - start_index;
    if (diff > 0)
    {
      buffer.insert(buffer.end(), data.data() + start_index, data.data() + start_index + diff);
    }
    return DataRef(buffer.data(), buffer.size());
  }
  return DataRef(data.data(), data.size());
}
} // namespace Internal
/// \private
template <>
struct TypeHandler<std::string>
{
  static inline Error to(std::string &to_type, ParseContext &context)
  {
    to_type.clear();
    Internal::handle_json_escapes_in(context.token.value, to_type);
    return Error::NoError;
  }

  static inline void from(const std::string &str, Token &token, Serializer &serializer)
  {
    std::string buffer;
    DataRef ref = Internal::handle_json_escapes_out(str, buffer);
    token.value_type = Type::String;
    token.value.data = ref.data;
    token.value.size = ref.size;
    serializer.write(token);
  }
};

} // namespace STFY

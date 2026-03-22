#pragma once
#include "structify_meta.h"
#include <float_tools.h>

namespace STFY
{
/// \private
template <>
struct TypeHandler<double>
{
  static inline Error to(double &to_type, ParseContext &context)
  {
    const char *pointer;
    auto result = ft::to_double(context.token.value.data, context.token.value.size, to_type, pointer);
    if (result != ft::parse_string_error::ok ||
        context.token.value.data + context.token.value.size != pointer)
      return Error::FailedToParseDouble;
    return Error::NoError;
  }

  template <typename WriterT>
  static inline void serializeWith(const double &d, Token &token, WriterT &writer)
  {
    // char buf[1/*'-'*/ + (DBL_MAX_10_EXP+1)/*308+1 digits*/ + 1/*'.'*/ + 6/*Default? precision*/ + 1/*\0*/];
    char buf[32];
    int size;
    size = ft::ryu::to_buffer(d, buf, sizeof(buf));

    if (size <= 0)
    {
      return;
    }

    token.value_type = Type::Number;
    token.value.data = buf;
    token.value.size = size_t(size);
    writer.write(token);
  }

  static inline void from(const double &d, Token &token, Serializer &serializer)
  {
    serializeWith(d, token, serializer);
  }
  static inline void fromYaml(const double &d, Token &token, YamlWriter &writer)
  {
    serializeWith(d, token, writer);
  }
  static inline void fromCbor(const double &d, Token &token, CborWriter &writer)
  {
    if (token.name.size > 0)
      writer.writeKey(token.name.data, token.name.size);
    writer.writeDouble(d);
  }
};

/// \private
template <>
struct TypeHandler<float>
{
  static inline Error to(float &to_type, ParseContext &context)
  {
    const char *pointer;
    auto result = ft::to_float(context.token.value.data, context.token.value.size, to_type, pointer);
    if (result != ft::parse_string_error::ok ||
        context.token.value.data + context.token.value.size != pointer)
      return Error::FailedToParseFloat;
    return Error::NoError;
  }

  template <typename WriterT>
  static inline void serializeWith(const float &f, Token &token, WriterT &writer)
  {
    char buf[16];
    int size;
    size = ft::ryu::to_buffer(f, buf, sizeof(buf));
    if (size < 0)
    {
      return;
    }

    token.value_type = Type::Number;
    token.value.data = buf;
    token.value.size = size_t(size);
    writer.write(token);
  }

  static inline void from(const float &f, Token &token, Serializer &serializer)
  {
    serializeWith(f, token, serializer);
  }
  static inline void fromYaml(const float &f, Token &token, YamlWriter &writer)
  {
    serializeWith(f, token, writer);
  }
  static inline void fromCbor(const float &f, Token &token, CborWriter &writer)
  {
    if (token.name.size > 0)
      writer.writeKey(token.name.data, token.name.size);
    writer.writeFloat(f);
  }
};

/// \private
template <typename T>
struct TypeHandlerIntType
{
  static inline Error to(T &to_type, ParseContext &context)
  {
    const char *pointer;
    auto parse_error =
      ft::integer::to_integer(context.token.value.data, context.token.value.size, to_type, pointer);
    if (parse_error != ft::parse_string_error::ok || context.token.value.data == pointer)
      return Error::FailedToParseInt;
    return Error::NoError;
  }

  template <typename WriterT>
  static inline void serializeWith(const T &from_type, Token &token, WriterT &writer)
  {
    char buf[40];
    int digits_truncated;
    int size = ft::integer::to_buffer(from_type, buf, sizeof(buf), &digits_truncated);
    if (size <= 0 || digits_truncated)
    {
      fprintf(stderr, "error serializing int token\n");
      return;
    }

    token.value_type = Type::Number;
    token.value.data = buf;
    token.value.size = size_t(size);
    writer.write(token);
  }

  static inline void from(const T &from_type, Token &token, Serializer &serializer)
  {
    serializeWith(from_type, token, serializer);
  }
  static inline void fromYaml(const T &from_type, Token &token, YamlWriter &writer)
  {
    serializeWith(from_type, token, writer);
  }
  static inline void fromCbor(const T &from_type, Token &token, CborWriter &writer)
  {
    if (token.name.size > 0)
      writer.writeKey(token.name.data, token.name.size);
    if (std::is_signed<T>::value)
      writer.writeInt(static_cast<int64_t>(from_type));
    else
      writer.writeUint(static_cast<uint64_t>(from_type));
  }
};

/// \private
template <>
struct TypeHandler<short int> : TypeHandlerIntType<short int>
{
};

/// \private
template <>
struct TypeHandler<unsigned short int> : TypeHandlerIntType<unsigned short int>
{
};

/// \private
template <>
struct TypeHandler<int> : TypeHandlerIntType<int>
{
};

/// \private
template <>
struct TypeHandler<unsigned int> : TypeHandlerIntType<unsigned int>
{
};

/// \private
template <>
struct TypeHandler<long int> : TypeHandlerIntType<long int>
{
};

/// \private
template <>
struct TypeHandler<unsigned long int> : TypeHandlerIntType<unsigned long int>
{
};

/// \private
template <>
struct TypeHandler<long long int> : TypeHandlerIntType<long long int>
{
};

/// \private
template <>
struct TypeHandler<unsigned long long int> : TypeHandlerIntType<unsigned long long int>
{
};

template <>
struct TypeHandler<uint8_t> : TypeHandlerIntType<uint8_t>
{
};

template <>
struct TypeHandler<int8_t> : TypeHandlerIntType<int8_t>
{
};

template <>
struct TypeHandler<char> : TypeHandlerIntType<char>
{
};

/// \private
template <typename T>
struct TypeHandler<Nullable<T>>
{
public:
  static inline Error to(Nullable<T> &to_type, ParseContext &context)
  {
    if (context.token.value_type == Type::Null)
      return Error::NoError;
    return TypeHandler<T>::to(to_type.data, context);
  }

  static inline void from(const Nullable<T> &opt, Token &token, Serializer &serializer)
  {
    TypeHandler<T>::from(opt(), token, serializer);
  }
  static inline void fromYaml(const Nullable<T> &opt, Token &token, YamlWriter &writer)
  {
    TypeHandler<T>::fromYaml(opt(), token, writer);
  }
  static inline void fromCbor(const Nullable<T> &opt, Token &token, CborWriter &writer)
  {
    TypeHandler<T>::fromCbor(opt(), token, writer);
  }
};

/// \private
template <typename T>
struct TypeHandler<NullableChecked<T>>
{
public:
  static inline Error to(NullableChecked<T> &to_type, ParseContext &context)
  {
    if (context.token.value_type == Type::Null)
    {
      to_type.null = true;
      return Error::NoError;
    }
    to_type.null = false;
    return TypeHandler<T>::to(to_type.data, context);
  }

  template <typename WriterT>
  static inline void serializeWith(const NullableChecked<T> &opt, Token &token, WriterT &writer)
  {
    if (opt.null)
    {
      const char nullChar[] = "null";
      token.value_type = Type::Null;
      token.value = DataRef(nullChar);
      writer.write(token);
    }
    else
    {
      WriterDispatch<WriterT>::template call<T>(opt(), token, writer);
    }
  }

  static inline void from(const NullableChecked<T> &opt, Token &token, Serializer &serializer)
  {
    serializeWith(opt, token, serializer);
  }
  static inline void fromYaml(const NullableChecked<T> &opt, Token &token, YamlWriter &writer)
  {
    serializeWith(opt, token, writer);
  }
  static inline void fromCbor(const NullableChecked<T> &opt, Token &token, CborWriter &writer)
  {
    serializeWith(opt, token, writer);
  }
};

/// \private
template <typename T>
struct TypeHandler<Optional<T>>
{
public:
  static inline Error to(Optional<T> &to_type, ParseContext &context)
  {
    return TypeHandler<T>::to(to_type.data, context);
  }

  static inline void from(const Optional<T> &opt, Token &token, Serializer &serializer)
  {
    TypeHandler<T>::from(opt(), token, serializer);
  }
  static inline void fromYaml(const Optional<T> &opt, Token &token, YamlWriter &writer)
  {
    TypeHandler<T>::fromYaml(opt(), token, writer);
  }
  static inline void fromCbor(const Optional<T> &opt, Token &token, CborWriter &writer)
  {
    TypeHandler<T>::fromCbor(opt(), token, writer);
  }
};

/// \private
template <typename T>
struct TypeHandler<OptionalChecked<T>>
{
public:
  static inline Error to(OptionalChecked<T> &to_type, ParseContext &context)
  {
    to_type.assigned = true;
    return TypeHandler<T>::to(to_type.data, context);
  }

  static inline void from(const OptionalChecked<T> &opt, Token &token, Serializer &serializer)
  {
    if (opt.assigned)
      TypeHandler<T>::from(opt(), token, serializer);
  }
  static inline void fromYaml(const OptionalChecked<T> &opt, Token &token, YamlWriter &writer)
  {
    if (opt.assigned)
      TypeHandler<T>::fromYaml(opt(), token, writer);
  }
  static inline void fromCbor(const OptionalChecked<T> &opt, Token &token, CborWriter &writer)
  {
    if (opt.assigned)
      TypeHandler<T>::fromCbor(opt(), token, writer);
  }
};

#ifdef STFY_STD_OPTIONAL
/// \private
template <typename T>
struct TypeHandler<std::optional<T>>
{
public:
  static inline Error to(std::optional<T> &to_type, ParseContext &context)
  {
    to_type.emplace();
    return TypeHandler<T>::to(to_type.value(), context);
  }

  static inline void from(const std::optional<T> &opt, Token &token, Serializer &serializer)
  {
    if (opt.has_value())
      TypeHandler<T>::from(opt.value(), token, serializer);
  }
  static inline void fromYaml(const std::optional<T> &opt, Token &token, YamlWriter &writer)
  {
    if (opt.has_value())
      TypeHandler<T>::fromYaml(opt.value(), token, writer);
  }
  static inline void fromCbor(const std::optional<T> &opt, Token &token, CborWriter &writer)
  {
    if (opt.has_value())
      TypeHandler<T>::fromCbor(opt.value(), token, writer);
  }
};
#endif

/// \private
template <typename T>
struct TypeHandler<std::shared_ptr<T>>
{
public:
  static inline Error to(std::shared_ptr<T> &to_type, ParseContext &context)
  {
    if (context.token.value_type != Type::Null)
    {
      if (!to_type)
        to_type = std::make_shared<T>();
      return TypeHandler<T>::to(*to_type.get(), context);
    }
    to_type.reset();
    return Error::NoError;
  }

  template <typename WriterT>
  static inline void serializeWith(const std::shared_ptr<T> &unique, Token &token, WriterT &writer)
  {
    if (unique)
    {
      WriterDispatch<WriterT>::template call<T>(*unique.get(), token, writer);
    }
    else
    {
      const char nullChar[] = "null";
      token.value_type = Type::Null;
      token.value = DataRef(nullChar);
      writer.write(token);
    }
  }

  static inline void from(const std::shared_ptr<T> &unique, Token &token, Serializer &serializer)
  {
    serializeWith(unique, token, serializer);
  }
  static inline void fromYaml(const std::shared_ptr<T> &unique, Token &token, YamlWriter &writer)
  {
    serializeWith(unique, token, writer);
  }
  static inline void fromCbor(const std::shared_ptr<T> &unique, Token &token, CborWriter &writer)
  {
    serializeWith(unique, token, writer);
  }
};

/// \private
template <typename T>
struct TypeHandler<std::unique_ptr<T>>
{
public:
  static inline Error to(std::unique_ptr<T> &to_type, ParseContext &context)
  {
    if (context.token.value_type != Type::Null)
    {
      if (!to_type)
        to_type.reset(new T());
      return TypeHandler<T>::to(*to_type.get(), context);
    }
    to_type.reset(nullptr);
    return Error::NoError;
  }

  template <typename WriterT>
  static inline void serializeWith(const std::unique_ptr<T> &unique, Token &token, WriterT &writer)
  {
    if (unique)
    {
      WriterDispatch<WriterT>::template call<T>(*unique.get(), token, writer);
    }
    else
    {
      const char nullChar[] = "null";
      token.value_type = Type::Null;
      token.value = DataRef(nullChar);
      writer.write(token);
    }
  }

  static inline void from(const std::unique_ptr<T> &unique, Token &token, Serializer &serializer)
  {
    serializeWith(unique, token, serializer);
  }
  static inline void fromYaml(const std::unique_ptr<T> &unique, Token &token, YamlWriter &writer)
  {
    serializeWith(unique, token, writer);
  }
  static inline void fromCbor(const std::unique_ptr<T> &unique, Token &token, CborWriter &writer)
  {
    serializeWith(unique, token, writer);
  }
};

/// \private
template <>
struct TypeHandler<bool>
{
  static inline Error to(bool &to_type, ParseContext &context)
  {
    if (context.token.value.size == sizeof("true") - 1 &&
        memcmp("true", context.token.value.data, sizeof("true") - 1) == 0)
      to_type = true;
    else if (context.token.value.size == sizeof("false") - 1 &&
             memcmp("false", context.token.value.data, sizeof("false") - 1) == 0)
      to_type = false;
    else
      return Error::FailedToParseBoolean;

    return Error::NoError;
  }

  template <typename WriterT>
  static inline void serializeWith(const bool &b, Token &token, WriterT &writer)
  {
    const char trueChar[] = "true";
    const char falseChar[] = "false";
    token.value_type = Type::Bool;
    if (b)
    {
      token.value = DataRef(trueChar);
    }
    else
    {
      token.value = DataRef(falseChar);
    }
    writer.write(token);
  }

  static inline void from(const bool &b, Token &token, Serializer &serializer)
  {
    serializeWith(b, token, serializer);
  }
  static inline void fromYaml(const bool &b, Token &token, YamlWriter &writer)
  {
    serializeWith(b, token, writer);
  }
  static inline void fromCbor(const bool &b, Token &token, CborWriter &writer)
  {
    if (token.name.size > 0)
      writer.writeKey(token.name.data, token.name.size);
    writer.writeBool(b);
  }
};

#ifdef STFY_STD_TIMEPOINT
/// \private
namespace Internal
{
    template <class T, template <class...> class Template>
    struct is_specialization : std::false_type {};

    template <template <class...> class Template, class... Args>
    struct is_specialization<Template<Args...>, Template> : std::true_type {};
}

/// \private
template <class T>
struct TypeHandler<T, typename std::enable_if_t<Internal::is_specialization<T, std::chrono::time_point>::value>>
{
    static inline Error to(T& to_type, ParseContext &context)
    {
        uint64_t t;
        Error err = TypeHandler<uint64_t>::to(t, context);
        if (err != Error::NoError)
            return err;

        if (t <= 1e11) // Seconds => 10 digits, normally
            to_type = T{std::chrono::seconds{t}};
        else if (t <= 1e14) // Milliseconds => 13 digits, normally
            to_type = T{std::chrono::milliseconds{t}};
        else if (t <= 1e17) // Microseconds
            to_type = T{std::chrono::microseconds{t}};
        else if (t <= 1e20) // Nanoseconds
            if constexpr (std::is_same_v<std::chrono::high_resolution_clock::time_point, T>)
                to_type = T{std::chrono::nanoseconds{t}};
            else
                return STFY::Error::IllegalDataValue;
        else
            return STFY::Error::IllegalDataValue;

        return STFY::Error::NoError;
    }

    template <typename WriterT>
    static inline void serializeWith(const T& val, Token &token, WriterT &writer)
    {
        uint64_t t;
        if constexpr (std::is_same_v<std::chrono::high_resolution_clock::time_point, T>)
        	t = std::chrono::duration_cast<std::chrono::nanoseconds>(val.time_since_epoch()).count();
		else
        	t = std::chrono::duration_cast<std::chrono::microseconds>(val.time_since_epoch()).count();
		while (t % 1000 == 0 && t > (uint64_t)1e10)
			t /= 1000;
        WriterDispatch<WriterT>::template call<uint64_t>(t, token, writer);
    }

    static inline void from(const T& val, Token &token, Serializer &serializer)
    {
        serializeWith(val, token, serializer);
    }
    static inline void fromYaml(const T& val, Token &token, YamlWriter &writer)
    {
        serializeWith(val, token, writer);
    }
    static inline void fromCbor(const T& val, Token &token, CborWriter &writer)
    {
        serializeWith(val, token, writer);
    }
};
#endif

/// \private
template <typename T, typename A>
struct TypeHandler<std::vector<T, A>>
{
  static inline Error to(std::vector<T, A> &to_type, ParseContext &context)
  {
    if (context.token.value_type != STFY::Type::ArrayStart)
      return Error::ExpectedArrayStart;
    Error error = context.nextToken();
    if (error != STFY::Error::NoError)
      return error;
    to_type.clear();
    to_type.reserve(10);
    while (context.token.value_type != STFY::Type::ArrayEnd)
    {
      to_type.push_back(T());
      error = TypeHandler<T>::to(to_type.back(), context);
      if (error != STFY::Error::NoError)
        break;
      error = context.nextToken();
      if (error != STFY::Error::NoError)
        break;
    }

    return error;
  }

  template <typename WriterT>
  static inline void serializeWith(const std::vector<T, A> &vec, Token &token, WriterT &writer)
  {
    token.value_type = Type::ArrayStart;
    token.value = DataRef("[");
    writer.write(token);

    token.name = DataRef("");

    for (auto &index : vec)
    {
      WriterDispatch<WriterT>::template call<T>(index, token, writer);
    }

    token.name = DataRef("");

    token.value_type = Type::ArrayEnd;
    token.value = DataRef("]");
    writer.write(token);
  }

  static inline void from(const std::vector<T, A> &vec, Token &token, Serializer &serializer)
  {
    serializeWith(vec, token, serializer);
  }
  static inline void fromYaml(const std::vector<T, A> &vec, Token &token, YamlWriter &writer)
  {
    serializeWith(vec, token, writer);
  }
  static inline void fromCbor(const std::vector<T, A> &vec, Token &token, CborWriter &writer)
  {
    serializeWith(vec, token, writer);
  }
};

/// \private
template <typename A>
struct TypeHandler<std::vector<bool, A>>
{
public:
  static inline Error to(std::vector<bool, A> &to_type, ParseContext &context)
  {
    if (context.token.value_type != STFY::Type::ArrayStart)
      return Error::ExpectedArrayStart;
    Error error = context.nextToken();
    if (error != STFY::Error::NoError)
      return error;
    to_type.clear();
    to_type.reserve(10);
    while (context.token.value_type != STFY::Type::ArrayEnd)
    {

      bool toBool;
      error = TypeHandler<bool>::to(toBool, context);
      to_type.push_back(toBool);
      if (error != STFY::Error::NoError)
        break;
      error = context.nextToken();
      if (error != STFY::Error::NoError)
        break;
    }

    return error;
  }

  template <typename WriterT>
  static inline void serializeWith(const std::vector<bool, A> &vec, Token &token, WriterT &writer)
  {
    token.value_type = Type::ArrayStart;
    token.value = DataRef("[");
    writer.write(token);

    token.name = DataRef("");

    for (bool index : vec)
    {
      WriterDispatch<WriterT>::template call<bool>(index, token, writer);
    }

    token.name = DataRef("");

    token.value_type = Type::ArrayEnd;
    token.value = DataRef("]");
    writer.write(token);
  }

  static inline void from(const std::vector<bool, A> &vec, Token &token, Serializer &serializer)
  {
    serializeWith(vec, token, serializer);
  }
  static inline void fromYaml(const std::vector<bool, A> &vec, Token &token, YamlWriter &writer)
  {
    serializeWith(vec, token, writer);
  }
  static inline void fromCbor(const std::vector<bool, A> &vec, Token &token, CborWriter &writer)
  {
    serializeWith(vec, token, writer);
  }
};

/// \private
template <>
struct TypeHandler<SilentString>
{
  static inline Error to(SilentString &to_type, ParseContext &context)
  {
    return TypeHandler<std::string>::to(to_type.data, context);
  }
  static inline void from(const SilentString &str, Token &token, Serializer &serializer)
  {
    if (str.data.size())
    {
      TypeHandler<std::string>::from(str.data, token, serializer);
    }
  }
  static inline void fromYaml(const SilentString &str, Token &token, YamlWriter &writer)
  {
    if (str.data.size())
    {
      TypeHandler<std::string>::fromYaml(str.data, token, writer);
    }
  }
  static inline void fromCbor(const SilentString &str, Token &token, CborWriter &writer)
  {
    if (str.data.size())
    {
      TypeHandler<std::string>::fromCbor(str.data, token, writer);
    }
  }
};

/// \private
template <typename T, typename A>
struct TypeHandler<SilentVector<T, A>>
{
public:
  static inline Error to(SilentVector<T, A> &to_type, ParseContext &context)
  {
    return TypeHandler<std::vector<T, A>>::to(to_type.data, context);
  }

  static inline void from(const SilentVector<T, A> &vec, Token &token, Serializer &serializer)
  {
    if (vec.data.size())
    {
      TypeHandler<std::vector<T, A>>::from(vec.data, token, serializer);
    }
  }
  static inline void fromYaml(const SilentVector<T, A> &vec, Token &token, YamlWriter &writer)
  {
    if (vec.data.size())
    {
      TypeHandler<std::vector<T, A>>::fromYaml(vec.data, token, writer);
    }
  }
  static inline void fromCbor(const SilentVector<T, A> &vec, Token &token, CborWriter &writer)
  {
    if (vec.data.size())
    {
      TypeHandler<std::vector<T, A>>::fromCbor(vec.data, token, writer);
    }
  }
};

/// \private
template <typename T>
struct TypeHandler<SilentUniquePtr<T>>
{
public:
  static inline Error to(SilentUniquePtr<T> &to_type, ParseContext &context)
  {
    return TypeHandler<std::unique_ptr<T>>::to(to_type.data, context);
  }

  static inline void from(const SilentUniquePtr<T> &ptr, Token &token, Serializer &serializer)
  {
    if (ptr.data)
    {
      TypeHandler<std::unique_ptr<T>>::from(ptr.data, token, serializer);
    }
  }
  static inline void fromYaml(const SilentUniquePtr<T> &ptr, Token &token, YamlWriter &writer)
  {
    if (ptr.data)
    {
      TypeHandler<std::unique_ptr<T>>::fromYaml(ptr.data, token, writer);
    }
  }
  static inline void fromCbor(const SilentUniquePtr<T> &ptr, Token &token, CborWriter &writer)
  {
    if (ptr.data)
    {
      TypeHandler<std::unique_ptr<T>>::fromCbor(ptr.data, token, writer);
    }
  }
};

/// \private
template <typename A>
struct TypeHandler<std::vector<Token, A>>
{
public:
  static inline Error to(std::vector<Token, A> &to_type, ParseContext &context)
  {
    if (context.token.value_type != STFY::Type::ArrayStart && context.token.value_type != STFY::Type::ObjectStart)
    {
      to_type.push_back(context.token);
      return context.error;
    }
    to_type.clear();
    to_type.push_back(context.token);

    size_t level = 1;
    Error error = Error::NoError;
    while (error == STFY::Error::NoError && level)
    {
      error = context.nextToken();
      to_type.push_back(context.token);
      if (context.token.value_type == Type::ArrayStart || context.token.value_type == Type::ObjectStart)
        level++;
      else if (context.token.value_type == Type::ArrayEnd || context.token.value_type == Type::ObjectEnd)
        level--;
    }

    return error;
  }

  static inline void from(const std::vector<Token, A> &from_type, Token &token, Serializer &serializer)
  {
    for (auto &t : from_type)
    {
      token = t;
      serializer.write(token);
    }
  }
};

/// \private
template <>
struct TypeHandler<JsonTokens>
{
public:
  static inline Error to(JsonTokens &to_type, ParseContext &context)
  {
    return TypeHandler<std::vector<Token>>::to(to_type.data, context);
  }
  static inline void from(const JsonTokens &from, Token &token, Serializer &serializer)
  {
    return TypeHandler<std::vector<Token>>::from(from.data, token, serializer);
  }
};

/// \private
template <>
struct TypeHandler<JsonArrayRef>
{
  static inline Error to(JsonArrayRef &to_type, ParseContext &context)
  {
    if (context.token.value_type != STFY::Type::ArrayStart)
      return Error::ExpectedArrayStart;

    to_type.ref.data = context.token.value.data;
    Error error = context.skipScope();
    to_type.ref.size = size_t(context.token.value.data + context.token.value.size - to_type.ref.data);
    return error;
  }

  static inline void from(const JsonArrayRef &from_type, Token &token, Serializer &serializer)
  {
    token.value = from_type.ref;
    token.value_type = Type::Verbatim;
    serializer.write(token);
  }
};

/// \private
template <>
struct TypeHandler<JsonArray>
{
  static inline Error to(JsonArray &to_type, ParseContext &context)
  {
    if (context.token.value_type != STFY::Type::ArrayStart)
      return Error::ExpectedArrayStart;

    const char *start = context.token.value.data;
    Error error = context.skipScope();
    if (error == STFY::Error::NoError)
      to_type.data.assign(start, context.token.value.data + context.token.value.size - start);
    return error;
  }

  static inline void from(const JsonArray &from_type, Token &token, Serializer &serializer)
  {
    token.value_type = STFY::Type::Verbatim; // Need to fool the serializer to just write value as verbatim

    if (from_type.data.empty())
    {
      std::string emptyArray("[]");
      token.value = DataRef(emptyArray);
      serializer.write(token);
    }
    else
    {
      token.value = DataRef(from_type.data);
      serializer.write(token);
    }
  }
};

/// \private
template <>
struct TypeHandler<JsonObjectRef>
{
  static inline Error to(JsonObjectRef &to_type, ParseContext &context)
  {
    if (context.token.value_type != STFY::Type::ObjectStart)
      return Error::ExpectedObjectStart;

    to_type.ref.data = context.token.value.data;
    Error error = context.skipScope();
    to_type.ref.size = size_t(context.token.value.data + context.token.value.size - to_type.ref.data);
    return error;
  }

  static inline void from(const JsonObjectRef &from_type, Token &token, Serializer &serializer)
  {
    token.value = from_type.ref;
    token.value_type = Type::Verbatim;
    serializer.write(token);
  }
};

/// \private
template <>
struct TypeHandler<JsonObject>
{
  static inline Error to(JsonObject &to_type, ParseContext &context)
  {
    if (context.token.value_type != STFY::Type::ObjectStart)
      return Error::ExpectedObjectStart;

    const char *start = context.token.value.data;
    Error error = context.skipScope();
    if (error == STFY::Error::NoError)
      to_type.data.assign(start, context.token.value.data + context.token.value.size - start);
    return error;
  }

  static inline void from(const JsonObject &from_type, Token &token, Serializer &serializer)
  {
    token.value_type = STFY::Type::Verbatim; // Need to fool the serializer to just write value as verbatim

    if (from_type.data.empty())
    {
      std::string emptyObject("{}");
      token.value = DataRef(emptyObject);
      serializer.write(token);
    }
    else
    {
      token.value = DataRef(from_type.data);
      serializer.write(token);
    }
  }
};

/// \private
template <>
struct TypeHandler<JsonObjectOrArrayRef>
{
  static inline Error to(JsonObjectOrArrayRef &to_type, ParseContext &context)
  {
    if (context.token.value_type != STFY::Type::ObjectStart && context.token.value_type != STFY::Type::ArrayStart)
      return Error::ExpectedObjectStart;

    to_type.ref.data = context.token.value.data;
    Error error = context.skipScope();
    to_type.ref.size = size_t(context.token.value.data + context.token.value.size - to_type.ref.data);
    return error;
  }

  static inline void from(const JsonObjectOrArrayRef &from_type, Token &token, Serializer &serializer)
  {
    token.value = from_type.ref;
    token.value_type = Type::Verbatim;
    serializer.write(token);
  }
};

/// \private
template <>
struct TypeHandler<JsonObjectOrArray>
{
  static inline Error to(JsonObjectOrArray &to_type, ParseContext &context)
  {
    if (context.token.value_type != STFY::Type::ObjectStart && context.token.value_type != STFY::Type::ArrayStart)
      return Error::ExpectedObjectStart;

    const char *start = context.token.value.data;
    Error error = context.skipScope();
    if (error == STFY::Error::NoError)
      to_type.data.assign(start, context.token.value.data + context.token.value.size - start);
    return error;
  }

  static inline void from(const JsonObjectOrArray &from_type, Token &token, Serializer &serializer)
  {
    token.value_type = STFY::Type::Verbatim; // Need to fool the serializer to just write value as verbatim

    if (from_type.data.empty())
    {
      std::string emptyObjectOrArray("{}"); // Use object as default
      token.value = DataRef(emptyObjectOrArray);
      serializer.write(token);
    }
    else
    {
      token.value = DataRef(from_type.data);
      serializer.write(token);
    }
  }
};

namespace Internal
{
template <size_t INDEX, typename... Ts>
struct TupleTypeHandler
{
  static inline Error to(STFY::Tuple<Ts...> &to_type, ParseContext &context)
  {
    using Type = typename STFY::TypeAt<sizeof...(Ts) - INDEX, Ts...>::type;
    Error error = TypeHandler<Type>::to(to_type.template get<sizeof...(Ts) - INDEX>(), context);
    if (error != STFY::Error::NoError)
      return error;
    error = context.nextToken();
    if (error != STFY::Error::NoError)
      return error;
    return TupleTypeHandler<INDEX - 1, Ts...>::to(to_type, context);
  }

  static inline void from(const STFY::Tuple<Ts...> &from_type, Token &token, Serializer &serializer)
  {
    using Type = typename STFY::TypeAt<sizeof...(Ts) - INDEX, Ts...>::type;
    TypeHandler<Type>::from(from_type.template get<sizeof...(Ts) - INDEX>(), token, serializer);
    TupleTypeHandler<INDEX - 1, Ts...>::from(from_type, token, serializer);
  }

  template <typename WriterT>
  static inline void serializeWith(const STFY::Tuple<Ts...> &from_type, Token &token, WriterT &writer)
  {
    using Type = typename STFY::TypeAt<sizeof...(Ts) - INDEX, Ts...>::type;
    WriterDispatch<WriterT>::template call<Type>(from_type.template get<sizeof...(Ts) - INDEX>(), token, writer);
    TupleTypeHandler<INDEX - 1, Ts...>::template serializeWith<WriterT>(from_type, token, writer);
  }
};

/// \private
template <typename... Ts>
struct TupleTypeHandler<0, Ts...>
{
  static inline Error to(STFY::Tuple<Ts...>, ParseContext &context)
  {
    STFY_UNUSED(context);
    return Error::NoError;
  }

  static inline void from(const STFY::Tuple<Ts...> &from_type, Token &token, Serializer &serializer)
  {
    STFY_UNUSED(from_type);
    STFY_UNUSED(token);
    STFY_UNUSED(serializer);
  }

  template <typename WriterT>
  static inline void serializeWith(const STFY::Tuple<Ts...> &from_type, Token &token, WriterT &writer)
  {
    STFY_UNUSED(from_type);
    STFY_UNUSED(token);
    STFY_UNUSED(writer);
  }
};
} // namespace Internal

/// \private
template <typename... Ts>
struct TypeHandler<STFY::Tuple<Ts...>>
{
  static inline Error to(STFY::Tuple<Ts...> &to_type, ParseContext &context)
  {
    if (context.token.value_type != STFY::Type::ArrayStart)
      return Error::ExpectedArrayStart;
    Error error = context.nextToken();
    if (error != STFY::Error::NoError)
      return error;
    error = STFY::Internal::TupleTypeHandler<sizeof...(Ts), Ts...>::to(to_type, context);
    if (error != STFY::Error::NoError)
      return error;
    if (context.token.value_type != STFY::Type::ArrayEnd)
      return Error::ExpectedArrayEnd;
    return Error::NoError;
  }

  template <typename WriterT>
  static inline void serializeWith(const STFY::Tuple<Ts...> &from_type, Token &token, WriterT &writer)
  {
    token.value_type = Type::ArrayStart;
    token.value = DataRef("[");
    writer.write(token);

    token.name = DataRef("");

    STFY::Internal::TupleTypeHandler<sizeof...(Ts), Ts...>::template serializeWith<WriterT>(from_type, token, writer);
    token.name = DataRef("");

    token.value_type = Type::ArrayEnd;
    token.value = DataRef("]");
    writer.write(token);
  }

  static inline void from(const STFY::Tuple<Ts...> &from_type, Token &token, Serializer &serializer)
  {
    serializeWith(from_type, token, serializer);
  }
  static inline void fromYaml(const STFY::Tuple<Ts...> &from_type, Token &token, YamlWriter &writer)
  {
    serializeWith(from_type, token, writer);
  }
  static inline void fromCbor(const STFY::Tuple<Ts...> &from_type, Token &token, CborWriter &writer)
  {
    serializeWith(from_type, token, writer);
  }
};

namespace Internal
{
template <size_t INDEX, typename... Ts>
struct StdTupleTypeHandler
{
  static inline Error to(std::tuple<Ts...> &to_type, ParseContext &context)
  {
    using Type = typename std::tuple_element<sizeof...(Ts) - INDEX, std::tuple<Ts...>>::type;
    Error error = TypeHandler<Type>::to(std::get<sizeof...(Ts) - INDEX>(to_type), context);
    if (error != STFY::Error::NoError)
      return error;
    error = context.nextToken();
    if (error != STFY::Error::NoError)
      return error;
    return StdTupleTypeHandler<INDEX - 1, Ts...>::to(to_type, context);
  }

  static inline void from(const std::tuple<Ts...> &from_type, Token &token, Serializer &serializer)
  {
    using Type = typename std::tuple_element<sizeof...(Ts) - INDEX, std::tuple<Ts...>>::type;
    TypeHandler<Type>::from(std::get<sizeof...(Ts) - INDEX>(from_type), token, serializer);
    StdTupleTypeHandler<INDEX - 1, Ts...>::from(from_type, token, serializer);
  }

  template <typename WriterT>
  static inline void serializeWith(const std::tuple<Ts...> &from_type, Token &token, WriterT &writer)
  {
    using Type = typename std::tuple_element<sizeof...(Ts) - INDEX, std::tuple<Ts...>>::type;
    WriterDispatch<WriterT>::template call<Type>(std::get<sizeof...(Ts) - INDEX>(from_type), token, writer);
    StdTupleTypeHandler<INDEX - 1, Ts...>::template serializeWith<WriterT>(from_type, token, writer);
  }
};

/// \private
template <typename... Ts>
struct StdTupleTypeHandler<0, Ts...>
{
  static inline Error to(std::tuple<Ts...> &, ParseContext &context)
  {
    STFY_UNUSED(context);
    return Error::NoError;
  }

  static inline void from(const std::tuple<Ts...> &from_type, Token &token, Serializer &serializer)
  {
    STFY_UNUSED(from_type);
    STFY_UNUSED(token);
    STFY_UNUSED(serializer);
  }

  template <typename WriterT>
  static inline void serializeWith(const std::tuple<Ts...> &from_type, Token &token, WriterT &writer)
  {
    STFY_UNUSED(from_type);
    STFY_UNUSED(token);
    STFY_UNUSED(writer);
  }
};
} // namespace Internal
/// \private
template <typename... Ts>
struct TypeHandler<std::tuple<Ts...>>
{
  static inline Error to(std::tuple<Ts...> &to_type, ParseContext &context)
  {
    if (context.token.value_type != STFY::Type::ArrayStart)
      return Error::ExpectedArrayStart;
    Error error = context.nextToken();
    if (error != STFY::Error::NoError)
      return error;
    error = STFY::Internal::StdTupleTypeHandler<sizeof...(Ts), Ts...>::to(to_type, context);
    if (error != STFY::Error::NoError)
      return error;
    if (context.token.value_type != STFY::Type::ArrayEnd)
      return Error::ExpectedArrayEnd;
    return Error::NoError;
  }

  template <typename WriterT>
  static inline void serializeWith(const std::tuple<Ts...> &from_type, Token &token, WriterT &writer)
  {
    token.value_type = Type::ArrayStart;
    token.value = DataRef("[");
    writer.write(token);

    token.name = DataRef("");

    STFY::Internal::StdTupleTypeHandler<sizeof...(Ts), Ts...>::template serializeWith<WriterT>(from_type, token, writer);
    token.name = DataRef("");

    token.value_type = Type::ArrayEnd;
    token.value = DataRef("]");
    writer.write(token);
  }

  static inline void from(const std::tuple<Ts...> &from_type, Token &token, Serializer &serializer)
  {
    serializeWith(from_type, token, serializer);
  }
  static inline void fromYaml(const std::tuple<Ts...> &from_type, Token &token, YamlWriter &writer)
  {
    serializeWith(from_type, token, writer);
  }
  static inline void fromCbor(const std::tuple<Ts...> &from_type, Token &token, CborWriter &writer)
  {
    serializeWith(from_type, token, writer);
  }
};

template <typename T>
struct OneOrMany
{
  std::vector<T> data;
};

template <typename T>
struct TypeHandler<OneOrMany<T>>
{
public:
  static inline Error to(OneOrMany<T> &to_type, ParseContext &context)
  {
    if (context.token.value_type == Type::ArrayStart)
    {
      context.error = TypeHandler<std::vector<T>>::to(to_type.data, context);
    }
    else
    {
      to_type.data.push_back(T());
      context.error = TypeHandler<T>::to(to_type.data.back(), context);
    }
    return context.error;
  }

  template <typename WriterT>
  static inline void serializeWith(const OneOrMany<T> &from, Token &token, WriterT &writer)
  {
    if (from.data.empty())
      return;
    if (from.data.size() > 1)
    {
      WriterDispatch<WriterT>::template call<std::vector<T>>(from.data, token, writer);
    }
    else
    {
      WriterDispatch<WriterT>::template call<T>(from.data.front(), token, writer);
    }
  }

  static void from(const OneOrMany<T> &from, Token &token, Serializer &serializer)
  {
    serializeWith(from, token, serializer);
  }
  static void fromYaml(const OneOrMany<T> &from, Token &token, YamlWriter &writer)
  {
    serializeWith(from, token, writer);
  }
  static void fromCbor(const OneOrMany<T> &from, Token &token, CborWriter &writer)
  {
    serializeWith(from, token, writer);
  }
};

template <typename T, size_t N>
struct TypeHandler<T[N]>
{
public:
  static inline Error to(T (&to_type)[N], ParseContext &context)
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

  template <typename WriterT>
  static inline void serializeWith(const T (&from)[N], Token &token, WriterT &writer)
  {
    token.value_type = Type::ArrayStart;
    token.value = DataRef("[");
    writer.write(token);

    token.name = DataRef("");
    for (size_t i = 0; i < N; i++)
      WriterDispatch<WriterT>::template call<T>(from[i], token, writer);

    token.name = DataRef("");
    token.value_type = Type::ArrayEnd;
    token.value = DataRef("]");
    writer.write(token);
  }

  static void from(const T (&from)[N], Token &token, Serializer &serializer)
  {
    serializeWith(from, token, serializer);
  }
  static void fromYaml(const T (&from)[N], Token &token, YamlWriter &writer)
  {
    serializeWith(from, token, writer);
  }
  static void fromCbor(const T (&from)[N], Token &token, CborWriter &writer)
  {
    serializeWith(from, token, writer);
  }
};

template <typename Key, typename Value, typename Map>
struct TypeHandlerMap
{
  static inline Error to(Map &to_type, ParseContext &context)
  {
    if (context.token.value_type != Type::ObjectStart)
    {
      return STFY::Error::ExpectedObjectStart;
    }

    Error error = context.nextToken();
    if (error != STFY::Error::NoError)
      return error;
    while (context.token.value_type != Type::ObjectEnd)
    {
      std::string str;
      Internal::handle_json_escapes_in(context.token.name, str);
      Key key(str.data(), str.size());
      Value v;
      error = TypeHandler<Value>::to(v, context);
      to_type[std::move(key)] = std::move(v);
      if (error != STFY::Error::NoError)
        return error;
      error = context.nextToken();
    }

    return error;
  }

  template <typename WriterT>
  static inline void serializeWith(const Map &from, Token &token, WriterT &writer)
  {
    token.value_type = Type::ObjectStart;
    token.value = DataRef("{");
    writer.write(token);
    for (auto it = from.begin(); it != from.end(); ++it)
    {
      token.name = DataRef(it->first);
      token.name_type = Type::String;
      WriterDispatch<WriterT>::template call<Value>(it->second, token, writer);
    }
    token.name.size = 0;
    token.name.data = "";
    token.name_type = Type::String;
    token.value_type = Type::ObjectEnd;
    token.value = DataRef("}");
    writer.write(token);
  }

  static void from(const Map &from, Token &token, Serializer &serializer)
  {
    serializeWith(from, token, serializer);
  }
  static void fromYaml(const Map &from, Token &token, YamlWriter &writer)
  {
    serializeWith(from, token, writer);
  }
  static void fromCbor(const Map &from, Token &token, CborWriter &writer)
  {
    serializeWith(from, token, writer);
  }
};

#ifdef STFY_STD_UNORDERED_MAP
template <typename Key, typename Value>
struct TypeHandler<std::unordered_map<Key, Value>> : TypeHandlerMap<Key, Value, std::unordered_map<Key, Value>>
{
};

#endif

namespace Internal
{
inline bool compareDataRefWithString(const DataRef &a, const std::string &b)
{
  return a.size == b.size() && memcmp(a.data, b.data(), a.size) == 0;
}
} // namespace Internal
struct Map
{
  struct It
  {
    using iterator_category = std::forward_iterator_tag;
    using difference_type = int;
    using value_type = Token;
    using pointer = Token *;
    using reference = Token &;
    const Map &map;
    uint32_t index = 0;
    uint32_t next_meta = 0;
    uint32_t next_complex = 0;

    It(const Map &map)
      : map(map)
    {
    }
    It(const It &other)
      : map(other.map)
      , index(other.index)
      , next_meta(other.next_meta)
      , next_complex(other.next_complex)
    {
    }
    inline const Token &operator*()
    {
      return map.tokens.data[index];
    }

    inline const Token *operator->()
    {
      return &map.tokens.data[index];
    }

    inline It &operator++()
    {
      if (index == next_complex)
      {
        index += map.meta[next_meta].size;
        next_meta += map.meta[next_meta].skip;
        next_complex = next_meta < uint32_t(map.meta.size()) ? uint32_t(map.meta[next_meta].position)
                                                             : uint32_t(map.tokens.data.size());
      }
      else
      {
        index++;
      }
      return *this;
    }
    inline bool operator==(const It &other) const
    {
      return index == other.index;
    }
    inline bool operator!=(const It &other) const
    {
      return index != other.index;
    }

    inline void operator=(const It &other)
    {
      assert(&map == &other.map);
      index = other.index;
      next_meta = other.next_meta;
      next_complex = other.next_complex;
    }
  };

  STFY::JsonTokens tokens;
  std::vector<JsonMeta> meta;
  std::vector<std::pair<int, std::string>> json_data;

  inline It begin() const
  {
    It b(*this);
    b.index = 1;
    b.next_meta = 1;
    b.next_complex =
      b.next_meta < uint32_t(meta.size()) ? uint32_t(meta[b.next_meta].position) : uint32_t(tokens.data.size());
    return b;
  }

  inline It end() const
  {
    It e(*this);
    e.index = uint32_t(tokens.data.size());
    e.next_meta = 0;
    e.next_complex = 0;
    return e;
  }

  inline It find(const std::string &name) const
  {
    return std::find_if(begin(), end(),
                        [&name](const Token &token) { return Internal::compareDataRefWithString(token.name, name); });
  }

  template <typename T>
  STFY::Error castToType(STFY::ParseContext &parseContext, T &to) const
  {
    parseContext.tokenizer.resetData(&tokens.data, 0);
    parseContext.nextToken();
    return STFY::TypeHandler<T>::to(to, parseContext);
  }

  template <typename T>
  STFY::Error castToType(const It &iterator, STFY::ParseContext &parseContext, T &to) const
  {
    assert(iterator.index < tokens.data.size());
    parseContext.tokenizer.resetData(&tokens.data, iterator.index);
    parseContext.nextToken();
    return STFY::TypeHandler<T>::to(to, parseContext);
  }

  template <typename T>
  STFY::Error castToType(const std::string &name, STFY::ParseContext &parseContext, T &to) const
  {
    if (tokens.data.empty() || tokens.data.front().value_type != STFY::Type::ObjectStart)
    {
      parseContext.error = STFY::Error::ExpectedObjectStart;
      return parseContext.error;
    }

    It it = find(name);
    if (it != end())
      return castToType(it, parseContext, to);
    parseContext.error = STFY::Error::KeyNotFound;
    return parseContext.error;
  }

  template <typename T>
  T castTo(STFY::ParseContext &parseContext) const
  {
    T t = {};
    castToType<T>(parseContext, t);
    return t;
  }

  template <typename T>
  T castTo(const std::string &name, STFY::ParseContext &parseContext) const
  {
    T t = {};
    castToType<T>(name, parseContext, t);
    return t;
  }

  template <typename T>
  STFY::Error setValue(STFY::ParseContext &parseContext, const T &value)
  {
    static_assert(sizeof(STFY::Internal::HasStructifyBase<T>::template test_in_base<T>(nullptr)) ==
                    sizeof(typename STFY::Internal::HasStructifyBase<T>::yes),
                  "Not a Json Object type\n");
    std::string obj = STFY::serializeStruct(value);
    parseContext.tokenizer.resetData(obj.data(), obj.size(), 0);
    tokens.data.clear();
    meta.clear();
    json_data.clear();
    auto error = parseContext.parseTo(tokens);
    if (error == STFY::Error::NoError)
      assert(tokens.data.size() && tokens.data[0].value_type == STFY::Type::ObjectStart);

    meta = metaForTokens(tokens);
    json_data.emplace_back(0, std::move(obj));
    return parseContext.error;
  }

  template <typename T>
  STFY::Error setValue(const std::string &name, STFY::ParseContext &parseContext, const T &value)
  {
    (void)parseContext;
    if (tokens.data.empty())
    {
      tokens.data.reserve(10);
      meta.reserve(10);
      STFY::Token token;
      token.value_type = STFY::Type::ObjectStart;
      token.value = STFY::DataRef("{");
      tokens.data.push_back(token);
      token.value_type = STFY::Type::ObjectEnd;
      token.value = STFY::DataRef("}");
      tokens.data.push_back(token);
      meta = STFY::metaForTokens(tokens);
    }

    auto it = find(name);
    if (it != end())
    {
      meta[0].children--;
      int tokens_removed = 0;
      if (it.index == it.next_complex)
      {
        auto theMeta = meta[it.next_meta];
        tokens_removed = theMeta.size;
        meta[0].complex_children--;
        meta[0].size -= theMeta.size;
        meta[0].skip -= theMeta.skip;
        auto start_token = tokens.data.begin() + it.index;
        tokens.data.erase(start_token, start_token + theMeta.size);
        int to_adjust_index = it.next_meta;
        auto start_meta = meta.begin() + it.next_meta;
        meta.erase(start_meta, start_meta + theMeta.skip);
        for (int i = to_adjust_index; i < int(meta.size()); i++)
        {
          meta[i].position -= theMeta.size;
        }
      }
      else
      {
        meta[0].size--;
        tokens.data.erase(tokens.data.begin() + it.index);
        tokens_removed = 1;
      }
      {
        int index_to_remove = -1;
        for (int i = 0; i < int(json_data.size()); i++)
        {
          if (uint32_t(json_data[i].first) == it.index)
            index_to_remove = i;
          else if (uint32_t(json_data[i].first) > it.index)
            json_data[i].first -= tokens_removed;
        }
        if (index_to_remove >= 0)
        {
          json_data.erase(json_data.begin() + index_to_remove);
        }
      }
    }
    static const char objectStart[] = "{";
    static const char objectEnd[] = "}";
    std::string out;
    STFY::SerializerContext serializeContext(out);
    serializeContext.serializer.setOptions(SerializerOptions(STFY::SerializerOptions::Compact));
    STFY::Token token;
    token.value_type = Type::ObjectStart;
    token.value = DataRef(objectStart);
    serializeContext.serializer.write(token);

    token.name = DataRef(name);
    token.name_type = Type::String;
    STFY::TypeHandler<T>::from(value, token, serializeContext.serializer);

    token.name = DataRef();
    token.value_type = Type::ObjectEnd;
    token.value = DataRef(objectEnd);
    serializeContext.serializer.write(token);

    serializeContext.flush();
    STFY::JsonTokens new_tokens;
    STFY::ParseContext pc(out.c_str(), out.size(), new_tokens);
    auto new_meta = metaForTokens(new_tokens);

    json_data.emplace_back(int(tokens.data.size() - 1), std::move(out));
    int old_tokens_size = int(tokens.data.size());
    tokens.data.insert(tokens.data.end() - 1, new_tokens.data.begin() + 1, new_tokens.data.end() - 1);
    meta[0].children++;
    if (new_meta[0].complex_children)
    {
      meta[0].complex_children++;
      meta[0].size += new_meta[1].size;
      meta[0].skip += new_meta[1].skip;
      int old_meta_size = int(meta.size());
      meta.insert(meta.end(), new_meta.begin() + 1, new_meta.end());
      for (int new_meta_i = old_meta_size; new_meta_i < int(meta.size()); new_meta_i++)
      {
        meta[new_meta_i].position +=
          old_tokens_size - 1 - 1; // position contains an extra and old_tokens_size has another extra
      }
    }
    else
    {
      meta[0].size++;
    }

    return STFY::Error::NoError;
  }
};

template <>
struct TypeHandler<Map>
{
  static inline Error to(Map &to_type, ParseContext &context)
  {
    Error error = TypeHandler<STFY::JsonTokens>::to(to_type.tokens, context);
    if (error == Error::NoError)
    {
      to_type.meta = metaForTokens(to_type.tokens);
    }

    return error;
  }

  static inline void from(const Map &from_type, Token &token, Serializer &serializer)
  {
    if (from_type.tokens.data.empty())
    {
      token.value_type = Type::ObjectStart;
      token.value = DataRef("{");
      serializer.write(token);
      token.name = DataRef("");
      token.value_type = Type::ObjectEnd;
      token.value = DataRef("}");
      serializer.write(token);
      return;
    }

    Token first_token = from_type.tokens.data.front();
    first_token.name = token.name;
    serializer.write(first_token);
    for (int i = 1; i < int(from_type.tokens.data.size()); i++)
    {
      serializer.write(from_type.tokens.data[i]);
    }
  }
};

template <typename T, size_t COUNT>
struct ArrayVariableContent//-V730
{
  T data[COUNT];
  size_t size = 0;
};

template <typename T, size_t COUNT>
struct TypeHandler<ArrayVariableContent<T, COUNT>>
{
  static inline Error to(ArrayVariableContent<T, COUNT> &to_type, ParseContext &context)
  {
    if (context.token.value_type != Type::ArrayStart)
      return STFY::Error::ExpectedArrayStart;

    context.nextToken();
    for (size_t i = 0; i < COUNT; i++)
    {
      if (context.error != STFY::Error::NoError)
        return context.error;
      if (context.token.value_type == Type::ArrayEnd)
      {
        to_type.size = i;
        break;
      }
      context.error = TypeHandler<T>::to(to_type.data[i], context);
      if (context.error != STFY::Error::NoError)
        return context.error;

      context.nextToken();
    }

    if (context.token.value_type != Type::ArrayEnd)
      return STFY::Error::ExpectedArrayEnd;
    return context.error;
  }

  template <typename WriterT>
  static inline void serializeWith(const ArrayVariableContent<T, COUNT> &from_type, Token &token, WriterT &writer)
  {
    token.value_type = Type::ArrayStart;
    token.value = DataRef("[");
    writer.write(token);

    token.name = DataRef("");
    for (size_t i = 0; i < from_type.size; i++)
      WriterDispatch<WriterT>::template call<T>(from_type.data[i], token, writer);

    token.name = DataRef("");
    token.value_type = Type::ArrayEnd;
    token.value = DataRef("]");
    writer.write(token);
  }

  static inline void from(const ArrayVariableContent<T, COUNT> &from_type, Token &token, Serializer &serializer)
  {
    serializeWith(from_type, token, serializer);
  }
  static inline void fromYaml(const ArrayVariableContent<T, COUNT> &from_type, Token &token, YamlWriter &writer)
  {
    serializeWith(from_type, token, writer);
  }
  static inline void fromCbor(const ArrayVariableContent<T, COUNT> &from_type, Token &token, CborWriter &writer)
  {
    serializeWith(from_type, token, writer);
  }
};

template <typename T, typename Set>
struct TypeHandlerSet
{
  static inline Error to(Set &to_type, ParseContext &context)
  {
    if (context.token.value_type != STFY::Type::ArrayStart)
      return Error::ExpectedArrayStart;
    Error error = context.nextToken();
    if (error != STFY::Error::NoError)
      return error;
    to_type.clear();
    while (context.token.value_type != STFY::Type::ArrayEnd)
    {
      T t;
      error = TypeHandler<T>::to(t, context);
      if (error != STFY::Error::NoError)
        break;
      auto insert_ret = to_type.insert(std::move(t));
      if (!insert_ret.second)
        return STFY::Error::DuplicateInSet;

      error = context.nextToken();
      if (error != STFY::Error::NoError)
        break;
    }

    return error;
  }

  template <typename WriterT>
  static inline void serializeWith(const Set &set, Token &token, WriterT &writer)
  {
    token.value_type = Type::ArrayStart;
    token.value = DataRef("[");
    writer.write(token);

    token.name = DataRef("");

    for (auto &index : set)
    {
      WriterDispatch<WriterT>::template call<T>(index, token, writer);
    }

    token.name = DataRef("");

    token.value_type = Type::ArrayEnd;
    token.value = DataRef("]");
    writer.write(token);
  }

  static inline void from(const Set &set, Token &token, Serializer &serializer)
  {
    serializeWith(set, token, serializer);
  }
  static inline void fromYaml(const Set &set, Token &token, YamlWriter &writer)
  {
    serializeWith(set, token, writer);
  }
  static inline void fromCbor(const Set &set, Token &token, CborWriter &writer)
  {
    serializeWith(set, token, writer);
  }
};
} // namespace STFY

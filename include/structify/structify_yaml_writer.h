#pragma once
#include "structify_core.h"
#include <string>
#include <vector>
#include <cstring>

namespace STFY
{

class YamlWriter
{
public:
  YamlWriter(std::string &output, int indent_size = 2)
    : output_(output)
    , indent_size_(indent_size)
  {
  }

  bool write(const Token &token)
  {
    switch (token.value_type)
    {
    case Type::ObjectStart:
      handleScopeStart(ScopeType::Mapping, token.name);
      break;
    case Type::ObjectEnd:
      handleScopeEnd(ScopeType::Mapping);
      break;
    case Type::ArrayStart:
      handleScopeStart(ScopeType::Sequence, token.name);
      break;
    case Type::ArrayEnd:
      handleScopeEnd(ScopeType::Sequence);
      break;
    default:
      writeScalarToken(token);
      break;
    }
    return true;
  }

private:
  enum class ScopeType
  {
    Mapping,
    Sequence
  };

  struct Scope
  {
    ScopeType type;
    int indent;
    bool first_on_dash_line;
    bool has_content;
  };

  std::string &output_;
  int indent_size_;
  std::vector<Scope> stack_;

  void handleScopeStart(ScopeType scope_type, const DataRef &name)
  {
    if (stack_.empty())
    {
      stack_.push_back({scope_type, 0, false, false});
      return;
    }

    Scope &parent = stack_.back();
    parent.has_content = true;

    if (parent.type == ScopeType::Sequence)
    {
      if (parent.first_on_dash_line)
      {
        // Nested scope right after "- " on same line — shouldn't happen normally,
        // but handle gracefully by starting on next line
        output_.push_back('\n');
        parent.first_on_dash_line = false;
        writeIndent(parent.indent);
        output_.append("- ");
      }
      else
      {
        writeIndent(parent.indent);
        output_.append("- ");
      }
      // Push the new scope. First child continues on the dash line.
      int child_indent = parent.indent + indent_size_;
      stack_.push_back({scope_type, child_indent, true, false});
    }
    else
    {
      // In a mapping — write the key header
      if (parent.first_on_dash_line)
      {
        // Continue on the dash line
        writeNameColon(name);
        output_.push_back('\n');
        parent.first_on_dash_line = false;
      }
      else
      {
        writeIndent(parent.indent);
        writeNameColon(name);
        output_.push_back('\n');
      }
      int child_indent = parent.indent + indent_size_;
      stack_.push_back({scope_type, child_indent, false, false});
    }
  }

  void handleScopeEnd(ScopeType scope_type)
  {
    if (stack_.empty())
      return;

    Scope &current = stack_.back();
    if (!current.has_content)
    {
      // Empty scope — write inline form
      if (current.first_on_dash_line)
      {
        // We're on a dash line, just write the empty form
        if (scope_type == ScopeType::Mapping)
          output_.append("{}\n");
        else
          output_.append("[]\n");
      }
      else if (stack_.size() <= 1)
      {
        // Top-level empty
        if (scope_type == ScopeType::Mapping)
          output_.append("{}\n");
        else
          output_.append("[]\n");
      }
      else
      {
        // Empty nested scope — the header "key:\n" was already written.
        // Replace the trailing \n with " {}\n" or " []\n"
        if (!output_.empty() && output_.back() == '\n')
        {
          output_.pop_back();
          if (scope_type == ScopeType::Mapping)
            output_.append(" {}\n");
          else
            output_.append(" []\n");
        }
        else
        {
          if (scope_type == ScopeType::Mapping)
            output_.append("{}\n");
          else
            output_.append("[]\n");
        }
      }
    }

    stack_.pop_back();
  }

  void writeScalarToken(const Token &token)
  {
    if (stack_.empty())
    {
      writeValue(token);
      output_.push_back('\n');
      return;
    }

    Scope &parent = stack_.back();
    parent.has_content = true;

    if (parent.type == ScopeType::Mapping)
    {
      if (parent.first_on_dash_line)
      {
        // Continue on dash line: "- key: value\n"
        writeNameColon(token.name);
        output_.push_back(' ');
        writeValue(token);
        output_.push_back('\n');
        parent.first_on_dash_line = false;
      }
      else
      {
        writeIndent(parent.indent);
        writeNameColon(token.name);
        output_.push_back(' ');
        writeValue(token);
        output_.push_back('\n');
      }
    }
    else
    {
      // Sequence
      if (parent.first_on_dash_line)
      {
        // "- value\n" continuing from outer dash
        output_.append("- ");
        writeValue(token);
        output_.push_back('\n');
        parent.first_on_dash_line = false;
      }
      else
      {
        writeIndent(parent.indent);
        output_.append("- ");
        writeValue(token);
        output_.push_back('\n');
      }
    }
  }

  void writeIndent(int level)
  {
    for (int i = 0; i < level; i++)
      output_.push_back(' ');
  }

  void writeNameColon(const DataRef &name)
  {
    if (needsQuoting(name.data, name.size))
    {
      writeQuotedString(name.data, name.size);
    }
    else
    {
      output_.append(name.data, name.size);
    }
    output_.push_back(':');
  }

  void writeValue(const Token &token)
  {
    switch (token.value_type)
    {
    case Type::String:
    case Type::Ascii:
      writeYamlScalar(token.value.data, token.value.size);
      break;
    case Type::Number:
      output_.append(token.value.data, token.value.size);
      break;
    case Type::Bool:
      output_.append(token.value.data, token.value.size);
      break;
    case Type::Null:
      output_.append("null", 4);
      break;
    default:
      output_.append(token.value.data, token.value.size);
      break;
    }
  }

  void writeYamlScalar(const char *data, size_t size)
  {
    if (size == 0)
    {
      output_.append("\"\"", 2);
      return;
    }
    if (needsQuoting(data, size))
    {
      writeQuotedString(data, size);
    }
    else
    {
      output_.append(data, size);
    }
  }

  bool needsQuoting(const char *data, size_t size) const
  {
    if (size == 0)
      return true;

    // Check for YAML reserved values
    if (looksLikeYamlSpecial(data, size))
      return true;

    // Check for characters that need quoting
    for (size_t i = 0; i < size; i++)
    {
      char c = data[i];
      if (c == ':' || c == '#' || c == '[' || c == ']' || c == '{' || c == '}' || c == ',' || c == '&' || c == '*' ||
          c == '!' || c == '|' || c == '>' || c == '\'' || c == '"' || c == '%' || c == '@' || c == '`' || c == '\n' ||
          c == '\r' || c == '\t')
        return true;
    }

    // Leading/trailing spaces
    if (data[0] == ' ' || data[size - 1] == ' ')
      return true;

    // Starts with indicator character
    char first = data[0];
    if (first == '-' || first == '?' || first == '!')
      return true;

    return false;
  }

  bool looksLikeYamlSpecial(const char *data, size_t size) const
  {
    // null, true, false, yes, no, on, off, ~
    if (size == 4 && (memcmp(data, "null", 4) == 0 || memcmp(data, "Null", 4) == 0 || memcmp(data, "NULL", 4) == 0 ||
                      memcmp(data, "true", 4) == 0 || memcmp(data, "True", 4) == 0 || memcmp(data, "TRUE", 4) == 0))
      return true;
    if (size == 5 &&
        (memcmp(data, "false", 5) == 0 || memcmp(data, "False", 5) == 0 || memcmp(data, "FALSE", 5) == 0))
      return true;
    if (size == 3 &&
        (memcmp(data, "yes", 3) == 0 || memcmp(data, "Yes", 3) == 0 || memcmp(data, "YES", 3) == 0))
      return true;
    if (size == 2 &&
        (memcmp(data, "no", 2) == 0 || memcmp(data, "No", 2) == 0 || memcmp(data, "NO", 2) == 0 ||
         memcmp(data, "on", 2) == 0 || memcmp(data, "On", 2) == 0 || memcmp(data, "ON", 2) == 0))
      return true;
    if (size == 3 &&
        (memcmp(data, "off", 3) == 0 || memcmp(data, "Off", 3) == 0 || memcmp(data, "OFF", 3) == 0))
      return true;
    if (size == 1 && data[0] == '~')
      return true;

    // Check if it looks like a number
    if (looksLikeNumber(data, size))
      return true;

    return false;
  }

  bool looksLikeNumber(const char *data, size_t size) const
  {
    if (size == 0)
      return false;
    size_t i = 0;
    if (data[0] == '-' || data[0] == '+')
      i = 1;
    if (i >= size)
      return false;
    bool has_digit = false;
    while (i < size && data[i] >= '0' && data[i] <= '9')
    {
      has_digit = true;
      i++;
    }
    if (i < size && data[i] == '.')
    {
      i++;
      while (i < size && data[i] >= '0' && data[i] <= '9')
      {
        has_digit = true;
        i++;
      }
    }
    if (has_digit && i < size && (data[i] == 'e' || data[i] == 'E'))
    {
      i++;
      if (i < size && (data[i] == '-' || data[i] == '+'))
        i++;
      while (i < size && data[i] >= '0' && data[i] <= '9')
        i++;
    }
    return has_digit && i == size;
  }

  void writeQuotedString(const char *data, size_t size)
  {
    // Use double-quoted style with YAML escape sequences
    output_.push_back('"');
    for (size_t i = 0; i < size; i++)
    {
      char c = data[i];
      switch (c)
      {
      case '"':
        output_.append("\\\"", 2);
        break;
      case '\\':
        output_.append("\\\\", 2);
        break;
      case '\n':
        output_.append("\\n", 2);
        break;
      case '\r':
        output_.append("\\r", 2);
        break;
      case '\t':
        output_.append("\\t", 2);
        break;
      case '\0':
        output_.append("\\0", 2);
        break;
      default:
        output_.push_back(c);
        break;
      }
    }
    output_.push_back('"');
  }
};

} // namespace STFY

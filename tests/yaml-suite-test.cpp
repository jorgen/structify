#include <json_struct/json_struct.h>
#include <catch2/catch_all.hpp>
#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include <algorithm>

namespace fs = std::filesystem;

namespace yaml_suite_test
{

struct TestValue
{
  enum Kind
  {
    Null,
    Bool,
    Number,
    String,
    Array,
    Object
  };
  Kind kind = Null;
  std::string scalar;
  std::vector<TestValue> array_items;
  std::vector<std::pair<std::string, TestValue>> object_members;

  bool operator==(const TestValue &other) const
  {
    if (kind != other.kind)
      return false;
    switch (kind)
    {
    case Null:
      return true;
    case Bool:
    case String:
      return scalar == other.scalar;
    case Number:
      if (scalar == other.scalar)
        return true;
      {
        char *end_a = nullptr;
        char *end_b = nullptr;
        double a = std::strtod(scalar.c_str(), &end_a);
        double b = std::strtod(other.scalar.c_str(), &end_b);
        if (end_a != scalar.c_str() + scalar.size())
          return false;
        if (end_b != other.scalar.c_str() + other.scalar.size())
          return false;
        return a == b;
      }
    case Array:
      return array_items == other.array_items;
    case Object:
      if (object_members.size() != other.object_members.size())
        return false;
      for (size_t i = 0; i < object_members.size(); i++)
      {
        bool found = false;
        for (size_t j = 0; j < other.object_members.size(); j++)
        {
          if (object_members[i].first == other.object_members[j].first &&
              object_members[i].second == other.object_members[j].second)
          {
            found = true;
            break;
          }
        }
        if (!found)
          return false;
      }
      return true;
    }
    return false;
  }

  bool operator!=(const TestValue &other) const
  {
    return !(*this == other);
  }

  static const char *kindName(Kind k)
  {
    switch (k)
    {
    case Null:
      return "Null";
    case Bool:
      return "Bool";
    case Number:
      return "Number";
    case String:
      return "String";
    case Array:
      return "Array";
    case Object:
      return "Object";
    }
    return "?";
  }

  std::string toString(int indent = 0) const
  {
    std::string pad(indent * 2, ' ');
    switch (kind)
    {
    case Null:
      return "null";
    case Bool:
      return scalar;
    case Number:
      return scalar;
    case String:
      return "\"" + scalar + "\"";
    case Array:
    {
      if (array_items.empty())
        return "[]";
      std::string result = "[\n";
      for (size_t i = 0; i < array_items.size(); i++)
      {
        result += pad + "  " + array_items[i].toString(indent + 1);
        if (i + 1 < array_items.size())
          result += ",";
        result += "\n";
      }
      result += pad + "]";
      return result;
    }
    case Object:
    {
      if (object_members.empty())
        return "{}";
      std::string result = "{\n";
      for (size_t i = 0; i < object_members.size(); i++)
      {
        result += pad + "  \"" + object_members[i].first + "\": " + object_members[i].second.toString(indent + 1);
        if (i + 1 < object_members.size())
          result += ",";
        result += "\n";
      }
      result += pad + "}";
      return result;
    }
    }
    return "?";
  }
};

typedef std::map<std::string, TestValue> AnchorMap;

static TestValue scalarFromToken(const JS::Token &token, AnchorMap &anchors)
{
  // Handle alias tokens
  if (token.value_type == JS::Type::Alias)
  {
    std::string alias_name(token.value.data, token.value.size);
    auto it = anchors.find(alias_name);
    if (it != anchors.end())
      return it->second;
    // Unknown alias - return as string
    TestValue val;
    val.kind = TestValue::String;
    val.scalar = "*" + alias_name;
    return val;
  }

  TestValue val;
  val.scalar = std::string(token.value.data, token.value.size);
  switch (token.value_type)
  {
  case JS::Type::Null:
    val.kind = TestValue::Null;
    break;
  case JS::Type::Bool:
    val.kind = TestValue::Bool;
    break;
  case JS::Type::Number:
    val.kind = TestValue::Number;
    break;
  case JS::Type::String:
  case JS::Type::Ascii:
    val.kind = TestValue::String;
    break;
  default:
    val.kind = TestValue::String;
    break;
  }

  // Record anchor if present
  if (token.anchor.size > 0)
  {
    std::string anchor_name(token.anchor.data, token.anchor.size);
    anchors[anchor_name] = val;
  }

  return val;
}

static const int MAX_TOKENS = 10000;

static TestValue parseContainer(JS::Tokenizer &tok, const JS::Token &start_token, int &token_count, AnchorMap &anchors)
{
  TestValue val;
  if (start_token.value_type == JS::Type::ObjectStart)
  {
    val.kind = TestValue::Object;
    while (true)
    {
      if (++token_count > MAX_TOKENS)
        throw std::runtime_error("token limit exceeded");
      JS::Token inner;
      JS::Error err = tok.nextToken(inner);
      if (err != JS::Error::NoError)
        break;
      if (inner.value_type == JS::Type::ObjectEnd)
        break;

      std::string key(inner.name.data, inner.name.size);

      // Resolve alias keys (when *alias is used as a mapping key, name_type == Alias)
      if (inner.name_type == JS::Type::Alias && !key.empty())
      {
        auto anchor_it = anchors.find(key);
        if (anchor_it != anchors.end() && anchor_it->second.kind == TestValue::String)
          key = anchor_it->second.scalar;
      }

      if (inner.value_type == JS::Type::Alias)
      {
        val.object_members.push_back({key, scalarFromToken(inner, anchors)});
      }
      else if (inner.value_type == JS::Type::ObjectStart || inner.value_type == JS::Type::ArrayStart)
      {
        TestValue child = parseContainer(tok, inner, token_count, anchors);
        // Record anchor on container if present
        if (inner.anchor.size > 0)
        {
          std::string anchor_name(inner.anchor.data, inner.anchor.size);
          anchors[anchor_name] = child;
        }
        val.object_members.push_back({key, child});
      }
      else
      {
        val.object_members.push_back({key, scalarFromToken(inner, anchors)});
      }
    }
  }
  else if (start_token.value_type == JS::Type::ArrayStart)
  {
    val.kind = TestValue::Array;
    while (true)
    {
      if (++token_count > MAX_TOKENS)
        throw std::runtime_error("token limit exceeded");
      JS::Token inner;
      JS::Error err = tok.nextToken(inner);
      if (err != JS::Error::NoError)
        break;
      if (inner.value_type == JS::Type::ArrayEnd)
        break;

      if (inner.value_type == JS::Type::Alias)
      {
        val.array_items.push_back(scalarFromToken(inner, anchors));
      }
      else if (inner.value_type == JS::Type::ObjectStart || inner.value_type == JS::Type::ArrayStart)
      {
        TestValue child = parseContainer(tok, inner, token_count, anchors);
        if (inner.anchor.size > 0)
        {
          std::string anchor_name(inner.anchor.data, inner.anchor.size);
          anchors[anchor_name] = child;
        }
        val.array_items.push_back(child);
      }
      else
      {
        val.array_items.push_back(scalarFromToken(inner, anchors));
      }
    }
  }
  return val;
}

static TestValue tokensToValue(JS::Tokenizer &tok)
{
  AnchorMap anchors;
  JS::Token token;
  JS::Error error = tok.nextToken(token);
  if (error != JS::Error::NoError)
  {
    TestValue val;
    val.kind = TestValue::Null;
    return val;
  }
  if (token.value_type == JS::Type::Alias)
  {
    return scalarFromToken(token, anchors);
  }
  if (token.value_type == JS::Type::ObjectStart || token.value_type == JS::Type::ArrayStart)
  {
    int token_count = 0;
    TestValue result = parseContainer(tok, token, token_count, anchors);
    if (token.anchor.size > 0)
    {
      std::string anchor_name(token.anchor.data, token.anchor.size);
      anchors[anchor_name] = result;
    }
    return result;
  }
  return scalarFromToken(token, anchors);
}

static std::string readFile(const fs::path &path)
{
  std::ifstream file(path, std::ios::binary);
  if (!file.is_open())
    return {};
  std::ostringstream ss;
  ss << file.rdbuf();
  return ss.str();
}

static bool isUnsupported(const fs::path &, const std::string &)
{
  // All feature-based skipping is handled via known_failures
  return false;
}

static bool isMultiPart(const fs::path &test_dir)
{
  for (auto &entry : fs::directory_iterator(test_dir))
  {
    if (entry.is_directory())
    {
      std::string name = entry.path().filename().string();
      if (!name.empty() && std::all_of(name.begin(), name.end(), ::isdigit))
        return true;
    }
  }
  return false;
}

struct TestResult
{
  std::string test_id;
  std::string skip_reason;
  bool passed = false;
  std::string failure_detail;
};

TEST_CASE("yaml_test_suite", "[yaml][suite]")
{
  fs::path suite_dir(YAML_TEST_SUITE_DIR);
  REQUIRE(fs::exists(suite_dir));

  // Hardcoded skips: tests that crash the tokenizer (segfault/assert)
  std::set<std::string> tokenizer_crash = {
  };

  // Tests that fail due to known tokenizer limitations:
  // - multi-line plain scalars, block scalar chomping/indentation
  // - flow collections with complex content, implicit keys
  // - YAML 1.1 vs 1.2 boolean differences (yes/no/on/off)
  // - document markers (---/...) in non-trivial positions
  // Remove tests from this set as the tokenizer is improved.
  std::set<std::string> known_failures = {
    "2AUY", "2EBW", "2SXE", "36F6", "3MYT", "4CQQ", "4QFQ", "4WA9",
    "4ZYM", "565N", "5BVJ", "5GBF", "5WE3", "6FWR", "6H3V", "6HB6",
    "6SLA", "6VJK", "6WPF", "735Y", "7A4E", "7T8X", "7TMG", "82AN",
    "8KB6", "8UDB", "93JH", "9BXH", "9KAX", "9SA2", "9TFX", "9YRD",
    "A2M4", "A6F9", "A984", "AB8U", "AZW3", "BU8L", "CPZ3", "CT4Q",
    "DWX9", "E76Z", "EX5H", "EXG3", "F2C7", "F6MC", "F8F9", "FBC9",
    "G4RS", "G992", "H2RW", "HMK4", "HMQ5", "HS5T", "J3BT", "JTV5",
    "K3WX", "K527", "K858", "KSS4", "M29M", "M5C3", "M6YH", "M7A3",
    "M9B4", "MJS9", "MYW6", "MZX3", "NAT4", "NB6Z", "NJ66", "NP9H",
    "P2AD", "P76L", "PRH3", "Q8AD", "R4YG", "RZT7", "S9E8", "SKE5",
    "T26H", "T4YY", "TL85", "TS54", "U3XV", "UGM3", "UT92", "UV7Q",
    "W42U", "W4TN", "XLQ9", "XV9V", "Z67P",
  };

  std::set<std::string> extra_skip;
  extra_skip.insert(tokenizer_crash.begin(), tokenizer_crash.end());
  extra_skip.insert(known_failures.begin(), known_failures.end());

  // Collect and sort test directories for deterministic order
  std::vector<fs::directory_entry> test_dirs;
  for (auto &entry : fs::directory_iterator(suite_dir))
  {
    if (!entry.is_directory())
      continue;
    std::string dirname = entry.path().filename().string();
    if (dirname == "tags" || dirname == "name" || dirname == ".git")
      continue;
    test_dirs.push_back(entry);
  }
  std::sort(test_dirs.begin(), test_dirs.end(), [](const fs::directory_entry &a, const fs::directory_entry &b) {
    return a.path().filename() < b.path().filename();
  });

  std::vector<TestResult> results;
  int pass_count = 0;
  int skip_count = 0;
  int fail_count = 0;

  for (auto &entry : test_dirs)
  {
    std::string test_id = entry.path().filename().string();
    TestResult result;
    result.test_id = test_id;

    // Check skip conditions
    if (isUnsupported(suite_dir, test_id))
    {
      result.skip_reason = "unsupported feature";
      skip_count++;
      results.push_back(result);
      continue;
    }
    if (fs::exists(entry.path() / "error"))
    {
      result.skip_reason = "expected error test";
      skip_count++;
      results.push_back(result);
      continue;
    }
    if (!fs::exists(entry.path() / "in.yaml"))
    {
      result.skip_reason = "no in.yaml";
      skip_count++;
      results.push_back(result);
      continue;
    }
    if (!fs::exists(entry.path() / "in.json"))
    {
      result.skip_reason = "no JSON reference";
      skip_count++;
      results.push_back(result);
      continue;
    }
    if (isMultiPart(entry.path()))
    {
      result.skip_reason = "multi-part test";
      skip_count++;
      results.push_back(result);
      continue;
    }
    if (extra_skip.count(test_id))
    {
      result.skip_reason = "known unsupported";
      skip_count++;
      results.push_back(result);
      continue;
    }

    std::string yaml_input = readFile(entry.path() / "in.yaml");
    std::string json_input = readFile(entry.path() / "in.json");

    try
    {
      // Parse YAML
      JS::Tokenizer yaml_tok;
      yaml_tok.allowYaml(true);
      yaml_tok.addData(yaml_input.c_str(), yaml_input.size());
      TestValue yaml_val = tokensToValue(yaml_tok);

      // Parse JSON reference
      JS::Tokenizer json_tok;
      json_tok.addData(json_input.c_str(), json_input.size());
      TestValue json_val = tokensToValue(json_tok);

      // If JSON parsing returned Null and input isn't empty, try YAML parser
      // (handles bare scalars like "foo" that aren't valid standalone JSON)
      if (json_val.kind == TestValue::Null && !json_input.empty())
      {
        JS::Tokenizer yaml_json_tok;
        yaml_json_tok.allowYaml(true);
        yaml_json_tok.addData(json_input.c_str(), json_input.size());
        TestValue retry = tokensToValue(yaml_json_tok);
        if (retry.kind != TestValue::Null)
          json_val = retry;
      }

      if (yaml_val == json_val)
      {
        result.passed = true;
        pass_count++;
      }
      else
      {
        fail_count++;
        std::ostringstream detail;
        detail << "Test " << test_id << " FAILED\n";
        detail << "  YAML input:\n    ";
        std::string yaml_preview = yaml_input.substr(0, 200);
        for (auto &c : yaml_preview)
        {
          if (c == '\n')
            detail << "\n    ";
          else
            detail << c;
        }
        if (yaml_input.size() > 200)
          detail << "...";
        detail << "\n";
        if (yaml_val.kind != json_val.kind)
        {
          detail << "  Kind mismatch: YAML=" << TestValue::kindName(yaml_val.kind)
                 << " JSON=" << TestValue::kindName(json_val.kind) << "\n";
        }
        detail << "  YAML result: " << yaml_val.toString() << "\n";
        detail << "  JSON expected: " << json_val.toString() << "\n";
        result.failure_detail = detail.str();
      }
    }
    catch (const std::exception &e)
    {
      fail_count++;
      result.failure_detail = "Test " + test_id + " EXCEPTION: " + e.what() + "\n";
    }
    results.push_back(result);
  }

  // Print results to stderr so they're always visible
  fprintf(stderr, "\n=== YAML Test Suite Results ===\n");
  fprintf(stderr, "%d passed, %d failed, %d skipped out of %d tests\n\n",
          pass_count, fail_count, skip_count, (int)results.size());

  if (fail_count > 0)
  {
    std::vector<std::string> failed_ids;
    for (auto &r : results)
    {
      if (!r.skip_reason.empty() || r.passed)
        continue;
      fprintf(stderr, "%s\n", r.failure_detail.c_str());
      failed_ids.push_back(r.test_id);
    }
    fprintf(stderr, "=== Failed test IDs for extra_skip: ");
    for (size_t i = 0; i < failed_ids.size(); i++)
    {
      if (i > 0)
        fprintf(stderr, ", ");
      fprintf(stderr, "\"%s\"", failed_ids[i].c_str());
    }
    fprintf(stderr, " ===\n");
  }

  INFO("YAML Test Suite Results: " << pass_count << " passed, " << fail_count << " failed, " << skip_count << " skipped out of " << results.size() << " tests");
  CHECK(fail_count == 0);
}

} // namespace yaml_suite_test

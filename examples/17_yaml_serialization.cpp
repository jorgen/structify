#include <string>
#include <vector>
#include <structify/structify.h>

// Example: Serializing C++ structs to YAML.
//
// structify can serialize structs to YAML block-style output,
// complementing the existing YAML parsing support.
// This enables full round-trip: YAML -> C++ struct -> YAML.

struct ServerSettings
{
  std::string host;
  int port = 0;
  int workers = 1;

  STFY_OBJ(host, port, workers);
};

struct DatabaseSettings
{
  std::string host;
  int port = 0;
  std::string name;
  int max_connections = 10;

  STFY_OBJ(host, port, name, max_connections);
};

struct AppConfig
{
  std::string name;
  double version = 0.0;
  ServerSettings server;
  DatabaseSettings database;
  std::vector<std::string> features;

  STFY_OBJ(name, version, server, database, features);
};

int main()
{
  // Build a config struct
  AppConfig config;
  config.name = "my-service";
  config.version = 2.1;
  config.server.host = "0.0.0.0";
  config.server.port = 8080;
  config.server.workers = 4;
  config.database.host = "db.example.com";
  config.database.port = 5432;
  config.database.name = "myapp_db";
  config.database.max_connections = 100;
  config.features.push_back("authentication");
  config.features.push_back("rate-limiting");
  config.features.push_back("metrics");

  // Serialize to YAML
  std::string yaml = STFY::serializeStructYaml(config);
  fprintf(stdout, "=== Serialized YAML ===\n%s\n", yaml.c_str());

  // Serialize to JSON for comparison
  std::string json = STFY::serializeStruct(config);
  fprintf(stdout, "=== Serialized JSON ===\n%s\n", json.c_str());

  // Round-trip: parse the YAML back into a new struct
  AppConfig parsed;
  STFY::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml.data(), yaml.size());
  context.allow_missing_members = true;
  context.allow_unasigned_required_members = true;
  auto error = context.parseTo(parsed);

  if (error != STFY::Error::NoError)
  {
    fprintf(stderr, "Round-trip parse error: %s\n", context.makeErrorString().c_str());
    return 1;
  }

  fprintf(stdout, "=== Round-trip verified ===\n");
  fprintf(stdout, "Name: %s\n", parsed.name.c_str());
  fprintf(stdout, "Version: %.1f\n", parsed.version);
  fprintf(stdout, "Server: %s:%d (%d workers)\n", parsed.server.host.c_str(), parsed.server.port,
          parsed.server.workers);
  fprintf(stdout, "Database: %s:%d/%s (max %d)\n", parsed.database.host.c_str(), parsed.database.port,
          parsed.database.name.c_str(), parsed.database.max_connections);
  fprintf(stdout, "Features:\n");
  for (const auto &f : parsed.features)
  {
    fprintf(stdout, "  - %s\n", f.c_str());
  }

  return 0;
}

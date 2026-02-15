#include <string>
#include <vector>
#include <json_struct/json_struct.h>

// Example: Parsing a YAML configuration file into C++ structs.
//
// json_struct's tokenizer supports YAML natively - no external YAML
// library needed. Just enable YAML mode and parse directly into your
// structs, the same way you would with JSON.

const char yaml_config[] = R"(# Application configuration
name: my-service
version: 2.1

server:
  host: 0.0.0.0
  port: 8080
  workers: 4

database:
  host: db.example.com
  port: 5432
  name: myapp_db
  max_connections: 100

features:
  - authentication
  - rate-limiting
  - metrics

description: >
  A high-performance API gateway
  that handles authentication and
  request routing.

tls_cert: |
  -----BEGIN CERTIFICATE-----
  MIIBxTCCAWugAwIBAgIJALP2kEZ5
  -----END CERTIFICATE-----
)";

struct ServerSettings
{
  std::string host;
  int port = 0;
  int workers = 1;

  JS_OBJ(host, port, workers);
};

struct DatabaseSettings
{
  std::string host;
  int port = 0;
  std::string name;
  int max_connections = 10;

  JS_OBJ(host, port, name, max_connections);
};

struct AppConfig
{
  std::string name;
  double version = 0.0;
  ServerSettings server;
  DatabaseSettings database;
  std::vector<std::string> features;
  std::string description;
  std::string tls_cert;

  JS_OBJ(name, version, server, database, features, description, tls_cert);
};

int main()
{
  AppConfig config;
  JS::ParseContext context;
  context.tokenizer.allowYaml(true);
  context.tokenizer.addData(yaml_config, sizeof(yaml_config) - 1);

  if (context.parseTo(config) != JS::Error::NoError)
  {
    std::string errorStr = context.makeErrorString();
    fprintf(stderr, "Error parsing YAML: %s\n", errorStr.c_str());
    return -1;
  }

  fprintf(stdout, "Application: %s v%.1f\n\n", config.name.c_str(), config.version);

  fprintf(stdout, "Server:\n");
  fprintf(stdout, "  Host: %s\n", config.server.host.c_str());
  fprintf(stdout, "  Port: %d\n", config.server.port);
  fprintf(stdout, "  Workers: %d\n\n", config.server.workers);

  fprintf(stdout, "Database:\n");
  fprintf(stdout, "  Host: %s\n", config.database.host.c_str());
  fprintf(stdout, "  Port: %d\n", config.database.port);
  fprintf(stdout, "  Name: %s\n", config.database.name.c_str());
  fprintf(stdout, "  Max Connections: %d\n\n", config.database.max_connections);

  fprintf(stdout, "Features:\n");
  for (const auto &feature : config.features)
  {
    fprintf(stdout, "  - %s\n", feature.c_str());
  }

  fprintf(stdout, "\nDescription: %s\n", config.description.c_str());
  fprintf(stdout, "TLS Certificate:\n%s", config.tls_cert.c_str());

  return 0;
}

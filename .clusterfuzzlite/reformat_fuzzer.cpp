#include <structify/structify.h>
#include <string>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
  std::string pretty;
  STFY::reformat((const char *)data, size, pretty);

  return 0;
}
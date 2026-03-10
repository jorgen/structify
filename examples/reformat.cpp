#include <string>
#include <structify/structify.h>

const char json[] = R"json({"key":"value","number":100,"boolean":true})json";

int main()
{

    std::string pretty_json;
    STFY::reformat(json, pretty_json);

    fprintf(stdout, "Json after reformat:\n%s\n", pretty_json.c_str());

    return 0;
}


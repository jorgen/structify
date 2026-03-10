#include <string>
#include <structify/structify.h>

const char json[] = R"json(
{
    "key" : "value",
    "number" : 100,
    "bool" : true
}
)json";

struct JsonData
{
    std::string key;
    int n;
    bool boolean;

    STFY_OBJECT(STFY_MEMBER(key),
              STFY_MEMBER_ALIASES(n, "number", "num", "or_something_else"),
              STFY_MEMBER_WITH_NAME(boolean, "bool"));
};

// STFY_MEMBER_ALIASES adds additional names to a member
// STFY_MEMBER_WITH_NAME replaces the lookup name

int main()
{
    JsonData dataStruct;
    STFY::ParseContext parseContext(json);
    if (parseContext.parseTo(dataStruct) != STFY::Error::NoError)
    {
        std::string errorStr = parseContext.makeErrorString();
        fprintf(stderr, "Error parsing struct %s\n", errorStr.c_str());
        return -1;
    }

    fprintf(stdout, "Key is: %s, number is %d bool is %d\n",
            dataStruct.key.c_str(),
            dataStruct.n,
            dataStruct.boolean);

    return 0;
}


#include <string>
#include <structify/structify.h>

const char json[] = R"json(
{
    "key" : "value",
    "number" : 100,
    "boolean" : true,
    "additional" : "This is defined in the superclass",
    "data" : 1234
}
)json";

struct JsonData
{
    std::string key;
    int number;
    bool boolean;

    STFY_OBJ(key, number, boolean);
};

struct SubClass : public JsonData
{
    std::string additional;
    double data;

    STFY_OBJ_SUPER(STFY_SUPER(JsonData), additional, data);
};

int main()
{
    SubClass dataStruct;
    STFY::ParseContext parseContext(json);
    if (parseContext.parseTo(dataStruct) != STFY::Error::NoError)
    {
        std::string errorStr = parseContext.makeErrorString();
        fprintf(stderr, "Error parsing struct %s\n", errorStr.c_str());
        return -1;
    }

    fprintf(stdout, "Key is: %s, number is %d bool is %d additional is %s data is %f\n",
            dataStruct.key.c_str(),
            dataStruct.number,
            dataStruct.boolean,
            dataStruct.additional.c_str(),
            dataStruct.data);

    return 0;
}


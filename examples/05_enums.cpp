#include <stdio.h>
#include <structify/structify.h>

STFY_ENUM(Color, Red , Green , Blue, Yellow4 , Purple )

struct ColorData
{
    Color color;

    STFY_OBJ(color);
};
STFY_ENUM_DECLARE_STRING_PARSER(Color)

const char json[] = R"json({
    "color" : "Green"
})json";

int main()
{
    ColorData dataStruct;
    STFY::ParseContext parseContext(json);
    if (parseContext.parseTo(dataStruct) != STFY::Error::NoError)
    {
        std::string errorStr = parseContext.makeErrorString();
        fprintf(stderr, "Error parsing struct %s\n", errorStr.c_str());
        return -1;
    }

    fprintf(stdout, "Color data is: %d, enum green has value %d\n",
            (int)dataStruct.color,
            (int)Color::Green);

    return 0;
}


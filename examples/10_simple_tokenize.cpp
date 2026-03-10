#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>

#include <structify/structify.h>

const char json[] = R"json(
{
    // This is a comment
    "key" : "value", // Another comment
    "number" : 100,
    // Comments can be on their own line
    "boolean" : true,
    "object" : {
      "array": [
      ]
    }
}
)json";

int main()
{
    std::string key;
    int number;
    bool boolean;
    STFY::Error error;
    STFY::Tokenizer tokenizer;
    tokenizer.allowComments(true);  // Enable comment support
    STFY::Token token;
    tokenizer.addData(json);
    error = tokenizer.nextTokens(&token, 1).error;
    if (token.value_type == STFY::Type::ObjectStart) {
        error = tokenizer.nextTokens(&token, 1).error;
        if (error != STFY::Error::NoError)
            exit(1);
        key = std::string(token.value.data, token.value.size);
        error = tokenizer.nextTokens(&token, 1).error;
        if (error != STFY::Error::NoError)
            exit(1);
        char *outpointer;
        number = strtol(token.value.data,
                        &outpointer,
                        10);
        error = tokenizer.nextTokens(&token, 1).error;
        if (error != STFY::Error::NoError)
            exit(1);
        boolean = std::string(token.value.data, token.value.size) == "true";
        error = tokenizer.nextTokens(&token, 1).error;
        if (error != STFY::Error::NoError)
            exit(1);
        fprintf(stdout, "Parsed data %s - %d - %d\n", key.c_str(), number, boolean);
    }
    return 0;
}

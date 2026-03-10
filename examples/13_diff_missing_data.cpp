#include <string>
#include <vector>
#include <structify/structify_diff.h>

const char jsonBase[] = R"json(
[
    {
        "key" : "val1",
        "number" : 1,
        "value" : 1.23456,
        "boolean" : true
    },
    {
        "key" : "val2",
        "boolean" : false
    },
    {
        "key" : "val3",
        "number" : 3,
        "value" : 1.23456,
        "boolean" : false
    },
    123
]
)json";

const char jsonDiff[] = R"json(
[
    {
        "number" : 0,
        "key" : "val1",
        "value" : 1.23456001
    },
    {
        "key" : "val2",
        "number" : 2,
        "boolean" : false,
        "value" : 1.23456
    },
    {
        "_number" : 3,
        "key" : "val3",
        "_value" : 1.23456,
        "_boolean" : false
    },
    1.23,
    "NewArrayItem"
]
)json";

bool isKeyValuePair(const STFY::Token& token)
{
    return token.name.size != 0;
}

int main()
{
    fprintf(stdout, "Performing JSON diff between\n%s\nand\n%s\n...\n\n", jsonBase, jsonDiff);

    STFY::DiffContext diffContext(jsonBase);
    size_t diffPos = diffContext.diff(jsonDiff);

    if (diffContext.error != STFY::DiffError::NoError)
    {
        fprintf(stderr, "Error while diffing JSON: %d\n", static_cast<int>(diffContext.error));
        return 1;
    }

    const STFY::DiffTokens& diffTokens = diffContext.diffs[diffPos];
    fprintf(stdout, "Number of diffs: %zu\n", diffTokens.diff_count);
    
    for (size_t i = 0; i < diffTokens.size(); i++)
    {
        const STFY::Token& diffToken = diffTokens.tokens.data[i];
        STFY::DiffType diffType = diffTokens.diffs[i];

        // Both of these may be empty.
        std::string key(diffToken.name.data, diffToken.name.size);
        std::string value(diffToken.value.data, diffToken.value.size);

        switch (diffType)
        {
            case STFY::DiffType::NoDiff:
            {
                if (isKeyValuePair(diffToken))
                    fprintf(stdout, "No diff: %s: %s\n", key.c_str(), value.c_str());
                else
                    fprintf(stdout, "No diff: %s\n", value.c_str());
                break;
            }
            case STFY::DiffType::ValueDiff:
            {
                if (isKeyValuePair(diffToken))
                    fprintf(stdout, "Value diff: %s: %s\n", key.c_str(), value.c_str());
                else
                    fprintf(stdout, "Value diff: %s\n", value.c_str());
                break;
            }
            case STFY::DiffType::TypeDiff:
            {
                fprintf(stdout, "Type diff: %s\n", value.c_str());
                break;
            }
            case STFY::DiffType::NewMember:
            {
                fprintf(stdout, "New member: %s: %s\n", key.c_str(), value.c_str());
                break;
            }
            case STFY::DiffType::NewArrayItem:
            {
                // Note: No new objects or arrays in this example. Otherwise we must check for
                // diffType == STFY::Type::ObjectStart or diffType == STFY::Type::ArrayStart and iterate
                // through that object or array.
                fprintf(stdout, "New array item: %s: %s\n", key.c_str(), value.c_str());
                break;
            }
            case STFY::DiffType::MissingMembers:
            {
                const auto* missingMembers = diffTokens.getMissingMembers(diffToken);
                if (missingMembers)
                {
                    for (const auto &m : *missingMembers)
                    {
                      std::string missingStr = "\"" + std::string(m.name.data, m.name.size) + "\"";
                      fprintf(stdout, "Missing member: %s\n", missingStr.c_str());
                    }
                }
                break;
            }
            case STFY::DiffType::MissingArrayItems:
            case STFY::DiffType::RootItemDiff:
            case STFY::DiffType::ErroneousRootItem:
            default:
            {
                fprintf(stdout, "Other diff type :%d.\n", static_cast<int>(diffType)); // Does not occur in this example.
                break;
            }
        }
    }

    return 0;
}

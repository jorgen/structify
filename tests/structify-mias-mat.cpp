#include "catch2/catch_all.hpp"
#include <structify/structify.h>

namespace
{

const char ingredientJsonError[] = R"json(
{
  "id": 666,
  "name": "Polser",
  "base_unit": "g",
  "description": "Fooofooofooo",
  "shop_group_id": 4,
  "energy_per_unit", 3.14,
  "vegetarian": true,
  "vegan": false,
  "grams_per_deciliter": 0.43,
  "grams_per_dimensionless": 0.0,
  "unit": "dl",
  "quantity": 2.0,
  "group_name": "topping",
  "recipe_id": 5,
  "recipe_name": "gryta",
  "use_ingredient_groups": true,
  "portions": 2,
  "portions_unit": "porsjon",
  "shop_group_name": "frukt und grunt",
  "allergens": [
    "fisk",
    "gluten"
  ]
}
)json";

const char shoppingListNameSkipJson[] = R"json(
{
  "ingredients": [
    {
      "id": 123,
      "recipe_name_id_list": {
        "items": []
      },
      "allergens": [],
      "name": "babyleafblader"
    }
  ],
  "userDefinedItems": [
  ],
  "notes": "",
  "fileVersion": 2,
  "sortOrder": 2,
  "name": "Handleliste",
  "dateExplicit": "9. november 2017",
  "timestamp": "2017-11-09 21-52-05",
  "isAutomaticSave": false
}
)json";

struct RecipeNameIdItem
{
  RecipeNameIdItem()
  {
  }

  RecipeNameIdItem(const std::string &recipe_name, int recipe_id)
    : recipe_name(recipe_name)
    , recipe_id(recipe_id)
  {
  }

  std::string recipe_name;
  int recipe_id = 0;

  STFY_OBJECT(STFY_MEMBER(recipe_name), STFY_MEMBER(recipe_id));
};

struct RecipeNameIdList
{
  std::vector<RecipeNameIdItem> items;

  STFY_OBJECT(STFY_MEMBER(items));
};

struct ShoppingListItemCPP
{
  bool selected;

  STFY_OBJECT(STFY_MEMBER(selected));
};

struct IngredientCPP : public ShoppingListItemCPP
{
  int id;
  std::string name;
  std::string base_unit;
  std::string description;
  int shop_group_id;
  float energy_per_unit;
  bool vegetarian;
  bool vegan;
  float grams_per_deciliter;
  float grams_per_dimensionless;

  std::string unit;
  float quantity;
  std::string group_name;

  int recipe_id;
  std::string recipe_name;
  bool use_ingredient_groups;
  float portions;
  std::string portions_unit;

  std::string shop_group_name;

  std::vector<std::string> allergens;

  RecipeNameIdList recipe_name_id_list;

  STFY_OBJECT_WITH_SUPER(STFY_SUPER_CLASSES(STFY_SUPER_CLASS(ShoppingListItemCPP)), STFY_MEMBER(id), STFY_MEMBER(name),
                       STFY_MEMBER(base_unit), STFY_MEMBER(description), STFY_MEMBER(shop_group_id),
                       STFY_MEMBER(energy_per_unit), STFY_MEMBER(vegetarian), STFY_MEMBER(vegan),
                       STFY_MEMBER(grams_per_deciliter), STFY_MEMBER(grams_per_dimensionless), STFY_MEMBER(unit),
                       STFY_MEMBER(quantity), STFY_MEMBER(group_name), STFY_MEMBER(recipe_id), STFY_MEMBER(recipe_name),
                       STFY_MEMBER(use_ingredient_groups), STFY_MEMBER(portions), STFY_MEMBER(portions_unit),
                       STFY_MEMBER(shop_group_name), STFY_MEMBER(allergens), STFY_MEMBER(recipe_name_id_list));
};

struct UserDefinedItemCPP : public ShoppingListItemCPP
{
  std::string text;

  STFY_OBJECT_WITH_SUPER(STFY_SUPER_CLASSES(STFY_SUPER_CLASS(ShoppingListItemCPP)), STFY_MEMBER(text));
};

struct ShoppingListFileBase
{
  int fileVersion;
  int sortOrder;
  std::string name;
  std::string dateExplicit;
  std::string timestamp;
  bool isAutomaticSave;

  STFY_OBJECT(STFY_MEMBER(fileVersion), STFY_MEMBER(sortOrder), STFY_MEMBER(name), STFY_MEMBER(dateExplicit),
            STFY_MEMBER(timestamp), STFY_MEMBER(isAutomaticSave));
};

struct ShoppingListFileVersion02 : public ShoppingListFileBase
{
  std::vector<IngredientCPP> ingredients;
  std::vector<UserDefinedItemCPP> userDefinedItems;
  std::string notes;

  STFY_OBJECT_WITH_SUPER(STFY_SUPER_CLASSES(STFY_SUPER_CLASS(ShoppingListFileBase)), STFY_MEMBER(ingredients),
                       STFY_MEMBER(userDefinedItems), STFY_MEMBER(notes));
};

TEST_CASE("mias_mat_special_unit_test", "[structify]")
{
  IngredientCPP ingredient;
  STFY::ParseContext pc(ingredientJsonError);
  auto error = pc.parseTo(ingredient);
  REQUIRE(error == STFY::Error::ExpectedDelimiter);

  ShoppingListFileBase fileBase;
  STFY::ParseContext nameContext(shoppingListNameSkipJson);
  error = nameContext.parseTo(fileBase);
  REQUIRE(error == STFY::Error::NoError);
  REQUIRE(fileBase.name == "Handleliste");
}
} // namespace

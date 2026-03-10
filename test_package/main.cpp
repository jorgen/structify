#include "structify/structify.h"
#include <iostream>

struct MyTestStruct
{
    std::string name;
    unsigned age;
    STFY_OBJ(name, age);
};

int main()
{
    MyTestStruct person;
    person.name="Jonh";
    person.age=23;

    std::string person_json = STFY::serializeStruct(person);
    std::cout << person_json << std::endl;

    return 0;
}

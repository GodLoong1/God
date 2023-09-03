#include "god/http/HttpController.h"
#include "god/utils/Object.h"

#include <iostream>
#include <cassert>

using namespace god;

class TestA : public Object<TestA>
{
};

class TestB : public Object<TestB>
{
};

int main()
{
    // TestA::ClassTypeName();
    // TestB::ClassTypeName();

    std::cout << "AllClassName start" << std::endl;
    for (const auto& name : ObjectMap::GetAllClassName())
    {
        std::cout << name << std::endl;
    }
    std::cout << "AllClassName end" << std::endl;
}
#include "god/utils/Object.h"

#include <iostream>
#include <cassert>

using namespace god;

class TestA : public Object<TestA>
{
public:
    TestA() { }
};

class TestB : public Object<TestB>
{
public:
    TestB() { }
};

int main()
{
    for (auto& str : ObjectMap::GetAllClassName())
    {
        std::cout << "register: " << str << std::endl;
    }
}
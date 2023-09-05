#include "god/utils/Object.h"

#include <iostream>

#define MakeClass(name) \
class name : public god::Object<name> \
{ \
public: \
    name() { } \
    void show() { std::cout << #name << std::endl; } \
};

MakeClass(饮月)
MakeClass(三月七)
MakeClass(雷神)
MakeClass(八重神子)
MakeClass(星)

int main()
{
    for (auto& str : god::ObjectMap::GetAllClassName())
    {
        std::cout << "register class: " << str << std::endl;
    }

    god::ObjectMap::GetSingleInstance<雷神>()->show();
    god::ObjectMap::GetSingleInstance<三月七>()->show();
    god::ObjectMap::GetSingleInstance<饮月>()->show();
    god::ObjectMap::GetSingleInstance<八重神子>()->show();

    auto ptr = god::ObjectMap::GetSingleInstance("星");
    std::cout << ptr->className() << std::endl;
}
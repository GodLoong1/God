#include "god/utils/FunctionTraits.h"
#include <iostream>

using namespace god;

void myFunc()
{

}

int main()
{
    auto func = [](int){};
    using traits = FunctionTraits<decltype(func)>;

    std::cout << traits::isClassFunction << std::endl;
    std::cout << traits::isObjectClass << std::endl;
    std::cout << traits::arity << std::endl;


    struct MyClass
    {
        void func(char, int)
        {
        }
    };

    using traits2 = FunctionTraits<decltype(&MyClass::func)>;

    std::cout << traits2::isClassFunction << std::endl;
    std::cout << traits2::isObjectClass << std::endl;
    std::cout << traits2::arity << std::endl;

    using traits3 = FunctionTraits<std::function<void(int, double, float)>>;

    std::cout << traits3::isClassFunction << std::endl;
    std::cout << traits3::isObjectClass << std::endl;
    std::cout << traits3::arity << std::endl;

    using traits4 = FunctionTraits<decltype(&myFunc)>;
    std::cout << traits4::isClassFunction << std::endl;
}
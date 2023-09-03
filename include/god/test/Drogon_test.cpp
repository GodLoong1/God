#include <iostream>

struct A
{
    A() { std::cout << "A()" << std::endl; }
    ~A() { std::cout << "~A()" << std::endl; }
    A(const A&) { std::cout << "A(const A&)" << std::endl; }
    A(A&&) { std::cout << "A(A&&)" << std::endl; }
    A& operator=(const A&) { std::cout << "A& operator=(const A&)" << std::endl; return *this; }
    A& operator=(A&&) { std::cout << "A& operator=(A&&)" << std::endl; return *this; }
};

int main()
{
    [[maybe_unused]] A&& obj = static_cast<A&&>(A{});
    std::cout << "wocao" << std::endl;
}
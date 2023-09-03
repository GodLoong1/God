#include "Test.h"

class Dog : public Controller<Dog>
{
public:
    METHOD_LIST_BEGIN
    METHOD_ADD
    METHOD_LIST_END

    Dog()
    {
        LOG_INFO << "Dog()";
    }
};

int main()
{
    [[maybe_unused]] auto p = Dog::register_;
}
#include <stdio.h>
#include <sys/types.h>

template<typename T>
class Controller
{
private:
    static struct MethodRegister
    {
        MethodRegister()
        {
            printf("MethodRegister\n");
        }
    } register_;

    virtual void touch()
    {
        (void)register_;
    }
};

template<typename T>
typename Controller<T>::MethodRegister Controller<T>::register_;

class MyContorller : public Controller<MyContorller>
{
public:
    MyContorller() { }
};

int main() { }
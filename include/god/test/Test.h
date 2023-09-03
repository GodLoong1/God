#ifndef GOD_TEST_H
#define GOD_TEST_H

#include "god/utils/Logger.h"

#define METHOD_LIST_BEGIN \
    static void InitPathRouting() \
    {
#define METHOD_ADD \
    registerMethod();
#define METHOD_LIST_END \
    }

/// Http控制器
template<typename T>
class Controller
{
protected:
    static void registerMethod()
    {
        LOG_INFO << "registerMethod()";
    }

public:
    struct MethodRegister
    {
        MethodRegister()
        {
            LOG_INFO << "MethodRegister()";
            T::InitPathRouting();
        }
    };

    static MethodRegister register_;

    /**
     * @brief 实例化
     */
    virtual void* touch()
    {
        LOG_INFO << "touch()";
        return &register_;
    }
};

template<typename T>
typename Controller<T>::MethodRegister Controller<T>::register_;

#endif
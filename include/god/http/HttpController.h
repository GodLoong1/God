#ifndef GOD_HTTP_HTTPCONTROLLER_H
#define GOD_HTTP_HTTPCONTROLLER_H

#include "god/utils/Logger.h"
#include "god/utils/Object.h"
#include "god/http/HttpAppFramework.h"
#include "god/http/HttpTypes.h"

#include <string>

namespace god
{

#define METHOD_LIST_BEGIN \
    static void InitPathRouting() \
    {
#define METHOD_ADD(function, pattern, ...) \
    registerMethod(&function, pattern, {__VA_ARGS__})
#define METHOD_LIST_END \
    }

/// Http控制器
template<typename T>
class HttpController : public Object<T>
{
protected:
    template<typename Func>
    static void registerMethod(Func&& function,
                               const std::string& pattern,
                               const std::vector<HttpMethod>& methods)
    {
        LOG_INFO << "registerMethod()";
        // 没有这行一样实例化
        app().registerHandler(pattern,
                              std::forward<Func>(function),
                              methods);
    }

private:
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
typename HttpController<T>::MethodRegister HttpController<T>::register_;

} // namespace god

#endif
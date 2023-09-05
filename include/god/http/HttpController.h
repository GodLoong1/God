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
    RegisterMethod(&function, pattern, {__VA_ARGS__})
#define METHOD_LIST_END \
    }

/// Http控制器
template<typename T>
class HttpController : public Object<T>
{
protected:
    template<typename Func>
    static void RegisterMethod(Func&& function,
                               const std::string& pattern,
                               const std::vector<HttpMethod>& methods)
    {
        app().registerHandler(pattern,
                              std::forward<Func>(function),
                              methods);
    }

private:
    struct MethodRegister
    {
        MethodRegister()
        {
            T::InitPathRouting();
        }
    };

    static MethodRegister methodRegister_;

    virtual void* methodRegisterInstance() noexcept final
    {
        return &methodRegister_;
    }
};

template<typename T>
typename HttpController<T>::MethodRegister HttpController<T>::methodRegister_;

} // namespace god

#endif
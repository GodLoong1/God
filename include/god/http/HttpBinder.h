#ifndef GOD_HTTP_HTTPBINDER_H
#define GOD_HTTP_HTTPBINDER_H

#include <charconv>
#include <deque>
#include <memory>
#include <string>
#include <functional>
#include <type_traits>
#include <utility>

#include "god/utils/NonCopyable.h"
#include "god/utils/Object.h"
#include "god/utils/FunctionTraits.h"
#include "god/http/HttpRequest.h"
#include "god/http/HttpResponse.h"
#include "god/http/HttpTypes.h"

namespace god
{

// http func
template<typename Ret, typename... Args>
struct func_traits<Ret (*)
            (const HttpRequestPtr&, HttpResponseHandler&&, Args...)> :
       func_traits<Ret (*)(Args...)>
{
    static constexpr bool is_http_function = true;
};

class HttpBinderBase : NonCopyable
{
public:
    virtual ~HttpBinderBase() noexcept = default;

    virtual void handleHttpRequest(
        const HttpRequestPtr& req,
        HttpResponseHandler&& respcb,
        std::deque<std::string>& params) const noexcept = 0;

    // 获取额外参数数量
    virtual size_t paramCount() const noexcept = 0;
};

using HttpBinderBasePtr = std::shared_ptr<HttpBinderBase>;

/// 绑定http请求处理函数
template<typename Func>
class HttpBinder : public HttpBinderBase
{
public:
    using traits = func_traits<Func>;

    template<size_t Index>
    using args_type = typename traits::template args_type<Index>;

    static constexpr size_t args_count = traits::args_count;

    explicit HttpBinder(Func&& func) noexcept
    : func_(std::move(func))
    {
        static_assert(traits::is_http_function);
    }

    void handleHttpRequest(
        const HttpRequestPtr& req,
        HttpResponseHandler&& respcb,
        std::deque<std::string>& params) const noexcept override
    {
        run(req, std::move(respcb), params);
    }

    size_t paramCount() const noexcept override
    {
        return args_count;
    }

private:
    template<typename... Values>
    void run(const HttpRequestPtr& req,
             HttpResponseHandler&& respcb,
             std::deque<std::string>& pathParams,
             Values&&... values) const noexcept
    {
        if constexpr (sizeof...(Values) == args_count)
        {
            call(req, std::move(respcb), std::move(values)...);
        }
        else
        {
            using ValueType = std::remove_cv_t<
                std::remove_reference_t<args_type<sizeof...(Values)>>>;

            ValueType value{};

            if (!pathParams.empty())
            {
                std::string v = std::move(pathParams.front());
                pathParams.pop_front();

                getParamValue(value, std::move(v));
            }

            run(req,
                std::move(respcb),
                pathParams,
                std::forward<Values>(values)...,
                std::move(value));
        }
    }

    template<typename T>
    std::enable_if_t<std::is_arithmetic_v<T>>
    getParamValue(T& value, std::string&& str) const noexcept
    {
        if constexpr (std::is_integral_v<T>)
        {
            std::from_chars(str.data(), str.data() + str.size(), value);
        }
        else if constexpr (std::is_floating_point_v<T>)
        {
            std::from_chars(str.data(), str.data() + str.size(),
                            value, std::chars_format::general);
        }
    }

    void getParamValue(std::string& value, std::string&& str) const noexcept
    {
        value = std::move(str);
    }

    template<typename... Values>
    void call(const HttpRequestPtr& req,
              HttpResponseHandler&& respcb,
              Values&&... values) const noexcept
    {
        if constexpr (traits::is_object_class)
        {
            static auto objPtr =
                ObjectMap::GetSingleInstance<typename traits::class_type>();
            (*objPtr.*func_)(req, std::move(respcb), std::move(values)...);
        }
        else
        {
            func_(req, std::move(respcb), std::move(values)...);
        }
    }

private:
    const Func func_;
};

} // namespace god

#endif
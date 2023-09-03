#ifndef GOD_UTILS_FUNCTIONTRAITS_H
#define GOD_UTILS_FUNCTIONTRAITS_H

#include <type_traits>

#include "god/utils/Object.h"

namespace god
{

// std::function lambda
template<typename Func>
struct func_traits : func_traits <decltype(&Func::operator())>
{
};

// class::func non const
template<typename Class, typename Ret, typename... Args>
struct func_traits<Ret (Class::*)(Args...)> :
       func_traits<Ret (*)(Args...)>
{
    using class_type = Class;

    static constexpr bool is_object_class =
        std::is_base_of_v<Object<Class>, Class>;
};

// class::func const
template<typename Class, typename Ret, typename... Args>
struct func_traits<Ret (Class::*)(Args...) const> :
       func_traits<Ret (*)(Args...)>
{
    using class_type = Class;

    static constexpr bool is_object_class =
        std::is_base_of_v<Object<Class>, Class>;
};

// func
template<typename Ret, typename... Args>
struct func_traits<Ret (*)(Args...)>
{
    template <size_t Index>
    using args_type = std::tuple_element_t<Index, std::tuple<Args...>>;

    static constexpr size_t args_count = sizeof...(Args);

    static constexpr bool is_object_class = false;
};

} // namespace god

#endif
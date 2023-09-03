#ifndef GOD_UTILS_OBJECT_H
#define GOD_UTILS_OBJECT_H

#include <string>
#include <type_traits>
#include <typeinfo>
#include <memory>
#include <functional>
#include <vector>
#include "god/utils/Logger.h"

namespace god
{

/// 所有反射类的基类
class ObjectBase
{
public:
    virtual ~ObjectBase() = default;

    virtual const std::string& className() const = 0;
};

using ObjectBasePtr = std::shared_ptr<ObjectBase>;
using SharedAllocFunc = std::function<ObjectBasePtr()>;

/// 所有反射类的实例
class ObjectMap
{
public:
    // 注册类的分配函数
    static void RegisterClass(const std::string& className,
                              SharedAllocFunc&& func);

    static ObjectBasePtr
    NewSharedObject(const std::string& className);
        
    // 根据类名获取单例对象
    static const ObjectBasePtr&
    GetSingleInstance(const std::string& className);

    // 根据类型获取单例对象
    template<typename T>
    static const std::shared_ptr<T>& GetSingleInstance()
    {
        static const std::shared_ptr<T> singleton =
            std::dynamic_pointer_cast<T>(
                GetSingleInstance(T::ClassTypeName()));
        return singleton;
    }

    // 获取typeid.name的类名
    static std::string Demangle(const char* mangledName);

    // 获取所有注册的类名
    static std::vector<std::string> GetAllClassName();
};

/// 继承此类注册构造方法
template<typename T>
class Object : public virtual ObjectBase
{
public:
    const std::string& className() const override
    {
        return alloc_.className();
    }

    /**
     * @brief 实例化
     */
    static const std::string& ClassTypeName()
    {
        LOG_INFO << "ClassTypeName()";
        return alloc_.className();
    }

protected:
    Object() = default;

private:
    struct AllocRegister
    {
        AllocRegister()
        {
            LOG_INFO << "AllocRegister()";
            ObjectMap::RegisterClass(
                className(),
                []() -> ObjectBasePtr { return std::make_shared<T>(); });
        }

        const std::string& className() const
        {
            LOG_INFO << "className()";
            static const std::string name =
                ObjectMap::Demangle(typeid(T).name());
            return name;
        }
    };

    static AllocRegister alloc_;
};

template<typename T>
typename Object<T>::AllocRegister Object<T>::alloc_;

} // namespace god

#endif
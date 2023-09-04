#ifndef GOD_UTILS_OBJECT_H
#define GOD_UTILS_OBJECT_H

#include <string>
#include <memory>
#include <functional>
#include <typeinfo>
#include <vector>

namespace god
{

/// 所有反射类的基类
class ObjectBase
{
public:
    virtual ~ObjectBase() noexcept = default;
};

using ObjectBasePtr = std::shared_ptr<ObjectBase>;
using SharedAllocFunc = std::function<ObjectBasePtr()>;

/// 所有反射类的实例
class ObjectMap
{
public:
    // 注册类的分配函数
    static void RegisterClass(const std::string& className,
                              SharedAllocFunc&& func) noexcept;

    // 获取typeid.name的类名
    static std::string Demangle(const char* mangledName) noexcept;

    // 根据类名获取单例对象
    static const ObjectBasePtr&
    GetSingleInstance(const std::string& className) noexcept;

    // 根据类型获取单例对象
    template<typename T>
    static const std::shared_ptr<T>& GetSingleInstance() noexcept
    {
        static const std::shared_ptr<T> singleton =
            std::dynamic_pointer_cast<T>(
                GetSingleInstance(T::ClassTypeName()));
        return singleton;
    }

    // 获取所有注册的类名
    static std::vector<std::string> GetAllClassName() noexcept;
};

/// 继承此类注册构造方(必须显式定义构造函数)
template<typename T>
class Object : public virtual ObjectBase
{
public:
    static const std::string& ClassTypeName() noexcept
    {
        return alloc_.className();
    }

protected:
    Object() = default;

private:
    struct AllocRegister
    {
        AllocRegister() noexcept
        {
            ObjectMap::RegisterClass(
                className(),
                []() -> ObjectBasePtr { return std::make_shared<T>(); });
        }

        const std::string& className() const noexcept
        {
            static const std::string name =
                ObjectMap::Demangle(typeid(T).name());
            return name;
        }
    };

    virtual void* allocInstance() noexcept final
    {
        return &alloc_;
    }

    static AllocRegister alloc_;
};

template<typename T>
typename Object<T>::AllocRegister Object<T>::alloc_;

} // namespace god

#endif
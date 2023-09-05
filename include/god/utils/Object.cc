#include "god/utils/Object.h"

#include <cxxabi.h>
#include <cstdlib>
#include <mutex>
#include <cassert>
#include <unordered_map>

namespace god
{

/// 获取分配函数
static std::unordered_map<std::string, SharedAllocFunc>&
getAllocMap() noexcept
{
    static std::unordered_map<std::string, SharedAllocFunc> allocMap_;
    return allocMap_;
}

// 获取类实例
static std::unordered_map<std::string, ObjectBasePtr>&
getObjMap() noexcept
{
    static std::unordered_map<std::string, ObjectBasePtr> instanceMap_;
    return instanceMap_;
}

// 获取锁
static std::mutex& getObjMutex() noexcept
{
    static std::mutex mutex;
    return mutex;
}

void ObjectMap::RegisterClass(const std::string& className,
                              SharedAllocFunc&& func) noexcept
{
    getAllocMap().emplace(className, std::move(func));
}

std::string ObjectMap::Demangle(const char* mangledName) noexcept
{
    size_t len = 0;
    int status = 0;
    std::unique_ptr<char, decltype(&::free)> ptr(
        __cxxabiv1::__cxa_demangle(mangledName, nullptr, &len, &status),
        &::free);
    assert(status == 0);

    return std::string(ptr.get());
}

const ObjectBasePtr&
ObjectMap::GetSingleInstance(const std::string& className) noexcept
{
    auto& mutex = getObjMutex();
    auto& map = getObjMap();
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (auto iter = map.find(className); iter != map.end())
            return iter->second;
    }

    auto iter = getAllocMap().find(className);
    assert(iter != getAllocMap().end());
    auto obj = iter->second();

    std::lock_guard<std::mutex> lock(mutex);
    auto ret = map.emplace(className, std::move(obj));
    assert(ret.second);
    return ret.first->second;
}

std::vector<std::string> ObjectMap::GetAllClassName() noexcept
{
    std::vector<std::string> ret;
    ret.reserve(getAllocMap().size());

    for (const auto& it : getAllocMap())
    {
        ret.push_back(it.first);
    }
    return ret;
}

} // namespace god
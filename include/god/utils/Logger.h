#ifndef GOD_UTILS_LOGGER_H
#define GOD_UTILS_LOGGER_H

#include <cassert>
#include <cstddef>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <string_view>
#include <functional>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>

#include "god/utils/NonCopyable.h"
#include "god/utils/LogStream.h"
#include "god/utils/Date.h"

/// 日志流程
/// LogLevel => LogEvent => LogFormat => Logger => LogPlace

#define GOD_LOG god::logger

#define LOG_LEVEL(lv)                                               \
    if (GOD_LOG->level() <= god::LogLevel::lv)                      \
        god::LogEvent(GOD_LOG, god::LogLevel::lv,                   \
                      god::Date::SystemTime(), god::getThreadId(),  \
                      __func__, __FILE__, __LINE__).stream()        \

#define LOG_TRACE LOG_LEVEL(trace)
#define LOG_DEBUG LOG_LEVEL(debug)
#define LOG_INFO  LOG_LEVEL(info)
#define LOG_WARN  LOG_LEVEL(warn)
#define LOG_ERROR LOG_LEVEL(error)
#define LOG_FATAL LOG_LEVEL(fatal)

namespace god
{

class Logger;

/// 获取线程id
int getThreadId() noexcept;

/// 获取线程名称
std::string getThreadName() noexcept;

/// 获取错误号字符串
std::string strerr(int err = errno) noexcept;

/// 日志级别
enum class LogLevel
{
    trace,
    debug,
    info,
    warn,
    error,
    fatal,
};

/// 转换成字符串
const std::string_view& toString(LogLevel level) noexcept;

/// 日志事件
class LogEvent : NonCopyable
{
    friend class LogFormat;
public:
    LogEvent(std::unique_ptr<Logger>& logger, LogLevel level, Date date,
             int tid, const char* func, const char* file,
             int line) noexcept;
    ~LogEvent() noexcept;

    LogStream& stream() noexcept
    {
        return stream_;
    }

private:
    std::unique_ptr<Logger>& logger_;
    LogLevel level_;
    Date date_;
    int tid_;
    const char* func_;
    const char* file_;
    int line_;
    LogStream stream_;
};

/// 日志格式
class LogFormat : NonCopyable
{
public:
    /**
     * @param d 日期
     * @param l 日志级别
     * @param t 线程id
     * @param f 函数名
     * @param m 日志消息
     * @param F 文件名
     * @param L 行号
     * @param n 换行
     */
    explicit LogFormat(const std::string_view& fmt =
                       "%d - %l - %t - [%f] %m - %F:%L\n") noexcept;
    void format(LogStream& msg, LogEvent& event) noexcept;

private:
    std::vector<std::function<void(LogStream&, LogEvent&)>> formats_;
};

/// 日志输出
class LogPlace : NonCopyable
{
public:
    virtual ~LogPlace() noexcept = default;

    virtual void write(const char* buf, size_t len) noexcept;
    virtual void flush() noexcept;
};

/// 日志器
class Logger : NonCopyable
{
public:
    virtual ~Logger() noexcept = default;

    virtual void log(LogStream& msg) noexcept;
    virtual void fatal() noexcept;

    LogLevel level() const noexcept;
    void format(LogStream& msg, LogEvent& event) noexcept;

    void setLevel(LogLevel level) noexcept;
    void setFormat(std::unique_ptr<LogFormat>&& format) noexcept;
    void setPlace(std::unique_ptr<LogPlace>&& place) noexcept;

protected:
    void write(const char* buf, size_t len) noexcept;
    void flush() noexcept;

private:
    inline static std::atomic<LogLevel> level_{LogLevel::trace};
    inline static std::unique_ptr<LogFormat> format_{new LogFormat};
    inline static std::unique_ptr<LogPlace> place_{new LogPlace};
    inline static std::mutex mutex_;
};

/// 文件输出
class LogFileOut : public LogPlace
{
public:
    explicit LogFileOut(std::string&& fileName,
                        size_t rollSize = 100 * 1024 * 1024) noexcept;
    ~LogFileOut() noexcept;
    
    void write(const char* buf, size_t len) noexcept override;
    void flush() noexcept override;

private:
    void open() noexcept;
    void close() noexcept;

    const std::string fileName_;
    const size_t roll_;
    size_t curr_;
    FILE* fileOut_;
    std::mutex mutex_;
};

/// 异步日志器
class AsyncLogger : public Logger
{
public:
    using Buffer = LogBuffer<40960>;
    using BufferPtr = std::unique_ptr<Buffer>;
    using BufferVec = std::vector<BufferPtr>;

    explicit AsyncLogger(int flush = 3) noexcept;
    ~AsyncLogger() noexcept;

    void log(LogStream& msg) noexcept override;
    void fatal() noexcept override;

private:
    void runInThread() noexcept;
    void closeThread() noexcept;

    const std::chrono::seconds flush_;

    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    std::atomic<bool> quit_;

    BufferPtr currBuffer_;
    BufferPtr nextBuffer_;
    BufferVec fullBuffers_;
};

/// 全局日志
inline static std::unique_ptr<Logger> logger(new Logger);

} // namespace god

#endif
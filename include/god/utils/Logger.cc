#include "god/utils/Logger.h"

#include <sys/prctl.h>
#include <unistd.h>

namespace god
{

int getThreadId() noexcept
{
    static thread_local int tid = ::gettid();
    return tid;
}

std::string getThreadName() noexcept
{
    char buf[32];
    ::prctl(PR_GET_NAME, buf);
    return buf;
}

std::string strerr(int err) noexcept
{
    return ::strerror(err);
}

const std::string_view& toString(LogLevel level) noexcept
{
    static constexpr std::string_view buf[] = {
        {"TRACE", 5},
        {"DEBUG", 5},
        {"INFO", 4},
        {"WARN", 4},
        {"ERROR", 5},
        {"FATAL", 5},
    };
    return buf[static_cast<int>(level)];
}

LogEvent::LogEvent(std::unique_ptr<Logger>& logger, LogLevel level, Date date,
                   int tid, const char* func, const char* file,
                   int line) noexcept
: logger_(logger),
  level_(level),
  date_(date),
  tid_(tid),
  func_(func),
  file_(file),
  line_(line)
{
}

LogEvent::~LogEvent() noexcept
{
    LogStream msg;
    logger_->format(msg, *this);
    logger_->log(msg);
    if (level_ == LogLevel::fatal)
    {
        logger_->fatal();
    }
}

LogFormat::LogFormat(const std::string_view& fmt) noexcept
{
    auto strFmt = [](std::string&& str) {
        return [str{std::move(str)}] (LogStream& msg, LogEvent&) {
            msg << str;
        };
    };

    size_t first = 0, last = 0;
    while (first != fmt.size())
    {
        if (fmt[first] == '%')
        {
            if (first != last)
            {
                formats_.emplace_back(strFmt({fmt, last, first - last}));
            }

            switch (++first != fmt.size() ? fmt[first] : '\0')
            {
            case 'd':
                formats_.emplace_back([](LogStream& msg, LogEvent& event) {
                    msg << event.date_.toString();
                });
                break;
            case 'l':
                formats_.emplace_back([](LogStream& msg, LogEvent& event) {
                    msg << toString(event.level_);
                });
                break;
            case 't':
                formats_.emplace_back([](LogStream& msg, LogEvent& event) {
                    msg << event.tid_;
                });
                break;
            case 'f':
                formats_.emplace_back([](LogStream& msg, LogEvent& event) {
                    msg << event.func_;
                });
                break;
            case 'm':
                formats_.emplace_back([](LogStream& msg, LogEvent& event) {
                    msg << event.stream_;
                });
                break;
            case 'F':
                formats_.emplace_back([](LogStream& msg, LogEvent& event) {
                    if (const char* pos = strrchr(event.file_, '/'))
                        msg << pos + 1;
                    else
                        msg << event.file_;
                });
                break;
            case 'L':
                formats_.emplace_back([](LogStream& msg, LogEvent& event) {
                    msg << event.line_;
                });
                break;
            default:
                ::fprintf(stderr, "LogFormat error fmt %s\n", fmt.data());
                ::exit(EXIT_FAILURE);
                break;
            }
            ++first;
            last = first;
        }
        else
        {
            ++first;
        }
    }

    if (first != last)
    {
        formats_.emplace_back(strFmt({fmt, last, first - last}));
    }
}

void LogFormat::format(LogStream& msg, LogEvent& event) noexcept
{
    for (auto& fmt : formats_)
    {
        fmt(msg, event);
    }
}

void LogPlace::write(const char* buf, size_t len) noexcept
{
    ::fwrite(buf, sizeof(*buf), len, stdout);
}

void LogPlace::flush() noexcept
{
    ::fflush(stdout);
}

void Logger::log(LogStream& msg) noexcept
{
    write(msg.data(), msg.size());
    flush();
}

void Logger::fatal() noexcept
{
    ::abort();
}

LogLevel Logger::level() const noexcept
{
    return level_.load(std::memory_order_acquire);
}

void Logger::format(LogStream& msg, LogEvent& event) noexcept
{
    std::lock_guard<std::mutex> lock(mutex_);
    format_->format(msg, event);
}

void Logger::setLevel(LogLevel level) noexcept
{
    level_.store(level, std::memory_order_release);
}

void Logger::setFormat(std::unique_ptr<LogFormat>&& format) noexcept
{
    std::lock_guard<std::mutex> lock(mutex_);
    format_ = std::move(format);
}

void Logger::setPlace(std::unique_ptr<LogPlace>&& place) noexcept
{
    std::lock_guard<std::mutex> lock(mutex_);
    place_ = std::move(place);
}

void Logger::write(const char* buf, size_t len) noexcept
{
    std::lock_guard<std::mutex> lock(mutex_);
    place_->write(buf, len);
}

void Logger::flush() noexcept
{
    std::lock_guard<std::mutex> lock(mutex_);
    place_->flush();
}

LogFileOut::LogFileOut(std::string&& fileName, size_t rollSize) noexcept
: fileName_(std::move(fileName)),
  roll_(rollSize),
  curr_(0),
  fileOut_(nullptr)
{
    open();
}

LogFileOut::~LogFileOut() noexcept
{
   close();
}

void LogFileOut::write(const char *buf, size_t len) noexcept
{
    std::lock_guard<std::mutex> lock(mutex_);
    ::fwrite(buf, sizeof(*buf), len, fileOut_);
    curr_ += len;

    if (curr_ >= roll_)
    {
        curr_ = 0;
        close();
        open();
    }
}

void LogFileOut::flush() noexcept
{
    std::lock_guard<std::mutex> lock(mutex_);
    ::fflush(fileOut_);
}

void LogFileOut::open() noexcept
{
    std::string time = Date::SystemTime().toString("-%Y%m%d-%H%M%S.log");
    std::string name(fileName_ + std::move(time));
    fileOut_ = ::fopen(name.c_str(), "a+");
}

void LogFileOut::close() noexcept
{
    ::fclose(fileOut_);
    fileOut_ = nullptr;
}

AsyncLogger::AsyncLogger(int flush) noexcept
: flush_(flush),
  thread_([this] { runInThread(); }),
  quit_(false),
  currBuffer_(new Buffer),
  nextBuffer_(new Buffer)
{
}

AsyncLogger::~AsyncLogger() noexcept
{
    closeThread();
}

void AsyncLogger::log(LogStream& msg) noexcept
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (currBuffer_->capacity() >= msg.size())
    {
        currBuffer_->append(msg.data(), msg.size());
    }
    else
    {
        fullBuffers_.emplace_back(std::move(currBuffer_));
        if (nextBuffer_)
        {
            currBuffer_ = std::move(nextBuffer_);
        }
        else
        {
            currBuffer_.reset(new Buffer);
        }
        currBuffer_->append(msg.data(), msg.size());

        lock.unlock();
        cond_.notify_one();
    }
}

void AsyncLogger::fatal() noexcept
{
    closeThread();
    Logger::fatal();
}

void AsyncLogger::runInThread() noexcept
{
    ::prctl(PR_SET_NAME, "AsyncLogger");

    BufferPtr currBuffer(new Buffer);
    BufferPtr nextBuffer(new Buffer);
    BufferVec fullBuffers;

    auto writeFunc = [this](const BufferVec& buffers)
    {
        for (const BufferPtr& buf : buffers)
        {
            write(buf->data(), buf->size());
        }
        flush();
    };

    auto resetFunc = [](BufferPtr& buf, BufferVec& buffers)
    {
        if (!buf)
        {
            buf = std::move(buffers.back());
            buffers.pop_back();
            buf->clear();
        }
    };

    while (!quit_.load(std::memory_order_acquire))
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (fullBuffers_.empty())
            {
                cond_.wait_for(lock, flush_);
            }

            fullBuffers_.emplace_back(std::move(currBuffer_));
            currBuffer_ = std::move(currBuffer);
            fullBuffers.swap(fullBuffers_);

            if (!nextBuffer_)
            {
                nextBuffer_ = std::move(nextBuffer);
            }
        }
        writeFunc(fullBuffers);

        if (fullBuffers.size() > 2)
        {
            fullBuffers.resize(2);
        }

        resetFunc(currBuffer, fullBuffers);
        resetFunc(nextBuffer, fullBuffers);
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        fullBuffers_.push_back(std::move(currBuffer_));
        currBuffer_ = std::move(currBuffer);
        fullBuffers.swap(fullBuffers_);
    }
    writeFunc(fullBuffers);
}

void AsyncLogger::closeThread() noexcept
{
    bool expected = false;
    if (quit_.compare_exchange_strong(expected, true,
                                      std::memory_order_release))
    {
        cond_.notify_one();
        thread_.join();
    }
}

} // namespace god
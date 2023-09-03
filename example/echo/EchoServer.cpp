#include "god/utils/Logger.h"
#include "god/utils/CmdLineParser.h"
#include "god/net/TcpServer.h"

#include <csignal>

using namespace god;

class EchoServer : NonCopyable
{
public:
    EchoServer(EventLoop* loop, const InetAddress& addr)
    : server_(loop, addr, "EchoServer")
    {
        server_.setConnectionCallback([](const TcpConnectionPtr& conn) {
            if (conn->isConnected())
            {
                LOG_INFO << "fd: " << conn->fd() << " connected";
            }
            else
            {
                LOG_INFO << "fd: " << conn->fd() << " disconnected";
            }
        });

        server_.setMessageCallback([](const TcpConnectionPtr& conn, TcpBuffer& buf) {
            LOG_INFO << "fd: " << conn->fd() << ", recv: " << buf.readByte() << " bytes";
            conn->send(buf.readAll());
        });
    }

    void setIoLoopNum(size_t num)
    {
        server_.setIoLoopNum(num);
    }

    void setTimeoutOff(size_t seconds)
    {
        server_.setTimeoutOff(seconds);
    }

    void start()
    {
        server_.start();
    }

    void stop()
    {
        server_.stop();
    }

private:
    TcpServer server_;
};

std::unique_ptr<EventLoop> loop;
std::unique_ptr<EchoServer> server;

/// -p 端口
/// -n 线程数量
/// -t 超时时间
int main(int argc, char** argv)
{
    GOD_LOG->setLevel(LogLevel::info);
    CmdLineParser parser(argc, argv);

    InetAddress addr(9981);
    if (auto opt = parser.get("-p"))
    {
        if (auto val = opt.value())
        {
            addr = InetAddress(std::stoi(val.value()));
        }
    }

    loop.reset(new EventLoop);
    server.reset(new EchoServer(loop.get(), addr));

    ::signal(SIGINT, [](int) {
        loop->addInLoop([]{
            server->stop();
            loop->quit();
        });
    });

    if (auto opt = parser.get("-n"))
    {
        if (auto val = opt.value())
        {
            server->setIoLoopNum(std::stoul(val.value()));
        }
    }

    if (auto opt = parser.get("-t"))
    {
        if (auto val = opt.value())
        {
            server->setTimeoutOff(std::stoul(val.value()));
        }
    }

    server->start();
    loop->loop();
}
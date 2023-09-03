#include "god/utils/Logger.h"
#include "god/utils/CmdLineParser.h"
#include "god/net/TcpClient.h"
#include "god/net/Channel.h"

#include <cstdio>
#include <iostream>
#include <csignal>

using namespace god;

class EchoClient
{
public:
    EchoClient(EventLoop* loop, const InetAddress& addr)
    : client_(loop, addr, "EchoClient"),
      stdinChannel_(loop, STDIN_FILENO)
    {
        stdinChannel_.setReadCallback([this] {
            char buf[1024];
            int n = ::read(STDIN_FILENO, buf, sizeof(buf));

            if (auto conn = client_.connection())
            {
                conn->send(std::string(buf, n));
            }
        });

        client_.setConnectionCallback([](const TcpConnectionPtr& conn) {
            char buf[1024];

            if (conn->isConnected())
            {
                int len = ::snprintf(buf, sizeof(buf),
                                     "Connected to server %s\n",
                                     conn->peerAddr().toIpPort().c_str());
                if (::write(STDOUT_FILENO, buf, len) < 0)
                {
                }
            }
            else
            {
                int len = ::snprintf(buf, sizeof(buf),
                                     "Disconnected from server %s\n",
                                     conn->peerAddr().toIpPort().c_str());
                if (::write(STDOUT_FILENO, buf, len) < 0)
                {
                }
            }
        });

        client_.setMessageCallback([](const TcpConnectionPtr&, TcpBuffer& buf) {
            std::string str = buf.readAll();
            if (::write(STDOUT_FILENO, str.data(), str.size()) < 0)
            {
            }
        });
    }

    void start()
    {
        client_.start();
        stdinChannel_.enableReading();
    }

    void stop()
    {
        client_.stop();
        stdinChannel_.disableAll();
    }

    void setRetry(bool on)
    {
        client_.setRetry(on);
    }

private:
    TcpClient client_;
    Channel stdinChannel_;
};

std::unique_ptr<EventLoop> loop;
std::unique_ptr<EchoClient> client;

/// -p 端口
/// -r 断线重连
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
    client.reset(new EchoClient(loop.get(), addr));

    ::signal(SIGINT, [](int) {
        loop->addInLoop([]{
            client->stop();
            loop->quit();
        });
    });

    if (auto opt = parser.get("-r"))
    {
        client->setRetry(true);
    }

    client->start();
    loop->loop();
}
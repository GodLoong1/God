# 源文件
set(god_src
    utils/Date.cc
    utils/Logger.cc
    utils/CmdLineParser.cc
    utils/Object.cc

    net/EventLoop.cc
    net/Channel.cc
    net/Poller.cc
    net/Timer.cc
    net/TimerHeap.cc
    net/EventLoopThread.cc 
    net/EventLoopThreadPool.cc
    net/InetAddress.cc
    net/Socket.cc
    net/Acceptor.cc
    net/TcpBuffer.cc
    net/TcpConnection.cc
    net/TcpServer.cc
    net/TimerWheel.cc
    net/Connector.cc
    net/TcpClient.cc

    http/HttpTypes.cc
    http/HttpRequest.cc
    http/HttpRequestParser.cc
    http/HttpResponse.cc
    http/HttpServer.cc
    http/HttpAppFramework.cc
    http/StaticFileRouter.cc
    http/HttpControllersRouter.cc
    http/ListenerManager.cc

    orm/Result.cc
    orm/SqlBinder.cc
    orm/DbClient.cc
    orm/DbClientManager.cc
    orm/MysqlResult.cc
    orm/MysqlConnection.cc
)

# 动态库
add_library(god SHARED ${god_src})
target_link_libraries(god mariadb)

add_subdirectory(./utils/test)
add_subdirectory(./orm/test)
add_subdirectory(./test)
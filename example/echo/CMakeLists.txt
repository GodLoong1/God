add_executable(EchoServer EchoServer.cpp)
target_link_libraries(EchoServer god)

add_executable(EchoClient EchoClient.cpp)
target_link_libraries(EchoClient god)
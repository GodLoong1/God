add_executable(LogStreamTest LogStreamTest.cpp)
target_link_libraries(LogStreamTest god)

add_executable(LoggerTest LoggerTest.cpp)
target_link_libraries(LoggerTest god)

add_executable(ObjectTest ObjectTest.cpp)
target_link_libraries(ObjectTest god)
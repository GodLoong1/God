add_executable(Logger_test Logger_test.cpp)
target_link_libraries(Logger_test god)

add_executable(HttpServer_test HttpServer_test.cpp)
target_link_libraries(HttpServer_test god)

add_executable(Object_test Object_test.cpp)
target_link_libraries(Object_test god)

# add_executable(FunctionTraits_test FunctionTraits_test.cpp)
# target_link_libraries(FunctionTraits_test god)

# add_executable(HttpBinder_test HttpBinder_test.cpp)
# target_link_libraries(HttpBinder_test god)

add_executable(Regex_test Regex_test.cpp)
target_link_libraries(Regex_test god)

add_executable(HttpControllersRouter_test HttpControllersRouter_test.cpp)
target_link_libraries(HttpControllersRouter_test god)

add_executable(Drogon_test Drogon_test.cpp)
target_link_libraries(Drogon_test god)

add_executable(CacheMap_test CacheMap_test.cpp)
target_link_libraries(CacheMap_test god)

add_executable(Test Test.cpp)
target_link_libraries(Test)
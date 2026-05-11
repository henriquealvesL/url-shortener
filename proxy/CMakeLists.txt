cmake_minimum_required(VERSION 3.16)

project(url_shortener_proxy LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_Declare(
    cpp_httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.15.3
)
FetchContent_MakeAvailable(nlohmann_json)
FetchContent_MakeAvailable(cpp_httplib)

add_executable(proxy
    main.cpp
    http/RestClient.cpp
    patterns/CircuitBreaker.cpp
    protocol/MessageParser.cpp
    socket/ClientHandler.cpp
    socket/SocketServer.cpp
)

find_package(Threads REQUIRED)

target_link_libraries(proxy PRIVATE nlohmann_json::nlohmann_json httplib::httplib Threads::Threads)

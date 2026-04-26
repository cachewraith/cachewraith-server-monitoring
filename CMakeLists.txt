cmake_minimum_required(VERSION 3.20)
project(cachewraith-server-monitoring VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(CACHEWRAITH_BUILD_TESTS "Build CacheWraith tests" ON)

find_package(CURL REQUIRED)
find_package(nlohmann_json QUIET)

if(NOT nlohmann_json_FOUND)
  include(FetchContent)
  FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
  )
  FetchContent_MakeAvailable(nlohmann_json)
endif()

add_library(cachewraith_core
  src/app/Application.cpp
  src/app/Config.cpp
  src/alert/AlertManager.cpp
  src/alert/CooldownManager.cpp
  src/alert/TelegramNotifier.cpp
  src/monitor/RamMonitor.cpp
  src/monitor/DiskMonitor.cpp
  src/monitor/CpuMonitor.cpp
  src/monitor/ServiceMonitor.cpp
  src/monitor/WebsiteMonitor.cpp
  src/security/LogTailer.cpp
  src/security/SshLogMonitor.cpp
  src/security/NginxLogMonitor.cpp
  src/util/Logger.cpp
  src/util/Time.cpp
  src/util/StringUtils.cpp
  src/util/Env.cpp
)

target_include_directories(cachewraith_core PUBLIC include)
target_link_libraries(cachewraith_core PUBLIC CURL::libcurl nlohmann_json::nlohmann_json)
target_compile_options(cachewraith_core PRIVATE -Wall -Wextra -Wpedantic)

add_executable(cachewraith-monitor src/main.cpp)
target_link_libraries(cachewraith-monitor PRIVATE cachewraith_core)
target_compile_options(cachewraith-monitor PRIVATE -Wall -Wextra -Wpedantic)

install(TARGETS cachewraith-monitor RUNTIME DESTINATION bin)
install(FILES config.example.json DESTINATION share/cachewraith-monitor)
install(FILES systemd/cachewraith-monitor.service DESTINATION share/cachewraith-monitor/systemd)

if(CACHEWRAITH_BUILD_TESTS)
  enable_testing()
  add_executable(cachewraith-tests tests/parser_tests.cpp)
  target_link_libraries(cachewraith-tests PRIVATE cachewraith_core)
  target_compile_options(cachewraith-tests PRIVATE -Wall -Wextra -Wpedantic)
  add_test(NAME cachewraith-tests COMMAND cachewraith-tests)
endif()

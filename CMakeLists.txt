cmake_minimum_required(VERSION 3.10)
project(FlightSystem VERSION 1.0 LANGUAGES CXX)

# C++11 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译选项
if(APPLE)
    # macOS 特定选项
    add_compile_options(-Wall -Wextra -g)
else()
    add_compile_options(-Wall -Wextra -g)
endif()

# 包含目录
include_directories(${CMAKE_SOURCE_DIR}/include)

# ============================================================
# 公共库
# ============================================================
add_library(common STATIC
    src/common/message_serializer.cpp
    src/common/udp_socket.cpp
    src/common/logger.cpp
)

target_include_directories(common PUBLIC
    ${CMAKE_SOURCE_DIR}/include/common
)

# ============================================================
# 客户端
# ============================================================
add_executable(client
    src/client/flight_client.cpp
    src/client/client_ui.cpp
    src/client/client_main.cpp
)

target_link_libraries(client common)

# ============================================================
# 服务器
# ============================================================
add_executable(server
    src/server/flight_server.cpp
    src/server/flight_manager.cpp
    src/server/monitor_manager.cpp
    src/server/request_history.cpp
    src/server/server_main.cpp
)

target_link_libraries(server common)

# ============================================================
# 测试（可选，如果有 Google Test）
# ============================================================
option(BUILD_TESTS "Build test executables" ON)

if(BUILD_TESTS)
    # 尝试查找 Google Test
    find_package(GTest QUIET)

    if(GTEST_FOUND)
        enable_testing()

        add_executable(tests
            tests/test_main.cpp
            tests/test_message_serializer.cpp
            tests/test_flight_manager.cpp
        )

        target_link_libraries(tests
            common
            GTest::GTest
            GTest::Main
        )

        add_test(NAME AllTests COMMAND tests)

        message(STATUS "Google Test found, building tests")
    else()
        message(STATUS "Google Test not found, skipping tests")
        message(STATUS "Install with: brew install googletest")
    endif()
endif()

# ============================================================
# 安装
# ============================================================
install(TARGETS client server DESTINATION bin)
install(DIRECTORY include/ DESTINATION include)

# ============================================================
# 清理目标
# ============================================================
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/*
    COMMENT "Cleaning all build files"
)

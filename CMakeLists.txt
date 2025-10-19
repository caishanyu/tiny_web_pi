# 指定最低CMake版本
cmake_minimum_required(VERSION 3.11.4)

# 定义项目名称和版本
project(tiny_web_pi LANGUAGES C VERSION 1.0)

# 添加编译选项
option(DBG "Enable debug macros (DBG_ON)" OFF)

# 开启编译信息打印
set(CMAKE_VERBOSE_MAKEFILE OFF)

# 添加头文件路径
include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/common)

# 添加库，指定其源文件
add_library(lib_sys_manage ${PROJECT_SOURCE_DIR}/app/sys_manage/sys_manage.c)
add_library(lib_sys_time ${PROJECT_SOURCE_DIR}/app/sys_time/sys_time.c)
add_library(lib_debug ${PROJECT_SOURCE_DIR}/platform/debug/debug.c)
if(DBG)
    target_compile_definitions(lib_debug PRIVATE DBG_ON)
endif()
add_library(lib_socket ${PROJECT_SOURCE_DIR}/platform/socket/socket.c)
add_library(lib_http ${PROJECT_SOURCE_DIR}/platform/http/http.c)
add_library(lib_thread_pool ${PROJECT_SOURCE_DIR}/platform/thread_pool/thread_pool.c)

# 添加宏定义
set(LOG_DIR "${PROJECT_SOURCE_DIR}/platform/logs/log.txt")
add_definitions(-DLOG_DIR="${LOG_DIR}")
set(HTML_MAIN_DIR "${PROJECT_SOURCE_DIR}/platform/http/page/main.html")
add_definitions(-DHTML_MAIN_DIR="${HTML_MAIN_DIR}")

# 创建可执行文件目标
add_executable(main ${PROJECT_SOURCE_DIR}/main.c)

# 添加全局编译器标志
add_compile_options(-pedantic -Wall -Wextra-g)

# 链接库到可执行文件
target_link_libraries(main lib_socket lib_http lib_sys_manage lib_thread_pool lib_debug lib_sys_time pthread)

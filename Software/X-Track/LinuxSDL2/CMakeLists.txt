cmake_minimum_required(VERSION 3.16)

project(xtrack LANGUAGES C CXX)

# Software/X-Track
set(XTRACK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(LVGL_DIR "${XTRACK_DIR}/Simulator/LVGL.Simulator")
set(APP_DIR "${XTRACK_DIR}/USER/App")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# 与原先 Makefile 一致：Release 用 -O3，不带调试符号
set(CMAKE_C_FLAGS_RELEASE "-O3 -g0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g0")

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 可执行文件仍放在 LinuxSDL2/，运行时 cwd 必须是该目录（LV_FS_PC_PATH=../../../）
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

add_compile_definitions(
    LV_HOR_RES=480
    LV_VER_RES=320
    USE_SDL=1
    USE_WIN32DRV=0
    USE_EVDEV=0
    LV_COLOR_DEPTH=32
    LV_COLOR_16_SWAP=0
    LV_FS_PC_PATH="../../../"
    CONFIG_MAP_EXT_NAME_DEFAULT="png"
)

# lvgl：只编 src（demos/examples 产品路径用不到，原先 Makefile 因 lvgl.mk 会编进去）
file(GLOB_RECURSE LVGL_SRCS CONFIGURE_DEPENDS "${LVGL_DIR}/lvgl/src/*.c")

file(GLOB LV_DRIVERS_SRCS CONFIGURE_DEPENDS
    "${LVGL_DIR}/lv_drivers/*.c"
    "${LVGL_DIR}/lv_drivers/wayland/*.c"
    "${LVGL_DIR}/lv_drivers/indev/*.c"
    "${LVGL_DIR}/lv_drivers/gtkdrv/*.c"
    "${LVGL_DIR}/lv_drivers/display/*.c"
    "${LVGL_DIR}/lv_drivers/sdl/*.c"
)

file(GLOB_RECURSE LV_FS_SRCS CONFIGURE_DEPENDS "${LVGL_DIR}/lv_fs_if/*.c")
file(GLOB_RECURSE APP_C_SRCS CONFIGURE_DEPENDS "${APP_DIR}/*.c")
file(GLOB_RECURSE HAL_CXX_SRCS CONFIGURE_DEPENDS "${LVGL_DIR}/HAL/*.cpp")
file(GLOB_RECURSE APP_CXX_SRCS CONFIGURE_DEPENDS "${APP_DIR}/*.cpp")
list(FILTER APP_CXX_SRCS EXCLUDE REGEX "/Utils/ArduinoJson/")
list(FILTER APP_CXX_SRCS EXCLUDE REGEX "/Utils/lv_img_png/")

add_executable(xtrack
    main.cpp
    mouse_cursor_icon.c
    ${LVGL_SRCS}
    ${LV_DRIVERS_SRCS}
    ${LV_FS_SRCS}
    ${APP_C_SRCS}
    ${HAL_CXX_SRCS}
    ${APP_CXX_SRCS}
)

target_include_directories(xtrack PRIVATE
    "${LVGL_DIR}"
    "${APP_DIR}"
    "${APP_DIR}/Utils/ArduinoJson/src"
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_include_directories(xtrack PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(xtrack PRIVATE
    ${SDL2_LIBRARIES}
    m
    Threads::Threads
)

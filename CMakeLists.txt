cmake_minimum_required(VERSION 3.1)

# common
set(CMAKE_CXX_STANDARD 14)
add_compile_options(-Wall)

project(ScopeGUI C CXX)

# OpenGL
find_package(OpenGL REQUIRED)
link_libraries(${OPENGL_LIBRARIES})

# GLFW
option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
option(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)
option(GLFW_INSTALL "Generate installation target" OFF)
option(GLFW_DOCUMENT_INTERNALS "Include internals in documentation" OFF)
add_subdirectory(third_party/glfw)
link_libraries(glfw)

# ImGui
set(IMGUI_DIR ${CMAKE_CURRENT_LIST_DIR}/third_party/imgui)
set(GL_DIR ${IMGUI_DIR}/examples/libs/gl3w)
add_library(imgui
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl2.cpp
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp)
target_include_directories(imgui PUBLIC ${IMGUI_DIR} ${GL_DIR} ${IMGUI_DIR}/backends)
link_libraries(imgui)

# SmartSerial
add_subdirectory(modules/SmartSerial)
link_libraries(smartserial)

# asio_net
add_subdirectory(modules/asio_net)
link_libraries(asio_net)

# log
include_directories(modules/log)

# asio
add_definitions(-DASIO_STANDALONE)
include_directories(third_party/asio/asio/include)

if (MINGW)
    link_libraries(ws2_32)
endif ()

if (CMAKE_BUILD_TYPE MATCHES "Release")
    if (MINGW)
        set(CMAKE_EXE_LINKER_FLAGS "-static -s")
    endif ()
    set(BUILD_AS_APP WIN32 MACOSX_BUNDLE)
endif ()

# ScopeCore
add_subdirectory(modules/ScopeCore)
link_libraries(ScopeCoreGUI)

# App
add_subdirectory(App)
link_libraries(App)

add_executable(${PROJECT_NAME} ${BUILD_AS_APP} main.cpp)

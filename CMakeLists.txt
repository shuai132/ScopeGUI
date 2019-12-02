cmake_minimum_required(VERSION 3.1)

project(ScopeGUI C CXX)

# OpenGL
find_package(OpenGL REQUIRED)
link_libraries(${OPENGL_LIBRARIES})

# GLFW
set(GLFW_DIR third_party/glfw)
option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
option(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)
option(GLFW_INSTALL "Generate installation target" OFF)
option(GLFW_DOCUMENT_INTERNALS "Include internals in documentation" OFF)
add_subdirectory(${GLFW_DIR} binary_dir EXCLUDE_FROM_ALL)
link_libraries(glfw)

# ImGui
set(IMGUI_DIR third_party/imgui)
set(GL_DIR ${IMGUI_DIR}/examples/libs/gl3w)
add_library(imgui
        ${IMGUI_DIR}/examples/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/examples/imgui_impl_opengl2.cpp
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${GL_DIR}/GL/gl3w.c)
target_include_directories(imgui PUBLIC
        ${IMGUI_DIR} ${GL_DIR} ${IMGUI_DIR}/examples)
link_libraries(imgui)

# SmartSerial
add_subdirectory(modules/SmartSerial)
link_libraries(smartserial)

# PacketProcessor
add_subdirectory(modules/PacketProcessor)
link_libraries(packetprocessor)
target_compile_definitions(packetprocessor PRIVATE NDEBUG)

# common
set(CMAKE_CXX_STANDARD 11)
add_compile_options(-Wall)
include_directories(modules)

add_definitions(-DSCOPE_IS_GUI)

file(GLOB sources App/*.cpp App/*.c)
add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE ${sources} main.cpp)

if(CMAKE_BUILD_TYPE MATCHES "Release")
    if(MINGW)
        set(CMAKE_EXE_LINKER_FLAGS "-static -s")
    endif()
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/output/${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_BUILD_TYPE})

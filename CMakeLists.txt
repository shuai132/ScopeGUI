cmake_minimum_required(VERSION 3.1)

project(app C CXX)

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
        ${IMGUI_DIR}/examples/imgui_impl_opengl3.cpp
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${GL_DIR}/GL/gl3w.c)
target_include_directories(imgui PUBLIC
        ${IMGUI_DIR} ${GL_DIR} ${IMGUI_DIR}/examples)
link_libraries(imgui)

# common
set(CMAKE_CXX_STANDARD 11)
add_compile_options(-Wall)
include_directories(modules)

file(GLOB sources *.cpp)
add_executable(${PROJECT_NAME} ${sources})

cmake_minimum_required(VERSION 3.1)

project(ScopeGUI C CXX)

# ImGui
add_subdirectory(third_party/imgui/examples)
link_libraries(ImGui::ImplGlfw ImGui::ImplOpenGL2)

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
add_executable(${PROJECT_NAME} ${sources} main.cpp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/output/${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_BUILD_TYPE})

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND cp ${CMAKE_CURRENT_LIST_DIR}/imgui.ini ${CMAKE_BINARY_DIR}
)

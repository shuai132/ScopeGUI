project(App)

include_directories(widgets)

file(GLOB sources *.cpp *.c)
add_library(${PROJECT_NAME} ${sources})

project(App)

file(GLOB sources *.cpp *.c)
add_library(${PROJECT_NAME} ${sources})

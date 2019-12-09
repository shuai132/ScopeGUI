project(App)

file(GLOB sources *.c*
        algorithm/*.c*
        base/*.c*
        comm/*.c*
        ui/*.c*
        ui/widgets/*.c*
        utils/*.c*)
add_library(${PROJECT_NAME} ${sources})
target_include_directories(${PROJECT_NAME} PUBLIC .)

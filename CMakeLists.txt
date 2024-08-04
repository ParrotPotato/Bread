cmake_minimum_required(VERSION 3.14) 


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(fullgame)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED) 

message(STATUS "SDL2 include headers : ${SDL2_INCLUDE_DIR}")
message(STATUS "OpenGL include headers : ${OPENGL_INCLUDE_DIR}")
message(STATUS "GLEW include headers : ${GLEW_INCLUDE_DIR}")

add_library(imgui 
    ./external/imgui/imgui.cpp 
    ./external/imgui/imgui_draw.cpp
    ./external/imgui/imgui_tables.cpp  
    ./external/imgui/imgui_widgets.cpp

    ./external/imgui/backends/imgui_impl_sdl2.cpp
    ./external/imgui/backends/imgui_impl_opengl3.cpp 
)

target_include_directories(imgui PUBLIC ${SDL2_INCLUDE_DIRS})
target_include_directories(imgui PUBLIC ${OPENGL_INCLUDE_DIR})
target_include_directories(imgui PUBLIC ${GLEW_INCLUDE_DIRS})
target_include_directories(imgui PUBLIC ./external/imgui/  ./external/imgui/backends)


add_executable(fullgame
    src/main.cc
)

target_include_directories(fullgame PUBLIC ${SDL2_INCLUDE_DIRS})
target_include_directories(fullgame PUBLIC ${OPENGL_INCLUDE_DIR})
target_include_directories(fullgame PUBLIC ${GLEW_INCLUDE_DIRS})
target_include_directories(fullgame PUBLIC ./external/imgui/   ./external/imgui/backends/)


# the order of linking matters :: why, can this be better

target_link_libraries(fullgame ${SDL2_LIBRARIES})
target_link_libraries(fullgame ${OPENGL_LIBRARIES})
target_link_libraries(fullgame ${GLEW_LIBRARIES})
target_link_libraries(fullgame imgui)
target_link_libraries(fullgame ${CMAKE_DL_LIBS})

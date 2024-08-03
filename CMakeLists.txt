cmake_minimum_required(VERSION 3.14) 


project(fullgame)

find_package(SDL2 REQUIRED)
find_package(GLEW REQUIRED) 
find_package(OpenGL REQUIRED)


message(STATUS "SDL2 include headers : ${SDL2_INCLUDE_DIR}")
message(STATUS "OpenGL include headers : ${OPENGL_INCLUDE_DIR}")
message(STATUS "GLEW include headers : ${GLEW_INCLUDE_DIR}")

add_executable(
    fullgame

    src/main.cc
)

target_include_directories(fullgame PUBLIC ${SDL2_INCLUDE_DIRS})
target_include_directories(fullgame PUBLIC ${OPENGL_INCLUDE_DIR})
target_include_directories(fullgame PUBLIC ${GLEW_INCLUDE_DIRS})


target_link_libraries(fullgame ${SDL2_LIBRARIES})
target_link_libraries(fullgame ${OPENGL_LIBRARIES})
target_link_libraries(fullgame ${GLEW_LIBRARIES})


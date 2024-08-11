cmake_minimum_required(VERSION 3.14) 

project(fullgame)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

# add a way for specifying dearIMGUI in the cmake dependencies


set(STB_IMAGE_URL "https://raw.githubusercontent.com/nothings/stb/master/stb_image.h")
set(STB_IMAGE_FOLDER_PATH "${PROJECT_SOURCE_DIR}/external/stb/")
set(STB_IMAGE_FILE_PATH   "${PROJECT_SOURCE_DIR}/external/stb/stb_image.h")

if (NOT EXISTS ${STB_IMAGE_FILE_PATH})
    message(STATUS           "File: ${STB_IMAGE_FILE_PATH} not found in the path")
    execute_process(
        COMMAND mkdir -p ${STB_IMAGE_FOLDER_PATH}
        RESULT_VARIABLE  mkdir_result
    )
    if(mkdir_result)
        message(FATAL_ERROR  "failed to create subdirectory")
    endif()
    execute_process(
        COMMAND curl -L -o ${STB_IMAGE_FILE_PATH}  ${STB_IMAGE_URL}
        RESULT_VARIABLE  curl_result
    )
    if (curl_result)
        message(FATAL_ERROR  "failted to curl ${STB_IMAGE_URL}")
    else()
        message(STATUS       "required file downloaded : ${STB_IMAGE_FILE_PATH}")
    endif()
else()
    message(STATUS           "stb_image already exists : ${STB_IMAGE_FILE_PATH}")
endif()



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

add_library(gamespace
    ./src/gamespace.cc
)

target_include_directories(gamespace PUBLIC ${SDL2_INCLUDE_DIRS})
target_include_directories(gamespace PUBLIC ${OPENGL_INCLUDE_DIR})
target_include_directories(gamespace PUBLIC ${GLEW_INCLUDE_DIRS})

add_executable(fullgame
    src/main.cc
    src/memory.cc
)

target_include_directories(fullgame PUBLIC ${SDL2_INCLUDE_DIRS})
target_include_directories(fullgame PUBLIC ${OPENGL_INCLUDE_DIR})
target_include_directories(fullgame PUBLIC ${GLEW_INCLUDE_DIRS})
target_include_directories(fullgame PUBLIC ./external/imgui/   ./external/imgui/backends/)
target_include_directories(fullgame PUBLIC ./external/stb)


# the order of linking matters :: why, can this be better

target_link_libraries(fullgame ${SDL2_LIBRARIES})
target_link_libraries(fullgame ${OPENGL_LIBRARIES})
target_link_libraries(fullgame ${GLEW_LIBRARIES})
target_link_libraries(fullgame imgui)
target_link_libraries(fullgame gamespace)
target_link_libraries(fullgame ${CMAKE_DL_LIBS})

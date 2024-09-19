cmake_minimum_required(VERSION 3.20)

project(Culling)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

function(download dest_dir url)
    file(DOWNLOAD ${url} __temp.zip)
    file(ARCHIVE_EXTRACT INPUT __temp.zip DESTINATION ${dest_dir})
    file(REMOVE __temp.zip)
endfunction()

download(
    ${CMAKE_SOURCE_DIR}/GLAD
    https://gitfront.io/r/Moztanku/8s4L1GHF3Tsc/Resources/raw/glad-cmake.zip)

add_subdirectory(GLAD)

download(
    ${CMAKE_SOURCE_DIR}/immer
    https://github.com/arximboldi/immer/archive/refs/tags/v0.8.1.zip)

add_executable(exec)

target_sources(exec
    PRIVATE
    src/main.cpp)

target_include_directories(exec
    PRIVATE
    inc
    immer/immer-0.8.1)

target_compile_options(exec
    PRIVATE
    -Ofast
    -w)

find_package(OpenGL REQUIRED)

target_link_libraries(exec
    PRIVATE
    OpenGL::GL
    GLAD
    glfw
    glm)

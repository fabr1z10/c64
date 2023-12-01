cmake_minimum_required(VERSION 3.3)

set(CMAKE_CXX_STANDARD 17)

project(c64)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

add_executable(c64 src/shader.cpp src/main.cpp src/c64.cpp src/settings.cpp src/vicii.cpp)

target_link_libraries(c64 PUBLIC ${OPENGL_LIBRARIES} glfw GLEW::GLEW)
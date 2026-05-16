cmake_minimum_required(VERSION 3.10)
project(PlungerGame LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)

find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)

add_executable(Plunger
	src/main.cpp
	src/renderer/Camera.cpp
	src/renderer/Mesh.cpp
	src/renderer/OpenGLFunctions.cpp
	src/renderer/Renderer.cpp
	src/renderer/Shader.cpp
	src/renderer/Texture.cpp
)

target_include_directories(Plunger PRIVATE src)
target_link_libraries(Plunger PRIVATE SFML::Graphics SFML::Window SFML::System OpenGL::GL)

add_custom_command(TARGET Plunger POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
			${CMAKE_CURRENT_SOURCE_DIR}/assets
			${CMAKE_CURRENT_BINARY_DIR}/assets)

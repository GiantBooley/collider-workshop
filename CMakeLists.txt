cmake_minimum_required(VERSION 3.10)
project(ColliderWorkshop)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

# include
include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(ColliderWorkshop
	src/main.cpp
	src/math.cpp
	src/sprite.cpp
	src/collider.cpp
	src/stb_impl.cpp

	src/imgui.cpp
	src/imgui_draw.cpp
	src/imgui_impl_glfw.cpp
	src/imgui_impl_opengl3.cpp
	src/imgui_tables.cpp
	src/imgui_widgets.cpp
	src/glad.c
)
target_include_directories(ColliderWorkshop PRIVATE src)

target_link_libraries(ColliderWorkshop
	${CMAKE_SOURCE_DIR}/lib/libglfw3linux.a
)

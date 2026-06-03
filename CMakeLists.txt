cmake_minimum_required(VERSION 3.10)
project(ColliderWorkshop)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# include
include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(ColliderWorkshop
	src/main.cpp
	src/math.cpp
	src/glad.c
)
target_include_directories(ColliderWorkshop PRIVATE src)

target_link_libraries(ColliderWorkshop
	${CMAKE_SOURCE_DIR}/lib/libglfw3linux.a
	${CMAKE_SOURCE_DIR}/lib/libfreetypelinux.a
)

cmake_minimum_required(VERSION 3.5.0)
project(pen VERSION 0.1.0 LANGUAGES C)
set(CMAKE_C_STANDARD 99)

add_executable(pen)

option(USE_GLFW "glfw backend" OFF)
option(USE_SDL2 "sdl2 backend" OFF)

target_include_directories(pen PUBLIC 
	source
)

if(USE_GLFW)
	add_subdirectory(3rd_party/glfw EXCLUDE_FROM_ALL)
	target_include_directories(pen PUBLIC 3rd_party/glfw/include)
	target_sources(pen PRIVATE "source/rglfw.c" "source/inglfw.c")
	if(WIN32)
		target_link_libraries(pen glfw opengl32)
	else()
		find_package(OpenGL REQUIRED)
		target_link_libraries(pen glfw OpenGL::GL)
	endif()
elseif(USE_SDL2)
	add_subdirectory(3rd_party/sdl2 EXCLUDE_FROM_ALL)
	target_include_directories(pen PUBLIC 3rd_party/sdl2/include)
	target_sources(pen PRIVATE "source/rsdl.c" "source/insdl.c")
	if(WIN32)
		target_link_libraries(pen SDL2::SDL2)
	else()
		target_link_libraries(pen m SDL2::SDL2)
	endif()
else()
	message(FATAL_ERROR "Please select either SDL2 or GLFW")
endif()

target_sources(pen PRIVATE 
	"source/main.c" 
	"source/ent.c" 
	"source/r.c" 
	"source/trans.c"  
	"source/polybezier.c" 
	"resource/font/terminus8x16.c" 
	"resource/font/terminus6x12.c" 
	"resource/font/font.c" 
)

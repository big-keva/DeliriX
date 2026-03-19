cmake_minimum_required(VERSION 3.10)
project(DeliriX)

find_package(MoonyCode QUIET)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

if(NOT DEFINED DEVELOPER_MODE)
  message(STATUS "'DEVELOPER_MODE' it not set, try detect the build environment...")

  if(DEFINED ENV{CLION_IDE})
    message(STATUS "Clion build environment detected")

    set(DEVELOPER_MODE TRUE CACHE BOOL "" FORCE)
  else()
    set(DEVELOPER_MODE FALSE CACHE BOOL "" FORCE)
  endif()
endif()

if(MSVC)
  add_compile_options(-D_CRT_SECURE_NO_WARNINGS)
else()
  add_compile_options(-mfpmath=sse -fstack-protector-all -g
	-W -Wall -Wextra -Wcast-align -pedantic -Wfloat-equal -Wpointer-arith -Wformat-security
	-Wmissing-format-attribute -Wformat=1 -Wwrite-strings -Wcast-align -Wno-long-long
	-Woverloaded-virtual -Wno-suggest-attribute=format)

  if (SANITIZE_ENABLED)
	add_compile_options(-fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract -fsanitize=leak)
	add_link_options(-fsanitize=address)
  endif()

  if (PROFILER_ENABLED)
	add_compile_options(-p -pg)
	add_link_options(-p -pg)
  endif()
endif()

include_directories(
	./
	contrib)

if (NOT TARGET mtc)
	add_subdirectory(contrib/mtc)
endif()

add_library(DeliriX
	src/api.cpp
	src/zip.cpp
	src/txt.cpp
	src/xml.cpp
	src/odt.cpp
	src/docx.cpp
	src/fb2.cpp
	src/dump-as-json.cpp
	src/dump-as-tags.cpp
	src/load-as-json.cpp
	src/load-as-tags.cpp)

add_subdirectory(tests)

cmake_minimum_required(VERSION 3.11)
project(X11HelloWorld)

message(STATUS "Use X11 library for native window management")
find_package(X11 REQUIRED)
find_package(OpenGL REQUIRED)

message(STATUS "Use GLEW for OpenGL extensions and functions loading")
set(glew-cmake_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(glew-cmake_BUILD_STATIC ON CACHE BOOL "" FORCE)
set(USE_GLU OFF CACHE BOOL "" FORCE)
set(PKG_CONFIG_REPRESENTATIVE_TARGET OFF CACHE BOOL "" FORCE)
set(ONLY_LIBS ON CACHE BOOL "" FORCE)
add_subdirectory(deps/glew)

message(STATUS "Use GLM for 3d math features")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_STATIC_LIBS OFF CACHE BOOL "" FORCE)
add_subdirectory(deps/glm)
target_include_directories(glm INTERFACE deps/glm)

set(X11HELLOWORLD_SOURCES
        src/x11hw/main.cpp
        src/x11hw/error.hpp
        src/x11hw/context.cpp
        src/x11hw/context.hpp
        src/x11hw/window.cpp
        src/x11hw/window.hpp
        src/x11hw/window_manager.cpp
        src/x11hw/window_manager.hpp
        src/x11hw/shader.cpp
        src/x11hw/shader.hpp
        src/x11hw/geometry.cpp
        src/x11hw/geometry.hpp
        )

message(STATUS "Configure \"x11helloworld\" as final executable application")
add_executable(x11helloworld ${X11HELLOWORLD_SOURCES})

target_include_directories(x11helloworld PRIVATE src)
target_link_libraries(x11helloworld PRIVATE X11)
target_link_libraries(x11helloworld PRIVATE OpenGL::GLX)
target_link_libraries(x11helloworld PRIVATE libglew_static)
target_link_libraries(x11helloworld PRIVATE glm)

set_target_properties(x11helloworld PROPERTIES CXX_STANDARD 11)
set_target_properties(x11helloworld PROPERTIES CXX_STANDARD_REQUIRED ON)
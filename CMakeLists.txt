
cmake_minimum_required(VERSION 3.19)
project(LabRender)

cmake_policy(SET CMP0077 NEW) # Make options actually work recursively

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(FetchContent)

set(LABRENDER_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

option(LABRENDER_EXAMPLES "" ON)


# notes: this block appears to be the canonically correct way to do FetchContent
#if (NOT TARGET Lab::LabCamera)
#    find_package(LabCamera QUIET)
#endif()
#if (NOT TARGET Lab::LabCamera)
#    message(STATUS "Installing LabCamera")
#    FetchContent_Declare(LabCamera
#        GIT_REPOSITORY "https://github.com/meshula/LabCamera.git"
#        GIT_TAG "main"
#        GIT_SHALLOW ON)
#
#    FetchContent_GetProperties(LabCamera)
#    if(NOT LabCamera_POPULATED)
#        FetchContent_Populate(LabCamera)
#        add_subdirectory(${labcamera_SOURCE_DIR} ${labcamera_BINARY_DIR})
#    endif()
#else()
#    message(STATUS "Found Lab::Camera")
#endif()

if (LABRENDER_EXAMPLES)

if (NOT TARGET GLEW::glew)
    find_package(GLEW QUIET)
endif()
if (NOT TARGET GLEW::glew)
    add_subdirectory("3rdParty/glew-build-club")
else()
    message(STATUS "Found GLEW::glew")
endif()


# glfw doesn't have a proper install so force it.
# it would be nice to detect glfw in _deps and simply add_subdirectory without
# the slow FetchContent step.

#------------------- glfw

message(STATUS "Installing glfw")

set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "shared")
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "glfw examples")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "glfw tests")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "glfw docs")
set(GLFW_INSTALL OFF CACHE INTERNAL "glfw install")
set(GLFW_VULKAN_STATIC OFF CACHE INTERNAL "glfw vulkan") # "Assume the Vulkan loader is linked with the application"

add_subdirectory("3rdParty/glfw")


endif(LABRENDER_EXAMPLES)

#------------------- Effekseer

message(STATUS "Configuring Effekseer")

set(BUILD_GL ON CACHE INTERNAL "Effekseer GL")
set(BUILD_EXAMPLES OFF CACHE INTERNAL "Effekseer Examples")
set(BUILD_DX9 OFF CACHE INTERNAL "Effekseer DX9")
set(BUILD_DX11 OFF CACHE INTERNAL "Effekseer DX11")
set(BUILD_DX12 OFF CACHE INTERNAL "Effekseer DX12")
set(BUILD_METAL OFF CACHE INTERNAL "Effekseer Metal")
set(USE_OPENGL3 ON CACHE INTERNAL "Effekseer OpenGL 3")

add_subdirectory("3rdParty/Effekseer")
    
#------------------- OpenGameTools)

set(OPENGAMETOOL_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/opengametools)

#------------------- DearImgui

set(IMGUI_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/imgui")

#------------------- stb

set(STB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/stb")

find_package(OpenGL)

add_subdirectory(3rdParty/LabCamera)
add_subdirectory(3rdParty/LabCmd)
add_subdirectory(3rdParty/LabMath)
add_subdirectory(3rdParty/LabText)
add_subdirectory(3rdParty/tinyobjloader)
set(TINYOBJLOADER_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/tinyobjloader)

add_subdirectory(src)
add_subdirectory(extras)

if (LABRENDER_EXAMPLES)
    add_subdirectory(examples)
endif()

configure_file(cmake/LabRenderConfig.cmake.in
  "${PROJECT_BINARY_DIR}/LabRenderConfig.cmake" @ONLY)
install(FILES
  "${PROJECT_BINARY_DIR}/LabRenderConfig.cmake"
  DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake")

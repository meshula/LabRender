
cmake_minimum_required(VERSION 3.19)
project(LabRender)

cmake_policy(SET CMP0077 NEW) # Make options actually work recursively

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(FetchContent)

set(LABRENDER_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

option(LABRENDER_EXAMPLES "Build examples" ON)

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

set(IMGUI_BACKEND_OPENGL3 ON)
set(IMGUI_BACKEND_SOKOL OFF)
set(IMGUI_BACKEND_GLFW ON)
set(IMGUI_BACKEND_WINAPI ON) 
add_subdirectory(3rdParty/LabImgui)

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


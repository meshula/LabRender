
cmake_minimum_required(VERSION 3.19)
project(LabRender)

cmake_policy(SET CMP0077 NEW) # Make options actually work recursively

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(FetchContent)

set(LABRENDER_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

option(LABRENDER_EXAMPLES "" ON)


#------------------- stb

FetchContent_Declare(stb
   GIT_REPOSITORY "https://github.com/nothings/stb.git"
   GIT_TAG "master"
   GIT_SHALLOW ON)
FetchContent_MakeAvailable(stb)
FetchContent_GetProperties(stb)
if(NOT stb_POPULATED)
  message(STATUS "Fetching stb")
  FetchContent_Populate(stb)
endif()

set(STB_ROOT ${PROJECT_BINARY_DIR}/_deps/stb-src)



if (NOT TARGET Lab::Cmd)
    find_package(LabCmd QUIET)
endif()
if (NOT TARGET Lab::Cmd)
    message(STATUS "Installing Lab::Cmd")
    FetchContent_Declare(LabCmd
        GIT_REPOSITORY "https://github.com/meshula/LabCmd.git"
        GIT_TAG "main"
        GIT_SHALLOW ON)

    FetchContent_GetProperties(LabCmd)
    if(NOT LabCmd_POPULATED)
        FetchContent_Populate(LabCmd)
        add_subdirectory(${labcmd_SOURCE_DIR} ${labcmd_BINARY_DIR})
    endif()
else()
    message(STATUS "Found Lab::Cmd")
endif()

if (NOT TARGET Lab::Math)
    find_package(LabMath QUIET)
endif()
if (NOT TARGET Lab::Math)
    message(STATUS "Installing Lab::Math")
    FetchContent_Declare(LabMath
        GIT_REPOSITORY "https://github.com/meshula/LabMath.git"
        GIT_TAG "master"
        GIT_SHALLOW ON)

    FetchContent_GetProperties(LabMath)
    if(NOT LabMath_POPULATED)
        FetchContent_Populate(LabMath)
        add_subdirectory(${labmath_SOURCE_DIR} ${labmath_BINARY_DIR})
    endif()
else()
    message(STATUS "Found Lab::Math")
endif()

if (NOT TARGET Lab::Text)
    find_package(LabText QUIET)
endif()
if (NOT TARGET Lab::Text)
    message(STATUS "Installing Lab::Text")
    FetchContent_Declare(LabText
        GIT_REPOSITORY "https://github.com/meshula/LabText.git"
        GIT_TAG "main"
        GIT_SHALLOW ON)

    FetchContent_GetProperties(LabText)
    if(NOT LabText_POPULATED)
        FetchContent_Populate(LabText)
        add_subdirectory(${labtext_SOURCE_DIR} ${labtext_BINARY_DIR})
    endif()
else()
    message(STATUS "Found Lab::Text")
endif()

if (LABRENDER_EXAMPLES)



if (NOT TARGET GLEW::glew)
    find_package(GLEW QUIET)
endif()
if (NOT TARGET GLEW::glew)
    message(STATUS "Installing glew")
    FetchContent_Declare(GLEW
        GIT_REPOSITORY "https://github.com/vfxpro99/glew-build-club.git"
        GIT_TAG "main"
        GIT_SHALLOW ON)

    FetchContent_GetProperties(GLEW)
    if(NOT GLEW_POPULATED)
        FetchContent_Populate(GLEW)
        add_subdirectory(${glew_SOURCE_DIR} ${glew_BINARY_DIR})
    endif()
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

FetchContent_Declare(glfw
    GIT_REPOSITORY "https://github.com/glfw/glfw.git"
    GIT_TAG "master"
    GIT_SHALLOW ON)

FetchContent_GetProperties(glfw)

if(NOT glfw_POPULATED)
    message(STATUS "Fetching glfw")
    FetchContent_Populate(glfw)
    add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR})
endif()


#------------------- DearImgui

FetchContent_Declare(imgui
   GIT_REPOSITORY "https://github.com/ocornut/imgui.git"
   GIT_TAG "master"
   GIT_SHALLOW ON)
FetchContent_MakeAvailable(imgui)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
  message(STATUS "Fetching ImGui v${IMGUI_VERSION}")
  FetchContent_Populate(imgui)
endif()

set(IMGUI_ROOT ${PROJECT_BINARY_DIR}/_deps/imgui-src)




endif(LABRENDER_EXAMPLES)

find_package(OpenGL)

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


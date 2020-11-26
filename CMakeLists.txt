
cmake_minimum_required(VERSION 3.08)
project(LabRender)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(LABRENDER_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../LabCMake/scripts
                      ${CMAKE_CURRENT_SOURCE_DIR}/../LabCMake/modules)

include(Utilities)
include(Settings)

find_package(LabCmd)
find_package(LabMath)
find_package(LabText)
find_package(OpenGL)

add_subdirectory(extras)
add_subdirectory(src)
add_subdirectory(examples)

configure_file(cmake/LabRenderConfig.cmake.in
  "${PROJECT_BINARY_DIR}/LabRenderConfig.cmake" @ONLY)
install(FILES
  "${PROJECT_BINARY_DIR}/LabRenderConfig.cmake"
  DESTINATION "${CMAKE_INSTALL_PREFIX}"
)


cmake_minimum_required (VERSION 3.4)
project (LabRender)
set(CMAKE_BUILD_TYPE Release)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/defaults
                      ${CMAKE_SOURCE_DIR}/cmake/modules
                      ${CMAKE_SOURCE_DIR}/cmake/macros)

set(LABRENDER_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(LOCAL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../local")

include(CXXDefaults)
add_definitions(${_PXR_CXX_DEFINITIONS})
set(CMAKE_CXX_FLAGS "${_PXR_CXX_FLAGS} ${CMAKE_CXX_FLAGS}")

include(Packages)

add_subdirectory (examples)
add_subdirectory (src)


cmake_minimum_required (VERSION 3.4)
project (LabRender)
set(CMAKE_BUILD_TYPE Release)

set(LABRENDER_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(LOCAL_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../local)

# override cmake install prefix if it's not set,
# per: http://public.kitware.com/pipermail/cmake/2010-December/041135.html
IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET(CMAKE_INSTALL_PREFIX ${LOCAL_ROOT} CACHE PATH "Install set to local" FORCE)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/defaults
                      ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
                      ${CMAKE_CURRENT_SOURCE_DIR}/cmake/macros)

include(CXXDefaults)
add_definitions(${_PXR_CXX_DEFINITIONS})
set(CMAKE_CXX_FLAGS "${_PXR_CXX_FLAGS} ${CMAKE_CXX_FLAGS}")

include(Packages)

add_subdirectory(src)
add_subdirectory(extras)
add_subdirectory(examples)

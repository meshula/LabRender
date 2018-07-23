
project(LabRender)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(LABRENDER_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../LabCMake/scripts
                      ${CMAKE_CURRENT_SOURCE_DIR}/../LabCMake/modules)

include(Utilities)
include(Settings)
include(Packages)

add_subdirectory(src)
add_subdirectory(extras)
add_subdirectory(examples)

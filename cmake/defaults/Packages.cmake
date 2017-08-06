
# -- OpenGL
find_package(OpenGL REQUIRED)

set(GLEW_LOCATION "${LOCAL_ROOT}")
find_package(GLEW REQUIRED)

set(GLFW_LOCATION "${LOCAL_ROOT}")
find_package(GLFW)

set(ASSIMP_LOCATION "${LOCAL_ROOT}")
find_package(Assimp)

# --math
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_library(M_LIB m)
endif()

# --z
find_file(Z_BINARY_RELEASE
    NAMES
        zlib.dll
    HINTS
        "${LOCAL_ROOT}/bin"
    DOC "The z library")


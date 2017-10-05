
# -- OpenGL
find_package(OpenGL REQUIRED)

#set(GLEW_LOCATION "${LOCAL_ROOT}")
set(GLEW_LOCATION ${LABRENDER_ROOT}/../../Pixar/build)
find_package(GLEW REQUIRED)

set(GLFW_LOCATION "${LOCAL_ROOT}")
find_package(GLFW)

set(ASSIMP_LOCATION "${LOCAL_ROOT}")
find_package(Assimp)

set(LABCMD_LOCATION "${LOCAL_ROOT}")
find_package(LabCmd)

# --math
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_library(M_LIB m)
endif()

# --z
if (WIN32)
    find_file(Z_BINARY_RELEASE
        NAMES
            zlib.dll
        HINTS
            "${GLEW_LOCATION}/bin"
            "${LOCAL_ROOT}/bin"
        DOC "The z library")
endif()

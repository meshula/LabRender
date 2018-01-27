
# author: Nick Porcino
# license: MIT

# LABRENDER_FOUND
# LABRENDER_INCLUDE_DIR
# LABRENDER_LIBRARY_RELEASE
# LABRENDER_LIBRARY_DEBUG
# LABRENDER_LIBRARIES
# LABRENDER_DYLIBS
# LABRENDER_DYLIBS_RELEASE
# LABRENDER_DYLIBS_DEBUG

include(FindPackageHandleStandardArgs)

find_path(LABRENDER_INCLUDE_DIR LabRender/Renderer.h
    PATHS
    ${LABRENDER_LOCATION}
    $ENV{LABRENDER_DIR}
    $ENV{PROGRAMFILES}/LabRender
    /usr
    /usr/local
    /sw
    /opt/local

    PATH_SUFFIXES
    /include

    DOC "LabRender include directory")

set(LABRENDER_LIB_NAMES LabRender LabModelLoader)

foreach(LIB ${LABRENDER_LIB_NAMES})
    find_library(LABRENDER_${LIB}_LIB_RELEASE ${LIB}
        HINTS ${LABRENDER_INCLUDE_DIR}/..

        PATHS
        ${LABRENDER_LOCATION}
        $ENV{LABRENDER_DIR}
        /usr
        /usr/local
        /sw
        /opt/local

        PATH_SUFFIXES
        /lib
        DOC "LABRENDER library ${LIB}")

        if (LABRENDER_${LIB}_LIB_RELEASE)
            list(APPEND LABRENDER_LIBRARIES "${LABRENDER_${LIB}_LIB_RELEASE}")
            set(LABRENDER_${LIB}_FOUND TRUE)
            set(${LIB}_LIBRARY "${LABRENDER_${LIB}_LIB_RELEASE}")
            set(${LIB}_LIBRARY_RELEASE "${LABRENDER_${LIB}_LIB_RELEASE}")
        else()
            set(${LIB}_FOUND FALSE)
        endif()

    find_library(LABRENDER_${LIB}_LIB_DEBUG ${LIB}_d
        HINTS ${LABRENDER_INCLUDE_DIR}/..

        PATHS
        ${LABRENDER_LOCATION}
        $ENV{LABRENDER_DIR}
        /usr
        /usr/local
        /sw
        /opt/local

        PATH_SUFFIXES
        /lib
        DOC "LABRENDER library ${LIB}")

        if (LABRENDER_${LIB}_LIB_DEBUG)
            set(${LIB}_DEBUG_FOUND TRUE)
            set(${LIB}_LIBRARY_DEBUG "${LABRENDER_${LIB}_LIB_DEBUG}")
        else()
            set(${LIB}_FOUND FALSE)
        endif()
endforeach()

find_package_handle_standard_args(LABRENDER
    REQUIRED_VARS LABRENDER_LIBRARIES LABRENDER_INCLUDE_DIR)

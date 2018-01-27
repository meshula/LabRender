
# author: Nick Porcino
# license: MIT

include(FindPackageHandleStandardArgs)

find_path(LABCMD_INCLUDE_DIR LabCmd/LabCmd.h
    PATHS
    ${LABCMD_LOCATION}
    $ENV{LABCMD_DIR}
    $ENV{PROGRAMFILES}/LabCmd
    /usr
    /usr/local
    /sw
    /opt/local

    PATH_SUFFIXES
    /include

    DOC "LabCmd include directory")

set(LABCMD_LIB_NAMES LabCmd)

foreach(LIB ${LABCMD_LIB_NAMES})
    find_library(LABCMD_${LIB}_LIB_RELEASE ${LIB}
        HINTS ${LABCMD_INCLUDE_DIR}/..

        PATHS
        ${LABCMD_LOCATION}
        $ENV{LABCMD_DIR}
        /usr
        /usr/local
        /sw
        /opt/local

        PATH_SUFFIXES
        /lib
        DOC "LABCMD library ${LIB}")

        if (LABCMD_${LIB}_LIB_RELEASE)
            list(APPEND LABCMD_LIBRARIES "${LABCMD_${LIB}_LIB_RELEASE}")
            set(${LIB}_FOUND TRUE)
            set(${LIB}_LIBRARY "${LABCMD_${LIB}_LIB_RELEASE}")
            set(${LIB}_LIBRARY_RELEASE "${LABCMD_${LIB}_LIB_RELEASE}")
        else()
            set(LABCMD_${LIB}_FOUND FALSE)
        endif()

    find_library(LABCMD_${LIB}_LIB_DEBUG ${LIB}_d
        HINTS ${LABCMD_INCLUDE_DIR}/..

        PATHS
        ${LABCMD_LOCATION}
        $ENV{LABCMD_DIR}
        /usr
        /usr/local
        /sw
        /opt/local

        PATH_SUFFIXES
        /lib
        DOC "LABCMD library ${LIB}")

        if (LABCMD_${LIB}_LIB_DEBUG)
            set(${LIB}_DEBUG_FOUND TRUE)
            set(${LIB}_LIBRARY_DEBUG "${LABCMD_${LIB}_LIB_DEBUG}")
        else()
            set(${LIB}_DEBUG_FOUND FALSE)
        endif()
endforeach()

find_package_handle_standard_args(LABCMD
    REQUIRED_VARS LABCMD_LIBRARIES LABCMD_INCLUDE_DIR)


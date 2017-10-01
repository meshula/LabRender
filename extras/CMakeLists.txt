
set(MODEL_LOADER_SRC
    src/modelLoader.cpp
    include/extras/modelLoader.h)

add_library(LabModelLoader SHARED ${MODEL_LOADER_SRC})

if (MSVC_IDE)
    # hack to get around the "Debug" and "Release" directories cmake tries to add on Windows
    #set_target_properties (LabRender PROPERTIES PREFIX "../")
    set_target_properties(LabRender PROPERTIES IMPORT_PREFIX "../")
endif()

target_include_directories(LabModelLoader PRIVATE "${LABRENDER_ROOT}/include")
target_include_directories(LabModelLoader PRIVATE "${LABRENDER_ROOT}/extras/include")
target_include_directories(LabModelLoader PRIVATE "${LOCAL_ROOT}/include")
target_compile_definitions(LabModelLoader PUBLIC
    PLATFORM_WINDOWS=1
    LABRENDER_MODELLOADER_DLL)

target_link_libraries(LabModelLoader debug
    ${OPENGL_LIBRARIES}
    ${Assimp_LIBRARY_RELEASE}
    LabRender)
target_link_libraries(LabModelLoader optimized
    ${OPENGL_LIBRARIES}
    ${Assimp_LIBRARY_RELEASE}
    LabRender)

set_target_properties(LabModelLoader
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
#set_target_properties(LabModelLoader
#    PROPERTIES
#    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/lib"
#    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/lib"
#    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin"
#)

install (FILES include/extras/modelLoader.h DESTINATION include/LabRender/extras)

install (TARGETS LabModelLoader
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)

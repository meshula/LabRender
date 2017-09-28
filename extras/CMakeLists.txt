
set(MODEL_LOADER_SRC
    src/modelLoader.cpp
    include/extras/modelLoader.h)

add_library(LabModelLoader SHARED ${MODEL_LOADER_SRC})

target_include_directories(LabModelLoader PRIVATE "${LABRENDER_ROOT}/include")
target_include_directories(LabModelLoader PRIVATE "${LABRENDER_ROOT}/extras/include")
target_include_directories(LabModelLoader PRIVATE "${LOCAL_ROOT}/include")
target_compile_definitions(LabModelLoader PUBLIC PLATFORM_WINDOWS=1)
target_link_libraries(LabModelLoader LabRender)
target_link_libraries(LabModelLoader debug
    ${OPENGL_LIBRARIES}
    ${Assimp_LIBRARY_RELEASE}
    LabRender)
target_link_libraries(LabModelLoader optimized
    ${OPENGL_LIBRARIES}
    ${Assimp_LIBRARY_RELEASE}
    LabRender)
file(GLOB MODEL_LOADER_INC "${CMAKE_CURRENT_SOURCE_DIR}/extras/include/extras/LabRender/*.h")
install (FILES ${MODEL_LOADER} DESTINATION include)
install (TARGETS LabModelLoader DESTINATION bin)



file(GLOB LABRENDEREXAMPLES_SRC "*.cpp")
add_executable(LabRenderExample ${LABRENDEREXAMPLES_SRC})
target_link_libraries(LabRenderExample LabRender)
target_compile_definitions(LabRenderExample PRIVATE PLATFORM_WINDOWS=1)
target_compile_definitions(LabRenderExample PRIVATE ASSET_ROOT="${LABRENDER_ROOT}/assets")
target_include_directories(LabRenderExample PRIVATE "${LOCAL_ROOT}/include")
target_include_directories(LabRenderExample PRIVATE "${LABRENDER_ROOT}/include")
target_include_directories(LabRenderExample PRIVATE "${LABRENDER_ROOT}/extras/include")
target_sources(LabRenderExample PRIVATE "${LABRENDER_ROOT}/extras/include/LabCmd/FFI.cpp")
target_sources(LabRenderExample PRIVATE "${LABRENDER_ROOT}/extras/include/LabCmd/webby/webby.c")
target_sources(LabRenderExample PRIVATE "${LABRENDER_ROOT}/extras/include/LabCmd/jsoncpp.cpp")
target_sources(LabRenderExample PRIVATE "${LABRENDER_ROOT}/extras/src/modelLoader.cpp")
target_sources(LabRenderExample PRIVATE "${LABRENDER_ROOT}/extras/include/extras/modelLoader.h")
target_sources(LabRenderExample PRIVATE "${LABRENDER_ROOT}/extras/include/LabCmd/FFI.cpp")
target_sources(LabRenderExample PRIVATE "${LABRENDER_ROOT}/extras/include/LabCmd/FFI.h")
target_link_libraries(LabRenderExample debug
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARY_RELEASE}
    ${Assimp_LIBRARY_RELEASE}
    LabRender)
target_link_libraries(LabRenderExample optimized
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARY_RELEASE}
    ${Assimp_LIBRARY_RELEASE}
    LabRender)

add_custom_command(TARGET LabRenderExample POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_BINARY_DIR}/src/$<CONFIGURATION>/LabRender.dll"
        $<TARGET_FILE_DIR:LabRenderExample>)
add_custom_command(TARGET LabRenderExample POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Assimp_BINARY_RELEASE}"
        $<TARGET_FILE_DIR:LabRenderExample>)
add_custom_command(TARGET LabRenderExample POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${GLEW_BINARY_RELEASE}"
        $<TARGET_FILE_DIR:LabRenderExample>)
add_custom_command(TARGET LabRenderExample POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Z_BINARY_RELEASE}"
        $<TARGET_FILE_DIR:LabRenderExample>)

install (TARGETS LabRenderExample DESTINATION bin)

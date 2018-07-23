
file(GLOB LABRENDEREXAMPLES_SRC "*.cpp")
add_executable(LabRenderExample ${LABRENDEREXAMPLES_SRC})

target_compile_definitions(LabRenderExample PRIVATE PLATFORM_WINDOWS=1)
target_compile_definitions(LabRenderExample PRIVATE ASSET_ROOT="${LABRENDER_ROOT}/assets")
target_include_directories(LabRenderExample PRIVATE "${LOCAL_ROOT}/include")
target_include_directories(LabRenderExample PRIVATE "${LABRENDER_ROOT}/include")
target_include_directories(LabRenderExample PRIVATE "${LABRENDER_ROOT}/extras/include")
target_include_directories(LabRenderExample PRIVATE "${GLEW_INCLUDE_DIR}")

target_link_libraries(LabRenderExample debug
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARY_RELEASE}
    ${Assimp_LIBRARY_RELEASE})

target_link_libraries(LabRenderExample optimized
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARY_RELEASE}
    ${Assimp_LIBRARY_RELEASE})

target_link_libraries(LabRenderExample
    LabCmd::Core
    LabRender::Core
    LabRender::ModelLoader
    Lab::Math)

set_property(TARGET LabRenderExample PROPERTY FOLDER "examples")

install (TARGETS LabRenderExample RUNTIME DESTINATION "${LOCAL_ROOT}/bin")


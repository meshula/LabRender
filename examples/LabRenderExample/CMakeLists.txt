
file(GLOB LABRENDEREXAMPLES_SRC "*.cpp")
add_executable(LabRenderExample ${LABRENDEREXAMPLES_SRC})

target_compile_definitions(LabRenderExample PRIVATE PLATFORM_WINDOWS=1)
target_compile_definitions(LabRenderExample PRIVATE ASSET_ROOT="${LABRENDER_ROOT}/assets")
target_include_directories(LabRenderExample PRIVATE "${LABRENDER_ROOT}/include")
target_include_directories(LabRenderExample PRIVATE "${LABRENDER_ROOT}/extras/include")

target_link_libraries(LabRenderExample debug
    ${OPENGL_LIBRARIES})

target_link_libraries(LabRenderExample optimized
    ${OPENGL_LIBRARIES})

target_link_libraries(LabRenderExample
    glfw
    Lab::Cmd
    Lab::Render
    Lab::ModelLoader
    Lab::Math)

set_property(TARGET LabRenderExample PROPERTY FOLDER "examples")

install (TARGETS LabRenderExample RUNTIME DESTINATION bin)


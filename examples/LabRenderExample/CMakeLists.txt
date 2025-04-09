
if (APPLE)
    set(APPLE_FRAMEWORKS
        "-framework AppKit"
        "-framework CoreGraphics"
        "-framework Foundation"
        "-framework GameController"
        "-framework Metal"
        "-framework MetalKit"
        "-framework QuartzCore"
        "-framework OpenGL"
    )
    set(MAIN_EXT "mm")
else()
    set(MAIN_EXT "cpp")
endif()
add_executable(LabRenderExample 
    LabRenderExampleApp.${MAIN_EXT}
    ../LabRenderDemoApp.h
    RenderMeshes.hpp)

target_compile_definitions(LabRenderExample PRIVATE PLATFORM_WINDOWS=1)
target_compile_definitions(LabRenderExample PRIVATE ASSET_ROOT="${LABRENDER_ROOT}/assets")
target_include_directories(LabRenderExample PRIVATE "${LABRENDER_ROOT}/include")
target_include_directories(LabRenderExample PRIVATE "${LABRENDER_ROOT}/extras/include")

target_link_libraries(LabRenderExample debug
    ${OPENGL_LIBRARIES})

target_link_libraries(LabRenderExample optimized
    ${OPENGL_LIBRARIES})

target_link_libraries(LabRenderExample
    Lab::Cmd
    Lab::Imgui
    Lab::Render
    Lab::ModelLoader
    Lab::Math)

if (NOT APPLE)
    target_link_libraries(LabRenderExample
        glfw)
else()
    target_link_libraries(LabRenderExample
        ${APPLE_FRAMEWORKS})
endif()

set_property(TARGET LabRenderExample PROPERTY FOLDER "examples")
target_compile_features(LabRenderExample PRIVATE cxx_std_17)

install (TARGETS LabRenderExample RUNTIME DESTINATION bin)



add_executable(Primitives
    PrimitivesApp.cpp
)

target_compile_definitions(Primitives PRIVATE PLATFORM_WINDOWS=1)
target_compile_definitions(Primitives PRIVATE ASSET_ROOT="${LABRENDER_ROOT}/assets")
target_include_directories(Primitives PRIVATE "${LABRENDER_ROOT}/include")
target_include_directories(Primitives PRIVATE "${LABRENDER_ROOT}/extras/include")

target_link_libraries(Primitives debug
    ${OPENGL_LIBRARIES}
    glfw
)

target_link_libraries(Primitives optimized
    ${OPENGL_LIBRARIES}
    glfw
)

target_link_libraries(Primitives
    Lab::Cmd
    Lab::Math
    Lab::Render
    Lab::RenderBinder
    Lab::Imgui
    Lab::ModelLoader
)

install(TARGETS Primitives
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)

set_property(TARGET Primitives PROPERTY FOLDER "examples")
target_compile_features(Primitives PRIVATE cxx_std_17)

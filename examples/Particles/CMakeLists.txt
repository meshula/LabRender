
add_executable(Particles
    Particles.cpp
)

target_compile_definitions(Particles PRIVATE PLATFORM_WINDOWS=1)
target_compile_definitions(Particles PRIVATE ASSET_ROOT="${LABRENDER_ROOT}/assets")
target_include_directories(Particles PRIVATE "${LABRENDER_ROOT}/include")
target_include_directories(Particles PRIVATE "${LABRENDER_ROOT}/extras/include")
target_include_directories(Particles PRIVATE "${LABRENDER_ROOT}/3rdParty/Effekseer/Dev/Cpp/Effekseer")

target_link_libraries(Particles debug
    ${OPENGL_LIBRARIES}
    glfw
)

target_link_libraries(Particles optimized
    ${OPENGL_LIBRARIES}
    glfw
)

target_link_libraries(Particles
    GLEW::glew
    Lab::Cmd
    Lab::Math
    Lab::Render
    Lab::RenderBinder
    Lab::RenderImgui
    Lab::ModelLoader
    Effekseer
    EffekseerRendererGL
)

install(TARGETS Particles
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)

set_property(TARGET Particles PROPERTY FOLDER "examples")


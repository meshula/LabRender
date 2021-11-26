
add_executable(Hydra
    Hydra.cpp
)

target_compile_definitions(Hydra PRIVATE PLATFORM_WINDOWS=1)
target_compile_definitions(Hydra PRIVATE ASSET_ROOT="${LABRENDER_ROOT}/assets")
target_include_directories(Hydra PRIVATE "${LABRENDER_ROOT}/include")
target_include_directories(Hydra PRIVATE "${LABRENDER_ROOT}/extras/include")
target_include_directories(Hydra PRIVATE "${VFX_ROOT}/include")

target_link_libraries(Hydra debug
    ${OPENGL_LIBRARIES}
    glfw
)

target_link_libraries(Hydra optimized
    ${OPENGL_LIBRARIES}
    glfw
)

target_link_libraries(Hydra
    Lab::Cmd
    Lab::Math
    Lab::Render
    Lab::RenderBinder
    Lab::Imgui
    Lab::ModelLoader
)

install(TARGETS Hydra
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)

set_property(TARGET Hydra PROPERTY FOLDER "examples")


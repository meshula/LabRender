
add_executable(OffScreen
    OffScreenApp.h
    OffScreenApp.cpp
)

target_compile_definitions(OffScreen PRIVATE PLATFORM_WINDOWS=1)
target_compile_definitions(OffScreen PRIVATE ASSET_ROOT="${LABRENDER_ROOT}/assets")
target_include_directories(OffScreen PRIVATE "${LABRENDER_ROOT}/include")
target_include_directories(OffScreen PRIVATE "${LABRENDER_ROOT}/extras/include")

target_link_libraries(OffScreen debug
    ${OPENGL_LIBRARIES}
)

target_link_libraries(OffScreen optimized
    ${OPENGL_LIBRARIES}
)

target_link_libraries(OffScreen
    Lab::Cmd
    Lab::Math
    Lab::Render
    Lab::RenderBinder
    Lab::Imgui
    Lab::ModelLoader
)

install(TARGETS OffScreen
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

set_property(TARGET OffScreen PROPERTY FOLDER "examples")

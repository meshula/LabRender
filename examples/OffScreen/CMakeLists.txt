
add_executable(OffScreen
    OffScreenApp.h
    OffScreenApp.cpp
)

target_compile_definitions(OffScreen PRIVATE PLATFORM_WINDOWS=1)
target_compile_definitions(OffScreen PRIVATE ASSET_ROOT="${LABRENDER_ROOT}/assets")
target_include_directories(OffScreen PRIVATE "${LOCAL_ROOT}/include")
target_include_directories(OffScreen PRIVATE "${LABRENDER_ROOT}/include")
target_include_directories(OffScreen PRIVATE "${LABRENDER_ROOT}/extras/include")
target_include_directories(OffScreen PRIVATE "${GLEW_INCLUDE_DIR}")

target_link_libraries(OffScreen debug
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARY_RELEASE}
    ${Assimp_LIBRARY_RELEASE}
)

target_link_libraries(OffScreen optimized
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARY_RELEASE}
    ${Assimp_LIBRARY_RELEASE}
)

target_link_libraries(OffScreen
    GLEW::Shared
    LabCmd::Core
    Lab::Math
    LabRender::Core
    LabRender::Binder
    LabRender::Imgui
    LabRender::ModelLoader
)

install(TARGETS OffScreen
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

set_property(TARGET OffScreen PROPERTY FOLDER "examples")

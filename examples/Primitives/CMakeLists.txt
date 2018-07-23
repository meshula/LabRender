
add_executable(Primitives 
    PrimitivesApp.h
    PrimitivesApp.cpp
)

target_compile_definitions(Primitives PRIVATE PLATFORM_WINDOWS=1)
target_compile_definitions(Primitives PRIVATE ASSET_ROOT="${LABRENDER_ROOT}/assets")
target_include_directories(Primitives PRIVATE "${LOCAL_ROOT}/include")
target_include_directories(Primitives PRIVATE "${LABRENDER_ROOT}/include")
target_include_directories(Primitives PRIVATE "${LABRENDER_ROOT}/extras/include")
target_include_directories(Primitives PRIVATE "${GLEW_INCLUDE_DIR}")

target_link_libraries(Primitives debug
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARY_RELEASE}
    ${Assimp_LIBRARY_RELEASE}
)

target_link_libraries(Primitives optimized
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARY_RELEASE}
    ${Assimp_LIBRARY_RELEASE}
)

target_link_libraries(Primitives
    GLEW::Shared
    LabCmd::Core
    Lab::Math
    LabRender::Core
    LabRender::Binder
    LabRender::Imgui
    LabRender::ModelLoader
)


install(TARGETS Primitives
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

set_property(TARGET Primitives PROPERTY FOLDER "examples")

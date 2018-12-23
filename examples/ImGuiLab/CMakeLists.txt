
set(IMGUILAB_SRC
    "${LABRENDER_ROOT}/examples/ImGuiLab/LabRenderDemoApp.h"
    "${LABRENDER_ROOT}/examples/ImGuiLab/ImGuiLabApp.cpp"
    "${LABRENDER_ROOT}/examples/ImGuiLab/ImGuiIntegration.cpp"
)

add_executable(ImGuiLab ${IMGUILAB_SRC})

target_compile_definitions(ImGuiLab PRIVATE PLATFORM_WINDOWS=1)
target_compile_definitions(ImGuiLab PRIVATE ASSET_ROOT="${LABRENDER_ROOT}/assets")
target_include_directories(ImGuiLab PRIVATE "${LOCAL_ROOT}/include")

target_link_libraries(ImGuiLab debug
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARY_RELEASE}
    ${Assimp_LIBRARY_RELEASE}
    ${LabCmd_LIBRARY_DEBUG})

target_link_libraries(ImGuiLab optimized
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARY_RELEASE}
    ${Assimp_LIBRARY_RELEASE}
    ${LabCmd_LIBRARY_RELEASE})

target_link_libraries(ImGuiLab
    GLM::Core
    GLEW::Shared
    LabCmd::Core
    Lab::Math
    LabRender::Core
    LabRender::Binder
    LabRender::Imgui
    LabRender::ModelLoader)


install (TARGETS ImGuiLab
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)

set_property(TARGET ImGuiLab PROPERTY FOLDER "examples")


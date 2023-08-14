
set(IMGUILAB_SRC
    "${LABRENDER_ROOT}/examples/ImGuiLab/ImGuiLabApp.cpp"
    "${LABRENDER_ROOT}/examples/ImGuiLab/ImGuiIntegration.cpp"
)

add_executable(ImGuiLab ${IMGUILAB_SRC})

target_compile_definitions(ImGuiLab PRIVATE PLATFORM_WINDOWS=1)
target_compile_definitions(ImGuiLab PRIVATE GLEW_STATIC)
target_compile_definitions(ImGuiLab PRIVATE ASSET_ROOT="${LABRENDER_ROOT}/assets")
target_include_directories(ImGuiLab PRIVATE "${LABRENDER_ROOT}/extras/include")
target_include_directories(ImGuiLab PRIVATE "${IMGUI_ROOT}")
target_include_directories(ImGuiLab PRIVATE "${IMGUI_ROOT}/backends")

target_link_libraries(ImGuiLab debug
    glfw
    Lab::Cmd
    ${OPENGL_LIBRARIES})

target_link_libraries(ImGuiLab optimized
    glfw
    Lab::Cmd
    ${OPENGL_LIBRARIES})

target_link_libraries(ImGuiLab
    Lab::Cmd
    Lab::Math
    Lab::Render
    Lab::RenderBinder
    Lab::RenderImgui
    Lab::ModelLoader)


install (TARGETS ImGuiLab
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)

set_property(TARGET ImGuiLab PROPERTY FOLDER "examples")
target_compile_features(ImGuiLab PRIVATE cxx_std_17)


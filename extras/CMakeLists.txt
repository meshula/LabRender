
#-------------------------------------------------------------------------------
# LabModelLoader


set(LabModelLoader_PUBLIC_HEADERS
        include/LabRenderModelLoader/modelLoader.h)


set(LabModelLoader_SRC
    src/modelLoader.cpp)

add_library(LabModelLoader STATIC
    ${LabModelLoader_PUBLIC_HEADERS} ${LabModelLoader_SRC})

target_include_directories(LabModelLoader PUBLIC include)
target_include_directories(LabModelLoader PRIVATE ${TINYOBJLOADER_ROOT})

set_target_properties(LabModelLoader PROPERTIES
    FOLDER "LabModelLoader"
    POSITION_INDEPENDENT_CODE ON
    PUBLIC_HEADER "${LabModelLoader_PUBLIC_HEADERS}"
    OUTPUT_NAME "LabModelLoader"
    OUTPUT_NAME_DEBUG "LabModelLoader_d"
    INTERFACE_INCLUDE_DIRECTORIES "${LABRENDER_ROOT}/include")

target_link_libraries(LabModelLoader
    GLEW::glew
    Lab::Render
    Lab::Math)

add_library(Lab::ModelLoader ALIAS LabModelLoader)


#-------------------------------------------------------------------------------
# LabRenderBinder

set(LabRenderBinder_PUBLIC_HEADERS
                include/LabRenderBinder/LabRenderBinder.h)


set(LabRenderBinder_SRC
    src/LabRenderBinder.cpp)

add_library(LabRenderBinder STATIC
    ${LabRenderBinder_PUBLIC_HEADERS} ${LabRenderBinder_SRC})

target_include_directories(LabRenderBinder PUBLIC include)

set_target_properties(LabRenderBinder PROPERTIES
    FOLDER "LabRenderBinder"
    POSITION_INDEPENDENT_CODE ON
    PUBLIC_HEADER "${LabRenderBinder_PUBLIC_HEADERS}"
    OUTPUT_NAME "LabRenderBinder"
    OUTPUT_NAME_DEBUG "LabRenderBinder_d"
    INTERFACE_INCLUDE_DIRECTORIES "${LABRENDER_ROOT}/include")

target_link_libraries(LabRenderBinder
    GLEW::glew
    Lab::Render
    Lab::ModelLoader
    Lab::Math)

add_library(Lab::RenderBinder ALIAS LabRenderBinder)


#-------------------------------------------------------------------------------
# LabRenderGraph


set(LabRenderGraph_PUBLIC_HEADERS
                RenderGraph/include/LabRenderGraph/LabRenderGraph.h)


set(LabRenderGraph_SRC
    RenderGraph/src/RenderGraph.cpp)

add_library(LabRenderGraph STATIC
    ${LabRenderGraph_PUBLIC_HEADERS} ${LabRenderGraph_SRC})

target_include_directories(LabRenderGraph PUBLIC "${LABRENDER_ROOT}/extras/RenderGraph/include")

set_target_properties(LabRenderGraph PROPERTIES
    FOLDER "LabRenderGraph"
    POSITION_INDEPENDENT_CODE ON
    PUBLIC_HEADER "${LabRenderGraph_PUBLIC_HEADERS}"
    OUTPUT_NAME "LabRenderGraph"
    OUTPUT_NAME_DEBUG "LabRenderGraph_d"
    INTERFACE_INCLUDE_DIRECTORIES "${LABRENDER_ROOT}/extras/RenderGraph/include")

target_link_libraries(LabRenderGraph
    Lab::RenderTypes
    Lab::Text)

add_library(Lab::RenderGraph ALIAS LabRenderGraph)

set(IMGUILAB_SRC
    "${LABRENDER_ROOT}/examples/ImGuiLab/LabRenderDemoApp.h"
    "${LABRENDER_ROOT}/examples/ImGuiLab/LabRenderExampleApp.cpp"
    "${LABRENDER_ROOT}/examples/ImGuiLab/ImGuiIntegration.cpp"
)

add_executable(RenderGraphTest RenderGraph/src/test.cpp)
target_compile_definitions(RenderGraphTest PRIVATE PLATFORM_WINDOWS=1)
target_compile_definitions(RenderGraphTest PRIVATE ASSET_ROOT="${LABRENDER_ROOT}/assets")
target_link_libraries(RenderGraphTest Lab::RenderGraph)

install (TARGETS RenderGraphTest
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)

set_property(TARGET RenderGraphTest PROPERTY FOLDER "examples")

#-------------------------------------------------------------------------------
# LabRenderImgui


set(LabRenderImgui_PUBLIC_HEADERS
    ${IMGUI_ROOT}/imconfig.h
        ${IMGUI_ROOT}/imgui.h
        ${IMGUI_ROOT}/imgui_internal.h
        ${IMGUI_ROOT}/backends/imgui_impl_glfw.h
        ${IMGUI_ROOT}/backends/imgui_impl_opengl3.h
        ${LABRENDER_ROOT}/extras/LabImgui/FontManager.h
        ${LABRENDER_ROOT}/extras/LabImgui/materialIconDefinitions.h
        ${LABRENDER_ROOT}/extras/LabImgui/meshula-icons.h
        ${LABRENDER_ROOT}/extras/LabImgui/Widgets.h)

set(LabRenderImgui_PRIVATE_HEADERS
        ${IMGUI_ROOT}/backends/imgui_impl_glfw.h
        ${IMGUI_ROOT}/backends/imgui_impl_opengl3.h
        ${IMGUI_ROOT}/imstb_rectpack.h
        ${IMGUI_ROOT}/imstb_textedit.h
        ${IMGUI_ROOT}/imstb_truetype.h
        ${LABRENDER_ROOT}/extras/LabImgui/roboto_regular.ttf.h
        ${LABRENDER_ROOT}/extras/LabImgui/robotomono_regular.ttf.h)

set(LabRenderImgui_SRC
        ${IMGUI_ROOT}/imgui.cpp
        ${IMGUI_ROOT}/imgui_demo.cpp
        ${IMGUI_ROOT}/imgui_draw.cpp
        ${IMGUI_ROOT}/imgui_tables.cpp
        ${IMGUI_ROOT}/imgui_widgets.cpp
        ${IMGUI_ROOT}/backends/imgui_impl_glfw.cpp
        ${IMGUI_ROOT}/backends/imgui_impl_opengl3.cpp
        ${LABRENDER_ROOT}/extras/LabImgui/FontManager.cpp
        ${LABRENDER_ROOT}/extras/LabImgui/Widgets.cpp)

add_library(LabRenderImgui STATIC
    ${LabRenderImgui_PUBLIC_HEADERS} ${LabRenderImgui_PRIVATE_HEADERS} ${LabRenderImgui_SRC})

target_compile_definitions(LabRenderImgui PRIVATE GLEW_STATIC)

target_include_directories(LabRenderImgui 
    PUBLIC "${IMGUI_ROOT}"
    PUBLIC "${LABRENDER_ROOT}/extras"
    PRIVATE "${IMGUI_ROOT}/backends")

set_target_properties(LabRenderImgui PROPERTIES
    FOLDER "LabRenderImgui"
    POSITION_INDEPENDENT_CODE ON
    PUBLIC_HEADER "${LabRenderImgui_PUBLIC_HEADERS}"
    OUTPUT_NAME "LabRenderImgui"
    OUTPUT_NAME_DEBUG "LabRenderImgui_d"
    INTERFACE_INCLUDE_DIRECTORIES "${LABRENDER_ROOT}/include")

target_link_libraries(LabRenderImgui
    GLEW::glew
    glfw
    Lab::Text)

target_compile_features(LabRenderImgui PRIVATE cxx_std_17)

add_library(Lab::RenderImgui ALIAS LabRenderImgui)


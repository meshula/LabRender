
#-------------------------------------------------------------------------------
# LabModelLoader

lab_library(LabModelLoader
    TYPE SHARED
    ALIAS LabRender::ModelLoader

    LIBRARIES
        ${GLEW_LIBRARY}
        ${OPENGL_LIBRARIES}
        ${Assimp_LIBRARY_RELEASE}
        LabRender::Core
        Lab::Math

    INCLUDE_DIRS
        ${LABRENDER_ROOT}/include
        ${LABRENDER_ROOT}/extras/include
        ${GLM_INCLUDE_DIR}

    PUBLIC_HEADERS
        include/LabRenderModelLoader/modelLoader.h

    PRIVATE_HEADERS

    CPPFILES
        src/modelLoader.cpp
)

#-------------------------------------------------------------------------------
# LabRenderBinder

lab_library(LabRenderBinder
    TYPE SHARED
    ALIAS LabRender::Binder

    LIBRARIES
        ${GLEW_LIBRARY}
        ${OPENGL_LIBRARIES}
        LabRender::Core
        LabRender::ModelLoader
        Lab::Math

    INCLUDE_DIRS
        ${LABRENDER_ROOT}/include
        ${LABRENDER_ROOT}/extras/include
        ${LABRENDER_ROOT}/extras/imgui
        ${GLM_INCLUDE_DIR}

    PUBLIC_HEADERS
        include/LabRenderBinder/LabRenderBinder.h

    PRIVATE_HEADERS

    CPPFILES
        src/LabRenderBinder.cpp
)

#-------------------------------------------------------------------------------
# LabRenderGraph

lab_library(LabRenderGraph
    TYPE STATIC
    ALIAS LabRender::LabRenderGraph

    CPPFILES
        RenderGraph/src/RenderGraph.cpp

    PUBLIC_HEADERS
        RenderGraph/include/LabRenderGraph/LabRenderGraph.h

    INCLUDE_DIRS
        ${LABRENDER_ROOT}/extras/RenderGraph/include

    LIBRARIES
        Lab::Text
)


set(IMGUILAB_SRC
    "${LABRENDER_ROOT}/examples/ImGuiLab/LabRenderDemoApp.h"
    "${LABRENDER_ROOT}/examples/ImGuiLab/LabRenderExampleApp.cpp"
    "${LABRENDER_ROOT}/examples/ImGuiLab/ImGuiIntegration.cpp"
)

add_executable(RenderGraphTest RenderGraph/src/test.cpp)
target_compile_definitions(RenderGraphTest PRIVATE PLATFORM_WINDOWS=1)
target_compile_definitions(RenderGraphTest PRIVATE ASSET_ROOT="${LABRENDER_ROOT}/assets")
target_link_libraries(RenderGraphTest LabRender::LabRenderGraph)
target_include_directories(RenderGraphTest PRIVATE "${LOCAL_ROOT}/include")

install (TARGETS RenderGraphTest
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)

set_property(TARGET RenderGraphTest PROPERTY FOLDER "examples")

#-------------------------------------------------------------------------------
# LabRenderImgui

lab_library(LabRenderImgui
    TYPE STATIC
    ALIAS LabRender::Imgui

    LIBRARIES
        ${GLEW_LIBRARY}
        ${GLFW_LIBRARY_RELEASE}
        ${OPENGL_LIBRARIES}

    INCLUDE_DIRS
        ${LABRENDER_ROOT}/include
        ${LABRENDER_ROOT}/extras/imgui

    PUBLIC_HEADERS
        ${LABRENDER_ROOT}/extras/imgui/imconfig.h
        ${LABRENDER_ROOT}/extras/imgui/imgui.h
        ${LABRENDER_ROOT}/extras/imgui/imgui_internal.h
        ${LABRENDER_ROOT}/extras/imgui/imgui_impl_glfw.h
        ${LABRENDER_ROOT}/extras/imgui/imgui_impl_opengl3.h
        ${LABRENDER_ROOT}/extras/imgui/LabImgui/FontManager.h
        ${LABRENDER_ROOT}/extras/imgui/LabImgui/materialIconDefinitions.h
        ${LABRENDER_ROOT}/extras/imgui/LabImgui/meshula-icons.h
        ${LABRENDER_ROOT}/extras/imgui/LabImgui/Widgets.h

    PRIVATE_HEADERS
        ${LABRENDER_ROOT}/extras/imgui/imgui_impl_glfw.h
        ${LABRENDER_ROOT}/extras/imgui/imgui_impl_opengl3.h
        ${LABRENDER_ROOT}/extras/imgui/stb_rect_pack.h
        ${LABRENDER_ROOT}/extras/imgui/stb_textedit.h
        ${LABRENDER_ROOT}/extras/imgui/stb_truetype.h
        ${LABRENDER_ROOT}/extras/imgui/LabImgui/roboto_regular.ttf.h
        ${LABRENDER_ROOT}/extras/imgui/LabImgui/robotomono_regular.ttf.h

    CPPFILES
        ${LABRENDER_ROOT}/extras/imgui/imgui.cpp
        ${LABRENDER_ROOT}/extras/imgui/imgui_demo.cpp
        ${LABRENDER_ROOT}/extras/imgui/imgui_draw.cpp
        ${LABRENDER_ROOT}/extras/imgui/imgui_impl_glfw.cpp
        ${LABRENDER_ROOT}/extras/imgui/imgui_impl_opengl3.cpp
        ${LABRENDER_ROOT}/extras/imgui/LabImgui/FontManager.cpp
        ${LABRENDER_ROOT}/extras/imgui/LabImgui/Widgets.cpp
)


#-------------------------------------------------------------------------------
# LabRenderBinder

lab_library(LabModelLoader
    TYPE SHARED
    ALIAS LabRender::ModelLoader

    LIBRARIES
        ${GLEW_LIBRARY}
        ${OPENGL_LIBRARIES}
        ${Assimp_LIBRARY_RELEASE}
        LabRender::Core

    INCLUDE_DIRS
        ${LABRENDER_ROOT}/include
        ${LABRENDER_ROOT}/extras/include
        ${GLM_INCLUDE_DIR}

    PUBLIC_HEADERS
        include/extras/modelLoader.h

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

    INCLUDE_DIRS
        ${LABRENDER_ROOT}/include
        ${LABRENDER_ROOT}/extras/include
        ${LABRENDER_ROOT}/extras/imgui
        ${GLM_INCLUDE_DIR}

    PUBLIC_HEADERS
        include/extras/LabRenderBinder.h

    PRIVATE_HEADERS

    CPPFILES
        src/LabRenderBinder.cpp
)


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
        ${LABRENDER_ROOT}/extras/include

    PUBLIC_HEADERS
        ${LABRENDER_ROOT}/extras/imgui/imconfig.h
        ${LABRENDER_ROOT}/extras/imgui/imgui.h
        ${LABRENDER_ROOT}/extras/imgui/imgui_internal.h
        ${LABRENDER_ROOT}/extras/imgui/imgui_impl_glfw.h
        ${LABRENDER_ROOT}/extras/imgui/imgui_impl_opengl3.h

    PRIVATE_HEADERS
        ${LABRENDER_ROOT}/extras/imgui/imgui_impl_glfw.h
        ${LABRENDER_ROOT}/extras/imgui/imgui_impl_opengl3.h
        ${LABRENDER_ROOT}/extras/imgui/stb_rect_pack.h
        ${LABRENDER_ROOT}/extras/imgui/stb_textedit.h
        ${LABRENDER_ROOT}/extras/imgui/stb_truetype.h
  
    CPPFILES
        ${LABRENDER_ROOT}/extras/imgui/imgui.cpp
        ${LABRENDER_ROOT}/extras/imgui/imgui_demo.cpp
        ${LABRENDER_ROOT}/extras/imgui/imgui_draw.cpp
        ${LABRENDER_ROOT}/extras/imgui/imgui_impl_glfw.cpp
        ${LABRENDER_ROOT}/extras/imgui/imgui_impl_opengl3.cpp
)



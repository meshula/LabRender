
lab_library(LabModelLoader
    TYPE
        SHARED
        
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

add_library(LabRender::ModelLoader ALIAS LabModelLoader)

#-------------------------------------------------------------------------------
# LabRenderBinder



lab_library(LabRenderBinder
    TYPE
        SHARED
        
    LIBRARIES
        ${GLEW_LIBRARY}
        ${OPENGL_LIBRARIES}
        LabRender::Core
        LabRender::ModelLoader

    INCLUDE_DIRS
        ${LABRENDER_ROOT}/include
        ${LABRENDER_ROOT}/extras/include
        ${GLM_INCLUDE_DIR}

    PUBLIC_HEADERS
        include/extras/LabRenderBinder.h

    PRIVATE_HEADERS

    CPPFILES
        src/LabRenderBinder.cpp
)

add_library(LabRender::Binder ALIAS LabRenderBinder)


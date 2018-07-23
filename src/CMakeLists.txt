
cmake_minimum_required (VERSION 3.4)


if (APPLE)
    set(DARWIN_SRC TextureLoader.mm)
endif()

lab_library(LabRender
    TYPE SHARED
    ALIAS LabRender::Core

    LIBRARIES
        ${GLEW_LIBRARY}
        ${OPENGL_LIBRARIES}
        LabCmd::Core
        Lab::Math
        LabText::Core

    INCLUDE_DIRS
        ${LABRENDER_ROOT}/include
        ${GLM_INCLUDE_DIR}

    PUBLIC_HEADERS
        ../include/LabRender/Camera.h
        ../include/LabRender/DepthTest.h
        ../include/LabRender/DrawList.h
        ../include/LabRender/ErrorPolicy.h
        ../include/LabRender/Export.h
        ../include/LabRender/FrameBuffer.h
        ../include/LabRender/Immediate.h
        ../include/LabRender/InOut.h
        ../include/LabRender/LabRender.h
        ../include/LabRender/Light.h
        ../include/LabRender/Material.h
        ../include/LabRender/Model.h
        ../include/LabRender/ModelBase.h
        ../include/LabRender/PassRenderer.h
        ../include/LabRender/Renderer.h
        ../include/LabRender/RendererSpec.h
        ../include/LabRender/Semantic.h
        ../include/LabRender/SemanticType.h
        ../include/LabRender/Shader.h
        ../include/LabRender/ShaderBuilder.h
        ../include/LabRender/Texture.h
        ../include/LabRender/TextureLoader.h
        ../include/LabRender/TextureType.h
        ../include/LabRender/Uniform.h
        ../include/LabRender/UtilityModel.h
        ../include/LabRender/Utils.h
        ../include/LabRender/Vertex.h
        ../include/LabRender/ViewMatrices.h

    PRIVATE_HEADERS
        tinyheaders/tinyalloc.h
        tinyheaders/tinypng.h
        tinyheaders/tinyspritebatch.h
        gl4.h
        json/json-forwards.h
        json/json.h

    CPPFILES
        Camera.cpp
        ErrorPolicy.cpp
        FrameBuffer.cpp
        Immediate.cpp
        jsoncpp.cpp
        LabRender.cpp
        Light.cpp
        Material.cpp
        Model.cpp
        PassRenderer.cpp
        RendererSpec.cpp
        SemanticType.cpp
        Shader.cpp
        ShaderBuilder.cpp
        Texture.cpp
        tiny.c
        UtilityModel.cpp
        Utils.cpp
        Vertex.cpp
        ${DARWIN_SRC}
)

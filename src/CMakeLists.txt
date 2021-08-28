
cmake_minimum_required (VERSION 3.19)

set(LABRENDERTYPES_PUBLIC_HEADERS
        ../include/LabRenderTypes/DepthTest.h
        ../include/LabRenderTypes/SemanticType.h
        ../include/LabRenderTypes/Texture.h
)

add_library(LabRenderTypes STATIC 
    ${LABRENDERTYPES_PUBLIC_HEADERS} odr.cpp)
target_include_directories(LabRenderTypes 
    PUBLIC ${LABRENDER_ROOT}/include)

set_target_properties(
    LabRenderTypes
    PROPERTIES
        FOLDER "LabRenderTypes"
        POSITION_INDEPENDENT_CODE ON
        PUBLIC_HEADER "${LABRENDERTYPES_PUBLIC_HEADERS}"
        OUTPUT_NAME "LabRenderTypes"
        OUTPUT_NAME_DEBUG "LabRenderTypes_d"
        INTERFACE_INCLUDE_DIRECTORIES ${LABRENDER_ROOT}/include
)

add_library(Lab::RenderTypes ALIAS LabRenderTypes)

if (APPLE)
    set(LABRENDER_PLATFORM_SRC TextureLoader.mm)
endif()

set(LABRENDER_PUBLIC_HEADERS
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
        ../include/LabRenderTypes/Texture.h
)

set(LABRENDER_PRIVATE_HEADERS
        tinyheaders/tinyalloc.h
        tinyheaders/tinypng.h
        tinyheaders/tinyspritebatch.h
        gl4.h
        json/json-forwards.h
        json/json.h
)

add_library(LabRender STATIC ${LABRENDER_PUBLIC_HEADERS} ${LABRENDER_PRIVATE_HEADERS}
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
        ${LABRENDER_PLATFORM_SRC}
)

target_compile_definitions(LabRender 
    PUBLIC LABRENDER_STATIC
    PUBLIC GLEW_STATIC)


target_include_directories(LabRender 
    PUBLIC ${LABRENDER_ROOT}/include
    PUBLIC ${STB_ROOT}
)

set_target_properties(
    LabRender
    PROPERTIES
        FOLDER "LabRender"
        POSITION_INDEPENDENT_CODE ON
        PUBLIC_HEADER "${LABRENDER_PUBLIC_HEADERS}"
        OUTPUT_NAME "LabRender"
        OUTPUT_NAME_DEBUG "LabRender_d"
        INTERFACE_INCLUDE_DIRECTORIES ${LABRENDER_ROOT}/include
)

target_link_libraries(LabRender 
    GLEW::glew
    ${OPENGL_LIBRARIES}
    Lab::LabCamera
    Lab::Cmd
    Lab::Math
    Lab::RenderGraph
)

add_library(Lab::Render ALIAS LabRender)

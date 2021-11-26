
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


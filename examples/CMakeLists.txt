

find_package(Fmt)
find_package(GLM)
find_package(GLEW)

set(SAMPLES
    CubeMapFilter
    ImGuiLab
    Immediate
    LabRenderExample
    OffScreen
    Primitives
)

foreach(SAMPLE ${SAMPLES})
    add_subdirectory(${SAMPLE})
endforeach()


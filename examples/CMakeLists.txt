

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


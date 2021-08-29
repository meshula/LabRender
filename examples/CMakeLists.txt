

set(SAMPLES
    CubeMapFilter
    Hydra
    ImGuiLab
    Immediate
    LabRenderExample
    OffScreen
    Particles
    Primitives
)

foreach(SAMPLE ${SAMPLES})
    add_subdirectory(${SAMPLE})
endforeach()


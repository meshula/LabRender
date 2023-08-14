

set(SAMPLES
    CubeMapFilter
    Hydra
    # ImGuiLab relies on docking branch
    #Immediate update to latest InsectAI
    LabRenderExample
    OffScreen
    #Particles Effekseer not set up
    Primitives
)

foreach(SAMPLE ${SAMPLES})
    add_subdirectory(${SAMPLE})
endforeach()


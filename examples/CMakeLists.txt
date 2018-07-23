
set(SAMPLES
    CubeMapFilter
    ImGuiLab
    Immediate
    LabRenderExample
    Primitives
)

foreach(SAMPLE ${SAMPLES})
    add_subdirectory(${SAMPLE})
endforeach()


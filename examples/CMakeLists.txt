
set(SAMPLES
    CubeMapFilter
    ImGuiLab
    Immediate
    LabRenderExample
)

foreach(SAMPLE ${SAMPLES})
    add_subdirectory(${SAMPLE})
endforeach()


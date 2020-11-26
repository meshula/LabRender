
file(GLOB CUBEMAPFILTER_SRC "*.cpp")
file(GLOB CMFT_SRC "cmft/*.cpp")
file(GLOB CMFT_INC "cmft/*.h")
add_executable(CubeMapFilterLab ${CUBEMAPFILTER_SRC} ${CMFT_SRC} ${CMFT_INC})

target_compile_definitions(CubeMapFilterLab PRIVATE PLATFORM_WINDOWS=1)
target_compile_definitions(CubeMapFilterLab PRIVATE ASSET_ROOT="${LABRENDER_ROOT}/assets")
target_include_directories(CubeMapFilterLab PRIVATE "${LOCAL_ROOT}/include")
target_include_directories(CubeMapFilterLab PRIVATE "${LABRENDER_ROOT}/include")
target_include_directories(CubeMapFilterLab PRIVATE "${LABRENDER_ROOT}/extras/include")
target_include_directories(CubeMapFilterLab PRIVATE "${GLEW_INCLUDE_DIR}")

target_link_libraries(CubeMapFilterLab debug
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARY_RELEASE}
    ${Assimp_LIBRARY_RELEASE})

target_link_libraries(CubeMapFilterLab optimized
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARY_RELEASE}
    ${Assimp_LIBRARY_RELEASE})

target_link_libraries(CubeMapFilterLab
    GLM::Core 
    GLEW::Shared
    Lab::Cmd
    Lab::Math
    LabRender::Core
    LabRender::Binder
    LabRender::Imgui
    LabRender::ModelLoader)

set_property(TARGET CubeMapFilterLab PROPERTY FOLDER "examples")

install (TARGETS CubeMapFilterLab RUNTIME DESTINATION "${LOCAL_ROOT}/bin")

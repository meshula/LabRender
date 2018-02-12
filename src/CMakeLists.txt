

set(LABRENDER_INC 
    ../include/LabRender/Camera.h
    ../include/LabRender/ConcurrentQueue.h
    ../include/LabRender/DepthTest.h
    ../include/LabRender/DrawList.h
    ../include/LabRender/ErrorPolicy.h
    ../include/LabRender/Export.h
    ../include/LabRender/FrameBuffer.h
    ../include/LabRender/InOut.h
    ../include/LabRender/LabRender.h
    ../include/LabRender/Light.h
    ../include/LabRender/Material.h
    ../include/LabRender/MathTypes.h
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
    ../include/LabRender/Transform.h
    ../include/LabRender/Uniform.h
    ../include/LabRender/UtilityModel.h
    ../include/LabRender/Utils.h
    ../include/LabRender/Vertex.h
    ../include/LabRender/ViewMatrices.h
    )

if (APPLE)
    set(DARWIN_SRC TextureLoader.mm)
endif()

add_library(LabRender SHARED 
    ${LABRENDER_INC}
    Camera.cpp
    ErrorPolicy.cpp
    FrameBuffer.cpp
    gl4.h
    jsoncpp.cpp
    LabRender.cpp
    Light.cpp
    Material.cpp
    MathTypes.cpp
    Model.cpp
    PassRenderer.cpp
    RendererSpec.cpp
    SemanticType.cpp
    Shader.cpp
    ShaderBuilder.cpp
    Texture.cpp
    Transform.cpp
    Types.cpp
    UtilityModel.cpp
    Utils.cpp
    Vertex.cpp
    ${DARWIN_SRC}
    )

if (MSVC_IDE)
    # hack to get around the "Debug" and "Release" directories cmake tries to add on Windows
    #set_target_properties(LabRender PROPERTIES PREFIX "../")
    set_target_properties(LabRender PROPERTIES IMPORT_PREFIX "../")
endif()

target_include_directories(LabRender PRIVATE "${LABRENDER_ROOT}/include")
target_include_directories(LabRender PRIVATE "${LOCAL_ROOT}/include")
target_compile_definitions(LabRender PRIVATE BUILDING_LABRENDER=1)
target_compile_definitions(LabRender PUBLIC PLATFORM_WINDOWS=1)

target_link_libraries(LabRender ${OPENGL_LIBRARIES} ${GLEW_LIBRARY})
set_target_properties(LabRender PROPERTIES OUTPUT_NAME_DEBUG LabRender_d)

set_target_properties(LabRender
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin"
)


install (TARGETS LabRender
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)

install (FILES ${LABRENDER_INC} DESTINATION include/LabRender)

install(
    FILES
        $<TARGET_PDB_FILE:LabRender>
    DESTINATION
        bin OPTIONAL)

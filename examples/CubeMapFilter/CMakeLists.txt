
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
    LabCmd::Core
    Lab::Math
    LabRender::Core
    LabRender::Binder
    LabRender::Imgui
    LabRender::ModelLoader)

#set_target_properties(CubeMapFilterLab
#    PROPERTIES
#    RUNTIME_OUTPUT_DIRECTORY "${LOCAL_ROOT}/bin"
#)

#set_target_properties( targets LABRENDER
#    PROPERTIES
#    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/lib"
#    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/lib"
#    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin"
#)

if (MSVC_IDE)
    # hack to get around the "Debug" and "Release" directories cmake tries to add on Windows
    #set_target_properties (CubeMapFilterLab PROPERTIES PREFIX "../")
    #set_target_properties(CubeMapFilterLab PROPERTIES IMPORT_PREFIX "../")
endif()

#add_custom_command(TARGET CubeMapFilterLab POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        "${PROJECT_BINARY_DIR}/src/$<CONFIGURATION>/LabRender.dll"
#        $<TARGET_FILE_DIR:CubeMapFilterLab>)
#add_custom_command(TARGET CubeMapFilterLab POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        "${Assimp_BINARY_RELEASE}"
#        $<TARGET_FILE_DIR:CubeMapFilterLab>)
#add_custom_command(TARGET CubeMapFilterLab POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        "${GLEW_BINARY_RELEASE}"
#        $<TARGET_FILE_DIR:CubeMapFilterLab>)
#add_custom_command(TARGET CubeMapFilterLab POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        "${Z_BINARY_RELEASE}"
#        $<TARGET_FILE_DIR:CubeMapFilterLab>)


message(INFO "------ ${LOCAL_ROOT}")

install (TARGETS CubeMapFilterLab RUNTIME DESTINATION "${LOCAL_ROOT}/bin")

#install (TARGETS CubeMapFilterLab DESTINATION bin)

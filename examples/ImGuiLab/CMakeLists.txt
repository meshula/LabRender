
set(IMGUILAB_SRC
    "${LABRENDER_ROOT}/examples/ImGuiLab/LabRenderDemoApp.h"
    "${LABRENDER_ROOT}/examples/ImGuiLab/LabRenderExampleApp.cpp"
    "${LABRENDER_ROOT}/examples/ImGuiLab/ImGuiIntegration.cpp"
)

add_executable(ImGuiLab ${IMGUILAB_SRC})

target_compile_definitions(ImGuiLab PRIVATE PLATFORM_WINDOWS=1)
target_compile_definitions(ImGuiLab PRIVATE ASSET_ROOT="${LABRENDER_ROOT}/assets")
target_include_directories(ImGuiLab PRIVATE "${LOCAL_ROOT}/include")

target_link_libraries(ImGuiLab debug
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARY_RELEASE}
    ${Assimp_LIBRARY_RELEASE}
    ${LabCmd_LIBRARY_DEBUG})

target_link_libraries(ImGuiLab optimized
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARY_RELEASE}
    ${Assimp_LIBRARY_RELEASE}
    ${LabCmd_LIBRARY_RELEASE})

target_link_libraries(ImGuiLab
    GLM::Core 
    GLEW::Shared
    LabCmd::Core
    LabRender::Core
    LabRender::Binder
    LabRender::Imgui
    LabRender::ModelLoader)

if (MSVC_IDE)
    # hack to get around the "Debug" and "Release" directories cmake tries to add on Windows
    #set_target_properties (ImGuiLab PROPERTIES PREFIX "../")
    #set_target_properties(ImGuiLab PROPERTIES IMPORT_PREFIX "../")
endif()

set_target_properties(ImGuiLab
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

install (TARGETS ImGuiLab
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)

#add_custom_command(TARGET ImGuiLab POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        "${PROJECT_BINARY_DIR}/src/$<CONFIGURATION>/LabRender.dll"
#        $<TARGET_FILE_DIR:ImGuiLab>)
#add_custom_command(TARGET ImGuiLab POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        "${Assimp_BINARY_RELEASE}"
#        $<TARGET_FILE_DIR:ImGuiLab>)
#add_custom_command(TARGET ImGuiLab POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        "${GLEW_BINARY_RELEASE}"
#        $<TARGET_FILE_DIR:ImGuiLab>)
#add_custom_command(TARGET ImGuiLab POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        "${Z_BINARY_RELEASE}"
#        $<TARGET_FILE_DIR:ImGuiLab>)

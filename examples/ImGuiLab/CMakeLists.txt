
set(IMGUILAB_SRC
    "${LABRENDER_ROOT}/examples/ImGuiLab/imgui_demo.cpp"
    "${LABRENDER_ROOT}/examples/ImGuiLab/imgui_draw.cpp"
    "${LABRENDER_ROOT}/examples/ImGuiLab/imgui.cpp"
    "${LABRENDER_ROOT}/examples/ImGuiLab/ImGuiImplementation.cpp"
    "${LABRENDER_ROOT}/examples/ImGuiLab/LabRenderDemoApp.h"
    "${LABRENDER_ROOT}/examples/ImGuiLab/LabRenderExampleApp.cpp"
)

add_executable(ImGuiLab ${IMGUILAB_SRC})

target_compile_definitions(ImGuiLab PRIVATE PLATFORM_WINDOWS=1)
target_compile_definitions(ImGuiLab PRIVATE ASSET_ROOT="${LABRENDER_ROOT}/assets")
target_include_directories(ImGuiLab PRIVATE "${LOCAL_ROOT}/include")
target_include_directories(ImGuiLab PRIVATE "${LABRENDER_ROOT}/include")
target_include_directories(ImGuiLab PRIVATE "${LABRENDER_ROOT}/extras/include")
target_include_directories(ImGuiLab PRIVATE "${GLEW_INCLUDE_DIR}")
target_sources(ImGuiLab PRIVATE "${LABRENDER_ROOT}/extras/src/modelLoader.cpp")
target_sources(ImGuiLab PRIVATE "${LABRENDER_ROOT}/extras/include/extras/modelLoader.h")

target_link_libraries(ImGuiLab debug
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARY_RELEASE}
    ${Assimp_LIBRARY_RELEASE}
    ${LabCmd_LIBRARY_DEBUG}
    LabRender)
target_link_libraries(ImGuiLab optimized
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARY_RELEASE}
    ${Assimp_LIBRARY_RELEASE}
    ${LabCmd_LIBRARY_RELEASE}
    LabRender)

#set_target_properties(ImGuiLab
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
    #set_target_properties (ImGuiLab PROPERTIES PREFIX "../")
    set_target_properties(ImGuiLab PROPERTIES IMPORT_PREFIX "../")
endif()

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


message(INFO "------ ${LOCAL_ROOT}")

install (TARGETS ImGuiLab RUNTIME DESTINATION "${LOCAL_ROOT}/bin")

#install (TARGETS ImGuiLab DESTINATION bin)

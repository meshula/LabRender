
file(GLOB LABRENDEREXAMPLES_SRC "*.cpp")
add_executable(LabRenderExample ${LABRENDEREXAMPLES_SRC})

target_compile_definitions(LabRenderExample PRIVATE PLATFORM_WINDOWS=1)
target_compile_definitions(LabRenderExample PRIVATE ASSET_ROOT="${LABRENDER_ROOT}/assets")
target_include_directories(LabRenderExample PRIVATE "${LOCAL_ROOT}/include")
target_include_directories(LabRenderExample PRIVATE "${LABRENDER_ROOT}/include")
target_include_directories(LabRenderExample PRIVATE "${LABRENDER_ROOT}/extras/include")
target_include_directories(LabRenderExample PRIVATE "${GLEW_INCLUDE_DIR}")
target_sources(LabRenderExample PRIVATE "${LABRENDER_ROOT}/extras/src/modelLoader.cpp")
target_sources(LabRenderExample PRIVATE "${LABRENDER_ROOT}/extras/include/extras/modelLoader.h")

target_link_libraries(LabRenderExample debug
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARY_RELEASE}
    ${Assimp_LIBRARY_RELEASE}
    ${LabCmd_LIBRARY_DEBUG}
    LabRender)
target_link_libraries(LabRenderExample optimized
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARY_RELEASE}
    ${Assimp_LIBRARY_RELEASE}
    ${LabCmd_LIBRARY_RELEASE}
    LabRender)

#set_target_properties(LabRenderExample
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
    #set_target_properties (LabRenderExample PROPERTIES PREFIX "../")
    #set_target_properties(LabRenderExample PROPERTIES IMPORT_PREFIX "../")
endif()

#add_custom_command(TARGET LabRenderExample POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        "${PROJECT_BINARY_DIR}/src/$<CONFIGURATION>/LabRender.dll"
#        $<TARGET_FILE_DIR:LabRenderExample>)
#add_custom_command(TARGET LabRenderExample POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        "${Assimp_BINARY_RELEASE}"
#        $<TARGET_FILE_DIR:LabRenderExample>)
#add_custom_command(TARGET LabRenderExample POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        "${GLEW_BINARY_RELEASE}"
#        $<TARGET_FILE_DIR:LabRenderExample>)
#add_custom_command(TARGET LabRenderExample POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        "${Z_BINARY_RELEASE}"
#        $<TARGET_FILE_DIR:LabRenderExample>)


message(INFO "------ ${LOCAL_ROOT}")

install (TARGETS LabRenderExample RUNTIME DESTINATION "${LOCAL_ROOT}/bin")

#install (TARGETS LabRenderExample DESTINATION bin)

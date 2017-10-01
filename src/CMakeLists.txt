
file(GLOB LABRENDER_SRC "*.cpp")
add_library(LabRender SHARED ${LABRENDER_SRC})


if (MSVC_IDE)
    # hack to get around the "Debug" and "Release" directories cmake tries to add on Windows
    #set_target_properties(LabRender PROPERTIES PREFIX "../")
    set_target_properties(LabRender PROPERTIES IMPORT_PREFIX "../")
endif()


target_include_directories(LabRender PRIVATE "${LABRENDER_ROOT}/include")
target_include_directories(LabRender PRIVATE "${LOCAL_ROOT}/include")
target_compile_definitions(LabRender PRIVATE BUILDING_LABRENDER=1)
target_compile_definitions(LabRender PUBLIC
    PLATFORM_WINDOWS=1)

target_link_libraries(LabRender ${OPENGL_LIBRARIES} ${GLEW_LIBRARY})

set_target_properties(LabRender
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
#set_target_properties( targets LABRENDER
#    PROPERTIES
#    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/lib"
#    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/lib"
#    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin"
#)

install (TARGETS LabRender
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)

file(GLOB LABRENDER_INC ${CMAKE_CURRENT_SOURCE_DIR}/../include/LabRender/*.h)
install (FILES ${LABRENDER_INC} DESTINATION include/LabRender)


set(TARGET_NAME ImmediateExample)

set(${TARGET_NAME}_SRC
    ImmediateDemoApp.cpp
    ImmediateDemoApp.h
    InsectAI/src/Actuator.cpp
    InsectAI/src/Agent.cpp
    InsectAI/src/BoidSensors.cpp
    InsectAI/src/Engine.cpp
    InsectAI/src/Function.cpp
    InsectAI/src/InsectAI3.cpp
    InsectAI/src/Light.cpp
    InsectAI/src/Sensor.cpp
    InsectAI/include/InsectAI/DynamicState.h
    InsectAI/include/InsectAI/InsectAI.h
    InsectAI/include/InsectAI/InsectAI3.h
    InsectAI/include/InsectAI/InsectAI_Actuator.h
    InsectAI/include/InsectAI/InsectAI_Agent.h
    InsectAI/include/InsectAI/InsectAI_BoidSensors.h
    InsectAI/include/InsectAI/InsectAI_Engine.h
    InsectAI/include/InsectAI/InsectAI_Sensor.h
    InsectAI/include/InsectAI/InsectAI_Sensors.h
    InsectAI/include/InsectAI/nanoflann.hpp
    InsectAI/include/InsectAI/OpenSteerLibrary.h
    InsectAI/include/InsectAI/OpenSteerUtilities.h
    InsectAI/include/InsectAI/OpenSteerVec3.h
)

add_executable(${TARGET_NAME} ${${TARGET_NAME}_SRC})
set_cxx_version(${TARGET_NAME} 14)

target_compile_definitions(${TARGET_NAME} PRIVATE PLATFORM_WINDOWS=1)
target_compile_definitions(${TARGET_NAME} PRIVATE ASSET_ROOT="${LABRENDER_ROOT}/assets")
target_include_directories(${TARGET_NAME} PRIVATE "${LOCAL_ROOT}/include")
target_include_directories(${TARGET_NAME} PRIVATE "${LABRENDER_ROOT}/include")
target_include_directories(${TARGET_NAME} PRIVATE "${LABRENDER_ROOT}/extras/include")

target_link_libraries(${TARGET_NAME} debug
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARY_RELEASE}
    ${Assimp_LIBRARY_RELEASE})

target_link_libraries(${TARGET_NAME} optimized
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARY_RELEASE}
    ${Assimp_LIBRARY_RELEASE})

target_link_libraries(${TARGET_NAME} 
    Lab::Math 
    LabCmd::Core 
    LabRender::Core 
    LabRender::ModelLoader
    Fmt::Format
    GLM::Core 
    GLEW::Shared)

set_property(TARGET ${TARGET_NAME} PROPERTY FOLDER "examples")

target_include_directories(${TARGET_NAME} PRIVATE InsectAI/include)

install (TARGETS ${TARGET_NAME} 
    RUNTIME DESTINATION "${LOCAL_ROOT}/bin")


set(TARGET_NAME ImmediateExample)

set(INSECTAI_SRC
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

set(${TARGET_NAME}_SRC
    ImmediateDemoApp.cpp
    ${INSECTAI_SRC}
)

source_group(InsectAI FILES ${INSECTAI_SRC})

add_executable(${TARGET_NAME} ${${TARGET_NAME}_SRC})

target_compile_definitions(${TARGET_NAME} PRIVATE PLATFORM_WINDOWS=1)
target_compile_definitions(${TARGET_NAME} PRIVATE ASSET_ROOT="${LABRENDER_ROOT}/assets")
target_include_directories(${TARGET_NAME} PRIVATE "${LABRENDER_ROOT}/include")
target_include_directories(${TARGET_NAME} PRIVATE "${LABRENDER_ROOT}/extras/include")

target_link_libraries(${TARGET_NAME} debug
    ${OPENGL_LIBRARIES})

target_link_libraries(${TARGET_NAME} optimized
    ${OPENGL_LIBRARIES})

target_link_libraries(${TARGET_NAME} 
    Lab::Math 
    Lab::Cmd 
    Lab::Render 
    Lab::ModelLoader
    glfw
    )
set_property(TARGET ${TARGET_NAME} PROPERTY FOLDER "examples")

target_include_directories(${TARGET_NAME} PRIVATE InsectAI/include)

install (TARGETS ${TARGET_NAME} 
    RUNTIME DESTINATION bin)

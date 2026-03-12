cmake_minimum_required(VERSION 3.10)

project(gsRender)

set(CMAKE_CXX_STANDARD 17)
set(THIRDPARTY ${CMAKE_SOURCE_DIR}/thirdParty)

link_directories(
    ${PROJECT_SOURCE_DIR}/lib
)

file(GLOB_RECURSE SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/source/*.cpp
    ${PROJECT_SOURCE_DIR}/source/*.c
    ${PROJECT_SOURCE_DIR}/thirdParty/imgui/*.cpp
    ${PROJECT_SOURCE_DIR}/thirdParty/tinyply-3.0/source/tinyply.cpp

)

add_executable(${PROJECT_NAME} app.cpp  ${SOURCE_FILES})


target_include_directories(${PROJECT_NAME} PRIVATE 
    ${THIRDPARTY}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/thirdParty/glad
    ${CMAKE_SOURCE_DIR}/thirdParty/glfw
    ${CMAKE_SOURCE_DIR}/thirdParty/glm
    ${CMAKE_SOURCE_DIR}/thirdParty/imgui
    ${CMAKE_SOURCE_DIR}/thirdParty/KHR
    ${CMAKE_SOURCE_DIR}/thirdParty/tinyply-3.0
)
target_link_libraries(${PROJECT_NAME}
    glfw3.lib
    opengl32.lib
)
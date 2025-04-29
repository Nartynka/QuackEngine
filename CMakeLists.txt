cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_SUPPRESS_REGENERATION true)

project(QuackEngine)

find_package(OpenGL REQUIRED)

set(GLFW_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/lib/GLFW/include")
set(GLFW_LIBRARY "${CMAKE_SOURCE_DIR}/lib/GLFW/lib-vc2022/glfw3.lib")

set(GLEW_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/lib/GLEW/include")
set(GLEW_LIBRARY "${CMAKE_SOURCE_DIR}/lib/GLEW/lib/Release/x64/glew32s.lib")

set(GLM_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/lib/glm")

set(STB_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/lib/stb_image")

set(IMGUI_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/lib/imgui")

set(SPDLOG_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/lib/spdlog/include")
set(SPDLOG_LIBRARY "${CMAKE_SOURCE_DIR}/lib/spdlog/Debug/spdlogd.lib")

file(GLOB_RECURSE IMGUI_FILES
	"${CMAKE_SOURCE_DIR}/lib/imgui/*.h"
	"${CMAKE_SOURCE_DIR}/lib/imgui/*.cpp"
)

source_group("imgui" FILES ${IMGUI_FILES})


file(GLOB_RECURSE HEADER_FILES
	"${CMAKE_SOURCE_DIR}/src/*.h"
)

file(GLOB_RECURSE SOURCE_FILES
	"${CMAKE_SOURCE_DIR}/src/*.cpp"
)

include_directories(
	${GLFW_INCLUDE_PATH}
	${GLEW_INCLUDE_PATH}
	${GLM_INCLUDE_PATH}
	${STB_INCLUDE_PATH}
	${IMGUI_INCLUDE_PATH}
	${SPDLOG_INCLUDE_PATH}
)

add_executable(${PROJECT_NAME}
	${HEADER_FILES}
	${SOURCE_FILES}
	${IMGUI_FILES}
)

target_link_libraries(${PROJECT_NAME}
	${OPENGL_LIBRARY}
	${GLFW_LIBRARY}
	${GLEW_LIBRARY}
	${STB_LIBRARY}
	${SPDLOG_LIBRARY}
)

target_compile_options(${PROJECT_NAME} PRIVATE "/utf-8")

set_target_properties(${PROJECT_NAME} PROPERTIES
	VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
)

add_compile_definitions(GLEW_STATIC)


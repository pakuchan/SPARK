# ############################################# #
#                                               #
#             SPARK Particle Engine             #
#                External - GLEW                #
#                                               #
# ############################################# #



# Project declaration
# ###############################################
cmake_minimum_required(VERSION 2.8)
project(GLEW)
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	set(SPARK_OPENGL_LIB "GL" CACHE STRING "The name of the library file of OpenGL.")
else()
	set(SPARK_OPENGL_LIB "opengl32" CACHE STRING "The name of the library file of OpenGL.")
endif()



# Sources
# ###############################################
set(SPARK_DIR ../../..)
if(NOT CMAKE_GENERATOR_PLATFORM STREQUAL "")
	set(SPARK_GENERATOR_POSTFIX " ${CMAKE_GENERATOR_PLATFORM}")
endif()
set(SPARK_GENERATOR "(${CMAKE_SYSTEM_NAME}@${CMAKE_GENERATOR}${SPARK_GENERATOR_POSTFIX})")
get_filename_component(SPARK_DIR ${SPARK_DIR}/void REALPATH)
get_filename_component(SPARK_DIR ${SPARK_DIR} PATH)
file(GLOB_RECURSE SRC_FILES
	${SPARK_DIR}/external/glew/src/glew.c
	${SPARK_DIR}/external/glew/src/glew.h
)



# Build step
# ###############################################
add_definitions(-DGLEW_BUILD)
add_library(GLEW SHARED ${SRC_FILES})
include_directories(${SPARK_DIR}/external/glew/src)
target_link_libraries(GLEW
	general ${SPARK_OPENGL_LIB}
)
set_target_properties(GLEW PROPERTIES
	OUTPUT_NAME glew
	DEBUG_POSTFIX _d
	RUNTIME_OUTPUT_DIRECTORY ${SPARK_DIR}/lib/${SPARK_GENERATOR}/external/glew
	LIBRARY_OUTPUT_DIRECTORY ${SPARK_DIR}/lib/${SPARK_GENERATOR}/external/glew
	ARCHIVE_OUTPUT_DIRECTORY ${SPARK_DIR}/lib/${SPARK_GENERATOR}/external/glew
	RUNTIME_OUTPUT_DIRECTORY_DEBUG ${SPARK_DIR}/lib/${SPARK_GENERATOR}/external/glew
	LIBRARY_OUTPUT_DIRECTORY_DEBUG ${SPARK_DIR}/lib/${SPARK_GENERATOR}/external/glew
	ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${SPARK_DIR}/lib/${SPARK_GENERATOR}/external/glew
	RUNTIME_OUTPUT_DIRECTORY_RELEASE ${SPARK_DIR}/lib/${SPARK_GENERATOR}/external/glew
	LIBRARY_OUTPUT_DIRECTORY_RELEASE ${SPARK_DIR}/lib/${SPARK_GENERATOR}/external/glew
	ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${SPARK_DIR}/lib/${SPARK_GENERATOR}/external/glew
)

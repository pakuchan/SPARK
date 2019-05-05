# ############################################# #
#                                               #
#         SPARK Particle Engine : Demos         #
#              Simple GLES2 Test                #
#                                               #
# ############################################# #



# Project declaration
# ###############################################
cmake_minimum_required(VERSION 2.8)
project(Test_GLES2)
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	set(SPARK_GLES2_LIB "GLESv2" CACHE STRING "The name of the library file of OpenGLES2.")
else()
	set(SPARK_GLES2_LIB "opengl32" CACHE STRING "The name of the library file of OpenGLES2.")
endif()



# Sources
# ###############################################
set(SPARK_DIR ../../..)
get_filename_component(SPARK_DIR ${SPARK_DIR}/void REALPATH)
get_filename_component(SPARK_DIR ${SPARK_DIR} PATH)
set(SRC_FILES
	${SPARK_DIR}/demos/src/SPKTestGLES2.cpp
)



# Build step
# ###############################################
if(NOT CMAKE_GENERATOR_PLATFORM STREQUAL "")
	set(SPARK_GENERATOR_POSTFIX " ${CMAKE_GENERATOR_PLATFORM}")
endif()
set(SPARK_GENERATOR "(${CMAKE_SYSTEM_NAME}@${CMAKE_GENERATOR}${SPARK_GENERATOR_POSTFIX})")
include_directories(${SPARK_DIR}/include)
include_directories(${SPARK_DIR}/demos/external/SDL2/include)
link_directories(${SPARK_DIR}/demos/external/SDL2/${LIB_DIR_NAME})
include_directories(${SPARK_DIR}/external/glew/src)
if(${DEMOS_USE_STATIC_LIBS})
	add_definitions(-DGLEW_STATIC)
	link_directories(${SPARK_DIR}/lib/${SPARK_GENERATOR}/static)
	link_directories(${SPARK_DIR}/lib/${SPARK_GENERATOR}/external_static/glew)
	link_directories(${SPARK_DIR}/lib/${SPARK_GENERATOR}/external/pugixml)
else()
	add_definitions(-DSPK_IMPORT)
	link_directories(${SPARK_DIR}/lib/${SPARK_GENERATOR}/dynamic)
	link_directories(${SPARK_DIR}/lib/${SPARK_GENERATOR}/external/glew)
endif()
add_executable(Test_GLES2
	${SRC_FILES}
)
target_link_libraries(Test_GLES2
	general SDL2.lib
	general SDL2main.lib
	general ${SPARK_GLES2_LIB}
	debug glew_d
	debug SPARK_debug
	debug SPARK_GLES2_debug
	optimized glew
	optimized SPARK
	optimized SPARK_GLES2
)
set_target_properties(Test_GLES2 PROPERTIES
	DEBUG_POSTFIX _debug
	RUNTIME_OUTPUT_DIRECTORY ${SPARK_DIR}/demos/${BIN_DIR_NAME}
	RUNTIME_OUTPUT_DIRECTORY_DEBUG ${SPARK_DIR}/demos/${BIN_DIR_NAME}
	RUNTIME_OUTPUT_DIRECTORY_RELEASE ${SPARK_DIR}/demos/${BIN_DIR_NAME}
)














# ############################################# #
#                                               #
#         SPARK Particle Engine : Demos         #
#                 Test Irrlicht                 #
#                                               #
# ############################################# #



# Project declaration
# ###############################################
cmake_minimum_required(VERSION 2.8)
project(Test_Irrlicht_Ctrl)
set(DEFAULT_IRRLICHT_DIR ../../../external/irrlicht)
set(DEMOS_IRRLICHT_LIB "Irrlicht.lib" CACHE STRING "Store the name of the Irrlicht library file (release)")
set(DEMOS_IRRLICHT_LIB_DEBUG "Irrlicht.lib" CACHE STRING "Store the name of the Irrlicht library file (debug)")
set(DEMOS_IRRLICHT_INCLUDE_DIR ${DEFAULT_IRRLICHT_DIR}/include CACHE PATH "Store the path to the 'include' folder of Irrlicht")
set(DEMOS_IRRLICHT_LIB_DIR ${DEFAULT_IRRLICHT_DIR}/${LIB_DIR_NAME} CACHE PATH "Store the path to the library folder of Irrlicht, ie 'lib/<config>'")



# Sources
# ###############################################
set(SPARK_DIR ../../..)
get_filename_component(SPARK_DIR ${SPARK_DIR}/void REALPATH)
get_filename_component(SPARK_DIR ${SPARK_DIR} PATH)
set(SRC_FILES
	${SPARK_DIR}/demos/src/SPKTestIrrlicht_Controllers.cpp
)



# Build step
# ###############################################
if(NOT CMAKE_GENERATOR_PLATFORM STREQUAL "")
	set(SPARK_GENERATOR_POSTFIX " ${CMAKE_GENERATOR_PLATFORM}")
endif()
set(SPARK_GENERATOR "(${CMAKE_SYSTEM_NAME}@${CMAKE_GENERATOR}${SPARK_GENERATOR_POSTFIX})")
add_definitions(-DSPK_IMPORT)
include_directories(${SPARK_DIR}/include)
include_directories(${DEMOS_IRRLICHT_INCLUDE_DIR})
get_filename_component(DEMOS_IRRLICHT_LIB_DIR ${DEMOS_IRRLICHT_LIB_DIR} REALPATH)
link_directories(${DEMOS_IRRLICHT_LIB_DIR})
if(${DEMOS_USE_STATIC_LIBS})
	link_directories(${SPARK_DIR}/lib/${SPARK_GENERATOR}/static)
else()
	link_directories(${SPARK_DIR}/lib/${SPARK_GENERATOR}/dynamic)
endif()
add_executable(Test_Irrlicht_Ctrl ${SRC_FILES})
if(("${DEMOS_IRRLICHT_LIB}" STREQUAL "") OR
   ("${DEMOS_IRRLICHT_LIB_DEBUG}" STREQUAL ""))
	message("	You have to set both DEMOS_IRRLICHT_LIB and DEMOS_IRRLICHT_LIB_DEBUG for the Irrlicht build to work.
	Ignore this message if you're not using Irrlicht.")
else()
	target_link_libraries(Test_Irrlicht_Ctrl
		debug SPARK_debug
		debug SPARK_IRR_debug
		debug ${DEMOS_IRRLICHT_LIB_DEBUG}
		optimized SPARK
		optimized SPARK_IRR
		optimized ${DEMOS_IRRLICHT_LIB}
	)
endif()
set_target_properties(Test_Irrlicht_Ctrl PROPERTIES
	DEBUG_POSTFIX _debug
	RUNTIME_OUTPUT_DIRECTORY ${SPARK_DIR}/demos/${BIN_DIR_NAME}
	RUNTIME_OUTPUT_DIRECTORY_DEBUG ${SPARK_DIR}/demos/${BIN_DIR_NAME}
	RUNTIME_OUTPUT_DIRECTORY_RELEASE ${SPARK_DIR}/demos/${BIN_DIR_NAME}
)














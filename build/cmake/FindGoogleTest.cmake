# - Find google test include folder and libraries
# This module finds tinyxml2 if it is installed and determines where 
# the files are. This code sets the following variables:
#
#  GTEST_FOUND             - have google test been found
#  GTEST_INCLUDE_DIR       - path to where tinyxml2.h is found
#
FIND_PATH(GTEST_INCLUDE_DIR
	NAMES gtest/gtest.h
	PATHS
		${GTEST_INCLUDE_DIR}
		${GTEST_ROOT}/include
		${GTEST_ROOT}
)

IF(CMAKE_TRACE)
	MESSAGE(STATUS "GTEST_INCLUDE_DIR=${GTEST_INCLUDE_DIR}")
ENDIF(CMAKE_TRACE)

SET(GTEST_FIND_COMPONENTS gtest gtest_main)

IF(GTEST_INCLUDE_DIR)
	SET(GTEST_FOUND TRUE)
	FOREACH(COMPONENT ${GTEST_FIND_COMPONENTS})
		STRING(TOUPPER ${COMPONENT} UPPERCOMPONENT)
		FIND_LIBRARY(GTEST_${UPPERCOMPONENT}_LIBRARY_RELEASE 
			NAMES ${COMPONENT}
			PATHS
			${GTEST_ROOT}/release
			${GNUWIN32_DIR}/lib
		)
		FIND_LIBRARY(GTEST_${UPPERCOMPONENT}_LIBRARY_DEBUG 
			NAMES ${COMPONENT}
			PATHS
			${GTEST_ROOT}/debug
			${GNUWIN32_DIR}/lib
		)
		IF(GTEST_${UPPERCOMPONENT}_LIBRARY_RELEASE AND GTEST_${UPPERCOMPONENT}_LIBRARY_DEBUG)
			SET(GTEST_${UPPERCOMPONENT}_FOUND TRUE)
			SET(GTEST_${UPPERCOMPONENT}_LIBRARY optimized ${GTEST_${UPPERCOMPONENT}_LIBRARY_RELEASE} debug ${GTEST_${UPPERCOMPONENT}_LIBRARY_DEBUG})
			set(GTEST_${UPPERCOMPONENT}_LIBRARY ${GTEST_${UPPERCOMPONENT}_LIBRARY} CACHE FILEPATH "The breakpad ${UPPERCOMPONENT} library")
		ELSE(GTEST_${UPPERCOMPONENT}_LIBRARY_RELEASE AND GTEST_${UPPERCOMPONENT}_LIBRARY_DEBUG)
			SET(GTEST_FOUND FALSE)
			SET(GTEST_${UPPERCOMPONENT}_FOUND FALSE)
			SET(GTEST_${UPPERCOMPONENT}_LIBRARY "${GTEST_${UPPERCOMPONENT}_LIBRARY-NOTFOUND}")
		ENDIF(GTEST_${UPPERCOMPONENT}_LIBRARY_RELEASE AND GTEST_${UPPERCOMPONENT}_LIBRARY_DEBUG)
		IF(CMAKE_TRACE)
			MESSAGE(STATUS "${COMPONENT}: GTEST_${UPPERCOMPONENT}_LIBRARY=${GTEST_${UPPERCOMPONENT}_LIBRARY}")
		ENDIF(CMAKE_TRACE)
	ENDFOREACH(COMPONENT)
ENDIF(GTEST_INCLUDE_DIR)

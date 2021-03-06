cmake_minimum_required(VERSION 2.6)
IF(WIN32)
	PROJECT(NSCPDOTNET)

	SET(SRCS
		AssemblyInfo.cpp
		NSCP.h
		stdafx.cpp
		stdafx.h
	)

	#ADD_DEFINITIONS(${NSCP_GLOBAL_DEFINES})

	STRING(REPLACE "/EHsc" "/EHa" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	STRING(REPLACE "/EHs" "/EHa" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	STRING(REPLACE "/MT" "/MD" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	STRING(REPLACE "/EHsc" "/EHa" CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
	STRING(REPLACE "/EHs" "/EHa" CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
	STRING(REPLACE "/MT" "/MD" CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
	STRING(REPLACE "/MT" "/MD" CMAKE_CXX_FLAGS_MINSIZEREL ${CMAKE_CXX_FLAGS_MINSIZEREL})
	STRING(REPLACE "/MT" "/MD" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
	STRING(REPLACE "/EHsc" "/EHa" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
	STRING(REPLACE "/EHs" "/EHa" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
	STRING(REPLACE "/MTd" "/MDd" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
	STRING(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
	STRING(REPLACE "/RTCs" "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
	STRING(REPLACE "/RTCu" "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
	STRING(REPLACE "/GZ" "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /clr /EHa")
	
	ADD_LIBRARY(NSCPDOTNET MODULE ${SRCS})

	SET_TARGET_PROPERTIES(NSCPDOTNET PROPERTIES FOLDER "libraries")
	
	INSTALL(TARGETS NSCPDOTNET LIBRARY DESTINATION .)
	SET_TARGET_PROPERTIES(NSCPDOTNET PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${BUILD_TARGET_EXE_PATH}")
	IF(MSVC_IDE)
		# hack to get around the "Debug" and "Release" directories cmake tries to add on Windows
		SET_TARGET_PROPERTIES(NSCPDOTNET PROPERTIES PREFIX "../")
	ENDIF(MSVC_IDE)
ENDIF(WIN32)

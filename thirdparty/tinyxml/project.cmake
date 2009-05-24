if(AQSIS_USE_EXTERNAL_TINYXML)
	find_package(TinyXML)

	if(AQSIS_TINYXML_FOUND)
		include_directories(${AQSIS_TINYXML_INCLUDE_DIR})
	else()
		message(STATUS "Cannot find external tinyxml library - using version included with the aqsis source.")
		set(AQSIS_USE_EXTERNAL_TINYXML OFF)
	endif()
endif()


if(NOT AQSIS_USE_EXTERNAL_TINYXML)
	set(tinyxml_srcs
		tinystr.cpp
		tinyxml.cpp
		tinyxmlerror.cpp
		tinyxmlparser.cpp
	)
	make_absolute(tinyxml_srcs ${tinyxml_SOURCE_DIR})

	set(tinyxml_hdrs
		tinystr.h
		tinyxml.h
	)
	make_absolute(tinyxml_hdrs ${tinyxml_SOURCE_DIR})

	include_directories(${tinyxml_SOURCE_DIR})
endif()

add_definitions(-DTIXML_USE_STL)

cmake_minimum_required(VERSION 2.6)
find_package(Qt4 COMPONENTS QtGui QtOpenGL REQUIRED)
find_package(Boost 1.34.1 COMPONENTS program_options)
find_path(ILMBASE_INCLUDE_DIR OpenEXR/ImathBox.h)
include_directories(${QT_INCLUDES} ${ILMBASE_INCLUDE_DIR})
include_directories(${Boost_INCLUDE_DIRS})

if(UNIX)
    if("${CMAKE_CXX_FLAGS}" STREQUAL "")
        set(CMAKE_CXX_FLAGS "-Wall -Werror" CACHE STRING
            "Flags used by the compiler during all build types." FORCE)
    endif()
endif()

#set(CMAKE_BUILD_TYPE Release)

qt4_wrap_cpp(moc_srcs ptview.h)

set(srcs
	${moc_srcs}
	cornellbox.cpp
	ptview.cpp
	microbuffer.cpp
)

add_executable(ptview ${srcs})
target_link_libraries(ptview ${QT_QTGUI_LIBRARY} ${QT_QTOPENGL_LIBRARY}
					  ${Boost_PROGRAM_OPTIONS_LIBRARY})

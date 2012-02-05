set(io_srcs
	itexinputfile.cpp
	itexoutputfile.cpp
	itiledtexinputfile.cpp
	magicnumber.cpp
	texfileheader.cpp
	tiffdirhandle.cpp
	tiffinputfile.cpp
	tiffoutputfile.cpp
	tifftest_examples.cpp
	tiledanyinputfile.cpp
	tiledtiffinputfile.cpp
	zinputfile.cpp
)
if(AQSIS_USE_OPENEXR)
    list(APPEND io_srcs exrinputfile.cpp)
endif()
if(AQSIS_USE_PNG)
	list(APPEND io_srcs pnginputfile.cpp)
endif()
make_absolute(io_srcs ${io_SOURCE_DIR})

set(io_hdrs
	exrinputfile.h
	magicnumber.h
	tiffdirhandle.h
	tifffile_test.h
	tiffinputfile.h
	pnginputfile.h
	tiffoutputfile.h
	tiledanyinputfile.h
	tiledtiffinputfile.h
	zinputfile.h
)
make_absolute(io_hdrs ${io_SOURCE_DIR})
include_directories(${io_SOURCE_DIR})

set(io_test_srcs
	magicnumber_test.cpp
	texfileheader_test.cpp
	tiffdirhandle_test.cpp
	tiffinputfile_test.cpp
)
if(AQSIS_USE_PNG)
	list(APPEND io_test_srcs pnginputfile_test.cpp)
endif()
make_absolute(io_test_srcs ${io_SOURCE_DIR})


# Setup stuff for linking with external libs
include_directories(${AQSIS_TIFF_INCLUDE_DIR})
set(io_linklibs
    ${AQSIS_TIFF_LIBRARIES}
    ${AQSIS_TIFFXX_LIBRARIES}
)
if(AQSIS_USE_PNG)
	list(APPEND io_linklibs ${AQSIS_PNG_LIBRARIES})
endif()


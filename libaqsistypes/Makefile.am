SUBDIRS = posix win32 .
EXTRA_DIST = libaqsistypes.dsp

lib_LTLIBRARIES = libaqsistypes.la

libaqsistypes_la_SOURCES = aqsis.h aqsis_types.h bitvector.cpp bitvector.h color.cpp color.h exception.h file.cpp file.h list.h matrix.cpp matrix.h memorypool.h random.h sstring.cpp sstring.h vector2d.cpp vector2d.h vector3d.cpp vector3d.h vector4d.cpp vector4d.h cellnoise.h cellnoise.cpp noise.h noise.cpp spline.h spline.cpp mtable.h mtable.cpp log.h log.cpp plugins.cpp

libaqsistypes_la_LIBADD = $(top_builddir)/libaqsistypes/posix/libplatform.la
if PLUGINS
libaqsistypes_la_LDFLAGS = -rpath $(libdir) -ldl
else
libaqsistypes_la_LDFLAGS = -rpath $(libdir) 
endif


include_HEADERS = aqsis.h aqsis_types.h bitvector.h color.h exception.h file.h list.h matrix.h memorypool.h random.h sstring.h vector2d.h vector3d.h vector4d.h refcount.h

if DEBUG
AM_CXXFLAGS = -g -Wall
else
AM_CXXFLAGS =
endif

INCLUDES = -I$(srcdir)/posix


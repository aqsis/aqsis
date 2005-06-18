SUBDIRS = posix win32 .
EXTRA_DIST = libaqsistypes.dsp libaqsistypes.vcproj

AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@

lib_LTLIBRARIES = libaqsistypes.la
INCLUDES = -I$(srcdir)/posix

libaqsistypes_la_SOURCES = aqsis.h aqsis_types.h bitvector.cpp bitvector.h color.cpp color.h exception.h file.cpp file.h list.h matrix.cpp matrix.h random.h random.cpp sstring.cpp sstring.h vector2d.cpp vector2d.h vector3d.cpp vector3d.h vector4d.cpp vector4d.h cellnoise.h cellnoise.cpp noise.h noise.cpp spline.h spline.cpp plugins.cpp plugins.h refcount.cpp refcount.h validate.h logging.h logging_streambufs.h logging.cpp

libaqsistypes_la_LIBADD = $(top_builddir)/libaqsistypes/posix/libplatform.la @PLUGIN_FLAGS@

pkginclude_HEADERS = aqsis.h aqsis_types.h bitvector.h color.h exception.h file.h list.h matrix.h random.h sstring.h vector2d.h vector3d.h vector4d.h refcount.h


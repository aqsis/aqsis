AC_INIT()

AM_INIT_AUTOMAKE(aqsis, 1.1.0)
AC_PREFIX_DEFAULT(/usr/local/aqsis)
AM_CONFIG_HEADER(libaqsistypes/config.h)

AC_DEFINE(STRNAME, "Aqsis", [Application Name] )

AC_LANG_CPLUSPLUS
AC_PROG_CXX

dnl We need to do this early so that we can disable 
dnl shared libraries if profiling is in use 
AC_ENABLE_SHARED
AQSIS_WITH_PROFILING
AC_PROG_LIBTOOL

AM_PROG_LEX
AC_PROG_YACC
AC_EXEEXT

AC_C_BIGENDIAN

AQSIS_WITH_FLTK
AQSIS_CHECK_LIBJPEG
AQSIS_CHECK_LIBTIFF
AQSIS_CHECK_LIBZ
AQSIS_CHECK_BOOST

AQSIS_WITH_DEBUGGING
AQSIS_WITH_OPENEXR
AQSIS_WITH_PLUGINS
AQSIS_WITH_XSLTPROC

AC_SUBST(AQSIS_CPPFLAGS)
AC_SUBST(AQSIS_CXXFLAGS)
AC_SUBST(AQSIS_CFLAGS)
AQSIS_CPPFLAGS=""
AQSIS_CXXFLAGS="$AQSIS_DEBUG_CFLAGS $AQSIS_PROFILE_CFLAGS $PTHREAD_CFLAGS"
AQSIS_CFLAGS="$AQSIS_DEBUG_CFLAGS $AQSIS_PROFILE_CFLAGS $PTHREAD_CFLAGS"

AC_CONFIG_FILES([
Makefile
all/AdditionalDeveloperLibs/Makefile 
all/Makefile 
aqsis/Makefile 
aqsl/Makefile 
aqsl/tests/Makefile 
aqsltell/Makefile 
bin/Makefile 
display/Makefile 
dsotest/Makefile 
images/Makefile 
libaqsistypes/Makefile 
libaqsistypes/posix/Makefile 
libaqsistypes/win32/Makefile 
libaqsistypes/win32/intel/Makefile 
libargparse/Makefile 
libaqfile/Makefile
libcodegenvm/Makefile 
libddmanager/Makefile 
libraytrace/Makefile 
libri2rib/Makefile 
librib2/Makefile 
librib2/tests/Makefile 
librib2ri/Makefile 
librib2stream/Makefile 
libshaderexecenv/Makefile 
libshadervm/Makefile 
libslparse/Makefile 
libslxargs/Makefile 
man/Makefile 
man/aqsis.1 
plugins/Makefile 
plugins/bake2tif/Makefile 
plugins/bmp2tif/Makefile 
plugins/common/Makefile 
plugins/exr2tif/Makefile 
plugins/gif2tif/Makefile 
plugins/jpg2tif/Makefile 
plugins/pcx2tif/Makefile 
plugins/ppm2tif/Makefile 
plugins/tga2tif/Makefile 
render/Makefile 
renderer.xcode/Makefile 
ribfiles/Makefile 
shaders/Makefile 
slpp/Makefile 
teqser/Makefile 
])

AC_OUTPUT


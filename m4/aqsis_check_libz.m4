#########################################################
# AQSIS_CHECK_LIBZ

AC_DEFUN([AQSIS_CHECK_LIBZ], [

AC_SUBST(AQSIS_Z_CPPFLAGS)
AC_SUBST(AQSIS_Z_LDFLAGS)

AC_ARG_WITH(zlib_include,[  --with-libz-include=DIR path to ZLIB includes [[/usr/include]]],
	[if test "$withval" != no; then
		AQSIS_Z_CPPFLAGS="-I$withval"
	else
		AQSIS_Z_CPPFLAGS=-I/usr/include
	fi],[AQSIS_Z_CPPFLAGS=-I/usr/include])

AC_ARG_WITH(zlib_lib,[  --with-libz-lib=DIR     path to ZLIB libraries [[/usr/lib]]],
	[if test "$withval" != no; then
		AQSIS_Z_LDFLAGS="-L$withval -lz" 
	else
		AQSIS_Z_LDFLAGS="-L/usr/lib -lz" 
	fi],[AQSIS_Z_LDFLAGS="-L/usr/lib -lz"])

save_cflags=$CFLAGS
save_libs=$LIBS

CFLAGS=$AQSIS_Z_CPPFLAGS
LIBS=$AQSIS_Z_LDFLAGS

AC_CHECK_HEADER(zlib.h)
AC_CHECK_LIB(z, gzopen)

CFLAGS=$save_cflags
LIBS=$save_libs

if test ! "$ac_cv_header_zlib_h" || test ! "ac_cv_lib_z_gzopen" ; then
        AC_MSG_ERROR([*** Please ensure you have zlib libraries and headers  installed ***]);
fi

])


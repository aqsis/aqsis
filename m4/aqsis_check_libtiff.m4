#####################################################################################
# AQSIS_CHECK_LIBTIFF

AC_DEFUN([AQSIS_CHECK_LIBTIFF], [

AC_SUBST(AQSIS_TIFF_CPPFLAGS)
AC_SUBST(AQSIS_TIFF_LDFLAGS)

AC_ARG_WITH(tiff_include,[  --with-tiff-include=DIR path to TIFF includes [[/usr/include]]],
	[if test "$withval" != no; then
		AQSIS_TIFF_CPPFLAGS="-I$withval"
	else
		AQSIS_TIFF_CPPFLAGS="-I/usr/include"
	fi],[AQSIS_TIFF_CPPFLAGS="-I/usr/include"])
	
AC_ARG_WITH(tiff_lib,[  --with-tiff-lib=DIR     path to TIFF libraries [[/usr/lib]]],
	[if test "$withval" != no; then
		AQSIS_TIFF_LDFLAGS="-L$withval -ltiff"
	else
		AQSIS_TIFF_LDFLAGS="-L/usr/local -ltiff"
	fi],[AQSIS_TIFF_LDFLAGS="-L/usr/local -ltiff"])

save_cflags=$CFLAGS
save_libs=$LIBS

CFLAGS=$AQSIS_TIFF_CPPFLAGS
LIBS=$AQSIS_TIFF_LDFLAGS

AC_CHECK_HEADER(tiff.h)
AC_CHECK_LIB(m, pow)
AC_CHECK_LIB(tiff, TIFFOpen)

CFLAGS=$save_cflags
LIBS=$save_libs

if test ! "$ac_cv_header_tiff_h" || test ! "ac_cv_lib_tiff_TIFFOpen" ; then
        AC_MSG_ERROR([*** Please ensure you have libtiff libraries and headers installed ***]);
fi

])


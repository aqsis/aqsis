######################################################################################
# AQSIS_CHECK_LIBJPEG

AC_DEFUN([AQSIS_CHECK_LIBJPEG], [

AC_SUBST(AQSIS_JPEG_CPPFLAGS)
AC_SUBST(AQSIS_JPEG_LDFLAGS)

AC_ARG_WITH(jpeg_include,[  --with-jpeg-include=DIR path to jpeg includes [[/usr/local/include]]],
	[if test "$withval" != no; then
		AQSIS_JPEG_CPPFLAGS="-I$withval"
	else
		AQSIS_JPEG_CPPFLAGS="-I/usr/include"
	fi],[AQSIS_JPEG_CPPFLAGS="-I/usr/include"])

AC_ARG_WITH(jpeg_lib,[  --with-jpeg-lib=DIR     path to jpeg libraries [[/usr/local/lib]]],
	[if test "$withval" != no; then
		AQSIS_JPEG_LDFLAGS="-L$withval -ljpeg"
	else
		AQSIS_JPEG_LDFLAGS="-L/usr/lib -ljpeg"
	fi],[AQSIS_JPEG_LDFLAGS="-L/usr/lib -ljpeg"])

save_cflags=$CFLAGS
save_libs=$LIBS

CFLAGS=$AQSIS_JPEG_CPPFLAGS
LIBS=$AQSIS_JPEG_LDFLAGS

AC_CHECK_HEADER(jpeglib.h)
AC_CHECK_LIB(jpeg, jpeg_read_header)

CFLAGS=$save_cflags
LIBS=$save_libs

if test ! "$ac_cv_header_jpeglib_h" || test ! "ac_cv_lib_jpeg_jpeg_read_header" ; then
        AC_MSG_ERROR([*** Please ensure you have libjpeg libraries and headers installed ***]);
fi

])


AC_DEFUN([AQSIS_LIBJPEG_ARGS],
[
  AC_CACHE_CHECK([for jpeg], have_libJPEG,
  [
  	AC_ARG_WITH(jpeg_lib,[  --with-jpeg-lib=DIR     path to jpeg libraries [[/usr/local/lib]]],
		[if test "$withval" != no; then
			JPEG_LIB="$withval"
		else
			JPEG_LIB=/usr/local/lib
		fi],[JPEG_LIB=/usr/local/lib])
	AC_ARG_WITH(jpeg_include,[  --with-jpeg-include=DIR path to jpeg includes [[/usr/local/include]]],
		[if test "$withval" != no; then
			JPEG_INC="-I$withval"
		else
			JPEG_INC=-I/usr/local/include
		fi],[JPEG_INC=-I/usr/local/include])
	JPEG_FLAGS="-L$JPEG_LIB -ljpeg"
	AC_SUBST(JPEG_FLAGS)
	AC_SUBST(JPEG_INC)
  ])
])

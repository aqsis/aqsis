AC_DEFUN([AQSIS_LIBTIFF_ARGS],
[
  AC_CACHE_CHECK([for libTIFF], have_libTIFF,
  [
  	AC_ARG_WITH(tiff_lib,[  --with-tiff-lib=DIR     path to TIFF libraries [[/usr/local/lib]]],
		[if test "$withval" != no; then
			TIFF_LIB="$withval"
		else
			TIFF_LIB=/usr/local/lib
		fi],[TIFF_LIB=/usr/local/lib])
	AC_ARG_WITH(tiff_include,[  --with-tiff-include=DIR path to TIFF includes [[/usr/local/include]]],
		[if test "$withval" != no; then
			TIFF_INC="-I$withval"
		else
			TIFF_INC=-I/usr/local/include
		fi],[TIFF_INC=-I/usr/local/include])
	TIFF_FLAGS="-L$TIFF_LIB -ltiff"
	AC_SUBST(TIFF_FLAGS)
	AC_SUBST(TIFF_INC)
  ])
])


dnl HAVE_LIBTIFF
dnl
AC_DEFUN([HAVE_LIBTIFF_ARGS],
[
  AC_CACHE_CHECK([for libTIFF], have_libTIFF,
  [
  	AC_ARG_WITH(tiff_lib,[  --with-tiff-lib=DIR path to TIFF libraries [defaults to /usr/local/lib]],
						[if test "$withval" != no; then
							TIFF_LIB="$withval"
						else
							TIFF_LIB=/usr/local/lib
						fi],[TIFF_LIB=/usr/local/lib])
	AC_ARG_WITH(tiff_include,[  --with-tiff-include=DIR path to TIFF includes [defaults to /usr/local/include]],
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

dnl HAVE_LIBZLIB
dnl
AC_DEFUN([HAVE_LIBZ_ARGS],
[
  AC_CACHE_CHECK([for zlib], have_libZLIB,
  [
  	AC_ARG_WITH(zlib_lib,[  --with-zlib-lib=DIR path to ZLIB libraries [defaults to /usr/local/lib]],
						[if test "$withval" != no; then
							ZLIB_LIB="$withval"
						else
							ZLIB_LIB=/usr/local/lib
						fi],[ZLIB_LIB=/usr/local/lib])
	AC_ARG_WITH(zlib_include,[  --with-zlib-include=DIR path to ZLIB includes [defaults to /usr/local/include]],
						[if test "$withval" != no; then
							ZLIB_INC="-I$withval"
						else
							ZLIB_INC=-I/usr/local/include
						fi],[ZLIB_INC=-I/usr/local/include])
	ZLIB_FLAGS="-L$ZLIB_LIB -lz" 
	AC_SUBST(ZLIB_FLAGS)
	AC_SUBST(ZLIB_INC)
  ])
])


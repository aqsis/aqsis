AC_DEFUN([AQSIS_LIBZ_ARGS],
[
  AC_CACHE_CHECK([for zlib], have_libZLIB,
  [
  	AC_ARG_WITH(zlib_lib,[  --with-zlib-lib=DIR     path to ZLIB libraries [[/usr/local/lib]]],
		[if test "$withval" != no; then
			ZLIB_LIB="$withval"
		else
			ZLIB_LIB=/usr/local/lib
		fi],[ZLIB_LIB=/usr/local/lib])
	AC_ARG_WITH(zlib_include,[  --with-zlib-include=DIR path to ZLIB includes [[/usr/local/include]]],
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


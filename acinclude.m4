dnl HAVE_LIBTIFF
dnl
AC_DEFUN([HAVE_LIBTIFF],
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

dnl HAVE_LIBARGPARSE
dnl
AC_DEFUN([HAVE_LIBARGPARSE],
[
  AC_CACHE_CHECK([for libARGPARSE], have_libARGPARSE,
  [
  	AC_ARG_WITH(argparse_lib,[  --with-argparse-lib=DIR path to argparse library [defaults to /usr/local/lib]],
						[if test "$withval" != no; then
							ARGPARSE_LIB="$withval"
						else
							ARGPARSE_LIB=/usr/local/lib
						fi],[ARGPARSE_LIB=/usr/local/lib])
	AC_ARG_WITH(argparse_include,[  --with-argparse-include=DIR path to argparse.h [defaults to /usr/local/include]],
						[if test "$withval" != no; then
							ARGPARSE_INC="-I$withval"
						else
							ARGPARSE_INC=-I/usr/local/include
						fi],[ARGPARSE_INC=-I/usr/local/include])
	ARGPARSE_FLAGS="-L$ARGPARSE_LIB -largparse" 
	AC_SUBST(ARGPARSE_FLAGS)
	AC_SUBST(ARGPARSE_INC)
  ])
])


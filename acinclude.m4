dnl HAVE_OPENGL
dnl
AC_DEFUN([HAVE_OPENGL],
[
  AC_CACHE_CHECK([for OpenGL], have_OpenGL,
  [
  AC_ARG_WITH(opengl_lib,[  --with-opengl-lib=DIR path to OpenGL libraries [defaults to /usr/local/lib]],
						[if test "$withval" != no; then
							OPENGL_LIB="$withval"
						else
							OPENGL_LIB=/usr/local/lib
						fi],[OPENGL_LIB=/usr/local/lib])
	AC_ARG_WITH(opengl_include,[  --with-opengl-include=DIR path to OpenGL includes [defaults to /usr/local/include]],
						[if test "$withval" != no; then
							OPENGL_INC="$withval"
						else
							OPENGL_INC=/usr/local/include
						fi],[OPENGL_INC=/usr/local/include])
	AC_SUBST(OPENGL_LIB)
	AC_SUBST(OPENGL_INC)
  ])
])

dnl WITH_GLUT
dnl
AC_DEFUN([HAVE_GLUT],
[
	AC_ARG_ENABLE(glut,[  --enable-glut             compile with glut library [default= yes]],
							[case "${enableval}" in
								true) glut=true;;
								false) glut=false;;
								yes) glut=true;;
								no) glut=false;;
								*) AC_MSG_ERROR(bad value ${enableval} for --enable-glut);;
							esac],
							[glut=true])
	AM_CONDITIONAL(GLUT, test x$glut = xtrue)
])

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
							TIFF_INC="$withval"
						else
							TIFF_INC=/usr/local/include
						fi],[TIFF_INC=/usr/local/include])
	AC_SUBST(TIFF_LIB)
	AC_SUBST(TIFF_INC)
  ])
])

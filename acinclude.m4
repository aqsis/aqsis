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
						fi])
	AC_ARG_WITH(opengl_include,[  --with-opengl-include=DIR path to OpenGL includes [defaults to /usr/local/include]],
						[if test "$withval" != no; then
							OPENGL_INC="$withval"
						else
							OPENGL_INC=/usr/local/include
						fi])
	AC_SUBST(OPENGL_LIB)
	AC_SUBST(OPENGL_INC)
  ])
])

dnl HAVE_GLUT
dnl
AC_DEFUN([HAVE_GLUT],
[
	AC_CACHE_CHECK([for glut library], have_glut,
	[
	AC_ARG_WITH(glut_lib,[  --with-glut=ARG compile with glut library [default=yes]],
						[if test "withval" != no; then
							COMPILE_WITH_GLUT=yes
						else
							COMPILE_WITH_GLUT=no
						fi])
	AC_SUBST(COMPILE_WITH_GLUT)
	])
])


########################################################
# AQSIS_WITH_OPENEXR

AC_DEFUN([AQSIS_WITH_OPENEXR], [
AC_SUBST(AQSIS_OPENEXR_CFLAGS)
AC_SUBST(AQSIS_OPENEXR_LIBS)
AC_SUBST(AQSIS_OPENEXR_SUBDIR)
AC_ARG_WITH(openexr, [  --with-openexr          build OpenEXR plug-ins [[no]]],[],[withval=no])
if test x$withval != xno; then
	AC_MSG_CHECKING([for OpenEXR])
	if pkg-config --exists OpenEXR >/dev/null 2>/dev/null; then
		AQSIS_OPENEXR_VERSION=`pkg-config --modversion OpenEXR`
		AQSIS_OPENEXR_CFLAGS=`pkg-config --cflags OpenEXR`
		AQSIS_OPENEXR_LIBS=`pkg-config --libs OpenEXR`
		AQSIS_OPENEXR_SUBDIR="exr2tif"
		AC_MSG_RESULT([found version $AQSIS_OPENEXR_VERSION])
	else
		AC_MSG_ERROR([couldn't find OpenEXR library ... if you don't require OpenEXR support, re-run configure with the --without-openexr option])
	fi
fi
])


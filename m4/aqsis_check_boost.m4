#########################################################
# AQSIS_CHECK_BOOST

AC_DEFUN([AQSIS_CHECK_BOOST],[

AC_SUBST(AQSIS_BOOST_CPPFLAGS)

AC_ARG_WITH(boost_include,[  --with-boost-include=DIR path to BOOST includes [[/usr/include]]],
	[if test "$withval" != no; then
		AQSIS_BOOST_CPPFLAGS="-I$withval"
	else
		AQSIS_BOOST_CPPFLAGS="-I/usr/include"
	fi],[AQSIS_BOOST_CPPFLAGS="-I/usr/include"])

save_cppflags=$CPPFLAGS
CPPFLAGS="$AQSIS_BOOST_CPPFLAGS"/boost
AC_CHECK_HEADER(version.hpp)
if test "$ac_cv_header_version_hpp" = no; then
	AC_MSG_ERROR([*** ERROR: Can't find BOOST headers. You may need to install BOOST ***])
fi
CPPFLAGS=$save_cppflags

])


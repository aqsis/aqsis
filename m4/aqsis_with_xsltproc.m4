########################################################
# AQSIS_WITH_XSLTPROC

AC_DEFUN([AQSIS_WITH_XSLTPROC], [

AC_CHECK_PROG(have_xsltproc, xsltproc, yes, no)
if test x$have_xsltproc = "xno" ; then
	AC_MSG_WARN([*** Could not find xsltproc, you will need it if you edit any of the xml or xsl files ***]);
fi

])


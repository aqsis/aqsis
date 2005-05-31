########################################################
# AQSIS_WITH_PLUGINS

AC_DEFUN([AQSIS_WITH_PLUGINS], [
AC_SUBST(AQSIS_PLUGIN_LDFLAGS)
AC_SUBST(AQSIS_PLUGIN_SUBDIR)
AC_ARG_WITH(plugins, [  --with-plugins          compile with plugin support [[default=yes]]], [plugins=$withval], [plugins=yes])

if test x$plugins = xyes; then
	AC_DEFINE(PLUGINS, , [Support dynamically loaded plugins])
	AQSIS_PLUGIN_LDFLAGS="-ldl"
	AQSIS_PLUGIN_SUBDIR="plugins"
	AC_MSG_NOTICE([Plugins enabled])
else
	AC_MSG_NOTICE([Plugins disabled])
fi

])


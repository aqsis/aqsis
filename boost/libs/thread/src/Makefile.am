SUBDIRS = . mac

noinst_LTLIBRARIES = libthread.la

libthread_la_SOURCES = \
condition.cpp \
exceptions.cpp \
mutex.cpp \
once.cpp \
recursive_mutex.cpp \
thread.cpp \
threadmon.cpp \
tss.cpp \
xtime.cpp

INCLUDES=-I$(top_srcdir)/boost

AM_CXXFLAGS = @AQSIS_CXXFLAGS@
AM_CPPFLAGS = @AQSIS_CPPFLAGS@


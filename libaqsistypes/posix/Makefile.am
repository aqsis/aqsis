noinst_LTLIBRARIES = libplatform.la

libplatform_la_SOURCES = aqsis_compiler.h criticalsection.h semaphore.cpp semaphore.h share.h

if DEBUG
CXXFLAGS = -g -Wall
else
CXXFLAGS =
endif

INCLUDES = -I$(top_srcdir)/libaqsistypes 


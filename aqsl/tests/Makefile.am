EXTRA_DIST = test.sl 
AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@

aqsl=$(top_srcdir)/aqsl/aqsl

check-local:
	rm -f *.slx
	for file in $$(find $(top_srcdir) -name "*.sl" | sort); do $(aqsl) $$file; done


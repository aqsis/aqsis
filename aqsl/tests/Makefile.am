EXTRA_DIST = test.sl 
AM_CXXFLAGS=@AQSIS_CXXFLAGS@
AM_CPPFLAGS=@AQSIS_CPPFLAGS@
AM_CFLAGS=@AQSIS_CFLAGS@

test-clean:
	rm -f *.slx

aqsl=$(top_srcdir)/aqsl/aqsl

test: test-clean 
	PATH=$$PATH:$(top_builddir)/aqsl; for file in $$(find $(top_srcdir) -name "*.sl" | sort); do $(aqsl) $$file; done

.PHONY: test-clean test


EXTRA_DIST = test.sl 

test-clean:
	rm -f *.slx

aqsl=$(top_builddir)/aqsl/aqsl

test: test-clean 
	for file in *.sl; do $(aqsl) $$file; done
	for file in $$(find $(top_srcdir)/shaders -name "*.sl"); do $(aqsl) $$file; done

.PHONY: test-clean test


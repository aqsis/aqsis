EXTRA_DIST = displays.ini

## Note: the displays.ini in CVS is a hard-coded version for Win32
## On Posix systems we generate displays.ini at install-time

CONFIG_FILE = $(sysconfdir)/displays.ini
DISPLAY_LIB = $(pkglibdir)/libdisplay.so

install-exec-local:
	mkdir -p @sysconfdir@
	rm -f $(CONFIG_FILE)
	echo "file	$(DISPLAY_LIB)" >> $(CONFIG_FILE)
	echo "tiff	$(DISPLAY_LIB)" >> $(CONFIG_FILE)
	echo "framebuffer	$(DISPLAY_LIB)" >> $(CONFIG_FILE)
	echo "zframebuffer	$(DISPLAY_LIB)" >> $(CONFIG_FILE)
	echo "zfile	$(DISPLAY_LIB)" >> $(CONFIG_FILE)
	echo "shadow	$(DISPLAY_LIB)" >> $(CONFIG_FILE)


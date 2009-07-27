# Unix makefile for new piqsl interface prototype.

# Paths to fltk install - may need to change these.
INCLUDE=-I /usr/include/fltk-1.1
LIBDIRS=-L /usr/lib/fltk-1.1

# Need to link with libfltk and libfltk_iamges:
LIBS=$(LIBDIRS) -lfltk -lfltk_images


# Targets
piqsl_new: CenterScroll.h Pane.h ZoomImage.h main.cpp
	g++ -o piqsl_new main.cpp $(INCLUDE) $(LIBS)


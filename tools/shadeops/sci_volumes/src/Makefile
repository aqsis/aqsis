# Unix Makefile to build the sample_vol shadeop.
#
# Customize this to the location of your installed renderman implementation
# (needed for shadeop.h)
AQSIS_INCLUDES=-I /home/foster/aqsis/trunk/build/output/include/aqsis
#AQSIS_INCLUDES=-I /usr/local/include/aqsis

# -fPIC flag needed for shared libs on x86_64 architecture.  Shouldn't need it on x86.
CPPFLAGS=-O3 -Wall -fPIC
#CPPFLAGS=-Wall -fPIC -g

all: sample_vol.so

volumesampler.o: volumesampler.cpp volumesampler.h array3d.h
	g++ -c $(CPPFLAGS) volumesampler.cpp

sample_vol_shadeops.o: sample_vol_shadeops.cpp volumesampler.h array3d.h
	g++ -c $(CPPFLAGS) $(AQSIS_INCLUDES) sample_vol_shadeops.cpp

sample_vol.so: sample_vol_shadeops.o volumesampler.o
	g++ $(CPPFLAGS) $(AQSIS_INCLUDES) -shared sample_vol_shadeops.o volumesampler.o -o sample_vol.so

clean:
	rm -f *.o *.so

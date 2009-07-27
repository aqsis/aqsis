CXXFLAGS=-O3 -Wall
#CXXFLAGS=-fno-inline -O3 -pg -Wall
#CXXFLAGS=-g -Wall

all: region_allocator2 region_allocator

region_allocator: region_allocator.cpp
	g++ $(CXXFLAGS) region_allocator.cpp -o region_allocator

region_allocator2: region_allocator2.cpp
	g++ $(CXXFLAGS) region_allocator2.cpp -o region_allocator2

clean:
	rm -f region_allocator region_allocator2

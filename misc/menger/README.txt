RiProcDynamicLoad Example - the Menger Sponge
=============================================

This example shows how to use the dynamic load procedural type to generate the
geometry of the Menger Sponge fractal on the fly.  In general, procedurals are
good because geometry is created only when the renderer hits the bounding box.
The bounding box may even allow procedural geometry to be culled before any
real geometry is even generated!

The 2D Menger sponge may be formed through a recursive process in which the
middle 1/9 of a square is removed at every stage.  Here's a rough
approximation of the first three stages of the process in ASCII art:

       Level 0              Level 1              Level 2

   X X X X X X X X X    X X X X X X X X X    X X X X X X X X X
   X X X X X X X X X    X X X X X X X X X    X   X X   X X   X
   X X X X X X X X X    X X X X X X X X X    X X X X X X X X X
   X X X X X X X X X    X X X       X X X    X X X       X X X
   X X X X X X X X X    X X X       X X X    X   X       X   X
   X X X X X X X X X    X X X       X X X    X X X       X X X
   X X X X X X X X X    X X X X X X X X X    X X X X X X X X X
   X X X X X X X X X    X X X X X X X X X    X   X X   X X   X
   X X X X X X X X X    X X X X X X X X X    X X X X X X X X X

... and so forth.

One obvious genralization of this to 3D (removing the centre 1/27th of the
cube) doesn't look very interesting since we can't see inside.  Instead we
choose a scheme which involves viewing the cube down each of the three axes in
turn, and removing the centre 1/9th of the material as seen down the axis.


Compiling
=========

The procedural must be compiled into a shared library before use.  On 64-bit
linux systems with g++ this may be achieved as follows:

g++ -Wall -fPIC -O3 -shared menger.cpp -I /path/to/rih -o menger.so

where path/to/rih is a path at which ri.h for the renderer can be found.
Other systems and compilers should be similar, but the compiler flags required
to obtain an appropriate shared library may vary.

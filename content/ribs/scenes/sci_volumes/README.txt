Overview
========
This directory contains some possibly useful parts from a scientific volume
data visualization project I played with a while back.

Eventually we found that the specific data sets we were interested in were
better visualized as an isosurface using global illumination.  However, the
volume sampling and integration code has been requested by a few people so
it's been cleaned up a little for distribution.


Data sets
=========
The data sets I was trying to visulalize were surface distance functions: A
surface was considered to lie within the cube [0,1]x[0,1]x[0,1] and every
value in a data set represented the signed distance between the associated
point and the closest part of the surface.

Data format
-----------
Volume data files have the following extremely simple ASCII format:

1) A header consisting of three integers representing the resolution of the
volume data in the x,y,z directions.

2) The data as a string of whitespace-separated floting point numbers.


Shading model
=============
The shading represents a basic volume-glow-and-attenuation model.  Each point
inside the volume contained within the surface glows with an intensity and
colour which is dependent on the distance to the nearest part of the surface.
The density also causes attenuation with distance.


Rendering the scene
===================
There's a few things to do to render the scene:
1) Compile the shaders (*.sl) using aqsl
2) Unzip the data file iso_3d_restart.dat.gz
3) Compile the volume shader DSO using the system compiler.  On unix this
   should be easily done with the provided makefile.
4) Render volume.rib aqsis (other RenderMan-compatible renderers should work
   too).


License
=======

The files here are all licenced under the GNU GPL - see COPYING for details.


- Chris F. (c42f), Jan 2008.

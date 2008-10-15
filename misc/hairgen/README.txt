Introduction
============

Rendering hair and fur is a challenging process.  On the one hand, the number
of individual hairs required for realistic-looking fur can be several million;
this suggests that hairs should be generated procedurally at render-time.
On the other hand, the user often needs artistic control over the hair to
achieve the effect they're after.

Blender solves these challenges using a system of parent (guide) hairs and
child (interpolated) hairs.  The parent hairs are under the control of the
user, while the child hairs are generated procedurally, based on interpolation
between parents and a small number of adjustable parameters.

Unfortunately, the interpolation machinery used by blender is closely
integrated with the blender internals which means that it can't effectively be
used by external renderers.  The procedural code here is designed to solve this
problem for RenderMan compatible renderers.

The procedural has been designed with generality in mind, so it should work
with any program which can output hairs and meshes in RIB format.

Compiling
=========
In order to compile the procedural, aqsis-1.5 is required, along with cmake for
the build system.  The procedural can be compiled with the supplied cmake script.

Linux
-----

$ cd $build_dir  # should be different from the source directory.
$ ccmake $path_to_source
$ # <-- user sets the AQSIS_HOME environment variable correctly inside the cmake GUI
$ make

Windows
-------
Windows is unlikely to work with the current script but should do so with
minimal modifications.


Usage
=====

Here's an overview of the process:
  * Create a set of parent hairs in a modelling program
  * Export emitting mesh for child hairs, and the parent hairs themselves in RIB format
  * Create a RIB file calling the procedural, as shown below
  * Write shaders making use of interpolated primitive variables on the child curves.
  * Render!

Required RIB files
------------------

In order to use the procedural, the user needs to specify the emitting
mesh for the hairs and the parent curves.  These should be supplied in standard
RIB format; the procedural parses this RIB and creates child hairs using the
interpolation scheme.

file/to/render.rib:
  # ...
  Procedural "DynamicLoad" [
  	"hairgen"
  	"30000 path/to/emitter.rib  path/to/curves.rib [ <emitter_to_curves_transform> ]"
  	]
  	[ <bounding_box_for_child_hairs> ]
  # ...

path/to/emitter.rib:
  PointsPolygons ...

path/to/curves.rib:
  Curves ...


Interpolated primvars
---------------------
One important thing about the procedural is that it interpolates all primitive
variables ("primvars") attached to the parent hairs *and* the emitting mesh.
These are attached to the child hairs and give us a lot of flexibility - for
instance, hair shaders can use texture coordinates from the emitting mesh.

In order to avoid naming clashes, primvars from the emitting mesh are renamed
with an "_emit" suffix.  Consider the following:

PointsPolygons [..] [..]
	"P" [..]
	"Cs" [..]
	"st" [..]

Curves "cubic" "nonperiodic" [..]
	"P" [..] 
	"width" [..]

(Here primvar types and interpolation classes have been omitted for brevity.)
For this input data, the procedural generates a set of curves with the
following primvars attached:

Curves "cubic" "nonperiodic" [..]
	"P_emit" [..]    # <-- interpolated from emitting mesh
	"Cs_emit" [..]   # <-- interpolated from emitting mesh
	"st_emit" [..]   # <-- interpolated from emitting mesh
	"P" [..]         # <-- interpolated from parent curves
	"width" [..]     # <-- interpolated from parent curves


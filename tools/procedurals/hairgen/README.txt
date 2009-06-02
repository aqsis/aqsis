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
The procedural is compiled as part of the aqsis distribution and needs
>= aqsis-1.5 to make use of the aqsis RIB parser.  However, given that the
aqsis RIB parser is installed, it should be possible to use hairgen with any
other RenderMan-based renderer which has support for standard
RiProcDynamicLoad procedural geometry.


Usage
=====

Here's an overview of the process:
  * Create a set of parent hairs in a modelling program
  * Export emitting mesh for child hairs, and the parent hairs themselves in RIB format
  * Create a RIB file calling the procedural, as shown below
  * Write shaders making use of interpolated primitive variables on the child curves.
  * Render!


Calling the procedural
----------------------
The procedural may be called by putting the following in a RIB file:

Procedural "DynamicLoad" [ "hairgen" "<params>" ] [ <bounding_box_for_child_hairs> ]

The parameter string "<params>" is a list of (name,value) pairs the form

"name1=value1; name2=value2; ..."

Whitespace (including newlines) is not significant within the list and
semicolons are required to separate each (name,value) pair.  Values are
specified as simply as possible:
  * integers and floats are just how you'd normally write them.
  * booleans are specified with the words "true" and "false"
  * strings cannot contain whitespace (though this could be relaxed with
    some effort)
  * values which may be written as an array of basic types are written as a
    whitespace separated list (eg, for a 4x4 matrix, we'd have have the value
    string "1 0 0 0  0 1 0 0  0 0 1 0  0 0 0 1")

Here's the available parameters, first the general ones:
  * num_hairs               - total number of child hairs to generate (1000)
  * emitter_file_name       - path to RIB file containing emitting PointsPolygons mesh ("")
  * hair_file_name          - path to RIB file containing parent Curves ("")
  * emitter_to_hair_matrix  - transformation applied to the emitting mesh to
                              take it into the space of the parent hairs (identity)
  * verbose                 - boolean specifying whether to print extra debug info (false)
  * root_index              - index of the control point representing the root
                              of the hair.  For spline types which don't go
                              exactly through their control points, this may
                              not be the first control point (root_index=0).

Now some parameters which modify the child hairs individually:
  * end_rough     - boolean specifying whether to attach extra randomness for
                    use in simulating the blender "end rough" randomness in a
                    shader.
  * clump         - Clump specifies clumping behaviour in which child hairs
                    clump toward the dominant parent.  This parameter is
                    modelled after blender, and should lie in the interval [-1,1]:
                    For clump > 0:
                      The tips of child hairs clump toward the parents.  At
                      the tip, the weight of the parent hair is clump, and the
                      weight of the non-clumped child hair is (1-clump)
                    clump < 0:
                      The root of the child hairs clump toward the parents in
                      an analogous way to the tip for clump > 0.
  * clump_shape   - clump_shape is used in conjunction with clump, to control
					the blending between parent hairs and child hairs and
					should lie in the interval [-1,1].  For surface parameter
					v not at the tip of the curves, the clump parameter is
					modified to be clump*pow(v, clumpPow) where
					clumpPow = 1+clump_shape  for  -1 <= clump_shape < 0 and
					clumpPow = 1+9*clump_shape  for  0= < clump_shape <= 1



Shaders
-------
Shaders related to hairs can be found in the shaders directory.

  * hair_gritz.sl is an example from a siggraph course showing how to do
    strand lighting based on the curve tangent.
  * endrough.sl is a displacement shader simulating blender "end rough"


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


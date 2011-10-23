==========
Attributes
==========

Identifier Attributes
---------------------

These values associate information with the primitives they apply to, that
allow various processes to identify the primitive. This information is used
internally to provide meaningful feedback during rendering, and can be accessed
by shaders using the ``attribute`` RSL command. They are grouped under the
"identifier" attribute.

name
  The name applied to primitives in the current attribute block.

  Type: ``"string"``

  Example: ``Attribute "identifier" "name" [""]``


Displacement Bound Attributes
-----------------------------

These values control how Aqsis compensates for changes in a primitives surface
due to displacement shading. They are grouped under the "displacementbound"
attribute.

sphere
  Apply the specified amount of extra space in all directions to the bound of
  the primitives within the current attribute block to account for surface
  changes as a result of displacement shading.

  Type: ``"float"``

  Example: ``Attribute "displacementbound" "sphere" [0.0]``

coordinatesystem
  Specifies the coordinate system that the extra displacement offset is specified in.

  Type: ``"string"``

  Example: ``Attribute "displacementbound" "coordinatesystem" ["object"]``

Trimcurve Attributes
--------------------

Certain primitive types in Aqsis can have trimcurves, that is a 2D curve in
parameter space that removes a certain portion of the surface, commonly used
with NURBS surfaces. These values allow the user to control how those trim
curves are applied. They only apply to surfaces for which trim curves are
applicable. They are grouped under the "trimcurve" attribute.

sense
  Control the part of trimmed geometry that gets discarded. By default, Aqsis
  will discard trimmed geometry that is inside (according to the winding rules)
  the trim curves, changing this to "outside" will swap the behaviour.

  Type: ``"string"``

  Example: ``Attribute "trimcurve" "sense" ["inside"]``

Dice Attributes
---------------

When Aqsis processes primitives, it has, at various times, to make a decision
regarding whether to dice or split, and if dicing, how finely to dice. These
values allow the user to influence those decisions to achieve a specific
effect. They are grouped under the "dice" attribute.

binary
  Setting this value to anything other than 0 will force Aqsis to dice
  primitives so that the number of micropolygons on an edge is a factor of 2.
  This can alleviate some cracking problems.

  Type: ``"integer"``

  Example: ``Attribute "dice" "binary" [0]``

Aqsis Internal Attributes
-------------------------

The "aqsis" attribute is used for enabling internal hacks.

expandgrids
  Setting this to a number greater than zero will cause all grids arising from
  the associated primitives to have their boundary micropolygons expanded
  outward.  This may be used to work around the grid cracking problem - the
  appearance of small holes in curved surfaces.  The value of the attribute
  specifies the amount of expansion as a fraction of a micropolygon.  An
  expansion amount of 0.01 can be sufficient to prevent cracking for smooth
  surfaces, while larger values are necessary for surfaces with a lot of
  displacement.  Using this option with semitransparent surfaces or the
  "midpoint" depth filter for shadow map generation will result in artifacts.

  Type: ``"float"``

  Example: ``Attribute "aqsis" "expandgrids" [0.01]``

Autoshadows Attributes
----------------------

When used with the "multipass" render option, these attributes control the generation of automatic shadow depth maps by Aqsis.

res
  Define the resolution of automatically generated shadow maps. The maps are
  always square, so only one resolution value is required.

  Type: ``"integer"``

  Example: ``Attribute "autoshadows" "res" [300]``

shadowmapname
  The file name of the automatically generated shadow maps. This same name
  should then be used in the appropriate argument to the lightsource shader.

  Type: ``"string"``

  Example: ``Attribute "autoshadows" "shadowmapname" [""]``

Matte Attributes
----------------

``RiMatte`` is typically used to allow portions of an image to be replaced in
compositing with live action shots or backgrounds ("matte paintings") etc.  For
this kind of thing we'd also like to render shadows cast by CG objects so that
they form dark opaque areas which can be composited over the live action.
Unfortunately this isn't possible in a single pass when using the matte objects
defined in the standard, since the colour and opacity of the matte are
interpreted in an unusual way.

Aqsis adds an additional setting to the ``RiMatte()`` interface call,
``RI_MATTEALPHA`` to support this kind of usage.  *Mattes with Alpha*
are a new kind of Matte object which are always opaque from the point of view
of the hider but retain both opacity ("alpha") and colour information from
shaders attached to them.  That is, an alpha matte fully occludes all objects
behind it in the scene but the user can at the same time specify a nonzero
alpha value and colour which make their way unmodified into the output image.

This special attribute is specified via the RiMatte command:

C API:
  ``RiMatte(RI_MATTEALPHA)``

RIB Binding:
  ``Matte 2``

GeometricApproximation Attributes
---------------------------------

The ``GeometricApproximation`` attribute allows some control over the
accuracy with which the renderer tesselates geometry into micropolygons for
renderering.  Normally, the micropolygon area is constrained to be smaller than
the ``ShadingRate`` attribute.  However, when a surface is highly blurred -
either by motion blur or depth of field effects - it is desirable to increase
the shading rate for efficiency.  This has the effect of coarsening the
tessellation, but this often doesn't matter since the details are lost to
blurring in any case.

Defaults for the various types of geometric approximation have been chosen with
the intention of preserving image quality compared to images rendered with the
approximations turned off.

focusfactor
  The "focusfactor" approximation type makes depth of field rendering more
  efficient by scaling the effective shading rate with the area of the circle
  of confusion.  This guarentees that the number of hit tests between samples
  and micropolygons stays under control as the amount of blurring increases.

  Example: ``GeometricApproximation "focusfactor" 1.0`` is the default.

motionfactor
  The "motionfactor" approximation type makes motion blur rendering more
  efficient by scaling the shading rate proportionally to the distance
  travelled by a surface across the screen.  This feature is somewhat
  experimental in aqsis-1.6.

  Example: ``GeometricApproximation "motionfactor" 0.0`` turns the motionfactor approximation off.


Setting Options from the Command Line
=====================================

Aqsis provides a very flexible and powerful mechanism to override or add to
options in the RIB file being rendered.  The **-option** command line argument
allows you to insert arbitrary options - and in fact, arbitrary RIB fragments -
into the command stream just prior to the ``RiWorldBegin`` request.  You can
provide multiple fragments via multiple ``-option`` arguments; these will be
processed immediately before ``RiWorldBegin`` in the order that they are
specified on the command line.

A typical use for this facility is to override the display request to force
output to a different file, or to output to an additional file.  For example::

  aqsis -option="Display \"myname.tif\" \"file\" \"rgba\""  some_file.rib

allows you to specify not only the type of display, but also the name, and even
the type of data that will be displayed.  Note that you must be careful to
*escape the use of double quotes on the command line* so that they get
through to the renderer correctly.  Using double quotes within a command line
parameter is likely to confuse the command line processor; mark them with a '\' to
prevent them closing the double quotes surrounding the argument to ``-option``.

Aqsis has some additional command line arguments which also affect the options
state of the renderer.  For example, changing or adding displays may also be
done with the simple command line options ``-type`` and ``-addtype``, though
these offer less flexibility than the ``-option`` mechanism described above.

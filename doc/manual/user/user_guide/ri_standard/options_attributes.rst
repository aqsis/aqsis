======================
Options And Attributes
======================

.. highlight:: RIB

The operation of a RenderMan renderer is mostly controlled by the Options and
Attributes mechanisms, both of which associate named values with the various
stages of the rendering pipeline.

**Options** are associated with the scene as a whole.  Once the
``RiWorldBegin`` directive has been reached, the options are fixed, it is then
illegal to call any Ri directives that can modify the options state.
**Attributes** are associated with the attribute stack, and as such are
assigned to light sources and geometric primitives.  The value of these
attributes are pushed and popped along with other attribute state by the
``RiAttributeBegin/End`` directives.

Options and attributes are stored as a named set of name/value pairs. That is,
each option or attribute has a unique name, and can contain any number of
values, each with a unique name and type. The general format of an option or
attribute directive is::

  Option "<option name>" "<value1 typespec>" [<value1 value(s)>] "<value2 typespec>" [<value2 value(s)>]

where ``typespec`` is either the name of an already declared value, or an
inline declaration.  For example::

  Declare "string mystringvalue"
  Option "myopt" "mystringvalue" ["some string"] "float myfloatvalue" [1.0]

Note from the second example above that it is perfectly reasonable to specify
multiple name/value pairs in a single Option directive, each will add a new
value to the same containing option or attribute.

There are a number of predefined options and attributes that Aqsis recognizes
for use internally to configure the operation of the renderer; these are listed
below.  In addition, Aqsis fully supports the specification of arbitrary
user-defined name/value pairs for both options and attributes.  These may be
queried from the shading language using the standard ``option()`` and
``attribute()`` functions.


Options
=======

Searchpath Options
------------------

Aqsis locates the various external assets required during rendering via a
standard Option called "searchpath". The "searchpath" Option has a number of
string values that tell Aqsis where to look for various asset types. The string
value for each of these specifies a list of search paths separated by colons or
semicolons.

There are several characters which have special meanings in searchpath options:

* The character & expands to the previous value of the path
* The character @ expands to the corresponding default path (for example, @
  in the "shader" searchpath will expand to the value of the "defaultshader"
  searchpath.)
* The character % is used to delimit environment variables: ``%MY_PATH%``
  will expand to the value of the environment variable ``MY_PATH``.

As an example, by using the & character you can append a custom path to the
current search path for shaders using the RIB fragment::

  Option "searchpath" "shader" ["/my/shaders:&"]

Each option value is described below, they are all of type "string", and follow
the same format as the example above.

archive
  Aqsis will search in these folders for external RIB archives.  This option is
  used by the ``RiReadArchive`` directive.

display
  Aqsis will search for display devices in these folders.  This option is used
  by the ``RiDisplay`` directive, when it is looking for a display shared
  object.

shader
  Aqsis will search for shaders, and shader related assets in these folders.
  This option is used by the various shader related directives, such as
  ``RiSurface``.  In addition, both the shader compiler aqsl and the renderer
  itself, will use these folders when searching for DSO shadeops used in
  shaders.

procedural
  Aqsis will search these folders for procedural shared objects.  This option
  is used by the ``RiProcDynamicLoad`` directive to locate DSO's for the
  procedural RIB plugin.

texture
  Aqsis will search in these folders for all texture files.  This option is
  used by the various texture related shading language commands, when searching
  for a specified texture file.

resource
  Aqsis will search these folders for any assets not found using the specific
  values above.  This is a fallback option that specifies a global searchpath
  for any and all file types.


Hider Options
-------------

The **hider** specifies the algorithm used to resolve surface visibility (that
is, to decide which surfaces appear "on top" in the final render).  Aqsis
supports only one hider type, "hidden", which uses stochastic point sampling
for the visibility computation.

The "hidden" hider supports several options:

jitter
  This is used to turn random jittering of sample positions on or off.  Jitter
  is turned on by default in order to turn aliasing artifacts into less
  objectionable noise, but should be turned off when rendering shadow maps for
  best results.  Jitter is necessary for rendering depth of field and motion
  blur; turning it off in these cases will result in obvious artifacts.

  Type: ``"integer"``

  Example: ``Hider "hidden" "jitter" [0]``

depthfilter
  The depth filter defines the way in which depth samples will be modified
  before being passed to the display.  Possible values are: "min" (the
  default), "midpoint", "max" and "average".  The "min" depth filter computes
  the depth of the surface closest to the camera, "max" the surface furtherest,
  and "average" the average depth of all surfaces between the camera and the
  far clipping plane.  The "midpoint" depth filter computes the average depth
  of the two surfaces closest to the camera, and gives better results than
  "min" when used for shadow map generation.

  Type: ``"string"``

  Example: ``Hider "hidden" "depthfilter" ["min"]``

Limits Options
--------------

These values control the various settings used during rendering that have an
effect on performance and memory use. They are grouped under the "limits"
option.

bucketsize
  Set the dimensions (in pixels) of a rendering bucket.

  Type: ``"integer[2]"``

  Example: ``Option "limits" "bucketsize" [16 16]``

eyesplits
  Set the maximum number of eye splits before the renderer is giving up and
  discarding the geometry in which case a "Max eyesplits exceeded" warning is
  issued.  Note: Always try to push the near clipping plane as much away from
  the camera as possible.

  Type: ``"integer"``

  Example: ``Option "limits" "eyesplits" [10]``

gridsize
  Set the desired number of micropolygons per grid.

  Type: ``"integer"``

  Example: ``Option "limits" "gridsize" [256]``

texturememory
  Set the buffer size (in kB) for texture tiles. Aqsis tries not to exceed the
  specified value if possible (by discarding unused tiles whenever new tiles
  are required that would overflow the buffer). When a single tile is larger
  than the specified buffer Aqsis issues an "Exceeding allocated texture
  memory" warning.  Note: Not working in aqsis 1.6 and 1.8!

  Type: ``"integer"``

  Example: ``Option "limits" "texturememory" [8192]``

zthreshold
  Define the opacity at which a surface is deemed to be opaque for the purposes
  of shadow map generation.  Any surface with all components of opacity greater
  than the components specified by zthreshold will be included in shadow map
  generation.  The default zthreshold is ``[1 1 1]`` which means that any
  partially transparent object will be omitted from shadow maps by default.

  Type: ``"color"``

  Example: ``Option "limits" "zthreshold" [1 1 1]``

Shadow Options
--------------

Aqsis supports shadows using depth maps, these values control various settings
that affect the sampling and generation of depth maps, and their use during
rendering. They are grouped under the "shadow" option.

bias
  Specifies a small amount to be added (in "camera" space) to the depth values
  stored in the shadow map. This value can be tweaked to overcome self
  shadowing artefacts. Self shadowing happens when a surface being lit is
  exactly the same as the surface sampled to produce the shadowmap, Aqsis
  cannot easily determine if the surface should be in shadow or not, and
  inaccuracies in the floating point code cause the check to toggle between
  shadow and not, resulting in a noisy pattern on the surface. By shifting the
  depth stored in the shadowmap a little, such self shadowing can be avoided.

  Type: ``"float"``

  Example: ``Option "shadow" "bias" [0.0]``

bias0 and bias1
  Specifies a range of bias values, a value is chosen randomly within this
  range for the the shadow bias value, explained above.

  Type: ``"float"``

  Example: ``Option "shadow" "bias0" [0.01] "bias1" [0.05]``


Render Options
--------------

Certain features in the rendering pipeline can be controlled and/or enabled
depending on the content being rendered. These values allow the user to control
the renderer at a general level. They are grouped under the "render" option.

bucketorder
  Determines the order in which buckets are processed. Possible values are:
  "horizontal", "vertical", "zigzag", "circle" and "random".

  Type: ``"string"``

  Example: ``Option "render" "bucketorder" ["horizontal"]``

multipass
  Enables the use of multipass rendering. Used in conjunction with the
  "autoshadows" [[doc:options#attributes|Attributes]], this option enables the
  generation of automatic shadow maps.

  Type: ``"integer"``

  Example: ``Option "render" "multipass" [0]``


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

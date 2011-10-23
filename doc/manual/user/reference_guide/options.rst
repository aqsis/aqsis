=======
Options
=======

Searchpath Options
------------------

Aqsis locates the various external assets required during rendering via a
standard Option called "searchpath". The "searchpath" Option has a number of
string values that tell Aqsis where to look for various asset types. The string
value for each of these specifies a list of search paths separated by a colon.

The special search path character "&" represents the previous value of the
option. This is only available in "searchpath" options. Using this character
you can append or prepend paths to the default path list, i.e::

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


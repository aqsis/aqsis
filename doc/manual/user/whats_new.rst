.. _whatsnew:

.. Image references declared here.
.. |aqsis_1_2| image:: /images/texture_swirl_blur_oldtex.png
    :width: 100% 
.. |aqsis_1_4| image:: /images/texture_swirl_blur_newtex.png
    :width: 100% 
.. |3delight|  image:: /images/texture_swirl_blur_3delight7.png
    :width: 100% 
.. |constant|  image:: /images/aqsis-shading_constant.png
    :width: 100%
.. |smooth|    image:: /images/aqsis-shading_smooth.png
    :width: 100%
.. |no_expandgrids| image:: /images/expandgrids-before.png
    :width: 100%
.. |expandgrids| image:: /images/expandgrids-after.png
    :width: 100%
.. |freedesktop| image:: /images/freedesktop-logo.png
    :target: http://www.freedesktop.org

.. _here: http://wiki.aqsis.org/dev/aqsistex

What's New
----------

Aqsistex Library
^^^^^^^^^^^^^^^^

The old texture-handling code within Aqsis was rewritten for 1.4 and broken-out into a separate library.

Improvements include faster texturing operations, EWA filtering, correct anisotropic filtering and intelligent mipmap-level interpolation... the latter of which seems a missing functionality even within most commercial renderers (to date):

+---------------------+---------------------+---------------------+
| Aqsis 1.2           |  Aqsis 1.4          | Commercial Renderer |
+=====================+=====================+=====================+
| |aqsis_1_2|         | |aqsis_1_4|         | |3delight|          |
+---------------------+---------------------+---------------------+

More information on the aqsistex library can be found here_. 


.. index:: Smooth shading interpolation

Smooth Shading Interpolation
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Smooth shading interpolation was added in 1.4, allowing colour to be interpolated between the vertices of a micropolygon rather than the constant method which produces a constant colour for each micropolygon.

This feature can be accessed by using the following RIB statement::

    ShadingInterpolation "smooth" 

+------------------------+----------------------+
| Constant Interpolation | Smooth Interpolation |
+========================+======================+
| |constant|             | |smooth|             |
+------------------------+----------------------+

Since *Aqsis* 1.6, smooth shading also supports the motion blur and depth of field features.


.. index:: Grid Crack Bypass

Grid Crack Bypass
^^^^^^^^^^^^^^^^^

An Attribute was introduced in 1.4 to help deal with the common 'Grid Crack' issue, experienced by many Reyes-based renderers. The method works by expanding the micropolygons at the edges of each grid outward by a specified fraction of the micropolygon width.

This feature can be accessed by using the following RIB statement::

    Attribute "aqsis" "expandgrids" [0.01]

+------------------+---------------------+
|  Normal Result   | Expand Grids Result |
+==================+=====================+
| |no_expandgrids| | |expandgrids|       |
+------------------+---------------------+
	
.. warning:: Semi-opaque surfaces and shadows using either depthfilter or midpoint are not currently supported by this method.


.. index:: Massive

Massive Support
^^^^^^^^^^^^^^^

The RunProgram procedural plugin supplied with Massive on both Linux and Windows platforms is supported by default in version 1.4 and above, with Aqsis automatically checking known locations of the plugin.

.. figure:: /images/massive-cloth.png

.. note:: The DynamicLoad procedural plugin is not currently supported, though Massive Software have confirmed that the performance difference is negligible between the two methods.


.. index:: Simbiont

Simbiont Support
^^^^^^^^^^^^^^^^

Both the official and AIR distributed versions of Darkling Simulation LLC procedural shader plugin are supported by default in version 1.4 and above, with Aqsis automatically checking known locations of the plugin.

More information on using Simbiont(RM) and its related tools can be found in the Simbiont guide.


Improved DBO Support
^^^^^^^^^^^^^^^^^^^^

Support for the 'Dynamic Blob Op' procedural used within AIR was improved with version 1.4, with Aqsis automatically checking known locations of the plugin.



.. index:: Mac OSX; Bundle

OS X Distribution
^^^^^^^^^^^^^^^^^

*Aqsis* version 1.4 and above is distributed using the 'Bundle' (.app) format under OS X platforms, as recommended by Apple for desktop applications.

Executing Aqsis.app will launch eqsl and automatically configure the current/personal environment (PATH), allowing your system to always locate the relevant binaries:

.. figure:: /images/aqsis-bundle.png
    :width: 100%

We support OS X 10.4 (Tiger) and above, providing binaries for both PPC and Intel architectures.


.. index:: Linux; Desktop Integration

Linux Desktop Integration
^^^^^^^^^^^^^^^^^^^^^^^^^

Aqsis version 1.4 and above provides full desktop integration under Linux using the standards defined by freedesktop.org.

|freedesktop|

.. note:: Want to render a RIB file or compile a series of shaders? just right-click on the item(s) and select 'Open with Aqsis ...', job done!

Other Enhancements in 1.4

* Implement vector noise() variations. (#1629458)
* Add support for RiMitchellFilter.
* Various improvements to the bake functionality.
* Add -decodeonly option to miqser to decode a binary file with no additional formatting.
* Reimplemented timer functionality to provide accurate timings at endofframe on all platforms.
* Support "scanlineorder" request to piqsl and framebuffer to support rendering a row of buckets at a time.
* Add support for @ in searchpaths to substitute the default value.
* Implement break/continue support in aqsl. (#1801181)
* Use boost::wave as a preprocessor for aqsl, as the slpp code was a bit opaque. (#1182387)
* Add support for %NAME% style environment substitution in "searchpaths".
* Support wildcards in aqsl.
* ShaderVM now checks the compiler version used to compile a shader, and reports an error if it doesn't match.
* Improve the handling of surfaces crossing the projection plane, should now deal with eyesplits much better.
* Improve the calculation of surface derivatives, especially for polygons. (#1829764)

Other Enhancements in 1.6

Version 1.6 of Aqsis focuses on optimisations and speed enhancements, including:

* Avoid recomputing samples at overlapping bucket boundaries
* CqMatrix optimisations
* Removal of IqBound
* Substantially refactored sampling and occlusion culling code
* Enable the "focusfactor" approximation for depth of field by default
* Implement the "motionfactor" approximation for motion blur
* Improved shadow map rendering speed
* Faster splitting code for large point clouds
* Piqsl interface refactor (Single window interface, improved keyboard shortcuts, improved zooming, Z-buffer support)
* Texturing improvements (environment filtering, filtering fine control)
* Smooth shading interpolation for depth of field and motion blur
* Matte alpha support for directly rendering shadows on otherwise transparent surfaces
* New RIB parser with better error reporting
* Multilayer support for OpenEXR display
* Side Effects Houdini plugin


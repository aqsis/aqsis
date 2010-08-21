.. include:: ../../common.rst

.. index:: aqsis

.. _aqsis:

===============
Renderer: aqsis
===============

Overview
--------

The aqsis executable is a command-line renderer which processes scene descriptions using then **RenderMan Interface Bytestream** (RIB) protocol, which is a standard streaming file format specified as part of the |RenderMan| interface.  When launched, aqsis processes the requests in the RIB stream, which in turn can force processing of other external assets such as compiled shaders and texture maps, to produce a final rendered image.  The resulting output can take many different forms, depending on the configuration specified in the RIB stream. The most common scenario is that the RIB stream will specify a number of output images that are written to disk for later processing. Another possibility is that the image is rendered into an interactive framebuffer, the supplied one is called *piqsl*, see :ref:`piqsl`.

RIB streams can be in plain 7-bit ASCII or binary encoded using an encoding scheme described as part of the |RenderMan| standard.  Either format can be zipped for further compression.  |Aqsis| includes a tool that can be used to encode RIB files in binary format, and apply compression, see :ref:`miqser` for more information.

By default aqsis will accept the name of the RIB file to be processed as the final argument on the command line, such as::

	aqsis myscene.rib

Additionally, aqsis can be passed a RIB stream via the standard input stream, ''stdin'', for example by piping in the output from another application::

	mygenerator.exe | aqsis

See the documentation for your chosen command shell for more information about pipes and piping data to applications. Most other options to aqsis, such as the output file name and its format, are normally specified in the RIB stream. The |RenderMan| interface covers both a 3D scene description format, a rich set of configurable settings for the renderer and some command-like constructs to create supporting files like textures and shadow maps.

More introductory information on using aqsis itself can be found in the [[guide:first_steps|first steps tutorial]].


.. index:: aqsis; command line options

Options
-------

Calling aqsis with the --help option will list all the avaialable command-line options. 

Usage: aqsis [options] [RIB file...]

  -h, -help					Print this help and exit
  -version                	Print version information and exit
  -pause                  	Wait for a keypress on completion
  -progress               	Print progress information
  -Progress               	Print PRMan-compatible progress information (ignores -progressformat)
  --progressformat=string  	Printf-style format string for -progress
  --endofframe=integer     	Equivalent to "endofframe" RIB option
  -nostandard             	Do not declare standard RenderMan parameters
  -v, --verbose=V         	Set log output level
						  	0 = errors
						  	1 = warnings (default)
						  	2 = information
						  	3 = debug
  -echoapi               	Echo all RI API calls to the log output (experimental)
  -z, --priority=integer  	Control the priority class of aqsis.
                         	0 = idle
                         	1 = normal(default)
                         	2 = high
                         	3 = RT
  --type=string           	Specify a display device type to use
  --addtype=string        	Specify a display device type to add
  --mode=string           	Specify a display device mode to use
  -d, -fb                	Same as --type="framebuffer" --mode="rgb"
  --crop <x1 x2 y1 y2>   	Specify a crop window, values are in screen space.
  -nc, --nocolor          	Disable colored output
  -beep                  	Beep on completion of all ribs
  --res <x y>             	Specify the resolution of the render.
  --option=string         	A valid RIB Option string, can be specified multiple times.
  --shaders=string        	Override the default shader searchpath(s)
  --archives=string       	Override the default archive searchpath(s)
  --textures=string       	Override the default texture searchpath(s)
  --displays=string       	Override the default display searchpath(s)
  --procedurals=string    	Override the default procedural searchpath(s)

All options can either begin with a single dash or two dashes and can appear anywhere on the command line. Most of the options are self explanatory, or adequately documented in the help output above, some require a little more explanation.

PRMan Style Progress
	Using this option will force aqsis to output progress information in a format that is compatible with Pixar's PRMan. This can be useful when using |Aqsis| in place of PRMan in an existing pipeline that makes assumptions about the progress output. 

Progress Format
	The progress format string is similar to the *C* printf() function and has default value ``Frame (%f) %p%% complete [ %s secs / %S left ]``\.  Valid format characters are listed in the following table:

	+--------+-----------------------------------------------+
	| Format | Character Value                               |
	+========+===============================================+
	|  %f    | Frame number                                  |
	+--------+-----------------------------------------------+
	|  %h    | Rendering time in hours                       |
	+--------+-----------------------------------------------+
	|  %H    | Estimated number of hours left                |
	+--------+-----------------------------------------------+
	|  %m    | Rendering time in minutes                     |
	+--------+-----------------------------------------------+
	|  %M    | Estimated number of minutes left              |
	+--------+-----------------------------------------------+
	|  %p    | Percentage  value                             |
	+--------+-----------------------------------------------+
	|  %s    | Rendering time in seconds                     |
	+--------+-----------------------------------------------+
	|  %S    | Estimated number of seconds left              |
	+--------+-----------------------------------------------+
	|  %t    | Rendering time in hours:mins:secs             |
	+--------+-----------------------------------------------+
	|  %T    | Estimated time left in hours:mins:secs        |
	+--------+-----------------------------------------------+
	|  %%    | A single % sign                               |
	+--------+-----------------------------------------------+


.. index:: aqsis; verbosity, verbosity, verbose

Verbosity
	The verbosity level that determines how much text output Aqsis generates while it is running. Possible values are:

	+--------+--------------------------------+
	| Level  | Verbosity                      |
	+========+================================+
	|  0     | Only display errors            |
	+--------+--------------------------------+
	|  1     | Display warnings (default)     |
	+--------+--------------------------------+
	|  2     | Display informational messages |
	+--------+--------------------------------+
	|  3     | Display debug information      |
	+--------+--------------------------------+

Display Type & Mode
	Using these options it is possible to override the RiDisplay setting in the RIB file. You can either replace **all** RiDisplay requests with -type, or add an extra display output with -addtype. The argument to these two options specifies the display type, such as "file", "framebuffer" etc., the complete list of available displays depends on your configuration. The -mode options specifies the mode to use for the display, i.e. "rgba", "Cs" etc.

	This options is somewhat limited, in that it is only possible to add or replace displays with a single new one, not add multiple new displays. However, it is possible to achieve this using the more flexible -option command-line switch.

Crop
	Define a crop window. Only the portion of the image inside the specified region will be rendered. The coordinates are in screen space, so a value of 0.0 is at the top resp. left and a value of 1.0 is at the right resp. bottom, irrespective of the actual resolution. Using this option is equivalent to the RIB command ''CropWindow x1 x2 y1 y2''.

No Color
	By default, Aqsis produces color coded output so that you can easily distinguish between errors, warnings and info messages. If this doesn't play well with your terminal you can disable the color encoding using this option.

Options
	This option can be used to inject RIB commands into the stream just before WorldBegin. The string must be a complete RIB command that is valid in the option block where the global options for a frame are specified. For example, you could set a new display device using ''-option="Display \"myname.tif\" \"file\" \"rgba\""'' which is more flexible than the ''-type'' and ''-mode'' options because it also allows you to set a new output file name. The option can be used multiple times to issue several RIB commands.


.. index:: aqsis; configuration 

Configuration
-------------

Aqsis is configured entirely though the options mechanism, everything that controls the behaviour of Aqsis can be set by setting options in a RIB file. There three ways to control the operation of Aqsis, in order of decreasing priority, the command line, user and local configuration files, and the RIB stream itself. 

Command Line
^^^^^^^^^^^^

The command line options described above provide various ways of setting some common options. For instance the searchpath options directly modify the "searchpath" options. Also, the --option command line flag allows more flexible control over all of the available options (see options_ for more information about the available options).

User and Local Configuration Files
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Aqsis will look for a file named ".aqsisrc" or "_aqsisrc" (_aqsisrc supported by version 1.6 and above) first in the users home directory as defined by the "HOME" envrionment variable, and then in the current folder from which aqsis was launched. This file contains any valid RIB requests, but is generally only used to set options.

RIB Stream
^^^^^^^^^^

Finally, options can be set as normal in the RIB stream that aqsis is processing.

Order of Configuration
^^^^^^^^^^^^^^^^^^^^^^

Aqsis processes the various configuration options in the following manner.

* RiBegin

	* Process $HOME/.aqsisrc or $HOME/_aqsisrc ((_aqsisrc supported by version 1.6 and above))
	* Process ./.aqsisrc or ./_aqsisrc ((_aqsisrc supported by version 1.6 and above))

* Process searchpath options specified on the command line
* Process options in the RIB stream
* RiWorldBegin

	* Process options specified via the --option command line flag

Each lower processing step will override settings already set further up the list.

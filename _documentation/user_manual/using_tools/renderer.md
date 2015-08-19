---
layout: documentation
title: "Renderer: aqsis"
category: using_tools
order: 10
---


Overview
--------

The aqsis executable is a command-line renderer which accepts RenderMan(tm)
scene descriptions in the form of sequences of requests, known as [the
RenderMan (tm)
Interface](https://renderman.pixar.com/products/rispec/index.htm).  These
requests are passed to the aqsis executable as a character stream known as the
*RenderMan Interface Bytestream* (RIB).  When launched, aqsis processes the
requests in the passed RIB stream, which in turn can force processing of other
external assets such as compiled shaders and texture maps, to produce a final
rendered image.  The output image data is presented via the
[[doc:dev:display|standard Dspy interface]] to the chosen [[doc:display|display
device]].

RIB streams can be in plain ASCII or using a custom binary encoding described
as part of the RenderMan(tm) standard.  Either format can be zipped for further
compression.  See [[doc:miqser]] for more information about producing binary
encoded RIB files.

By default aqsis will take the RIB file to be processed as the final argument
on the command line, such as

    aqsis myscene.rib

Additionally, aqsis can be passed a RIB stream via ''stdin'', for example by
piping in the output from another application:

    mygenerator.exe | aqsis

Most other options to aqsis, such as the output file name and its format, are
normally specified in the input file. The RenderMan Interface covers both a 3D
scene description format, a rich set of configurable settings for the renderer
and some command-like constructs to create supporting files like textures and
shadow maps.

More introductory information on using aqsis itself can be found in the
[[guide:first_steps|first steps tutorial]].



Options
-------

A short help text listing all options can be displayed using the ''aqsis
-help'' option. All options can either begin with a single dash or two dashes
and can appear anywhere on the command line.

    Usage: aqsis [options] [RIB file...]
      -h, -help              Print this help and exit
      -version               Print version information and exit
      -pause                 Wait for a keypress on completion
      -progress              Print progress information
      -Progress              Print PRMan-compatible progress information (ignores -progressformat)
      -progressformat=string printf-style format string for -progress
      -endofframe=integer    Equivalent to "endofframe" RIB option
      -nostandard            Do not declare standard RenderMan parameters
      -v, -verbose=integer   Set log output level
                             0 = errors
                             1 = warnings (default)
                             2 = information
                             3 = debug
      -echoapi               Echo all RI API calls to the log output (experimental)
      -z, -priority=integer  Control the priority class of aqsis.
                             0 = idle
                             1 = normal(default)
                             2 = high
                             3 = RT
      -type=string           Specify a display device type to use
      -addtype=string        Specify a display device type to add
      -mode=string           Specify a display device mode to use
      -d, -fb                Same as --type="framebuffer" --mode="rgb"
      -crop x1 x2 y1 y2      Specify a crop window, values are in screen space.
      -frames f1 f2          Specify a starting/ending frame to render (inclusive).
      -frameslist=string     Specify a range of frames to render, ',' separated with '-' to indicate ranges.
      -nc, -nocolor          Disable colored output
      -beep                  Beep on completion of all ribs
      -res x y               Specify the resolution of the render.
      -option=string         A valid RIB Option string, can be specified multiple times.
      -shaders=string        Override the default shader searchpath(s)
      -archives=string       Override the default archive searchpath(s)
      -textures=string       Override the default texture searchpath(s)
      -displays=string       Override the default display searchpath(s)
      -procedurals=string    Override the default procedural searchpath(s)

-h / -help
  : Display the above help text.

-version
  : Display the version of the renderer and exit.

-progress
  : Print progress information to the console during rendering.

-Progress
  : Print progress information the same way than PRMan can print it (this can be
    used so that render farm software such as Pixar's Alfred will be able to scan
    progress information and display it in their GUI). This option ignores the
    format that is specified in the -progressformat option.

-progressformat=string
  : Defines the format of the progress string. The string is a format string
    similar to the C printf() function and has default value `"Frame (%f)
    %p%% complete [ %s secs / %S left ]"`.  Valid format characters are
    listed in Table 1:

-endofframe=integer
  : Determines if and what statistics information is printed after rendering a
    frame. The integer can be a value between 0 (default) and 3. The higher the
    value the more information about the render will be printed. Using this option
    is equivalent to the following RIB statement:
    `Option "statistics" "endofframe" [integer]`
    The command line option takes precedence over the value in RIB stream.

-nostandard
  : This option prevents all variables from being declared automatically at
    startup. You will have to declare every variable either using RiDeclare() or an
    inline declaration.

-v / -verbose=integer
  : The verbosity level that determines how much text output Aqsis generates
    while it is running. See Table 2 below for settings.

-echoapi
  : When this option is specified all RenderMan API calls will be echoed to the
    console.

-z / -priority=integer
  : Sets the process priority of the renderer. The default value is 1 which is
    the normal default process priority. You can reduce the priority by setting the
    value to 0 or increase the priority by setting the value to 2 or even 3 for the
    highest priority.

-type=string
  : Set an alternative display device that should be used instead of the one
    specified in the RIB. You may pass any device name that may also appear in a
    "Display" call.

-addtype=string
  : Similar to ''-type'' but instead of replacing the devices set in the RIB
    this option adds the specified device.

-mode=string
  : This option can be used in conjunction with ''-type'' or ''-addtype'' to
    specify the display mode string.

-d / -fb
  : This is a shortcut for ''-type "framebuffer" -mode "rgb"'', i.e. it
    replaces the output devices from the RIB with the framebuffer device in rgb
    mode.

-crop x1 x2 y1 y2
  : Define a crop window. Only the portion of the image inside the specified
    region will be rendered. The coordinates are in screen space, so a value of 0.0
    is at the top resp. left and a value of 1.0 is at the right resp. bottom,
    irrespective of the actual resolution. Using this option is equivalent to the
    RIB command ''CropWindow x1 x2 y1 y2''.

-frames f1 f2
  : If the RIB contains several frames, using this option will instruct Aqsis
    to only render the interval from f1 to f2.

-frameslist=string
  : Using this option you can specify the frames to render explicitly as a
    comma-separated list of either individual frames or frame ranges. Example:
    `-frameslist=1-5,20-25,30,40`.

-nc / -nocolor
  : By default, Aqsis produces color coded output so that you can easily
    distinguish between errors, warnings and info messages. If this doesn't play
    well with your terminal you can disable the color encoding using this option.

-beep
  : Beep on completion of all RIBs. What exactly this "beep" is (or if it beeps
    at all) depends on your system.

-res x y
  : Set the output image pixel resolution regardless of what is specified in
    the RIB.

-option=string
  : This option can be used to inject RIB commands into the stream just before
    WorldBegin. The string must be a complete RIB command that is valid in the
    option block where the global options for a frame are specified. For example,
    you could set a new display device using ''-option="Display \"myname.tif\"
    \"file\" \"rgba\""'' which is more flexible than the ''-type'' and ''-mode''
    options because it also allows you to set a new output file name. The option
    can be used multiple times to issue several RIB commands.

-syslog
  : *(Posix only)*
  : Log messages to the operating system log rather than the standard output streams.

-mpdump
  : *(only when enabled at compile time)*
  : When this option is specified Aqsis will write all micro polygons that are
    generated during rendering into the file "mpdump.mp". This is mainly supposed
    to be a debugging tool. The dump file can then be processed using a special
    utility that is part of the Aqsis source code (in `tools/mpdump`). Warning:
    Only use this option in conjunction with an appropriate crop window to select
    the region of interest, otherwise the dump file might be huge and impossible to
    handle by the analyse tool.

-shaders=string
  : Set shader search paths. This is equivalent to the RIB call `Option
    "searchpath" "shader" [string]`.

-archives=string
  : Set archive search paths. This is equivalent to the RIB call `Option
    "searchpath" "archive" [string]`.

-textures=string
  : Set texture search paths. This is equivalent to the RIB call `Option
    "searchpath" "texture" [string]`.

-displays=string
  : Set display search paths. This is equivalent to the RIB call `Option
    "searchpath" "display" [string]`.

-procedurals=string
  : Set procedural search paths. This is equivalent to the RIB call `Option
    "searchpath" "procedural" [string]`.
 

### Tables


|---
| Format character | Value 
|:-|:-
|  %f  |Frame number |
|  %h  |Rendering time in hours |
|  %H  |Estimated number of hours left |
|  %m  |Rendering time in minutes |
|  %M  |Estimated number of minutes left |
|  %p  |Percentage  value|
|  %s  |Rendering time in seconds |
|  %S  |Estimated number of seconds left  |
|  %t  |Rendering time in hours:mins:secs |
|  %T  |Estimated time left in hours:mins:secs  |
|  %%  |A single % sign |
|---

Table 1


|-------+--------------------------------|
| Level | Verbosity                      |
|-------|:-------------------------------|
|  0    | Only display errors            |
|  1    | Display warnings (default)     |
|  2    | Display informational messages |
|  3    | Display debug information      |
|-------+--------------------------------|

Table 2


Information regarding the usage of configuration files and environment
variables can be found [[doc:install#environment|here]].


Configuration
-------------

Aqsis is configured entirely though the [[doc:options|options]] mechanism,
everything that controls the behaviour of Aqsis can be set by setting options
in a RIB file. There three ways to control the operation of Aqsis, in order of
decreasing priority, the command line, user and local configuration files, and
the RIB stream itself. 

### Command Line

The command line options described above provide various ways of setting some
common options. For instance the searchpath options directly modify the
[[doc:options#searchpath_options|"searchpath"]] options. Also, the --option
command line flag allows more flexible control over all of the available
options (see [[doc:options#options|options]] for more information about the
available options).

### User and Local Configuration Files

Aqsis will look for a file named ".aqsisrc" or "_aqsisrc" ((_aqsisrc supported
by version 1.6 and above)) first in the users home directory as defined by the
"HOME" envrionment variable, and then in the current folder from which aqsis
was launched. This file contains any valid RIB requests, but is generally only
used to set options.

### RIB Stream

Finally, options can be set as normal in the RIB stream that aqsis is
processing.

### Order of Configuration

Aqsis processes the various configuration options in the following manner.

  * RiBegin
    * Process $HOME/.aqsisrc or $HOME/_aqsisrc ((_aqsisrc supported by version
      1.6 and above))
    * Process ./.aqsisrc or ./_aqsisrc ((_aqsisrc supported by version 1.6 and
      above))
  * Process searchpath options specified on the command line
  * Process options in the RIB stream
  * RiWorldBegin
    * Process options specified via the --option command line flag

Each lower processing step will override settings already set further up the list.


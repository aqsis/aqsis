---
layout: documentation
title: "RIB Processor: miqser"
category: using_tools
order: 50
---


Overview
--------

**miqser** ("mixer") is a tool for processing RIB streams. Its primary purpose
is to alter the encoding of a RIB stream, it can convert ASCII encoded RIB to
binary and vice versa, as well as applying/removing gzip compression. In
addition, it can alter the indentation of an ASCII RIB stream, to make it more
readable - a useful tool for tracking down problems with RIB files.

Miqser works by reading in a RIB file using the Aqsis RIB parsing library (rib2
& rib2ri), and then sending the results to the Aqsis RIB client library
(ri2rib) for output. The client library is configured to control the output
using the [[miqser#options|command line arguments]] of miqser.

The most common use for miqser is to convert a RIB stream to/from binary
encoding. To do so, use the following command line to convert `myrib.rib`
from ASCII to binary and save it as `myrib.bin.rib`:

    miqser -binary -o myrib.bin.rib myrib.rib

Alternatively, convert `myrib.bin.rib` from binary to ASCII and save it as `myrib.rib` by using:

    miqser -o myrib.rib myrib.bin.rib

Options
-------

A short help text listing all options can be displayed using the miqser -help
option. All options can either begin with a single dash or two dashes and can
appear anywhere on the command line. 

    Usage: miqser [options] [RIB file...]
      -h, -help              Print this help and exit
      -version               Print version information and exit
      -pause                 Wait for a keypress on completion
      -o, -output=string     Set the output filename, default to <stdout>
      -nostandard            Do not declare standard RenderMan parameters
      -outputstandard        Print the standard declarations to the resulting RIB
      -v, -verbose=integer   Set log output level
                             0 = errors
                             1 = warnings (default)
                             2 = information
                             3 = debug
      -i, -indentation=integer
                             Set output indentation type
                             0 = none (default)
                             1 = space
                             2 = tab
      -l, -indentlevel=integer
                             Set the indetation amount
      -compression=integer   Set output compression type
                             0 = none (default)
                             1 = gzip
      -b, -binary            Output a binary encoded RIB file
      -frames f1 f2          Specify a starting/ending frame to render (inclusive).
      -frameslist=string     Specify a range of frames to render, ',' separated with '-' to indicate ranges.
      -nc, -nocolor          Disable colored output
      -syslog                Log messages to syslog
      -archives=string       Override the default archive searchpath(s)
      -decodeonly            Decode a binary rib into text, *without* validating or formatting the result.  (Debug use only)


-h / -help
  : Display the above help text.

-version
  : Display the version of the renderer and exit.

-pause
  : If this option is specified, miqser will wait on completion for you to
    press 'enter' before exiting.

-o / -output=string
  : The name of the file that the resulting RIB stream will be written to.

-nostandard
  : This option prevents all variables from being declared automatically at
    startup. You will have to declare every variable either using RiDeclare() or an
    inline declaration.

-outputstandard
  : Include the standard declarations in the output stream.

-v / -verbose=integer
  : The verbosity level that determines how much text output Aqsis generates
    while it is running. Possible values are:

-i / -indentation=integer
  : When miqser is used to output ASCII RIB, this controls how miqser indents
    logical blocks of RIB. For example, the requests between a RiTransformBegin and
    RiTransformEnd pair would be indented one further level. Possible values are:

-l / -indentlevel=integer
  : Used in combination with -i, this value defines how many characters of the
    chosen type will be used per indentation level.

-compression=integer
  : Miqser is able to apply compression to the output stream, this value
    defines the type of compression used. Possible values are:

  * **none** - Apply no compression.
  * **gzip** - Apply [Gzip](http://en.wikipedia.org/wiki/Gzip) compression, a
    convenient option supported by most RenderMan compatible renderers.

-b / -binary
  : Use the standard binary RIB encoding when outputting the RIB stream. This
    encoding is more compact than teh standard ASCII encoding, however, it is not
    human readable. This option can be used in conjunction with -compression to
    produce a binary encoded and Gzip compressed stream, which is often the most
    space efficient encoding option available.

-frames f1 f2
  : If the RIB contains several frames, using this option will instruct Aqsis
    to only render the interval from f1 to f2.

-frameslist=string
  : Using this option you can specify the frames to render explicitly as a
    comma-separated list of either individual frames or frame ranges. Example:
    `-frameslist=1-5,20-25,30,40`.

-nc / -nocolor
  : By default, miqser produces color coded output so that you can easily
    distinguish between errors, warnings and info messages. If this doesn't play
    well with your terminal you can disable the color encoding using this option.

-syslog
  : _(Posix only)_
  : Log messages to the operating system log rather than the standard output
    streams.

-archives=string
  : Set archive search paths. This is equivalent to the RIB call `Option
    "searchpath" "archive" [string]`.

-decodeonly
  : When specified, miqser will simply decode a binary RIB to text, it will
    apply no validation or formatting to the output such as indentation. This
    option is intended primarily for debug purposes, to aid in validation of binary
    RIB streams.



### Tables

|-------+--------------------------------|
| Level | Verbosity                      |
|-------|:-------------------------------|
|  0    | Only display errors            |
|  1    | Display warnings (default)     |
|  2    | Display informational messages |
|  3    | Display debug information      |
|-------+--------------------------------|

Table 1


|-------+--------------------------------|
| Value | Indentation type               |
|-------|:-------------------------------|
|  0    | Don't indent                   |
|  1    | Indent using spaces            |
|  2    | Indent using tab characters    |
|-------+--------------------------------|

Table 2


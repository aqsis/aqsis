---
layout: documentation
title: Environment Variables 
category: installation
order: 50
---

In order to render an image, ''aqsis'' has to be able to find files such as
compiled shaders, texture images, display devices, procedurals, RIB archives,
etc. By default, Aqsis should already be set up so that it finds at least the
standard files that come with Aqsis itself. However, the situation may arise
where you want to customize the search paths.

### General Setup

When ''aqsis'' is launched, it reads a number of configuration files which are
just ordinary RIB files.  The order in which the config files are processed is
as follows:

- **System config file:** This file is either in the same directory as
  ''aqsis.exe'' (on Windows), or in ''$install_prefix/etc'' on POSIX. The file
  is named ''aqsisrc''. 
- **User config file:** `$HOME/.aqsisrc` (or `%HOME%\.aqsisrc` on Windows).
- **Project config file:** `.aqsisrc` in the directory from which aqsis was run.
- **Environment variables:** The various `AQSIS_*_PATH` environment variables
  are then inspected:
  * `AQSIS_ARCHIVE_PATH`: These paths are used for locating RIB archives.
  * `AQSIS_DISPLAY_PATH`: Display devices
  * `AQSIS_PROCEDURAL_PATH`: Procedurals (see `RiProcedural()`)
  * `AQSIS_SHADER_PATH`: Shaders and DSOs
  * `AQSIS_TEXTURE_PATH`: Texture images
- **Command line:** Finally, the paths provided as command line options
  (`-shaders`, `-textures`, etc.) are applied.

The configuration files may have any valid RIB commands, but by default simply
set a number of renderman interface [[doc:options|options]], including
[[doc:options#Searchpath_Options|search paths]] for important external
resources.  As an example, the default config file includes something like the
following when installed on a linux system:

    # Mapping from RI display names to the correspndoning dynamic library names.
    Option "display" "string file" ["libdisplay.so"]
    Option "display" "string framebuffer" ["libpiqsldisplay.so"]
    Option "display" "string zfile" ["libdisplay.so"]
    Option "display" "string zframebuffer" ["libdisplay.so"]
    Option "display" "string shadow" ["libdisplay.so"]
    Option "display" "string tiff" ["libdisplay.so"]
    # ...

    # Default search paths
    Option "defaultsearchpath" "string shader" ["/usr/local/share/aqsis/shaders"]
    Option "defaultsearchpath" "string display" ["/usr/local/lib/aqsis"]
    # ...

    # Search paths.  @ is expanded to the default search path.
    Option "searchpath" "string shader" ["@:."]
    Option "searchpath" "string display" ["@:."]
    # ...

It's essential that aqsis finds at least the default configuration file.  If
not, you won't be able to render any images since at least one display device
is needed to provide a destination for the generated pixel data.

### Other Options

In order for other systems to easily find/locate the Aqsis toolsuite
(binaries), like [[http://liquidmaya.sf.net|Liquid]], we have standardised on
using the `AQSISHOME` environment variable.

This variable should be enabled by default within our official
installers/packages, else it can be added manually (linking to the full path
of the install directory).

---
layout: documentation
title: Building From Source 
category: part_ii
order: 10
subcategory: building 
---


Aqsis uses [CMake](http://www.cmake.org) as the build configuration system. This
tool generates makefiles and workspaces specific to any of a number of different
compilers and IDEs depending on your platform. In order to reduce the
duplication of information, the set of CMake variables supported by the Aqsis
build will be listed and described in a separate section at the end of this
document. However, variables that are specifically applicable to a particular
platform, or have a particular relevance to that platform, will be described in
the platform section for clarity.

Windows
-------

This section describes the process of building Aqsis under the various supported
toolsets on a Windows platform. Aqsis can be built from any folder location,
however, there are various points in the documentation where we need to refer to
a specific point within the build hierarchy, so to make that easier, we will be
building using the following folder hierarchy. If you choose to build somewhere
different, you will need to adjust the locations accordingly during the process.


        C:\ 
        +---projects 
          +---aqsis
              +---aqsis                    <-- Aqsis source checkout
              |   +---cmake
              |   +---distribution
              |   +---doc
              |   \---...
              +---build                    <-- Aqsis will be built here...
              |   +---ALL_BUILD.dir        <-- ...these will be generated for you by CMake.
              |   +---CMakeFiles
              |   +---distribution
              |   \---...
              +---install                  <-- Aqsis will be installed here...
              |   +---bin                  <-- ...these will be generated during the build.
              |   +---examples
              |   +---include
              |   \---...
              \---dependencies             <-- The dependencies will be copied here...
                  +---bin                  <-- ...these will be created from the dependencies archive.
                  +---include
                  +---lib
                  \---...


Dependencies
------------

### Tools

  * [CMake](http://www.cmake.org) (v2.6.3+)
  * flex (a recent version like 2.5.34. Version 2.5.31 caused problems in the past, see FAQ 2.2)
  * bison (v1.35+)
  * xsltproc

### Libraries

  * [Boost](http://www.boost.org)

    The Boost installer will not work for Aqsis on Windows, it doesn't build
    with zlib, you'll need to build Boost yourself, this process is explained
    below.

  * libtiff (v3.7.1+)

  * libzlib (v1.1.4+)

  * [FLTK](http://www.fltk.org/) with fluid 

    FLTK v1.1.x - known to work with v1.1.9 which is currently the latest
    stable. Be aware that the "latest snapshot" - which is named similarly - is
    usually not tested with Aqsis and might not work.)

    This is only required in order to build the GUI tools that are part of the
    suite, if you only require the renderer and a non-GUI display, you can
    safely ignore this requirement.

  * [OpenEXR](http://www.openexr.com) - if you want to read and write OpenEXR
    HDR image files.

------

Obtaining the Dependencies
--------------------------

### MinGW

The dependencies for building with MinGW are available as a binary download.
They have been built using the currently available version(s) of gcc under
MinGW, if they do not match your version of gcc, you will need to build them
yourself. A build script and some patches have been provided to make this
simple, see below.

Get the version of the dependencies archive that matches your version of the gcc
compiler from <here>, and unzip the archive to C:\projects\aqsis, see the folder
layout at the start of this section.
           
|---
| Note
|---
| If you choose to unzip to a different folder, you can point the build system at the correct place using the AQSIS_DEPENDENCIES CMake parameter.
|---

<br/>


### Visual Studio


|---
| Note
|---
| This section describes the old dependencies solution, it is being replaced.
|---

<br/>


Many of these dependencies are available via our SVN repository, using either
the `win32libs` or `win64libs` directories. The libraries are available
compiled for use with [MinGW\(http://www.mingw.org) and [Visual Studio
Express](http://www.microsoft.com/express/vc) versions 2005 (VC8) and 2008
(VC9). 

These repositories do not include versions of Boost or CMake.

Alternatively, you can build the dependencies yourself. The individual projects
usually have sufficient documentation for anyone wishing to build them. Some of
them however, require specific build configurations for use with Aqsis,
described below.

#### Building Boost under Visual Studio

By default the zlib and bzip2 compression options are not compiled into the
Boost iostreams libraries, this applies also to the installer build available,
thus making the installer inappropriate for our use. Therefore it will be
necessary to build Boost for Windows manually, the following procedure outlines
the process, consult the
[Boost documentation](http://www.boost.org/doc/libs/1_42_0/more/getting_started/windows.html) for more detail.

  - Get the source to boost and unzip to a suitable folder, for example: `C:\projects\boost`
  - Change to that folder and cd into: `tools/jam`
  - Build `bjam` using the following command line: `build_dist.bat`
  - Add `C:\projects\boost\tools\jam\src\bin.ntx86` to the `PATH` environment variable
  - In the `win32libs` folder (obtained from SVN) `cd` into `zlib` and unzip the `zlib123.zip` file.
  - Change back to `C:\projects\boost`
  - Build with: `bjam threading=multi link=static -sZLIB_SOURCE=//<zlib unzip directory>// stage`

|---
| Note
|---
| **Win64 Users:** `bjam` will require an additional argument/parameter to build Boost: `bjam address-model=64 threading=multi link=static -sZLIB_SOURCE=//<zlib unzip directory>// stage`
|---

<br/>


This will build the libraries within `C:\projects\boost\stage\lib`, which is
the folder you'll need to pass to `cmake` for the location of the Boost
libraries. The build should now include the libboost_zlib library required
iostreams compression.


-------


Obtaining the Aqsis Source Code
-------------------------------

You can either download the stable sources as a zip from
[SourceForge](http://sourceforge.net/projects/aqsis/files) or checkout the
current (unstable) development sources from our Git repository.

There are some integration tools for Git under Windows that provide a more
familiar interface, however, we will describe the process using the command line
tools that the default install of Git provides, the instructions should be
easily translatable to any other Git based interface.

To obtain a clean checkout from the git repository, first install git and make
sure that you either provide the full path to the git binary (`git.exe` on
Windows) or that you add git to your `PATH`.  Then create a new directory
`aqsisroot`, and check out the sources into a `src` directory:

    C:\> mkdir C:\projects\aqsis
    C:\> cd C:\projects\aqsis
    C:\> git clone git://aqsis.git.sourceforge.net/gitroot/aqsis/aqsis aqsis

If everything works fine, you should see some output like

    Initialized empty Git repository in aqsisroot/src/.git/
    remote: Counting objects: 45437, done.
    remote: Compressing objects: 100% (13351/13351), done.
    ...

and the source will be downloaded into the `aqsis` directory.

|---
| Note
|---
| The Aqsis codebase uses UNIX-style line endings, you can automatically instruct [Git](http://git-scm.com) to convert these to Windows-style line endings using the following command on your repository: `git config core.autocrlf true`
|---

<br/>


### Build Setup

CMake works by reading the various configuration files included in the source,
and then generating configuration files for the chosen native build system on
your platform. For Windows, this usually means a version of Visual Studio, or
the gcc based MinGW toolchain.

The aqsis build system enforces an "out of source" build so that the source tree
doesn't get cluttered with object files.  This also has the benefit that you can
build multiple versions with different options from the same source tree.  Now
that you've got the source, make a `build` directory at the same level as the
source:

    C:\> cd \projects\aqsis
    C:\> mkdir build
    C:\> cd build

Run CMake(cmake-gui) from the start menu, in the **Where is the source code:**
field, browse to the location of the Aqsis source (C:\projects\aqsis\aqsis), and
in the **Where to build the binaries:** field, browse to the build folder you
just created (C:\projects\aqsis\build).

The area below those fields will, for the first build, be blank. Click
"Configure" and CMake will first ask you which generator to use, i.e. which
compiler and native build system you wish to target. You will be presented with
a list of possible generators that can be used under Windows, Aqsis has only at
this time been tested with Visual Studio and MinGW makefiles. Once you have
chosen a generator, CMake will attempt to first verify the tools, and then to
locate the necessary dependencies to build Aqsis. It will probably fail in that
second step, don't worry, we're going to resolve that next. 

You can tweak many of these settings to control the build, the majority of the
values are detailed in a separate section at the end of this documentation. We
will just detail the specific ones necessary to get a basic build working under
Windows here.

Dependencies
  : Presuming you're using the binary dependencies for MinGW, set the
    **AQSIS_DEPENDENCIES** entry to "C:\projects\aqsis\dependencies" will allow
    the CMake system to locate the dependencies it provides.

Installation
  : Point the **CMAKE_INSTALL_PREFIX** to C:\projects\aqsis\install.

Boost
  : Point **BOOST_ROOT** to C:\projects\aqsis\dependencies.

-----

**DEPRECATED**

Presuming you're using the provided win32libs package for the dependencies, much
of this can be resolved by changing the **AQSIS_WIN32LIBS** entry, under the
**AQSIS** group, to point to the win32libs folder.

|---
| Note
|---
| These instructions always refer to building on a 32bit Windows system, when building under 64bit Windows, use `win64libs` in place of `win32libs`, although the CMake configuration variable is still called **AQSIS_WIN32LIBS**, irrespective of architecture.
|---

<br/>


-----

If you are using custom builds of any of the dependencies, you'll need to point
the appropriate CMake variables at your custom installation of those libraries.
Most of these are grouped under the AQSIS group, some however are separately
grouped, such as Boost.

Once all the CMake variables are suitably configured, click "Generate" and CMake
will create the build files for your chosen tool. 

### Building 

#### Visual Studio 

The CMake generator for Visual Studio will create project and solution files in
the build folder. Load the solution `aqsis_all.sln` into Visual Studio. You
will be presented with a long list of project targets. Building the solution
should be sufficient to build all of the required parts of Aqsis. This creates
the necessary data structure in the build folder within a folder named after the
selected configuration, i.e. Debug, Release etc.

You will not be able to run Aqsis from this point as the build does not layout
the files in a suitable way. For example, the Debug version of the aqsis
executable will be in `build/tools/aqsis/Debug`, but the aqsis DLL it requires
will be in `build/libs/core/Debug`, and all the other DLL's will be in similar
separate folders. Therefore aqsis cannot find all of it's dependencies without a
huge effort in changing the system PATH environment. Fortunately, there is a
solution that is much more friendly.

The INSTALL target doesn't get built by default, but if you right click it, and
select build, it will. This step of the build process takes care of putting all
the parts of aqsis into a suitable format for running. 

Once the INSTALL target is built, all that is needed is to change into the
`C:\projects\aqsis\install\bin` folder and run aqsis.exe, simply typing...

    aqsis --help
  
...will confirm that the build has successfully completed.


#### MinGW/GCC

The CMake generator for MinGW will create a `Makefile` in the build folder.
Open a DOS prompt and run `mingw32-make`.

|---
| Note
|---
| Ensure that the C:\MinGW\bin folder is on your PATH
|---

<br/>


You will not be able to run Aqsis from this point as the build does not layout
the files in a suitable way. For example, the aqsis executable will be in
`build/tools/aqsis`, but the aqsis DLL it requires will be in
`build/libs/core`, and all the other DLL's will be in similar separate
folders. Therefore aqsis cannot find all of it's dependencies without a huge
effort in changing the system PATH environment. Fortunately, there is a solution
that is much more friendly.

From the DOS prompt, run `mingw32-make install`. This step of the build
process takes care of putting all the parts of aqsis into a suitable format for
use. 

Once the INSTALL target is built, all that's needed is to change into the
`C:\projects\aqsis\install\bin` folder and run aqsis.exe, simply typing...

    aqsis --help
  
...will confirm that the build has successfully completed.


Building the Dependencies
-------------------------

### MinGW

If you need to build from source, you will need the following MinGW/MSYS setup.

MinGW
  : Install the appropriate version of MinGW from the
    [MinGW](http://www.mingw.org) site.
  : Make sure you download the correct version of gcc for your requirements,
    usually this
  : will be the latest.

MSYS
  : Install the appropriate version of MSYS from the
    [MinGW](http://www.mingw.org) site.

Additional tools
  : Just download these and unarchive into your MSYS folder (usually C:\msys).

    * MSYS autoconf bin
    * MSYS automake bin
    * MSYS perl bin
    * MSYS crypt dll
    * MSYS libtool bin

The build system expects MinGW to be in C:\MinGW and MSYS to be in C:\msys, if
you have not chosen these locations to install, you'll need to modify the build
batch file.

Once you have this setup, download the build configuration from <here>, and
unzip to a suitable location, for the purposes of this example, we'll assume
C:\projects\aqsis\dependencies.

Obtain the appropriate versions of the dependency source archives, the build
system is only setup to support a single version of each library, so make sure
you have exactly the right version, and that the archive is appropriately named.
To make this easier, we have made the dependency source archives available to
download from <here>. They should be copied, not unzipped, to the dependencies
folder.

Run a DOS prompt, and cd into the C:\projects\aqsis\dependencies, then run the
build.bat script. You will be presented with a menu that looks like...

    = Menu =================================================

     Options:
     1  Build Boost             : 'Yes' [Yes,No]
     2  Build OpenEXR           : 'Yes' [Yes,No]
     3  Build libTiff           : 'Yes' [Yes,No]
     4  Build libPng            : 'Yes' [Yes,No]
     5  Build fltk              : 'Yes' [Yes,No]
     6  Build zlib              : 'Yes' [Yes,No]

     Execute:
     B  Build libraries
     C  Clear Screen

    Make a choice or hit ENTER to quit:

Typing any of the option numbers 1 through 6 followed by enter, will toggle the
building of that dependency. Note that some dependencies depend on others, if
you've chosen to build libTiff for example, zlib will be built automatically, as
it is required. Once you have chosen the set of dependencies to build, type 'b'
followed by enter and the system should build the dependencies in a folder
C:\projects\aqsis\dependencies\build\mingw. You will need to point
AQSIS_DEPENDENCIES at this 'mingw' folder.



import os.path
import glob
import sys
from modulefinder import AddPackagePath

from build_support import SelectBuildDir
from build_support import zipperFunction
from build_support import print_config
from build_support import AddSysPath
from build_support import Glob
from build_support import embedManifest
from build_support import getSCMRevision
Export('embedManifest')

import version
Export('version')

EnsurePythonVersion(2,3)

# Determine the target 
target_config_dir =  '#' + SelectBuildDir('platform')

# Temporary environment for use of configuration only, mainly to determine file locations.
# Will be replaced by the real environment when all options are set.
tempenv = Environment()

# Read in the platform specific options.
AddSysPath(tempenv.Dir(target_config_dir).abspath)

# Read in options from custom.py if it exists, or options.cache otherwise
if os.path.exists('custom.py'):
	print 'Using options from custom.py'
	opts = Options(os.path.abspath('custom.py'))
else:
	print 'Using options from options.cache'
	opts = Options(os.path.abspath('options.cache'))

# Setup the command line options.
opts.Add('tiff_include_path', 'Point to the tiff header files', '')
opts.Add('tiff_lib_path', 'Point to the tiff library files', '')
opts.Add('boost_include_path', 'Point to the boost header files', '')
opts.Add('jpeg_include_path', 'Point to the jpeg header files', '')
opts.Add('jpeg_lib_path', 'Point to the jpeg library files', '')
opts.Add('zlib_include_path', 'Point to the zlib header files', '')
opts.Add('zlib_lib_path', 'Point to the zlib library files', '')
opts.Add('fltk_include_path', 'Point to the fltk header files', '')
opts.Add('fltk_lib_path', 'Point to the fltk library files', '')
opts.Add('exr_include_path', 'Point to the OpenEXR header files', '')
opts.Add('exr_lib_path', 'Point to the OpenEXR library files', '')
opts.Add(BoolOption('no_fltk', 'Build without FLTK support', '0'))
opts.Add(BoolOption('no_exr', 'Build without OpenEXR support', '0'))
opts.Add(BoolOption('with_pdiff', 'Build and install the third-party pdiff utility', '0'))
opts.Add('cachedir', 'Examine a build cache dir for previously compiled files', '')
opts.Add(BoolOption('debug', 'Build with debug options enabled', '0'))
opts.Add(BoolOption('enable_mpdump', 'Build with micropolygon dumping mode enabled', '1'))

# This will hopefully import the target specific options
import Options
Options.PlatformOptions(opts,tempenv)

Export('opts')

# Create the default environment
# Set up the tools, compiler, etc.
import SCons.Util
def ENV_update(tgt_ENV, src_ENV):
    for K in src_ENV.keys():
        if K in tgt_ENV.keys() and K in [ 'PATH', 'LD_LIBRARY_PATH',
                                          'LIB', 'INCLUDE' ]:
            tgt_ENV[K]=SCons.Util.AppendPath(tgt_ENV[K], src_ENV[K])
        else:
            tgt_ENV[K]=src_ENV[K]

opts.Update(tempenv)

if tempenv.has_key('mingw') and tempenv['mingw']:
	env = Environment(options = opts, tools = ['mingw', 'lex', 'yacc', 'zip', 'tar'])
elif sys.platform == 'win32':
	env = Environment(options = opts, tools = ['default', 'lex', 'yacc', 'zip', 'tar'])
else:
	env = Environment(options = opts, tools = ['default','gcc','g++','lex', 'yacc', 'zip', 'tar'])

ENV_update(env['ENV'], os.environ)
env.Glob = Glob

# Add an option to control the root location of the 'install' target
defout = env.Dir('#/output')
target_dir =  env.Dir('#/build')
if env['debug']:
	defout = env.Dir('#/output_debug')
	target_dir =  env.Dir('#/build_debug')
opts.Add('install_prefix', 'Root folder under which to install Aqsis', defout)
opts.Add('build_prefix', 'Root folder under which to build Aqsis', target_dir)

# Make scons store build signatures in a single file _outside_ the install
# hierarchy --- no more .sconsign files...
env.SConsignFile()

if env['cachedir'] != '':
	# Examine cachedir for identical files built during another build
	CacheDir(env['cachedir'])

# add builders to zip/gtar files
zipBuilder = Builder(action=zipperFunction,
   source_factory=Dir,
   target_factory=File,
   multi=0)
env.Append(BUILDERS = {'Zipper':zipBuilder})

# Create the configure object here, as you can't do it once a call
# to SConscript has been processed.
conf = Configure(env)
Export('env opts conf')

# Setup the distribution stuff, this should be non-platform specific, the distribution
# archive should apply to all supported platforms.
env['ZIPDISTDIR'] = env.Dir('#/aqsis-%d.%d.%d%s' %(version.major, version.minor, version.build, version.type))
def Distribute(files, dir = None):
	if dir is None:
		dir = env.Dir('./').srcnode().path
	env.Install('$ZIPDISTDIR/' + dir, files)
env.Distribute = Distribute
env.Alias('dist', '$ZIPDISTDIR')
archive_name = 'aqsis-%d.%d.%d%s' %(version.major, version.minor, version.build, version.type)
zip_target = env.Zip('%s%s' %(archive_name, env['ZIPSUFFIX']), '$ZIPDISTDIR')
env.Alias('dist_zip', zip_target)
env.AppendUnique(TARFLAGS = ['-c', '-z'])
env.AppendUnique(TARSUFFIX = '.gz')
tar_target = env.Tar('%s%s' %(archive_name, env['TARSUFFIX']), '$ZIPDISTDIR')
env.Alias('dist_tar', tar_target)

opts.Update(env)

# Setup the output directories for binaries and libraries.
env.Replace(BINDIR = env.Dir('$install_prefix').abspath + os.sep + 'bin')
env.Replace(RENDERENGINEDIR = '$BINDIR')
env.Replace(DISPLAYSDIR = '$BINDIR')
env.Replace(PLUGINDIR = '$BINDIR')
env.Replace(LIBDIR = env.Dir('$install_prefix').abspath + os.sep + 'lib')
env.Replace(STATICLIBDIR = '$LIBDIR')
env.Replace(SHADERDIR = env.Dir('$install_prefix').abspath + os.sep + 'shaders')
env.Replace(SYSCONFDIR = env.Dir('$install_prefix').abspath + os.sep + 'bin')
env.Replace(INCLUDEDIR = env.Dir('$install_prefix').abspath + os.sep + 'include/aqsis')
env.Replace(CONTENTDIR = env.Dir('$install_prefix').abspath + os.sep + 'content')
env.Replace(SCRIPTSDIR = env.Dir('$install_prefix').abspath + os.sep + 'scripts')
# The following install directories may be modified by the platform specific config files.
env.Replace(INSTALL_DIRS = ['$BINDIR','$RENDERENGINEDIR', '$DISPLAYSDIR', '$PLUGINDIR',
				'$SHADERDIR', '$SYSCONFDIR', '$INCLUDEDIR', '$CONTENTDIR', '$SCRIPTSDIR'] )

# Setup a default version of the various platform specific functions that can be overridden in the 
# platform specific configurations below.
# PostInstallSharedLibrary is called right after the library has been installed
# to its target folder
# PostBuildSharedLibrary is called just after building
def PostInstallSharedLibrary(env, dir, source):
	pass
env.PostInstallSharedLibrary = PostInstallSharedLibrary
def PostBuildSharedLibrary(env, source):
	pass
env.PostBuildSharedLibrary = PostBuildSharedLibrary

# Read in the platform specific configuration.
# Allowing it to override the settings defined above.
SConscript(target_config_dir + os.sep + 'SConscript')

# Save the command line options to a cache file, allowing the user to just run scons without 
# command line options in future runs.
opts.Save('options.cache', env)

opts.Update(env)

target_dir = env.Dir('$build_prefix')


# Setup common environment settings to allow includes from the various local folders
env.AppendUnique(
	CPPPATH = [ os.path.join(target_dir.abspath, dir) for dir in
		[''] + Split('''
		aqsistypes
		renderer/render
		shadercompiler/shaderexecenv
		rib/rib2
		shadercompiler/shadervm
		rib/rib2ri
		argparse
		shadercompiler/slparse
		shadercompiler/codegenvm
		rib/api
		''')
	]
)

libincludes = [''] + Split('''
		$zlib_include_path
		$tiff_include_path
		$jpeg_include_path
		$boost_include_path
		$fltk_include_path
		$exr_include_path
	''')
#for a in libincludes:
#	env.AppendUnique(CPPPATH = [env.subst(a)] )
env.AppendUnique(CPPPATH = ['$boost_include_path'])
env.AppendUnique(CPPDEFINES=[('DEFAULT_PLUGIN_PATH', '\\"' + env.Dir('${PLUGINDIR}').abspath + '\\"')])

#
# Enable MP dumping mode if specified
#
env.AppendUnique(CPPDEFINES=[('ENABLE_MPDUMP', env.subst('${enable_mpdump}'))])

# Add paths to librarys generated in the build dir.
def prependBuildDir(subDirs):
	'''Prepend the build directory to each directory in a list, or to a string
	if the input is simply a string'''
	if isinstance(subDirs, str):
		return os.path.join(target_dir.abspath, subDirs)
	else:
		return [ os.path.join(target_dir.abspath, subDir) for subDir in subDirs ]
env.AppendUnique(LIBPATH = prependBuildDir( Split('''
	rib/rib2
	rib/rib2ri
	rib/ri2rib
	aqsistypes
	argparse
	renderer/ddmanager
	renderer/raytrace
	renderer/render
	shadercompiler/shaderexecenv
	shadercompiler/shadervm
	shadercompiler/slparse
	shadercompiler/codegenvm
	shadercompiler/slpp
	shadercompiler/slxargs
	texturing/plugins/common
	texturing/plugins/bake2tif
	texturing/plugins/jpg2tif
	texturing/plugins/bmp2tif
	texturing/plugins/exr2tif
	texturing/plugins/gif2tif
	texturing/plugins/pcx2tif
	texturing/plugins/ppm2tif
	texturing/plugins/tga2tif
	texturing/plugins/png2tif
	thirdparty/tinyxml
''' ) ) )

# Setup the include path to the tiff headers (should have been determined in the system specific sections above).
env.AppendUnique(LIBPATH = ['$tiff_lib_path', '$jpeg_lib_path', '$zlib_lib_path', '$fltk_lib_path', '$exr_lib_path'])

# Create the output for the command line options defined above and in the platform specific configuration.
Help(opts.GenerateHelpText(env))

# Check for the existence of the various dependencies
SConscript('build_check.py')

# Transfer any findings from the build_check back to the environment
env = conf.Finish()

# Prepare the NSIS installer tool
env.Tool('NSIS', toolpath=['./'])
env.Distribute('NSIS.py')

# Set the build directory for all sub-project build operations.
env.BuildDir(target_dir, '.')

# Load the sub-project SConscript files.
sub_sconsdirs_noret = prependBuildDir(Split('''
	rib/api
	aqsistypes
	argparse
	renderer/ddmanager
	renderer/raytrace
	rib/rib2
	rib/rib2ri
	shadercompiler/shaderexecenv
	shadercompiler/shadervm
	renderer/render
	shadercompiler/slparse
	shadercompiler/codegenvm
	shadercompiler/slpp
	shadercompiler/aqsl
	shadercompiler/slxargs
	shadercompiler/aqsltell
	rib/ri2rib
	rib/rib2stream
	rib/miqser
	texturing/teqser
	texturing/plugins
	shaders
	thirdparty/pdiff
	thirdparty/dbo_plane
	thirdparty/tinyxml
	tools
	content/ribs/scenes/vase
	content/ribs/features/layeredshaders
	content/shaders/light
	content/shaders/displacement
'''))

env.SConscript( dirs = sub_sconsdirs_noret )

# The following subdirectories have SConscript return values.
sub_sconsdirs_withret = prependBuildDir(Split('''
		renderer/aqsis
		displays/display
		displays/d_exr
		displays/d_sdcBMP
		displays/d_sdcWin32
		displays/d_xpm
		displays/eqshibit
'''))
(	aqsis,
	display,
	exr,
	bmp,
	win32,
	xpm,
	eqshibitdisplay,
) = env.SConscript( dirs = prependBuildDir(sub_sconsdirs_withret) )

# needed (?) by macosx distribution (there should be a better way to achieve
# this than using a global? )
Export('aqsis')

env.SConscript( dirs = prependBuildDir(['distribution']) )

#
# Generate and install the 'aqsisrc' configuration file from the template '.aqsisrc.in'
#
def aqsis_rc_build(target, source, env):
	# Code to build "target" from "source"
	displaylib = os.path.basename(display[0].path)
	eqshibitdisplaylib = os.path.basename(eqshibitdisplay[0].path)
	xpmlib = os.path.basename(xpm[0].path)
	bmplib = os.path.basename(bmp[0].path)
	win32lib = ""
	if sys.platform == 'win32':
		win32lib = os.path.basename(win32[0].path)
	defines = {
		"displaylib": displaylib,
		"eqshibitdisplaylib": eqshibitdisplaylib,
		"xpmlib": xpmlib,
		"bmplib": bmplib,
		"win32lib": win32lib,
		"shaderpath": env.Dir('$SHADERDIR').abspath,
		"displaypath": env.Dir('$DISPLAYSDIR').abspath,
		"exrlib": ""
	    }
	if not env['no_exr']:
		exrlib = os.path.basename(exr[0].path)
		defines["exrlib"] = exrlib

	print_config("Building aqsisrc with the following settings:", defines.items())

	for a_target, a_source in zip(target, source):
		aqsisrc_out = file(str(a_target), "w")
		aqsisrc_in = file(str(a_source), "r")
		aqsisrc_out.write(aqsisrc_in.read() % defines)
		aqsisrc_out.close()
		aqsisrc_in.close()

aqsisrc = env.Command(prependBuildDir('aqsisrc'), 'aqsisrc.in', aqsis_rc_build)
env.Install('$SYSCONFDIR', aqsisrc)
env.Depends(aqsisrc, display)

#
# Generate and install the version.h file from the template 'version.h.in'
#
def version_h_build(target, source, env):
	# Code to build "target" from "source"
	revision = getSCMRevision()
	defines = {
		"major": version.major,
		"minor": version.minor,
		"build": version.build,
		"type": version.type,
		"revision": revision.encode('ascii', 'replace')
	    }

	print_config("Building version.h with the following settings:", defines.items())

	for a_target, a_source in zip(target, source):
		aqsisrc_out = file(str(a_target), "w")
		aqsisrc_in = file(str(a_source), "r")
		aqsisrc_out.write(aqsisrc_in.read() % defines)
		aqsisrc_out.close()
		aqsisrc_in.close()

version_h = env.Command(prependBuildDir('version.h'), 'version.h.in', version_h_build)
env.Distribute('version.py')
env.Distribute('version.h.in')

# Note that the distribution directory is not included in the build alias.
env.Alias('build', sub_sconsdirs_noret + sub_sconsdirs_withret + [aqsisrc])

env.Alias(['install','release'], env['INSTALL_DIRS'])
Default('release')

# Define any files that need to be included in a source distribution.
main_distfiles = Split("""
	SConstruct
	AUTHORS
	build_check.py
	build_support.py
	COPYING
	Doxyfile
	INSTALL
	README
	ReleaseNotes
	aqsisrc.in""")

env.Distribute(main_distfiles)

# Distribute the platform build configurations.
platforms = glob.glob('platform/*/SConscript')
for platform in platforms:
	path, name = os.path.split(platform)
	env.Distribute(platform, path)
options = glob.glob('platform/*/*.py')
for option in options:
	path, name = os.path.split(option)
	env.Distribute(option, path)

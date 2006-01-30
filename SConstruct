import os
import os.path
import glob
import sys
from modulefinder import AddPackagePath

from build_support import SelectBuildDir
from build_support import print_config
from build_support import AddSysPath
from build_support import Glob

# Setup the common command line options.
opts = Options([os.path.abspath('options.cache'), os.path.abspath('custom.py')])
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
opts.Add(BoolOption('no_exr', 'Build without OpenEXR support', '0'))
opts.Add(BoolOption('debug', 'Build with debug options enabled', '0'))


# Temporary environment for use of configuration only, mainly to determine file locations.
# Will be replaced by the real environment when all options are set.
tempenv = Environment(options=opts)

# Add an option to control the root location of the 'install' target
defout = tempenv.Dir('#/output')
target_dir =  tempenv.Dir('#/build')
if tempenv['debug']:
	defout = tempenv.Dir('#/output_debug')
	target_dir =  tempenv.Dir('#/build_debug')
opts.Add('install_prefix', 'Root folder under which to install Aqsis', defout)
opts.Add('build_prefix', 'Root folder under which to build Aqsis', target_dir)

# Determine the target 
target_config_dir =  '#' + SelectBuildDir('platform')
Export('opts')

# Read in the platform specific options.
AddSysPath(tempenv.Dir(target_config_dir).abspath)

# This will hopefully import the target specific options
import Options
Options.PlatformOptions(opts,tempenv)

# Create the default environment
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
else:
	env = Environment(options = opts, tools = ['default', 'lex', 'yacc', 'zip', 'tar'])

ENV_update(env['ENV'], os.environ)
env.Glob = Glob

# Create the configure object here, as you can't do it once a call
# to SConscript has been processed.
conf = Configure(env)
Export('env opts conf')

# Setup the distribution stuff, this should be non-platform specific, the distribution
# archive should apply to all supported platforms.
env['ZIPDISTDIR'] = '#/dist'
def Distribute(dir, files):
        env.Install('$ZIPDISTDIR/%s' % dir, files)
env.Distribute = Distribute
env.Alias('dist', '$ZIPDISTDIR')
zip_target = env.Zip('aqsis', '$ZIPDISTDIR')
env.Alias('dist-zip', zip_target)
env.AppendUnique(TARFLAGS = '-c -z')
env.AppendUnique(TARSUFFIX = '.tgz')
tar_target = env.Tar('aqsis', '$ZIPDISTDIR')
env.Alias('dist-tar', tar_target)


# Read in the platform specific configuration.
# Allowing it to override the settings defined above.
SConscript(target_config_dir + os.sep + 'SConscript')


# Save the command line options to a cache file, allowing the user to just run scons without 
# command line options in future runs.
opts.Save('options.cache', env)

opts.Update(env)

# Setup the output directories for binaries and libraries.
env.Replace(BINDIR = env.Dir('$install_prefix').abspath + os.sep + 'bin')
env.Replace(LIBDIR = env.Dir('$install_prefix').abspath + os.sep + 'lib')
env.Replace(SHADERDIR = env.Dir('$install_prefix').abspath + os.sep + 'shaders')
env.Replace(SYSCONFDIR = env.Dir('$install_prefix').abspath + os.sep + 'bin')
env.Replace(INCLUDEDIR = env.Dir('$install_prefix').abspath + os.sep + 'include/aqsis')

target_dir = env.Dir('$build_prefix')


# Setup common environment settings to allow includes from the various local folders
env.AppendUnique(CPPPATH = [target_dir.abspath, target_dir.abspath + '/aqsistypes', target_dir.abspath + '/renderer/render', target_dir.abspath + '/shadercompiler/shaderexecenv', target_dir.abspath + '/rib/rib2', target_dir.abspath + '/shadercompiler/shadervm', target_dir.abspath + '/rib/rib2ri', target_dir.abspath + '/argparse', target_dir.abspath + '/shadercompiler/slparse', target_dir.abspath + '/shadercompiler/codegenvm', target_dir.abspath + '/rib/api', '$zlib_include_path', '$tiff_include_path', '$jpeg_include_path', '$boost_include_path', '$fltk_include_path', '$exr_include_path'])
env.AppendUnique(CPPDEFINES=[('DEFAULT_PLUGIN_PATH', '\\"' + env.Dir('${BINDIR}').abspath + '\\"')])
env.AppendUnique(CPPDEFINES=['SCONS_BUILD'])

# Setup the include path to the tiff headers (should have been determined in the system specific sections above).
env.AppendUnique(LIBPATH = ['$LIBDIR', '$BINDIR', '$tiff_lib_path', '$jpeg_lib_path', '$zlib_lib_path', '$fltk_lib_path', '$exr_lib_path'])

# Create the output for the command line options defined above and in the platform specific configuration.
Help(opts.GenerateHelpText(env))
	
# Check for the existence of the various dependencies
SConscript('build_check.py')

# Transfer any findings from the build_check back to the environment
env = conf.Finish()


# Load the sub-project sconscript files.
SConscript('rib/api/SConscript', build_dir=target_dir.abspath + '/rib/api')
SConscript('aqsistypes/SConscript', build_dir=target_dir.abspath + '/aqsistypes')
SConscript('argparse/SConscript', build_dir=target_dir.abspath + '/argparse')
SConscript('renderer/ddmanager/SConscript', build_dir=target_dir.abspath + '/renderer/ddmanager')
SConscript('renderer/raytrace/SConscript', build_dir=target_dir.abspath + '/renderer/raytrace')
SConscript('rib/rib2/SConscript', build_dir=target_dir.abspath + '/rib/rib2')
SConscript('rib/rib2ri/SConscript', build_dir=target_dir.abspath + '/rib/rib2ri')
SConscript('shadercompiler/shaderexecenv/SConscript', build_dir=target_dir.abspath + '/shadercompiler/shaderexecenv')
SConscript('shadercompiler/shadervm/SConscript', build_dir=target_dir.abspath + '/shadercompiler/shadervm')
SConscript('renderer/render/SConscript', build_dir=target_dir.abspath + '/renderer/render')
SConscript('renderer/aqsis/SConscript', build_dir=target_dir.abspath + '/renderer/aqsis')
SConscript('shadercompiler/slparse/SConscript', build_dir=target_dir.abspath + '/shadercompiler/slparse')
SConscript('shadercompiler/codegenvm/SConscript', build_dir=target_dir.abspath + '/shadercompiler/codegenvm')
SConscript('shadercompiler/slpp/SConscript', build_dir=target_dir.abspath + '/shadercompiler/slpp')
SConscript('shadercompiler/aqsl/SConscript', build_dir=target_dir.abspath + '/shadercompiler/aqsl')
SConscript('shadercompiler/slxargs/SConscript', build_dir=target_dir.abspath + '/shadercompiler/slxargs')
SConscript('shadercompiler/aqsltell/SConscript', build_dir=target_dir.abspath + '/shadercompiler/aqsltell')
display = SConscript('displays/display/SConscript', build_dir=target_dir.abspath + '/displays/display')
xpm = SConscript('displays/d_xpm/SConscript', build_dir=target_dir.abspath + '/displays/d_xpm')
if not env['no_exr']:
	exr = SConscript('displays/d_exr/SConscript', build_dir=target_dir.abspath + '/displays/d_exr')
SConscript('rib/ri2rib/SConscript', build_dir=target_dir.abspath + '/rib/ri2rib')
SConscript('rib/rib2stream/SConscript', build_dir=target_dir.abspath + '/rib/rib2stream')
SConscript('texturing/teqser/SConscript', build_dir=target_dir.abspath + '/texturing/teqser')
SConscript('texturing/plugins/SConscript')
SConscript('shaders/SConscript', build_dir=target_dir.abspath + '/shaders')
SConscript('thirdparty/tinyxml/SConscript', build_dir=target_dir.abspath + '/thirdparty/tinyxml')

#
# Generate and install the 'aqsisrc' configuration file from the template '.aqsisrc.in'
#
def aqsis_rc_build(target, source, env):
	# Code to build "target" from "source"
	displaylib = os.path.basename(display[0].path)
	xpmlib = os.path.basename(xpm[0].path)
	defines = {
		"displaylib": displaylib,
		"xpmlib": xpmlib,
		"shaderpath": env.Dir('$SHADERDIR').abspath,
		"displaypath": env.Dir('$BINDIR').abspath,
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

aqsisrc = env.Command('aqsisrc', 'aqsisrc.in', aqsis_rc_build)
env.Install('$SYSCONFDIR', aqsisrc)
env.Depends(aqsisrc, display)

#
# Generate and install the version.h file from the template 'version.h.in'
#
import version
def version_h_build(target, source, env):
	# Code to build "target" from "source"
	defines = {
		"major": version.major,
		"minor": version.minor,
		"build": version.build,
	    }

	print_config("Building version.h with the following settings:", defines.items())

	for a_target, a_source in zip(target, source):
		aqsisrc_out = file(str(a_target), "w")
		aqsisrc_in = file(str(a_source), "r")
		aqsisrc_out.write(aqsisrc_in.read() % defines)
		aqsisrc_out.close()
		aqsisrc_in.close()

version_h = env.Command('version.h', 'version.h.in', version_h_build)
env.Install(target_dir.abspath, version_h)
env.Distribute('./', version_h)

env.Alias('release', ['$BINDIR','$LIBDIR', '$SHADERDIR','$SYSCONFDIR','$INCLUDEDIR'])
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
	aqsisrc.in""")

env.Distribute('', main_distfiles)

# Distribute the platform build configurations.
platforms = glob.glob('build/*/SConscript')
for platform in platforms:
	path, name = os.path.split(platform)
	env.Distribute(path, platform)

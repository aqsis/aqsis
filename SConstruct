import os
import os.path
from build_support import *

# This allows the developer to choose the version of msvs from the command line.
opts = Options(['options.cache', 'custom.py'])
opts.Add('tiff_include_path', 'Point to the tiff header files', '')
opts.Add('tiff_lib_path', 'Point to the tiff library files', '')
opts.Add('boost_include_path', 'Point to the boost header files', '')
opts.Add('jpeg_include_path', 'Point to the jpeg header files', '')
opts.Add('jpeg_lib_path', 'Point to the jpeg library files', '')
opts.Add('zlib_include_path', 'Point to the zlib header files', '')
opts.Add('zlib_lib_path', 'Point to the zlib library files', '')
opts.Add('fltk_include_path', 'Point to the fltk header files', '')
opts.Add('fltk_lib_path', 'Point to the fltk library files', '')

# Create the default environment
env = Environment(options = opts)
conf = Configure(env)
Export('env opts conf')

# Setup the distribution stuff, this should be non-platform specific, the distribution
# archive should apply to all supported platforms.
env['ZIPDISTDIR'] = '#/dist'
def Distribute(dir, files):
        env.Install('$ZIPDISTDIR/%s' % dir, files)
env.Distribute = Distribute

Environment.UseTargetOptions = UseTargetOptions

# Determine the target and load the appropriate configuration SConscript
target_dir =  '#' + SelectBuildDir('build')

# Setup the output directories for binaries and libraries.
env.Replace(BINDIR = target_dir + os.sep + 'bin')
env.Replace(LIBDIR = target_dir + os.sep + 'lib')
env.Replace(SHADERDIR = target_dir + os.sep + 'shaders')

# Read in any platform specific configuration.
SConscript(target_dir + os.sep + 'SConscript')

# Setup common environment settings to allow includes from the various local folders
env.AppendUnique(CPPPATH = ['#/libaqsistypes','#/render', '#/libshaderexecenv', '#/librib2', '#/libshadervm', '#/librib2ri', '#/libargparse', '#/libslparse', '#/libcodegenvm', '$tiff_include_path', '$jpeg_include_path', '$zlib_include_path', '$boost_include_path'])
env.AppendUnique(CPPDEFINES=[('DEFAULT_PLUGIN_PATH', '\\"' + env.Dir('${BINDIR}').abspath + '\\"')])
env.AppendUnique(CPPDEFINES=['SCONS_BUILD'])

# Setup the include path to the tiff headers (should have been determined in the system specific sections above).
env.AppendUnique(LIBPATH = ['$LIBDIR', '$BINDIR', '$tiff_lib_path', '$jpeg_lib_path', '$zlib_lib_path'])

Help(opts.GenerateHelpText(env))
	
# Check for the existence of the various dependencies
SConscript('build_check.py')

env = conf.Finish()
opts.Save('options.cache', env)

SConscript('libaqsistypes/SConscript')
SConscript('libargparse/SConscript')
SConscript('libddmanager/SConscript')
SConscript('libraytrace/SConscript')
SConscript('librib2/SConscript')
SConscript('librib2ri/SConscript')
SConscript('libshaderexecenv/SConscript')
SConscript('libshadervm/SConscript')
SConscript('render/SConscript')
SConscript('aqsis/SConscript')
SConscript('libslparse/SConscript')
SConscript('libcodegenvm/SConscript')
SConscript('slpp/SConscript')
SConscript('aqsl/SConscript')
SConscript('libslxargs/SConscript')
SConscript('aqsltell/SConscript')
display = SConscript('display/SConscript')
SConscript('libri2rib/SConscript')
SConscript('librib2stream/SConscript')
SConscript('teqser/SConscript')
#SConscript('plugins/SConscript')
SConscript('shaders/SConscript')

# Generate and install the '.aqsisrc' configuration file from the template '.aqsisrc.in'
def build_function(target, source, env):
	# Code to build "target" from "source"
	for x in target:
		print str(x)
	displaylib = os.path.basename(display[0].path)
	defines = {
		"displaylib": displaylib,
		"shaderpath": env.Dir('$SHADERDIR').abspath,
		"displaypath": env.Dir('$BINDIR').abspath,
	    }

	for a_target, a_source in zip(target, source):
		aqsisrc_out = file(str(a_target), "w")
		aqsisrc_in = file(str(a_source), "r")
		aqsisrc_out.write(aqsisrc_in.read() % defines)
		aqsisrc_out.close()
		aqsisrc_in.close()

aqsisrc = env.Command('.aqsisrc', 'aqsisrc.in', build_function)
env.Install('$BINDIR', aqsisrc)
env.Depends(aqsisrc, display)

env.Alias('release', ['$BINDIR','$LIBDIR', '$SHADERDIR'])
Default('release')

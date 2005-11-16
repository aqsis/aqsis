import os
import os.path
import glob

# Setup the common command line options.
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
opts.Add(BoolOption('debug', 'Build with debug options enabled', '0'))

# Create the default environment
env = Environment(options = opts, tools = ['default', 'lex', 'yacc', 'zip', 'tar'])


# Create the configure object here, as you can't do it once a call
# to SConscript has been processed.
conf = Configure(env)
Export('env opts conf')

SConscript('build_support.py')
Import('SelectBuildDir')

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

# Determine the target 
target_config_dir =  '#' + SelectBuildDir('platform')
target_dir =  '#/build'

# Add an option to control the root location of the 'install' target
opts.Add('install_prefix', 'Root folder under which to install Aqsis', env.Dir('#/output'))
opts.Update(env)

# Setup the output directories for binaries and libraries.
env.Replace(BINDIR = env.Dir('$install_prefix').abspath + os.sep + 'bin')
env.Replace(LIBDIR = env.Dir('$install_prefix').abspath + os.sep + 'lib')
env.Replace(SHADERDIR = env.Dir('$install_prefix').abspath + os.sep + 'shaders')
env.Replace(SYSCONFDIR = env.Dir('$install_prefix').abspath + os.sep + 'bin')

# Read in the platform specific configuration.
# Allowing it to override the settings defined above.
SConscript(target_config_dir + os.sep + 'SConscript')

# Setup common environment settings to allow includes from the various local folders
env.AppendUnique(CPPPATH = ['#/build/libaqsistypes','#/build/render', '#/build/libshaderexecenv', '#/build/librib2', '#/build/libshadervm', '#/build/librib2ri', '#/build/libargparse', '#/build/libslparse', '#/build/libcodegenvm', '$zlib_include_path', '$tiff_include_path', '$jpeg_include_path', '$boost_include_path'])
env.AppendUnique(CPPDEFINES=[('DEFAULT_PLUGIN_PATH', '\\"' + env.Dir('${BINDIR}').abspath + '\\"')])
env.AppendUnique(CPPDEFINES=['SCONS_BUILD'])

# Setup the include path to the tiff headers (should have been determined in the system specific sections above).
env.AppendUnique(LIBPATH = ['$LIBDIR', '$BINDIR', '$tiff_lib_path', '$jpeg_lib_path', '$zlib_lib_path'])

# Create the output for the command line options defined above and in the platform specific configuration.
Help(opts.GenerateHelpText(env))
	
# Check for the existence of the various dependencies
SConscript('build_check.py')

# Transfer any findings from the build_check back to the environment
env = conf.Finish()

# Save the command line options to a cache file, allowing the user to just run scons without 
# command line options in future runs.
opts.Save('options.cache', env)

# Load the sub-project sconscript files.
SConscript('libaqsistypes/SConscript', build_dir=target_dir + '/libaqsistypes')
SConscript('libargparse/SConscript', build_dir=target_dir + '/libargparse')
SConscript('libddmanager/SConscript', build_dir=target_dir + '/libddmanager')
SConscript('libraytrace/SConscript', build_dir=target_dir + '/libraytrace')
SConscript('librib2/SConscript', build_dir=target_dir + '/librib2')
SConscript('librib2ri/SConscript', build_dir=target_dir + '/librib2ri')
SConscript('libshaderexecenv/SConscript', build_dir=target_dir + '/libshaderexecenv')
SConscript('libshadervm/SConscript', build_dir=target_dir + '/libshadervm')
SConscript('render/SConscript', build_dir=target_dir + '/render')
SConscript('aqsis/SConscript', build_dir=target_dir + '/aqsis')
SConscript('libslparse/SConscript', build_dir=target_dir + '/libslparse')
SConscript('libcodegenvm/SConscript', build_dir=target_dir + '/libcodegenvm')
SConscript('slpp/SConscript', build_dir=target_dir + '/slpp')
SConscript('aqsl/SConscript', build_dir=target_dir + '/aqsl')
SConscript('libslxargs/SConscript', build_dir=target_dir + '/libslxargs')
SConscript('aqsltell/SConscript', build_dir=target_dir + '/aqsltell')
display = SConscript('display/SConscript', build_dir=target_dir + '/display')
SConscript('libri2rib/SConscript', build_dir=target_dir + '/libri2rib')
SConscript('librib2stream/SConscript', build_dir=target_dir + '/librib2stream')
SConscript('teqser/SConscript', build_dir=target_dir + '/teqser')
#SConscript('plugins/SConscript')
SConscript('shaders/SConscript', build_dir=target_dir + '/shaders')
SConscript('thirdparty/tinyxml/SConscript', build_dir=target_dir + '/thirdpart/tinyxml')

# Generate and install the 'aqsisrc' configuration file from the template '.aqsisrc.in'
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

aqsisrc = env.Command('aqsisrc', 'aqsisrc.in', build_function)
env.Install('$SYSCONFDIR', aqsisrc)
env.Depends(aqsisrc, display)

env.Alias('release', ['$BINDIR','$LIBDIR', '$SHADERDIR','$SYSCONFDIR'])
Default('release')

# Define any files that need to be included in a source distribution.
main_distfiles = Split("""
        SConstruct
	AUTHORS
	build_check.py
	build_support.py
	ChangeLog
	COPYING
	Doxyfile
	INSTALL
	NEWS
	README
	SConstruct
        aqsis.spec
	aqsisrc.in""")

env.Distribute('', main_distfiles)

# Distribute the platform build configurations.
platforms = glob.glob('build/*/SConscript')
for platform in platforms:
	path, name = os.path.split(platform)
	env.Distribute(path, platform)

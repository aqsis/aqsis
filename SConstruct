# This allows the developer to choose the version of msvs from the command line.
opts = Options()
opts.Add(EnumOption('MSVS_VERSION', 'MS Visual C++ Version', '6.0', allowed_values=('6.0', '7.1')))
opts.Add('BISON', 'Point to the bison executable', File('#/../win32libs/bin/bison'))
opts.Add('FLEX', 'Point to the flex executable', File('#/../win32libs/bin/flex'))

# Create the default environment
env = Environment(options = opts)

# Setup common environment settings
env.AppendUnique(CPPPATH = ['#/libaqsistypes','#/render', '#/libshaderexecenv', '#/librib2', '#/libshadervm', '#/librib2ri', '#/libargparse', '#/libslparse', '#/libcodegenvm'])

# Setup important stuff specific to each platform.
if 'win32' ==  env['PLATFORM']:
	env.AppendUnique(CCFLAGS=['/GX', '/MD', '/GR', '/Zm200', '/O2'])
	env.AppendUnique(CPPDEFINES=['NO_SYSLOG', 'WIN32', 'PLUGINS'])
	env.Append(LIBDIR = '#/lib')
	env.Append(BINDIR = '#/bin')
	env.AppendUnique(CPPPATH = ['#/libaqsistypes/win32/intel'])
	env.AppendUnique(TPLIBS = '#/../win32libs/lib')
	env.Replace(YACC = '${BISON}')
	env.Replace(LEX = '${FLEX}')
	env.Replace(YACCFLAGS = '--no-lines -d')

env.Alias('install', ['$LIBDIR', '$BINDIR'])

Help(opts.GenerateHelpText(env))
Export('env')

SConscript('api/SConscript')
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
SConscript('display/SConscript')
SConscript('libri2rib/SConscript')
SConscript('librib2stream/SConscript')
SConscript('teqser/SConscript')
SConscript('plugins/SConscript')
Alias('release', [Dir('$BINDIR'), Dir('$LIBDIR')])
Default('release')

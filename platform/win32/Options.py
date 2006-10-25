from SCons.Script.SConscript import SConsEnvironment
from SCons.Options import EnumOption, BoolOption

def PlatformOptions(opts, tempenv):
	opts.Add(EnumOption('MSVS_VERSION', 'MS Visual C++ Version', '6.0', allowed_values=('6.0', '7.1')))
	opts.Add(BoolOption('mingw', 'Use MingW compiler', 0))
	opts.Add('bison', 'Point to the bison executable', tempenv.File('#/../win32libs/GnuWin32/bin/bison'))
	opts.Add('flex', 'Point to the flex executable', tempenv.File('#/../win32libs/GnuWin32/bin/flex'))

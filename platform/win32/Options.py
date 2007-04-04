from SCons.Script.SConscript import SConsEnvironment
from SCons.Options import EnumOption, BoolOption

def PlatformOptions(opts, tempenv):
	opts.Add(EnumOption('MSVS_VERSION', 'MS Visual C++ Version', '6.0', allowed_values=('6.0', '7.1', '8.0')))
	opts.Add(EnumOption('SSE', 'Enable SSE support', 'off', allowed_values=('off', 'SSE1', 'SSE2')))
	opts.Add(BoolOption('FP_Fast', 'Enable fast FP calculations at the expense of precision', 0))
	opts.Add(BoolOption('mingw', 'Use MinGW compiler', 0))
	opts.Add('bison', 'Point to the bison executable', tempenv.File('#/../win32libs/GnuWin32/bin/bison'))
	opts.Add('flex', 'Point to the flex executable', tempenv.File('#/../win32libs/GnuWin32/bin/flex'))
	# Microsoft Windows Platform SDK include/lib
	opts.Add('psdk_include_path', 'Point to the MS Platform SDK header files', '')
	opts.Add('psdk_lib_path', 'Point to the MS Platform SDK library files', '')

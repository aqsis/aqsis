from SCons.Script.SConscript import SConsEnvironment
from SCons.Options import EnumOption, BoolOption

def PlatformOptions(opts, tempenv):
	opts.Add('bison', 'Point to the bison executable', '/usr/bin/bison')
	opts.Add('flex', 'Point to the flex executable', '/usr/bin/flex')


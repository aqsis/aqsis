from SCons.Script.SConscript import SConsEnvironment
from SCons.Options import EnumOption, BoolOption

def PlatformOptions(opts, tempenv):
	opts.Add('bison', 'Point to the bison executable', '/opt/local/bin/bison')
	opts.Add('flex', 'Point to the flex executable', '/opt/local/bin/flex')

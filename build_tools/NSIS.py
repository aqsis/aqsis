# NSIS Support for SCons
# Written by Mike Elkins, January 2004
# Provided 'as-is', it works for me!


"""
This tool provides SCons support for the Nullsoft Scriptable Install System
a windows installer builder available at http://nsis.sourceforge.net/home


To use it you must copy this file into the scons/SCons/Tools directory or use
the tooldir arg in the Tool function and put a line like 'env.Tool("NSIS")'
into your file. Then you can do 'env.Installer("foobar")' which will read foobar.nsi and
create dependencies on all the files you put into your installer, so that if
anything changes your installer will be rebuilt.  It also makes the target
equal to the filename you specified in foobar.nsi.  Wildcards are handled correctly.

In addition, if you set NSISDEFINES to a dictionary, those variables will be passed
to NSIS.
"""



import SCons.Builder
import SCons.Util
import SCons.Scanner
import SCons.Sig
import os.path
import glob


def nsis_parse( sources, keyword, multiple ):
  """
  A function that knows how to read a .nsi file and figure
  out what files are referenced, or find the 'OutFile' line.


  sources is a list of nsi files.
  keyword is the command ('File' or 'OutFile') to look for
  multiple is true if you want all the args as a list, false if you
  just want the first one.
  """
  stuff = []
  for s in sources:
    c = s.get_contents()
    for l in c.split('\n'):
      semi = l.find(';')
      if (semi != -1):
        l = l[:semi]
      hash = l.find('#')
      if (hash != -1):
        l = l[:hash]
      # Look for the keyword
      l = l.strip()
      spl = l.split(None,1)
      if len(spl) > 1:
        if spl[0].capitalize() == keyword.capitalize():
          arg = spl[1]
          if arg.startswith('"') and arg.endswith('"'):
            arg = arg[1:-1]
          if multiple:
            stuff += [ arg ]
          else:
            return arg
  return stuff


def nsis_path( filename, nsisdefines, rootdir ):
  """
  Do environment replacement, and prepend with the SCons root dir if
  necessary
  """
  # We can't do variables defined by NSIS itself (like $INSTDIR),
  # only user supplied ones (like ${FOO})
  varPos = filename.find('${')
  while varPos != -1:
    endpos = filename.find('}',varPos)
    assert endpos != -1
    if not nsisdefines.has_key(filename[varPos+2:endpos]):
      raise KeyError ("Could not find %s in NSISDEFINES" % filename[varPos+2:endpos])
    val = nsisdefines[filename[varPos+2:endpos]]
    if type(val) == list:
      if varPos != 0 or endpos+1 != len(filename):
        raise Exception("Can't use lists on variables that aren't complete filenames")
      return val
    filename = filename[0:varPos] + val + filename[endpos+1:]
    varPos = filename.find('${')
  return filename


def nsis_scanner( node, env, path ):
  """
  The scanner that looks through the source .nsi files and finds all lines
  that are the 'File' command, fixes the directories etc, and returns them.
  """
  nodes = node.rfile()
  if not node.exists():
    return []
  nodes = []
  source_dir = node.get_dir()
  for include in nsis_parse([node],'file',1):
    exp = nsis_path(include,env['NSISDEFINES'],source_dir)
    if type(exp) != list:
      exp = [exp]
    for p in exp:
      for filename in glob.glob( os.path.abspath(
        os.path.join(str(source_dir),p))):
          # Why absolute path?  Cause it breaks mysteriously without it :(
          nodes.append(filename)
  return nodes


def nsis_emitter( source, target, env ):
  """
  The emitter changes the target name to match what the command actually will
  output, which is the argument to the OutFile command.
  """
  nsp = nsis_parse(source,'outfile',0)
  if not nsp:
    return (target,source)
  x  = (
    nsis_path(nsp,env['NSISDEFINES'],''),
    source)
  return x

def quoteIfSpaced(text):
  if ' ' in text:
    return '"'+text+'"'
  else:
    return text

def toString(item,env):
  if type(item) == list:
    ret = ''
    for i in item:
      if ret:
        ret += ' '
      val = toString(i,env)
      if ' ' in val:
        val = "'"+val+"'"
      ret += val
    return ret
  else:
    # For convienence, handle #s here
    if str(item).startswith('#'):
      item = env.File(item).get_abspath()
    return str(item)

def runNSIS(source,target,env,for_signature):
  ret = env['NSIS']+" "
  if env.has_key('NSISFLAGS'):
    for flag in env['NSISFLAGS']:
      ret += flag
      ret += ' '
  if env.has_key('NSISDEFINES'):
    for d in env['NSISDEFINES']:
      ret += '/D'+d
      if env['NSISDEFINES'][d]:
        ret +='='+quoteIfSpaced(toString(env['NSISDEFINES'][d],env))
      ret += ' '
  for s in source:
    ret += quoteIfSpaced(str(s))
  return ret

def generate(env):
  """
  This function adds NSIS support to your environment.
  """
  env['BUILDERS']['Installer'] = SCons.Builder.Builder(generator=runNSIS,
                                 src_suffix='.nsi',
                                 emitter=nsis_emitter)
  env.Append(SCANNERS = SCons.Scanner.Scanner( function = nsis_scanner,
             skeys = ['.nsi']))
  if not env.has_key('NSISDEFINES'):
    env['NSISDEFINES'] = {}
  env['NSIS'] = find_nsis(env)

def find_nsis(env):
  """
  Try and figure out if NSIS is installed on this machine, and if so,
  where.
  """
  if SCons.Util.can_read_reg:
    # If we can read the registry, get the NSIS command from it
    try:
      k = SCons.Util.RegOpenKeyEx(SCons.Util.hkey_mod.HKEY_LOCAL_MACHINE,
                                  'SOFTWARE\\NSIS')
      val, tok = SCons.Util.RegQueryValueEx(k,None)
      ret = val + os.path.sep + 'makensis.exe'
      if os.path.exists(ret):
        return '"' + ret + '"'
      else:
        return None
    except:
      pass # Couldn't find the key, just act like we can't read the registry
  # Hope it's on the path
  return env.WhereIs('makensis.exe')

def exists(env):
  """
  Is NSIS findable on this machine?
  """
  if find_nsis(env) != None:
    return 1
  return 0


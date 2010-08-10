#!/usr/bin/python
#
# original author - Chris Foster
# license - new BSD

'''
This is a utility to run the cog codegen for all aqsis source files containing
cog generator code.

The utility can be run from anywhere inside the aqsis source tree; it
automatically detects the root of the source tree, finds files containing the
string "[[[cog" and runs the generator against those files.
'''

import os
import os.path
import sys

def findFilesToCog(basePaths):
    '''
    Search through all source files to find those that need processing with the
    cog code generator.
    '''
    import re
    isSourcePattern = re.compile(r'.(h|cpp)$')
    cogPattern = re.compile(r'\[\[\[cog')
    def walkCallback(cogList, dirname, fnames):
        for f in fnames:
            fileName = os.path.join(dirname,f)
            if os.path.isfile(fileName) and isSourcePattern.search(fileName) is not None:
                file = open(fileName, 'r')
                hasCogBlock = False
                for line in file.readlines():
                    if cogPattern.search(line) is not None:
                        hasCogBlock = True
                        break
                file.close()
                if hasCogBlock:
                    cogList.append(fileName)
    filesToCog = []
    # Walk the tree, finding files which need cogging.
    for basePath in basePaths:
        os.path.walk(basePath, walkCallback, filesToCog)
    return filesToCog


# Try to import the modules we know will be needed for the codegen
try:
    import cogapp
    import Cheetah.Template
except ImportError, err:
    print >> sys.stderr, "!! Could not import a module required for the codegen:"
    print >> sys.stderr, "!!", err
    sys.exit(1)


# Paths are relative to aqsis root directory
riXmlPath = 'libs/ribparse/ri.xml'
cogUtilsPath = 'libs/ribparse'
searchDirs = ['libs', 'include']


# Find the aqsis root directory by going up a directory until we find a
# directory containing a '.git' directory
prevwd = ''
cwd = os.getcwd()
while cwd != prevwd:
    if os.path.isdir('.git'):
        break
    prevwd = cwd
    os.chdir('..')
    cwd = os.getcwd()
if cwd == prevwd:
    print 'Cannot find aqsis source root directory.  You must run the codgen from somewhere inside the tree.'
    sys.exit(1)


print 'Searching for source files...'
filesToCog = findFilesToCog(searchDirs)

print 'Cogging...'
for fileName in filesToCog:
    argv = ['run_codegen.py',
            '-r',                              # run in place
            '-I', cogUtilsPath,                # location of utilities
            '-D', 'riXmlPath=%s'%(riXmlPath,), # location of ri.xml
            fileName]                          # file to cog
    # Run cog directly via the cogapp module rather than through the cog.py
    # wrapper.
    cogapp.Cog().main(argv)


# vi: set et:

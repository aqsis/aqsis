## Aqsis
## Copyright © 1997 - 2001, Paul C. Gregory
##
## Contact: pgregory@aqsis.com
##
## This library is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public
## License as published by the Free Software Foundation; either
## version 2 of the License, or (at your option) any later version.
##
## This library is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public
## License along with this library; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
##
##
## \file
##		\brief A batch shader compiler
##		\author Matthaeus G. Chajdas (Anteru@darkside-conflict.net)
##

print """Aqsis batch SL compiler
    """

import os, fnmatch, string, sys

# vars
compiled = 0
error = 0
warn = 0
dso = 0
i = 0
efiles = []
cfiles = []
wfiles = []
dfiles = []

files = os.listdir('./') # Get all files

for f in files:
    if fnmatch.fnmatch(f, '*.sl'):
        streams = os.popen3('aqsl ' + f)
        
        # Get std::cerr
        if streams[2].read() != '':
            error = error + 1
            efiles.append( f )
        else:
            # Get std::cout
            stdout = streams[1].read()
            # How many warnings?
            en = string.count( stdout, 'WARN' )
            if en:
                # if only 2 warnings, and a DSO, we've hit a DSO call
                if en == 2 and string.find( stdout, 'DSO' ) != -1:
                    # Found the two DSO warnings, ignore
                    dfiles.append( f )
                    dso = dso + 1
                else:
                    warn = warn + 1
                    wfiles.append( f )
            else:
                cfiles.append( f )
            compiled = compiled + 1
            
all = compiled + error + warn

# Print all compiled shaders including those with DSO warnings only
if compiled > 0:
    print 'Compiled %(compiled)i of %(all)i shader(s):' % vars()
    for f in cfiles:
        print '\t' + f
    print
    print 'Found %(dso)i shader(s) with DSO function calls:' % vars()
    for f in dfiles:
        print '\t' + f

# Print all shaders with warnings        
if warn > 0:
    print 'Compiled %(warn)i of %(all)i shader(s) with warnings:' % vars()
    for f in wfiles:
        print '\t' + f
print

# Print all shaders which couldn't be compiled        
if error > 0:
    print 'Could not compile %(error)i of %(all)i shader(s):' % vars()
    for f in efiles:
        print '\t' + f
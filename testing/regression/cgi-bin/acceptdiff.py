#!/usr/bin/env python
######################################################################
# Copy the last render output into the reference directory
######################################################################

import os, os.path, shutil, cgi
import cgitb
cgitb.enable()

print "Content-Type: text/html"
print

# The base directory of the regression test suite
testSuiteHome = os.getcwd()
rendererName = "Aqsis"

form = cgi.FieldStorage()
# The full path to the main test case file (.rib or .py)
testCase = os.path.normpath(os.path.join(testSuiteHome, form["testcase"].value))
# The directory where the test case and the rendered images are located
testCaseDir = os.path.dirname(testCase)
# The name (without path) of the test case
testCaseBase = os.path.basename(testCase)
# A list of output image names (without path & renderer)
outImageNames = form["outimagenames"].value.split(",")

# The reference images
refImgs = []
# The new references images
outImgs = []
# Missing output images
missing = []
for imgName in outImageNames:
    outImg = os.path.join(testCaseDir, "%s_%s"%(rendererName, imgName))
    outImgs.append(outImg)
    refImgs.append(os.path.join(testSuiteHome, "reference", imgName))
    if not os.path.exists(outImg):
        missing.append(outImg)

print "<html><head><title>Accept new reference image for %s</title></head>"%testCaseBase

if len(missing)==0:
    print "<h1>Updated reference image for %s</h1>"%testCaseBase
    print 'Copied files:<p><ul>'
    for outImg,refImg in zip(outImgs, refImgs):
        shutil.copyfile(outImg, refImg)
        print '<li> <tt>%s</tt> to <tt>%s</tt>'%(outImg, refImg)
    print '</ul><p>To update the render results you have to run the regression tests again.'
else:
    print "<h1>Error</h1>"
    print 'Cannot copy new reference image, the rendered output is not available anymore:<p><ul>'
    for name in missing:
        print '<li> <tt>%s</tt>'%name
    print "</ul>"
   
# The following will not work when other directories than "html" are browsed... 
#print '<p><a href="../html/%s_%s.html">Back to %s</a>'%(os.path.splitext(testCaseBase)[0], rendererName, testCaseBase)

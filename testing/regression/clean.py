#!/usr/bin/env python
# Remove all temporary files

import sys, os, glob

def rm(pattern):
    """Emulate the rm command."""
    
    for filename in glob.glob(pattern):
        print 'Removing "%s"...'%filename
        os.remove(filename)

######################################################################

rm("*~")
rm("RegressionTests/*~")
rm("RegressionTests/*.tif")
rm("RegressionTests/*.env")
rm("RegressionTests/*.z")
rm("RegressionTests/*.map")
rm("RegressionTests/*.temp")
rm("RegressionTests/shaders/*.slx")
rm("RegressionTests/shaders/*.slc")
rm("RegressionTests/shaders/*.sdl")
rm("RegressionTests/shaders/*.sdr")
rm("RegressionTests/shaders/*.i")

rm("RIBs/*/*.tif")
rm("RIBs/*/*~")
rm("RIBs/*/*.z")
rm("RIBs/*/*.map")
rm("shaders/*.slx")
rm("shaders/*.slc")
rm("shaders/*.sdl")
rm("shaders/*.sdr")
rm("shaders/*.i")

rm("RIBs/*/*/*.tif")
rm("RIBs/*/*/*~")

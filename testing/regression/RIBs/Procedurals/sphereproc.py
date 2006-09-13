# Procedural that gets invoked by runprog.rib
# The script generates a sphere with a radius that's given as input to
# the script.

import sys
from cgkit.ri import *

while 1:
    s = sys.stdin.readline()
    if s=="":
        break
    a = s.split(" ")
    detail = float(a[0])
    rad = float(a[1])

    RiBegin(RI_NULL)
    RiSphere(rad,-rad,rad,360)

    RiArchiveRecord(RI_COMMENT,"\377")
    sys.stdout.flush()
    RiEnd()



# Procedural used by rundetailproc.rib
# The script creates a bilinear patch that displays the "detail" value
# that was passed to the script. 

import sys
import Image, ImageDraw
from cgkit.ri import *

def createImage(imgname, detail):
    img = Image.new("RGB", (200,100))
    draw = ImageDraw.Draw(img)
    draw.rectangle((0,0,200,100), fill=(255,255,255))
    draw.text((10,10), "Detail: %f"%detail, fill=(0,0,0))
    img.save(imgname)

######################################################################

while 1:
    s = sys.stdin.readline()
    if s=="":
        break
    a = s.split(" ")
    detail = float(a[0])

    createImage("tmp_tex.tif", detail)

    RiBegin(RI_NULL)
    RiSurface("constanttex", "string mapname", "tmp_tex.tif")
    RiPatch(RI_BILINEAR, P=[-2,1,0, 2,1,0, -2,-1,0, 2,-1,0])

    RiArchiveRecord(RI_COMMENT,"\377")
    sys.stdout.flush()
    RiEnd()



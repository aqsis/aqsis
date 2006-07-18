##
# BtoRTypes.py
# Simple object types for PyProtocols
#
#
#
#

class BtoRBasicObject:
	def __init__(self, obj):
		self.obj = obj

	def getObject(self):
		return self.obj
		
# these names correspond to blender object types returned by Blender.Object.Object.getType()
class BtoRMesh(BtoRBasicObject): pass
class BtoRLamp(BtoRBasicObject): pass
class BtoRSurf(BtoRBasicObject): pass
class BtoRCamera(BtoRBasicObject): pass
class BtoRLattice(BtoRBasicObject): pass
class BtoRArmature(BtoRBasicObject): pass
class BtoRCurve(BtoRBasicObject): pass
class BtoRMBall(BtoRBasicObject): pass
class BtoRUnknown(BtoRBasicObject): pass
class BtoREmpty(BtoRBasicObject): pass
class BtoRWave(BtoRBasicObject): pass
	
	
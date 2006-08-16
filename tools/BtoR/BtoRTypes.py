##
# BtoRTypes.py
# Simple object types for PyProtocols
class BtoRBasicObject:
	def __init__(self, obj):
		self.obj = obj

	def getObject(self):
		return self.obj

# standard geometry objects
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
# preview object
class BtoRPreview(BtoRBasicObject): pass
# complex property types
class BtoRShaderType(BtoRBasicObject): 
	def getShaderName(self):
		return self.obj.getShaderName()
	def registerCallback(self, signal, function):
		self.obj.registerCallback(signal, function)
class BtoRMaterialType(BtoRBasicObject): pass
class BtoRRotationType(BtoRBasicObject): pass
	
# shader parameter objects
class BtoRBasicParam:
	def __init__(self, matName = None, shader = None, param = None, type = None, value = None, size = None, parent = None, name = None):
		self.mat = matName
		self.shader = shader
		self.param = param
		self.type = type
		self.value = value
		self.size = size
		self.parent = parent
		self.name = name
	def getType(self):
		return self.type
	def getShader(self):
		return self.shader
	def getMaterialName(self):
		return self.mat
	def getParam(self):
		return self.param
	def getValue(self):
		return self.value
		
class BtoRFloatParam(BtoRBasicParam): pass
class BtoRStringParam(BtoRBasicParam): pass
class BtoRSpaceParam(BtoRBasicParam): pass
class BtoRFileParam(BtoRBasicParam): pass
class BtoRColorSpaceParam(BtoRBasicParam): pass
class BtoRProjectionParam(BtoRBasicParam): pass
class BtoRArrayParam(BtoRBasicParam): pass
class BtoRPointParam(BtoRBasicParam): pass
class BtoRNormalParam(BtoRBasicParam):pass
class BtoRVectorParam(BtoRBasicParam):pass
class BtoRColorParam(BtoRBasicParam): pass
class BtoRMatrixParam(BtoRBasicParam): pass
	
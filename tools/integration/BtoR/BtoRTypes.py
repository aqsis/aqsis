##
# BtoRTypes.py
# Simple object types for PyProtocols
class BtoRBasicObject:
	def __init__(self, obj):
		self.obj = obj
		self.isRenderable = False
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
		print "registered callback!"
		self.obj.registerCallback(signal, function)
class BtoRMaterialType(BtoRBasicObject): pass
class BtoRRotationType(BtoRBasicObject): pass
class BtoRCustomRIB(BtoRBasicObject): pass
		
# shader parameter objects
class BtoRBasicParam: 
	height = 27 # should work for most
	def __init__(self, matName = None, shader = None, param = None, p_type = None, value = None, size = None, parent = None, name = None):
		self.mat = matName
		self.shader = shader
		self.param = param
		self.type = p_type
		self.value = value
		self.size = size
		self.parent = parent
		self.name = name
		self.saveable = True
		self.labelWidth = 0
		self.editorWidth = 0
		print self.value
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
	def setValue(self, value):		
		self.value = value
	def setHeight(self, height):
		self.height = height
	def setWidth(self, width):
		self.width = width
	def setName(self, name):
		self.name = name
	def getName(self):
		return self.name
	def setDefault(self, default):
		self.default = default
	def getKeys(self):
		return value.keys()
		
	def toXML(self, xml):
		xmlProp = xml.createElement("property")
		xmlProp.setAttribute("type", type(self.getValue()).__name__)
		xmlProp.setAttribute("value", str(self.getValue()))
		#print "set property of type ", type(self.value), " to ", type(str(self.value).__name__)
		return xmlProp		
		
class BtoRPath(BtoRBasicParam):
	pass
class BtoRFloatParam(BtoRBasicParam): 
	def getValue(self):
		return float(self.value)
class BtoRStringParam(BtoRBasicParam):
	def getStrValue(self):
		return self.value # a string parameter will return itself and only itself
class BtoRArrayParam(BtoRBasicParam): pass
class BtoRPointParam(BtoRBasicParam): pass
class BtoRNormalParam(BtoRBasicParam):pass
class BtoRVectorParam(BtoRBasicParam):
	def getValue(self):
		return [float(self.value[0]), float(self.value[1]), float(self.value[2])]
class BtoRColorParam(BtoRBasicParam): 
	def getValue(self):
		return [float(self.value[0]), float(self.value[1]), float(self.value[2])]
		
class BtoRMatrixParam(BtoRBasicParam): pass
	
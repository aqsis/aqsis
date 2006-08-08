import os
import btor
from btor import BtoRGUIClasses as ui
from btor.BtoRTypes import *
reload(ui)
import cgkit
import cgkit.rmshader
reload(cgkit.rmshader)
import cgkit.cgtypes
import cgkit.quadrics
from cgkit import ri as ri
from cgkit import ribexport as export
import Blender
import xml.dom.minidom
import new
import math
from sets import Set
import sys
import StringIO	
import protocols
import random
import traceback

class IProperty(protocols.Interface):
	def getValue():
		pass
	def setValue():
		pass
		
class Property:
	height = 27 # should work for most
	def __init__(self, value):
		self.value = value
	def setHeight(self, height):
		self.height = height
	def setWidth(self, width):
		self.width = width
	def setName(self, name):
		self.name = name
	def getName(self):
		return self.name
	def setValue(self, value):
		self.value = value
	def setDefault(self, default):
		self.default = default
	def getValue(self):
		return self.value
		
protocols.declareAdapter(Property, [IProperty])

class StringProperty(Property): pass
protocols.declareAdapter(StringProperty, [IProperty], forTypes=[str])

class IntProperty(Property): pass
protocols.declareAdapter(IntProperty, [IProperty], forTypes=[int])

class FloatProperty(Property): pass
protocols.declareAdapter(FloatProperty, [IProperty], forTypes=[float])

class ColorProperty(Property): pass
protocols.declareAdapter(ColorProperty, [IProperty], forTypes=[list])
	
class DictProperty(Property): pass
protocols.declareAdapter(DictProperty, [IProperty], forTypes=[dict])
	
class BooleanProperty(Property): pass
protocols.declareAdapter(BooleanProperty, [IProperty], forTypes=[bool])

class IPropertyEditor(protocols.Interface):
	def getValue():
		"""" get the value of the property """
	def setValue():
		""" set the value of the property """
class PropertyEditor: # this needs no interface declaration, since all this is doing is providing a baseclass
	fontsize = 'small'
	def __init__(self, property):		
		self.property = property
		width = self.property.width
		self.height = self.property.height		
		self.editor = ui.Panel(0, 0, width, self.height, "", "", None, False)
		self.editor.hasHeader = False
		self.editor.shadowed = False
		self.editor.normaColor = [128, 128, 128, 0]
		self.editor.hoverColor = [128, 128, 128, 0]
		self.editor.outlined = True
		self.editor.cornermask = 0
		self.editor.outlined = True
		self.editor.cornermask = 0
		self.label = ui.Label(2, 3, self.property.getName(), self.property.getName(), self.editor, True, fontsize = self.fontsize)
		self.label.fontsize = 'small'
		self.func = None
		
	def setValue(self, value):
		self.property.setValue(value)
		self.value.setValue(value)
		
	def setParent(self, parent):
		self.editor.parent = parent
		self.editor.invalid = True
		
	def setPropertyCallback(self, func):
		self.func = func
		
	def getValue(self):
		return self.property.getValue()
		
	def updateValue(self, obj):
		self.property.setValue(obj.getValue())
		if self.func != None:
			self.func() # invoke the update function for this property
			
	def getEditor(self):
		return self.editor
		
class BasicPropertyEditor(PropertyEditor):
	#protocols.advise(instancesProvide=["IProperty"], asAdapterForTypes=[StringProperty, IntProperty, FloatProperty])
	""" A basic property, a label and a text box """
	def __init__(self, property):
		PropertyEditor.__init__(self, property)
		width = property.width
		height = property.height
		self.value = ui.TextField(width / 2, 0, width / 2, height, self.property.getName(), self.property.getValue(), self.editor, True, fontsize = self.fontsize)
		self.value.registerCallback("update", self.updateValue)
		
protocols.declareAdapter(BasicPropertyEditor, [IPropertyEditor], forTypes=[StringProperty, IntProperty, FloatProperty])

class MenuPropertyEditor(PropertyEditor):
	#protocols.advise(instancesProvide=[IPropertyEditor], asAdapterForTypes=[DictProperty])
	def __init__(self, property):
		PropertyEditor.__init__(self, property)
		width = self.property.width
		height = self.property.height
		menu = self.property.getValue().keys()
		self.value = ui.Menu(width / 2, 2, width / 2, height - 4, self.property.getName(), menu, self.editor, True, fontsize = self.fontsize)
		self.value.registerCallback("select", self.updateValue)
		self.value.setShadowed(False)
		# the property should handle making sure I've got the right value here I think
	def setValue(self, value):
		self.value.setValueString(value)
		
protocols.declareAdapter(MenuPropertyEditor, [IPropertyEditor], forTypes=[DictProperty])

class ColorPropertyEditor(PropertyEditor):
	#protocols.advise(instancesProvide=[IPropertyEditor], asAdapterForTypes=[ColorProperty])
	def __init__(self, property):
		PropertyEditor.__init__(self, property)
		width = self.property.width
		height = self.property.height
		color = self.property.getValue()
		# I need 3 RGB values
		inc = (width / 2) / 4
		self.R = ui.TextField((width / 2), 0, inc -1, height, "Red", color[0], self.editor, True)
		self.G = ui.TextField((width / 2) + inc, 0, inc -1, height, "Green", color[0], self.editor, True)
		self.B = ui.TextField((width / 2) + (inc * 2), 0, inc -1, height, "Blue", color[0], self.editor, True)
		self.colorButton = ui.ColorButton((width / 2) + (inc * 3), 0, inc - 4, height - 2, "Color", color, self.editor, True)
		self.colorButton.outlined = True
		self.R.registerCallback("update", self.updateColor)
		self.G.registerCallback("update", self.updateColor)
		self.B.registerCallback("update", self.updateColor)
		self.colorButton.picker.registerCallback("ok", self.updateFields)
	def updateFields(self, color):
		self.color_red.setValue(float(float(color[0]) / 255))
		self.color_green.setValue(float(float(color[1]) / 255))
		self.color_blue.setValue(float(float(color[2]) / 255))
	
	def updateColor(self, obj):
		self.colorButton.setValue([self.R.getValue(), self.G.getValue(), self.B.getValue(), 255]) 
		self.property.setValue([self.R.getValue(), self.G.getValue(), self.B.getValue(), 255])
protocols.declareAdapter(ColorPropertyEditor, [IPropertyEditor], forTypes=[ColorProperty])

class BooleanPropertyEditor(PropertyEditor):
	def __init__(self, property):
		PropertyEditor.__init__(self, property)
		width = self.property.width
		height = self.property.height
		self.value = ui.CheckBox(width / 2 + ((width / 2) - 55), 5, "", " ", property.getValue(), self.editor, True, fontsize = self.fontsize)
		self.value.height = 15
		self.value.x_offset = 2
		self.value.y_offset = 0
		#self.value.outlined = True
		self.value.registerCallback("release", self.updateValue)
		# again, the property should handle this
protocols.declareAdapter(BooleanPropertyEditor, [IPropertyEditor], forTypes=[BooleanProperty])

class IObjectAdapter(protocols.Interface):
	def render():
		""" Render the object"""
	def getInfo():
		""" Get the object's information hash. """
	def initObjectData():
		""" Initialize the object's BtoR data """
	def loadData():
		""" Load object's data """
	def saveData():
		""" Save object's Data """
	
class IObjectUI(protocols.Interface):
	def getEditor():
		""" Get the object's editor panel. """	
	def setExportCallback(self, func):
		""" Assign an export function """
class IShaderParamUI(protocols.Interface):
	def getVariance():
		""" Returns a shader param that has been modified according to rules specified by the user """
	def getEditor():
		""" returns the UI for the shader parameter """
class IShaderParamEditor(protocols.Interface):
	def setValue():
		""" Set the value of the parameter """
	def getValue():
		""" get the value of the parameter """
class ObjectAdapter:
	protocols.advise(instancesProvide=[IObjectAdapter], asAdapterForTypes=[BtoRLattice, BtoRArmature, BtoRBasicObject, BtoREmpty, BtoRWave])
	def __init__(self, obj):
		""" initialize an object adapter based on the incoming object """
		dict = globals()		
		self.settings = dict["instBtoRSettings"]
		self.evt_manager = dict["instBtoREvtManager"]
		self.scene = dict["instBtoRSceneSettings"]
		self.materials = dict["instBtoRMaterials"]
		self.object = obj.obj # this is a BtoR object instance
		# why can't I use the objectUI protocol to get an appropriate editor?
		# let's do that!
		self.objEditor = protocols.adapt(obj, IObjectUI)		
		self.initObjectData() # initializes object data 
		self.properties = self.objEditor.properties
		# self.objType = obj.obj.getType()
	
	# convenience method to return the editor object for this adapter
	def getProperty(self, property):
		if self.properties.has_key(property):
			return self.properties[property].getValue()
		else:
			return False
		
	def setProperty(self, property, value):
		if self.properties.has_key(property):
			self.properties[property].setValue(value)
		
	def getEditor(self):
		return self.objEditor

	def getInfo(self):
		""" Return the object data for this object. """
		#objData = self.scene.object_data[self.objectName.getValue()]
		for key in self.objData.keys():
			print "Key: ", key, " value: ", self.objData[key]
			
	def callFactory(self, func, *args, **kws):
		""" construct a curried function for use with a button """
		def curried(*moreargs, **morekws):
			kws.update(morekws) # the new kws override any keywords in the original
			# print kws, " ", args
			return func(*(args + moreargs), **kws)
		return curried
		
	def render(self):
		""" Generate Renderman data from this call. """
		# decide here what to do with the object's data. Do I render it using the normal adapter method, or do I do something special for cases involving ReadArchive and what-not?
		
		return True
	def renderAsCamera(self):
		return True
		
	def renderArchive(self, archive):
		# this should be all that's neccessary 
		# I do need to handle dupliverts situations and animated curves, array modifer, all that crap.
		ri.RiBegin(archive)
		self.render()
		ri.RiEnd()
		
	def initObjectData(self):			
		""" Generate the object data for this  object. """
		self.objData = {}
		self.objData["name"] = self.object.getName()
		self.objData["type"] = self.object.getType()
		
	def populateShaderParamsList(self, params, shader, initialized):
		
		convtypes = {"float":"double",
				"str":"string",
				"vec3":"vec3",
				"matrix":"mat4"}

		val = ""
		# print dir(shader)
		for key in params.keys():
			param = params[key]
			if isinstance(param, list):				
				if len(param) == 3:  # color, vector, normal, or point
					ptype = "vec3"
					val = cgkit.cgtypes.vec3(float(param[0]), float(param[1]), float(param[2]))				
				elif len(param) == 16: # matrix
					val = cgkit.cgtypes.mat4(float(param[0]),
							float(param[2]),
							float(param[2]),
							float(param[3]),
							float(param[4]),
							float(param[5]),
							float(param[6]),
							float(param[7]),
							float(param[8]),
							float(param[9]),
							float(param[10]),
							float(param[11]),
							float(param[12]),
							float(param[13]),
							float(param[14]),
							float(param[15]))
							
			elif isinstance(param, float) or isinstance(param, int):
				ptype = "float"
				val = float(param)
				
			elif isinstance(param, str):
				ptype = "string"
				val = param			
			
				
			if initialized == False:
				shader.declare(key,type=convtypes[ptype], default=val ) # declare the shader here!
				# shader.createSlot(key + "_slot", convtypes[ptype], None, val) # Here we set the default value to the parameters incoming value...
			# print "Setting shader parameter ", key, " to ", val
			# and set the value 
			# print key, ", ", val
			if param != None:
				setattr(shader, key, val)
			
	def populateShaderParams(self, parms, shader, initialized):
		
		for parm in parms:	
			convtypes = {"float":"double",
					"string":"string",
					"color":"vec3",
					"point":"vec3",
					"vector":"vec3",
					"normal":"vec3",
					"matrix":"mat4"}			
			p_type = parm.getAttribute("type")
			p_name = parm.getAttribute("name")
				
			if p_type == "float":
				parm_value = float(parm.getAttribute("value"))
			elif p_type == "string":
				parm_value = parm.getAttribute("value")
			elif p_type == "color":
				parm_value = cgkit.cgtypes.vec3(float(parm.getAttribute("red")), float(parm.getAttribute("green")), float(parm.getAttribute("blue")))
			elif p_type in ["normal", "vector", "point"]:
				parm_value = cgkit.cgtypes.vec3(float(parm.getAttribute("x")), float(parm.getAttribute("y")), float(parm.getAttribute("z")))
			elif p_type == "matrix":
				mat_value = []
				sep = "_"
				index = 0
				for x in range(4):
					for y in range(4):
						mat_value.append(float(parmNode.getAttribute(sep.join(["value", index]))))
						index = index + 1
				parm_value = cgkit.cgtypes.mat4(mat_value[0], 
										mat_value[1], 
										mat_value[2], 
										mat_value[3], 
										mat_value[4], 
										mat_value[5], 
										mat_value[6], 
										mat_value[7], 
										mat_value[8], 
										mat_value[9],
										mat_value[10],
										mat_value[11],
										mat_value[12],
										mat_value[13],
										mat_value[14],
										mat_value[15])		
						
			if initialized == False:
				shader.declare(p_name, type=convtypes[p_type], default=parm_value)
				# shader.createSlot(p_name, convtypes[p_type], None, parm_value) # Here we set the default value to the parameters incoming value.
			
			# and set the value 
			setattr(shader, p_name, parm_value)
	
	def checkReset(self):
		# check the object in question
		pass # do nothing unless overridden by a subclass interested in object changes
		
	def saveData(self, xml):
		""" Generate an XML representation of this object """
		objXml = xml.createElement("Object") 
		#print self.objData
		for key in self.objData.keys():
			if key not in ["lightcolor", "shaderparms", "shaderparams"]: # ignore light colors & shader parameters here. Shader parms are rebuilt from the shader object itself.
				
				if isinstance(self.objData[key], int) or isinstance(self.objData[key], float):
					objXml.setAttribute(key, '%f' % self.objData[key])
				else:
					objXml.setAttribute(key, self.objData[key])
		if self.__dict__.has_key("shader"):
			
			if self.shader.getShaderName() not in ["None Selected", None]:
				# if I need to add other extra processing, I simply add another case.
				shaderNode = xml.createElement("shader")
				# update all the stuff...
				self.shader.updateShaderParams()
				
				shaderNode.setAttribute("name", self.shader.getShaderName())
				if self.shader.shader.filename != None:
					shaderNode.setAttribute("path", os.path.normpath(self.shader.shader.filename))
				else:
					shaderNode.setAttribute("path", "None")
				print "Shader Parameters: ", self.shader.shader.shaderparams
				for parm in self.shader.shader.shaderparams:
					# get the param and stuff into my dict				
					# create the node
					parmNode = xml.createElement("Param")
					value = getattr(self.shader.shader, parm)	
					# create an XML element for this value.
					s_type = self.shader.shader.shaderparams[parm].split()[1]
					# setup as much of the node element as I can here
					
					parmNode.setAttribute("name", parm)
					parmNode.setAttribute("type", s_type)
					
					if s_type == "float":
						parmNode.setAttribute("value", '%f' % value)
					elif s_type == "string":
						parmNode.setAttribute("value", value)
					elif s_type == "color":
						parmNode.setAttribute("red", '%f' % value[0])
						parmNode.setAttribute("green", '%f' % value[1])
						parmNode.setAttribute("blue", '%f' % value[2])
					elif s_type in ["point", "normal", "vector"]:
						parmNode.setAttribute("x", '%f' % value[0])
						parmNode.setAttribute("y", '%f' % value[1])
						parmNode.setAttribute("z", '%f' % value[2])
					elif s_stype == "matrix":
						sep = "_"
						index = 0
						for x in range(4):
							for y in range(4):
								parmNode.setAttribute(sep.join(["value", index]), value[x, y])
								index = index + 1
					
					# now commit this node to the shader node
					shaderNode.appendChild(parmNode)
				objXml.appendChild(shaderNode)		
				
		return objXml
				
				
	def loadData(self, xml):
		""" Recreate this object from an XML representation of it. """
		self.objData = {}
		atts = xml.attributes
		# setup the attributes first, since I will need the shader filename if I'm using slparams		
		for att in range(atts.length):
			xAtt = atts.item(att)
			self.objData[xAtt.name] = xAtt.value.encode("ascii")
		self.initMaterial()
		
		
		# test for any shaders
		parms = xml.getElementsByTagName("shader")
		if len(parms) > 0:
			# I've got some shader or another
			# strip the params out and stuff them into the "shaderparams" key
			self.initShader(useXML = True, xml = parms)
		
		self.checkReset()
			
	def initMaterial(self):
		pass
	def initShader(self, useXML = False, xml = None):
		print "Initializing a ", self.shader_type, " shader"
		try:
			if self.settings.use_slparams:
				shaderPath = self.settings.shaderpaths.getValue().split(";")[0]
				
				# try to find the shader file in question. In truth, the default files should exist
				# get the first shader search path for pre-generated shaders				
				initialized = True
				
				shader = cgkit.rmshader.RMShader(self.objData["shaderfilename"])
				if useXML:
					self.populateShaderParams(xml, shader, initialized)
				else:
					self.populateShaderParamsList(self.objData["shaderparms"], shader, initialized)		
				
			else:
				
				initialized = False
				shader = cgkit.rmshader.RMShader(self.objData["shadername"])
				self.populateShaderParamsList(self.ObjData["shaderParams"], shader, initialized)		
			self.shader = btor.BtoRMain.GenericShader(shader, self.shader_type, self)
		except:
			traceback.print_exc()
			self.shader = btor.BtoRMain.GenericShader(None, self.shader_type, self)
		
		self.objEditor.setShader(self.shader)
		
		self.objEditor.shaderButton.setTitle(self.shader.getShaderName())
		
class MeshAdapter(ObjectAdapter):
	""" BtoR mesh Adapter """
	protocols.advise(instancesProvide=[IObjectAdapter], asAdapterForTypes=[BtoRMesh])
	def __init__(self, object):
		""" Initialize a mesh export adapter """		
		ObjectAdapter.__init__(self, object)	
	def initMaterial(self):
		print self.objData["material"]
		if self.objData["material"] != None and self.objData["material"] != "None":
			self.setMaterial(self.materials.getMaterial(self.objData["material"]), True)
		
	def render(self):
		""" Generate Renderman data for this object."""
		#1st test, is this is an RiObject?
		# print "exporting!"
		# let's assume that if I'm here, that RiBegin has already been called
		# start gathering data
		if ObjectAdapter.render(self) == True:			
			# immediately test for dupliverts
			if self.object.enableDupVerts and len(self.object.DupObjects) > 0:
				# I might have dupliverts, react accordingly
				# test for rotation by object normal, etc.
				# step 1, create an object definition
				objSequence = ri.RiObjectBegin() # keep the sequence number for referencing the instances
				# don't render materials here, only the object data, and for safety's sake, keep at the world origin
				self.renderMeshData()
				ri.RiObjectEnd()
				# now for the instances				
				objs = self.object.DupObjects
				ri.RiAttributeBegin() # I do this out here, because the material attribute will remain the same for all instances
				# unless of course I come up with a way of specifying a different material per instance...which would be really interesting.
				self.renderMaterial() # render the material
				instance = 1
				for obj in objs:					
					ri.RiTransformBegin()					
					# transform the object
					ri.RiAttributeBegin()
					ri.RiAttribute("identifier", "name", self.objData["name"] + instance)
					ri.RiTransform(obj[1]) # that should be the duplicated matrix and in fact should take into account rotations and scaling, all that
					ri.RiObjectInstance(objSequence)	
					ri.RiAttributeEnd()
					ri.RiTransformEnd()
					instance = instance + 1
				ri.RiAttributeEnd()
						
			else:
				# test for archive rendering, otherwise render to the current RIB stream
				ri.RiAttributeBegin()				
				ri.RiAttribute("identifier", "name", [self.objData["name"]]) # give it a name
				ri.RiTransformBegin()				
				self.renderMaterial() # render the material				
				ri.RiTransform(self.object.matrix) # transform				
				self.renderMeshData() # and render the mesh				
				ri.RiTransformEnd()
				ri.RiAttributeEnd()
				# aaaannnnnd done.
	
	def renderMeshData(self):
		subsurf = False
		modifiers = self.object.modifiers
		for modifier in modifiers:			
			if modifier.type == Blender.Modifier.Type["SUBSURF"]:
				subsurf = True				
			else:
				subsurf = False
		if subsurf:
			# print "Exporting a subdivision mesh"
			self.renderSubdivMesh()
		else:
			# print "Exporting a standard mesh"
			self.renderPointsPolygons()
		
		
	def renderMaterial(self):
		if self.objData["material"] == "None":
			# generate a default material - make sure to setup a default material button in the scene settings dialog
			ri.RiColor(cgkit.cgtypes.vec3(1.0, 1.0, 1.0))
			ri.RiOpacity(cgkit.cgtypes.vec3(1.0, 1.0, 1.0))
			ri.RiSurface("matte", { "Ka" : 1.0, "Kd" : 1.0 })
				
		elif self.objData["material"] != None:			
			Bmaterial = self.materials.getMaterial(self.objData["material"])			
			Bmaterial.surface.updateShaderParams()
			Bmaterial.displacement.updateShaderParams()
			Bmaterial.volume.updateShaderParams()
			material = Bmaterial.material
			ri.RiColor(material.color())			
			ri.RiOpacity(material.opacity())
			# thus
			if material.surfaceShaderName() != None:				
				ri.RiSurface(material.surfaceShaderName(), material.surfaceShaderParams(0))
			if material.displacementShaderName() != None:				
				ri.RiDisplacement(material.displacementShaderName(), material.displacementShaderParams(0))
			if material.interiorShaderName() != None:
				ri.RiAtmosphere(material.interiorShaderName(), material.interiorShaderParams(0))
				
	def renderArchive(self, archive):
		""" Write this object to an external archive file. """
		# add support here for detecting if materials should be exported to archives or not.
		# I should probably add that as an option in the export settings dialog.
		ri.RiBegin(self.objData["archive"]) 
		# no transformations are neccessary here, unless I absolutely want them in the archive
		# they should actually 
		
		ri.RiEnd()
		
	def initObjectData(self):
		""" Initialize the object data for this object. """
		# do some mesh-related stuff
		# all I'm concerned with at the moment is whether or not the mesh in question is subsurfed or not. I don't care about levels and what-not,
		# since I can grab that from the blender object itself.
		self.objData = {}
		self.objData["name"] = self.object.getName()		
		self.objData["output_type"] = "Mesh"
		self.objData["type"] = self.object.getType()
		meshObject = self.object.getData()
		modifiers = self.object.modifiers
		
		subsurf = False
		for modifier in modifiers:
			# hold up: This is a render-time issue, doesn't belong here.
			if modifier.type == "Subsurf":
				# and modifier[Blender.Modifier.Settings.TYPES] == 0:
				subsurf = True
				# modified = True
				renderLevels = modifier[Blender.Modifier.Settings.RENDLEVELS]
				UV = modifier[Blender.Modifier.Settings.UV]
				# hold up: This is a render-time issue, doesn't belong here.
				# modifier[Blender.Modifier.Settings.TYPES] = 1
			else:
				subsurf = False
		
		if subsurf == True:
			self.objData["mesh_type"] = "Subsurf"
			self.objData["renderLevels"] = renderLevels
			self.objData["UV"] = UV
		else:
			self.objData["mesh_type"] = "mesh"
			
		self.objData["material"] = "None"
		
	def setMaterial(self, material, preInit = False):	
		if material != None:
			self.material = material
			if preInit == False:
				self.objData["material"] = material.name
				
			self.objEditor.materialButton.setTitle(material.name)
			self.objEditor.setImage(material.image)
	def setGroup(self, group):
		self.objData["group"] = group
		self.objEditor.objectGroupButton.setTitle(group)
	def renderPointsPolygons(self):
		""" Export Renderman PointsPolygons object. """
		mesh = self.object.getData(False, True)
		points = []
		normals = []
		for v in mesh.verts:
			points.append(v.co)
			normals.append(v.no)
			# print v.index

		Cs = range(len(mesh.verts)) 
		# params = {"P":points}	
		nfaces = len(mesh.faces)
		# print nfaces, " faces found."
		nverts = []
		vertids = []
		st = range(len(mesh.verts))
		for face in mesh.faces:
			nverts.append(len(face.v))
			if mesh.vertexColors == 1:
				if len(face.v) > 2:
					for vertIdx in range(len(face.v)):
						Cs[face.v[vertIdx].index] = face.col[vertIdx] # should actually average the vert color across
			if mesh.faceUV == 1:
				if len(face.v) > 2:
					for vertIdx in range(len(face.v)):
						st[face.v[vertIdx].index] = [face.uv[vertIdx][0], 1.0 - face.uv[vertIdx][1]]
			#else:
				#if len(mesh.faces[0].uv) != 0:
				#	vtuv = []
				#	for vertIdx in range(len(face.v)):
				#		uv = face.uv[vertIdx]
				#		uv = uv[0], 1.0 - uv[1]
				#		vertTexUV[face.v[vertIdx].index] = uv
			if mesh.vertexUV:				
				pass
			fVerts = []
			

			for v in face.v:
				vertids.append(v.index)
				fVerts.append(v.index)
			# print "Face verts: ", fVerts
			
		params = {"P":points, "N":normals}
			
		if mesh.faceUV == 1:
			#print st
			params["st"] = st
		elif mesh.vertexColors == 1: 
			vCol = []
			for vertCol in Cs:
				vCol.append([vertCol.r / 256.0, vertCol.g / 256.0, vertCol.b / 256.0])
			#print Cs
			params["Cs"] = vCol
		else:
			#print vertTexUV
			# params["st"] = vertTexUV
			pass
		#print nfaces
		#print nverts
		#print vertids
		#print params
		ri.RiPointsGeneralPolygons(nfaces*[1], nverts, vertids, params)
			
		
	def renderSubdivMesh(self):
		""" Export Subdivision mesh. """
		mesh = self.object.getData(False, True)
		modifiers = self.object.modifiers

		points = []
		normals = []
		uv = []
		faceModes = []
		st = []
		
				
		# get the verts by ID		
		vertTexUV = []
		for vert in mesh.verts:
			points.append(vert.co)
			normals.append(vert.no)		
			if mesh.faceUV == 0:
				vertTexUV.append(0)
			
		nfaces = len(mesh.faces)
		nverts = []
		vertids = []
		
		Cs = range(len(mesh.verts)) 
		st = range(len(mesh.verts))
		# get the faces by vertex ID
		for face in mesh.faces:
			nverts.append(len(face.v))
			if mesh.vertexColors == 1:
				if len(face.v) > 2:
					for vertIdx in range(len(face.v)):
						Cs[face.v[vertIdx].index] = face.col[vertIdx] # should actually average the vert color across
			if mesh.faceUV == 1:
				if len(face.v) > 2:
					for vertIdx in range(len(face.v)):
						st[face.v[vertIdx].index] = [face.uv[vertIdx][0], 1.0 - face.uv[vertIdx][1]]
			#else:
			#	if len(mesh.faces[0].uv) != 0:
			#		vtuv = []
			#		for vertIdx in range(len(face.v)):
			#			uv = face.uv[vertIdx]
			#			uv = uv[0], 1.0 - uv[1]
			#			vertTexUV[face.v[vertIdx].index] = uv
						
			for vert in face.v:
				vertids.append(vert.index)
		
		# get the creases
		creases = {}
		# develop a list of creases based on crease value.
		for edge in mesh.edges:		
			if edge.crease > 0:
				if edge.crease not in creases:
					creases[edge.crease] = []
				creases[edge.crease].append([edge.v1.index, edge.v2.index])
		
		creaselist = []
		for crease in creases: # for each crease group, create a set of vertices and merge 
			verts = []
			i_set = Set()
			setlist = []
			edgelist = creases[crease]
			for edge in edgelist:
				i_set.add(edge[0])
				i_set.add(edge[1])
			
			for item in i_set:
				set = Set()
				set.add(item)
				setlist.append(set)
			
			for edge in edgelist:
				seta = self.find_set(edge[0], setlist)
				if edge[1] not in seta:
					setb = self.find_set(edge[1], setlist)
					newset = self.merge_set(seta, setb)
					setlist.remove(seta)
					setlist.remove(setb)
					setlist.append(newset)
			# print "Creases for crease level: ", crease, " are ", setlist
			
			for item in setlist:
				creaselist.append([crease, item]) # this will add to the flat list of crease objects that I need.

		# don't forget reference geometry. I need the base mesh before lattice/armature transforms are applied to it.
		# I can probably disable all modifiers except for decimate and gather that mesh, 
		# then turn them all back on (excepting subsurf) and gather *that* mesh
		# then I do
		# params["PRef"] =  refPoints
		# for all the other stuff, I also need
		# params["Cs"] = vertColors  # per vertex colors
		# params["Cs"] = vertCoors # or Face UV colors		
		# params["FModes"] = faceModes # face display modes to pass to custom shaders
		
		tags = []
		nargs = []
		intargs = []
		floatargs = []
		
		for crease in creaselist:
			# print crease
			tags.append("crease")
			nargs.append(len(crease[1]))
			nargs.append(1)
			for item in crease[1]:
				intargs.append(item)
			
			val = (float(crease[0]) / 255) * 5.0
			floatargs.append(val) # normalized currently for the Aqsis renderer
		tags.append("interpolateboundary")
		nargs.append(0)
		nargs.append(0)
			
		params = {"P":points, "N":normals}
			
		if mesh.faceUV == 1:
			params["st"] = st
		elif mesh.vertexColors == 1: 
			vCol = []
			for vertCol in Cs:
				vCol.append([vertCol.r / 256.0, vertCol.g / 256.0, vertCol.b / 256.0])
			#print Cs
			params["Cs"] = vCol
		else:
			pass
			#params["st"] = vertTexUV
			
		if 1 == 2: 
			print "nfaces: ", nfaces
			print "nverts: ", nverts
			print "vertids: ", vertids
			print "ntags: ", len(tags)
			print "tags: ", tags
			print "nargs: ", nargs
			print "intargs: ", intargs
			print "floatargs: ", floatargs
			print "params: ", params
			
		
		# and now to build the call	
		if mesh.faceUV == 1:
			ri.RiDeclare("st", "facevarying float[2]") # declare ST just in case
			
		subdiv = ri.RiSubdivisionMesh("catmull-clark", nverts, vertids, tags, nargs, intargs, floatargs, params)
				
	def find_set(self, val, set_list):
		""" Find a set in a set"""
		for set in set_list:
			if val in set:
				return set
				
	def merge_set(self, seta, setb):
		""" merge two sets """
		return seta.union(setb)

		
class LampAdapter(ObjectAdapter):
	""" BtoR Lamp Adapter object """
	protocols.advise(instancesProvide=[IObjectAdapter], asAdapterForTypes=[BtoRLamp])
	def __init__(self, object):
		""" Initialize a Lamp export adapter """
		
		ObjectAdapter.__init__(self, object)	
		self.isAnimated = False 
	def render(self): 
		""" Generate renderman data for this object. """
		
		#params = {}
		#for param in self.shader.shader.params()
			# get the value of each
		#	params[param] = getattr(self.shader.shader, param)
		#	ri.RiSurface(material.surfaceShaderName(), material.surfaceShaderParams(0))
		ri.RiLightSource(self.shader.shader.shadername, self.shader.shader.params())
		
	def doCameraTransform(self, axis):
		if axis != None:
			# I still need to transform based on the light's matrix
			# get the inverse matrix first
			# if this is point light, I should probably set the rotation values to zero
			cmatrix = self.object.getMatrix()
			sMat = Blender.Mathutils.ScaleMatrix(-1, 4)
			
			mat = cmatrix * sMat

			ri.RiTransform(mat)
			
			ri.RiRotate(180, 1, 0, 0) # things are looking up!
			# do a rotation along some axis or another			
			if axis == "px":				
				ri.RiRotate(-90, 0, 1, 0)
			elif axis == "nx":
				ri.RiRotate(90, 0, 1, 0)				
			elif axis == "py":				
				ri.RiRotate(90, 1, 0, 0)
			elif axis == "ny":
				ri.RiRotate(-90, 1, 0, 0)				
				#ri.RiRotate(180, 1, 0, 0)
			elif axis == "nz":				
				ri.RiRotate(180, 1, 0, 0) 	
		else:			
			cmatrix = self.object.getMatrix()
			matrix = [cmatrix[0][0],
					cmatrix[0][1],
					-cmatrix[0][2],
					cmatrix[0][3],
					cmatrix[1][0],
					cmatrix[1][1],
					-cmatrix[1][2],
					cmatrix[1][3],
					cmatrix[2][0],
					cmatrix[2][1],
					-cmatrix[2][2],
					cmatrix[2][3],
					cmatrix[3][0],
					cmatrix[3][1],
					-cmatrix[3][2],
					cmatrix[3][3]]
			# otherwise, do nothing
			ri.RiTransform(matrix)
			# ri.RiTranslate(0, 0, 1) # dunno if this is neccessary here or not.
			
	def getRenderProjection(self):
		if self.object.getData().getType() == 0:
			projection = "perspective"
		elif self.object.getData().getType() == 1:
			projection = "orthographic"
		elif self.object.getData().getType() == 2:
			projection = "perspective"
		else: 
			projection = "perspective"
		return projection
			
	def setShadowParms(self, params):
		shadername = self.shader.shader.shadername
		if shadername == "shadowpoint":
			setattr(self.shader.shader, "sfpx", params["px"]["shadowName"])
			print "Shadowmap: ", getattr(self.shader.shader, "sfpx")
			setattr(self.shader.shader, "sfpy", params["py"]["shadowName"])
			print "Shadowmap: ", getattr(self.shader.shader, "sfpy")
			setattr(self.shader.shader, "sfpz", params["pz"]["shadowName"])
			print "Shadowmap: ", getattr(self.shader.shader, "sfpz")
			setattr(self.shader.shader, "sfnx", params["nx"]["shadowName"])
			print "Shadowmap: ", getattr(self.shader.shader, "sfnx")
			setattr(self.shader.shader, "sfny", params["ny"]["shadowName"])
			print "Shadowmap: ", getattr(self.shader.shader, "sfny")
			setattr(self.shader.shader, "sfnz", params["nz"]["shadowName"])
			print "Shadowmap: ", getattr(self.shader.shader, "sfnz")
		elif shadername == "shadowspot" or shadername == "shadowdistant" or shadername == "bml":
			setattr(self.shader.shader, "shadowname", params["shadow"]["shadowName"])
			
		
	def getRenderDirections(self):
		if self.object.getData().getType() == 0:
			return ["px", "py", "pz", "nx", "ny", "nz"]
		elif self.object.getData().getType() == 1:			
			# determine how to bring the shadow map back here
			return ["shadow"]
		elif self.object.getData().getType() == 2:
			# figure out how to render the direction this light is pointing
			return ["shadow"]
		
	def initObjectData(self):
		self.objData = {}
		self.objData["name"] = self.object.getName()
		self.objData["type"] = self.object.getType()
		self.shader_type = "light"
		
		# figure out the type of lamp, and accordingly use the basic types available via BML
		
		shaderParms = {}
		lamp = self.object.getData()
		x = self.object.matrix[3][0] / self.object.matrix[3][3]
		y = self.object.matrix[3][1] / self.object.matrix[3][3]
		z = self.object.matrix[3][2] / self.object.matrix[3][3]
		tox = -self.object.matrix[2][0] + self.object.matrix[3][0]
		toy = -self.object.matrix[2][1] + self.object.matrix[3][1]
		toz = -self.object.matrix[2][2] + self.object.matrix[3][2]
		if lamp.getMode() & lamp.Modes['Negative']:
			negative = -1
		else:
			negative = 1
		
		# am I going to worry about intensity and color? Maybe not, because that will change per shader I think.
		# perhaps I should gather the parameters for the bml shader and use that as my primary lighting shader.
		shaderParms["intensity"] = lamp.getEnergy()
		# I'm only worried at the moment about deriving the correct shader to use as a starting point for the lamp.
		# thus
		shaderParms["from"] = [x, y, z]
		shaderParms["lightcolor"] = [lamp.R, lamp.G, lamp.B]
		self.objData["lightcolor"] = [lamp.R, lamp.G, lamp.B]
		if lamp.type == 0 or lamp.type == 4:
			self.objData["type"] = 0
			energyRatio = lamp.dist * negative
			# get the first selected shader path...and hope it's setup correctly
			if self.settings.use_slparams:
				shaderPath = self.settings.shaderpaths.getValue().split(";")[0]
				if self.settings.useShadowMaps.getValue():
					sFilename = "shadowpoint.sl"
				else:
					sFilename = "pointlight.sl"
				self.objData["shaderfilename"] = os.path.normpath(shaderPath + os.sep + sFilename)
				# I'm only really concerned about this if I'm using sl params
			self.objData["shadername"] = "pointlight"			
			shaderParms["intensity"] = (energyRatio * lamp.energy) * self.scene.lightMultiplier
			
		elif lamp.type == 1:
			self.objData["type"] = 1
			energyRatio = negative
			self.objData["shadername"] = "distantlight"			
			if self.settings.use_slparams:
				shaderPath = self.settings.shaderpaths.getValue().split(";")[0]
				if self.settings.useShadowMaps.getValue():
					sFilename = "shadowdistant.sl"
				else:
					sFilename = "distantlight.sl"
				self.objData["shaderfilename"] = os.path.normpath(shaderPath + os.sep + sFilename)

			shaderParms["to"] = [ tox, toy, toz]
			
			
			# parms for shadowMapping
			#shaderParms["name"] = "distantshadow"
			#shaderParms["shadowname"] = "shadow"
			#shaderParms["shadowsamples"] = 1			
			#shaderParms["shadowblur"] = 0.0
			
		elif lamp.type == 2:
			self.objData["type"] = 2
			energyRatio = lamp.dist * negative
			self.objData["shadername"] = "bml"
			if self.settings.use_slparams:				
				shaderPath = self.settings.shaderpaths.getValue().split(";")[0]
				#if self.settings.useShadowMaps.getValue():
				#	sFilename = "shadowspot.sl"
				#else:
				sFilename = "bml.sl"
				self.objData["shaderfilename"] = os.path.normpath(shaderPath + os.sep + sFilename)
			
			shaderParms["shadowbias"] = 1
			shaderParms["blur"] = 0.0
			shaderParms["samples"] = 1
			shaderParms["coneangle"] = (lamp.spotSize * math.pi / 360)
			shaderParms["conedeltaangle"] = (lamp.spotBlend * (lamp.spotSize * math.pi / 360))			
			shaderParms["to"] = [tox, toy, toz]
			shaderParms["intensity"] = (energyRatio * lamp.energy) * self.scene.lightMultiplier	
			# This might need to be animated, so I need to add a function to deal with that
			if self.settings.useShadowMaps.getValue():
				shaderParms["shadowname"] = self.object.getName() + ".tx"
			else:
				shaderParms["shadowname"] = None
		elif lamp.type == 3:
			self.objData["type"] = 3
			energyRatio = negative
			self.objData["shadername"] = "hemilight"
			if self.settings.use_slparams:
				shaderPath = self.settings.shaderpaths.getValue().split(";")[0]
				self.objData["shaderfilename"] = os.path.normpath(shaderPath + os.sep + "hemilight.sl")

			shaderParms["to"] = [tox, toy, toz]
			shaderParms["falloff"] = 0
			shaderParms["intensity"] = energyRatio * lamp.energy
			
		self.objData["shaderparms"] = shaderParms
		
		self.initShader() # initialize my light shader
				
	def checkReset(self):
		
		# here I simply want to check if the lamp settings need changing or not.
		# if the user has selected a shader type that doesn't match the lamp settings, all I want to affect in that case
		# is the light color.
		
		shaderParms = self.objData["shaderparms"]
		
		lamp = self.object.getData()		
		
		# for the most part, follow the parameters for the given light object and stuff the values into the 
		# shader parms 
		if self.objData["type"] == lamp.type:
			mat = self.object.getMatrix()
			#trans = mat.translationPart()
			#x = trans[0]
			#y = trans[1]
			#z = trans[2]
			
			x = self.object.matrix[3][0] / self.object.matrix[3][3]
			y = self.object.matrix[3][1] / self.object.matrix[3][3]
			z = self.object.matrix[3][2] / self.object.matrix[3][3]
			
			tox = -self.object.matrix[2][0] + self.object.matrix[3][0]
			toy = -self.object.matrix[2][1] + self.object.matrix[3][1]
			toz = -self.object.matrix[2][2] + self.object.matrix[3][2]
			if lamp.getMode() & lamp.Modes['Negative']:
				negative = -1
			else:
				negative = 1
			
			# am I going to worry about intensity and color? Maybe not, because that will change per shader I think.
			shaderParms["intensity"] = lamp.getEnergy()
			# I'm only worried at the moment about deriving the correct shader to use as a starting point for the lamp.
			# thus
			shaderParms["from"] = [x, y, z]
			shaderParms["lightcolor"] = [lamp.R, lamp.G, lamp.B]
			self.objData["lightcolor"] = [lamp.R, lamp.G, lamp.B]
			if (lamp.type == 0 or lamp.type == 4) and self.shader.getShaderName() == "pointlight":
				energyRatio = lamp.dist * negative
				# get the first selected shader path...and hope it's setup correctly
				shaderParms["intensity"] = (energyRatio * lamp.energy) * self.scene.lightMultiplier
				
				# I will have to deal with shadowmapping at some point
				#shaderParms["name"] = "shadowpoint"
			elif lamp.type == 1 and self.shader.getShaderName() == "distantlight":
				energyRatio = negative
				self.objData["shadername"] = "distantlight"			

				shaderParms["to"] = [ tox, toy, toz]
							
				# parms for shadowMapping
				#shaderParms["name"] = "distantshadow"
				#shaderParms["shadowname"] = "shadow"
				#shaderParms["shadowsamples"] = 1			
				#shaderParms["shadowblur"] = 0.0
				
			elif lamp.type == 2 and self.shader.getShaderName() == "bml":
				energyRatio = lamp.dist * negative
				shaderParms["shadowbias"] = 1
				shaderParms["blur"] = 0.0
				shaderParms["samples"] = 1
				shaderParms["coneangle"] = (lamp.spotSize * math.pi / 360)
				shaderParms["conedeltaangle"] = (lamp.spotBlend * (lamp.spotSize * math.pi / 360))			
				shaderParms["to"] = [tox, toy, toz]
				shaderParms["intensity"] = (energyRatio * lamp.energy) * self.scene.lightMultiplier			
				# shaderParms["shadowname"] = None # ignore the shadow name. if that's been set, I want to keep the value
							
			elif lamp.type == 3 and self.shader.getShaderName() == "hemilight":
				energyRatio = negative
				shaderParms["to"] = [tox, toy, toz]
				shaderParms["falloff"] = 0
				shaderParms["intensity"] = energyRatio * lamp.energy
			
			self.objData["shaderparms"] = shaderParms
			
			# and reset the light color.
			self.objData["lightcolor"] = [lamp.R, lamp.G, lamp.B]
			
			for key in shaderParms:				
				self.shader.setParamValue(key, shaderParms[key])
			self.objData["reset"] = False
		else:			
			self.initObjectData()
			self.objData["reset"] = True
		


		
class MBallAdapter(ObjectAdapter):
	protocols.advise(instancesProvide=[IObjectAdapter], asAdapterForTypes=[BtoRMBall])
	def __init__(self, object):
		""" Initialize a metaball export adapter """
		ObjectAdapter.__init__(self, object)					
		
	def render(self):
		""" generate Renderman data for this object """
		pass
	def initObjectData(self):
		""" Initialize BtoR object data for this object """
		pass
		
class CurveAdapter(ObjectAdapter):
	""" Curve export adapter """
	protocols.advise(instancesProvide=[IObjectAdapter], asAdapterForTypes=[BtoRCurve])
	def __init__(self, object):
		""" initialize a CurveAdapter """
		ObjectAdapter.__init__(self, object)	
		self.object = object
		
	def render(self):
		""" generate Renderman data for this object """
		pass
	def initObjectData(self):
		""" Initialize BtoR object data for this object """
		pass
		
class SurfaceAdapter(ObjectAdapter):
	protocols.advise(instancesProvide=[IObjectAdapter], asAdapterForTypes=[BtoRSurf])
	def __init__(self, object):		
		""" Iniitialize a surface export adapter """
		ObjectAdapter.__init__(self, object)					
		self.editorPanel.title = "Surface Export Settings:"

	def render(self):
		""" generate Renderman data for this object """
		pass
	def initObjectData(self):
		""" Initialize BtoR object data for this object """
		pass


class CameraAdapter(ObjectAdapter):
	protocols.advise(instancesProvide=[IObjectAdapter], asAdapterForTypes=[BtoRCamera])
	def __init__(self, object):		
		ObjectAdapter.__init__(self, object)					
				
	def render(self):
		""" generate Renderman data for this object """		
		
		# self.objEditor.shaderButton.title = self.imagerShader.shader_menu.getValue()
		if self.shader.shader != None:
			if self.shader.getShaderName() != None:
				self.shader.updateShaderParams()
				ishader = self.shader.shader
				ri.RiImager(ishader.shadername, ishader.params()) # and done
			else:
				bWorld = Blender.World.GetCurrent()
				if bWorld != None: 
					if bWorld.hor != [0, 0, 0]:
						iparams = { "bgcolor" : [bWorld.hor[0], bWorld[1], bWorld[2]] }
						ri.RiImager( "background", iparams )
					
		scene = Blender.Scene.GetCurrent()
		render = scene.getRenderingContext()
		camera = self.object.getData()
		cType = camera.getType()
		ri.RiFormat(render.imageSizeX(), render.imageSizeY(), 1) # leave aspect at 1 for the moment
		
		if cType == 0:	
			factor = render.imageSizeX() / render.imageSizeY()
			fov = 360.0 * math.atan((factor * 16.0) / camera.lens) / math.pi
			ri.RiProjection("perspective", "fov", fov)
			# Depth of field
			print self.properties
			if self.properties["DOF"].getValue():
				ri.RiDepthOfField(self.properties["fstop"].getValue(), self.properties["focallength"].getValue(), self.properties["focaldistance"].getValue())
			# Depth of field done
			
		else:
			self.objData["scale"] = camera.getScale() 
			ri.RiProjection("orthographic") 
		ri.RiFrameAspectRatio(factor)	
		# Camera clipping
		ri.RiClipping(camera.getClipStart(), camera.getClipEnd())
		
		# Viewpoint transform
		cmatrix = self.object.getInverseMatrix()
		#cmatrix = self.object.getMatrix()
		sMat = Blender.Mathutils.ScaleMatrix(-1, 4)
		
		mat = cmatrix * sMat
		matrix = [cmatrix[0][0],
				cmatrix[0][1],
				-cmatrix[0][2],
				cmatrix[0][3],
				cmatrix[1][0],
				cmatrix[1][1],
				-cmatrix[1][2],
				cmatrix[1][3],
				cmatrix[2][0],
				cmatrix[2][1],
				-cmatrix[2][2],
				cmatrix[2][3],
				cmatrix[3][0],
				cmatrix[3][1],
				-cmatrix[3][2],
				cmatrix[3][3]]
		ri.RiTransform(matrix)
		#ri.RiRotate(180, 1, 0, 0)
		ri.RiTranslate(0, 0, 1)
		# that should take care of the camera
		
	def initObjectData(self):
		# object initia.ize
		""" Initialize BtoR object data for this object """
		self.objData = {}
		self.objData["name"] = self.object.getName()
		self.objData["type"] = self.object.getType() 
		self.shader_type = "imager"
		shaderParams = {}
		shaderPath = self.settings.shaderpaths.getValue().split(";")[0]
		self.objData["shaderfilename"] = os.path.normpath(shaderPath + os.sep + "background.sl")
		bWorld = Blender.World.GetCurrent()
		shaderParams["bgcolor"] = [bWorld.hor[0], bWorld.hor[1], bWorld.hor[2]]
		self.objData["shaderparams"] = shaderParams
		self.initShader()
class PreviewAdapter(ObjectAdapter):
	protocols.advise(instancesProvide=[IObjectAdapter], asAdapterForTypes=[BtoRPreview])
	def __init__(self, object):
		self.object = object
		
	def initObjectData(self):
		# what does a preview object need?
		pass
	def getInfo(self):
		pass
	def loadData(self):
		pass
	def saveData(self):
		pass
	def render(self):
		# what do I do here? Just push out the same verts that I need
		# this is a special case anyway
		if self.isSubdiv:
			self.renderSubdivMesh()
		else:
			self.renderPointsPolygons()
			
	def renderPointsPolygons(self):
		""" Export Renderman PointsPolygons object. """
		mesh = self.object.obj
		points = []
		normals = []
		for v in mesh.verts:
			points.append(v.co)
			normals.append(v.no)
			# print v.index

		Cs = range(len(mesh.verts)) 
		# params = {"P":points}	
		nfaces = len(mesh.faces)
		# print nfaces, " faces found."
		nverts = []
		vertids = []
		for face in mesh.faces:
			nverts.append(len(face.v))
			if mesh.vertexColors == 1:
				if len(face.v) > 2:
					for vertIdx in range(len(face.v)):
						Cs[face.v[vertIdx].index] = face.col[vertIdx] # should actually average the vert color across
			if mesh.faceUV == 1:
				if len(face.v) > 2:
					for vertIdx in range(len(face.v)):
						st[face.v[vertIdx].index] = [face.uv[vertIdx][0], 1.0 - face.uv[vertIdx][1]]
			#else:
				#if len(mesh.faces[0].uv) != 0:
				#	vtuv = []
				#	for vertIdx in range(len(face.v)):
				#		uv = face.uv[vertIdx]
				#		uv = uv[0], 1.0 - uv[1]
				#		vertTexUV[face.v[vertIdx].index] = uv
			if mesh.vertexUV:				
				pass
			fVerts = []
			

			for v in face.v:
				vertids.append(v.index)
				fVerts.append(v.index)
			# print "Face verts: ", fVerts
			
		params = {"P":points, "N":normals}
			
		if mesh.faceUV == 1:
			#print st
			params["st"] = st
		elif mesh.vertexColors == 1: 
			vCol = []
			for vertCol in Cs:
				vCol.append([vertCol.r / 256.0, vertCol.g / 256.0, vertCol.b / 256.0])
			#print Cs
			params["Cs"] = vCol
		else:
			#print vertTexUV
			# params["st"] = vertTexUV
			pass

		ri.RiPointsGeneralPolygons(nfaces*[1], nverts, vertids, params)
		
		
	def renderSubdivMesh(self):
		""" Export Subdivision mesh. """
		mesh = self.object.obj

		points = []
		normals = []
		uv = []
		faceModes = []
		st = []
		
				
		# get the verts by ID		
		vertTexUV = []
		for vert in mesh.verts:
			points.append(vert.co)
			normals.append(vert.no)		
			if mesh.faceUV == 0:
				vertTexUV.append(0)
			
		nfaces = len(mesh.faces)
		nverts = []
		vertids = []
		
		Cs = range(len(mesh.verts)) 
		# get the faces by vertex ID
		for face in mesh.faces:
			nverts.append(len(face.v))
			if mesh.vertexColors == 1:
				if len(face.v) > 2:
					for vertIdx in range(len(face.v)):
						Cs[face.v[vertIdx].index] = face.col[vertIdx] # should actually average the vert color across
			if mesh.faceUV == 1:
				if len(face.v) > 2:
					for vertIdx in range(len(face.v)):
						st[face.v[vertIdx].index] = [face.uv[vertIdx][0], 1.0 - face.uv[vertIdx][1]]
			#else:
			#	if len(mesh.faces[0].uv) != 0:
			#		vtuv = []
			#		for vertIdx in range(len(face.v)):
			#			uv = face.uv[vertIdx]
			#			uv = uv[0], 1.0 - uv[1]
			#			vertTexUV[face.v[vertIdx].index] = uv
						
			for vert in face.v:
				vertids.append(vert.index)
		
		# get the creases
		creases = {}
		# develop a list of creases based on crease value.
		for edge in mesh.edges:		
			if edge.crease > 0:
				if edge.crease not in creases:
					creases[edge.crease] = []
				creases[edge.crease].append([edge.v1.index, edge.v2.index])
		
		creaselist = []
		for crease in creases: # for each crease group, create a set of vertices and merge 
			verts = []
			i_set = Set()
			setlist = []
			edgelist = creases[crease]
			for edge in edgelist:
				i_set.add(edge[0])
				i_set.add(edge[1])
			
			for item in i_set:
				set = Set()
				set.add(item)
				setlist.append(set)
			
			for edge in edgelist:
				seta = self.find_set(edge[0], setlist)
				if edge[1] not in seta:
					setb = self.find_set(edge[1], setlist)
					newset = self.merge_set(seta, setb)
					setlist.remove(seta)
					setlist.remove(setb)
					setlist.append(newset)
			# print "Creases for crease level: ", crease, " are ", setlist
			
			for item in setlist:
				creaselist.append([crease, item]) # this will add to the flat list of crease objects that I need.

		# don't forget reference geometry. I need the base mesh before lattice/armature transforms are applied to it.
		# I can probably disable all modifiers except for decimate and gather that mesh, 
		# then turn them all back on (excepting subsurf) and gather *that* mesh
		# then I do
		# params["PRef"] =  refPoints
		# for all the other stuff, I also need
		# params["Cs"] = vertColors  # per vertex colors
		# params["Cs"] = vertCoors # or Face UV colors		
		# params["FModes"] = faceModes # face display modes to pass to custom shaders
		
		tags = []
		nargs = []
		intargs = []
		floatargs = []
		
		for crease in creaselist:
			# print crease
			tags.append("crease")
			nargs.append(len(crease[1]))
			nargs.append(1)
			for item in crease[1]:
				intargs.append(item)
			
			val = (float(crease[0]) / 255) * 5.0
			floatargs.append(val) # normalized currently for the Aqsis renderer
		tags.append("interpolateboundary")
		nargs.append(0)
		nargs.append(0)
			
		params = {"P":points, "N":normals}
			
		if mesh.faceUV == 1:
			params["st"] = st
		elif mesh.vertexColors == 1: 
			vCol = []
			for vertCol in Cs:
				vCol.append([vertCol.r / 256.0, vertCol.g / 256.0, vertCol.b / 256.0])
			#print Cs
			params["Cs"] = vCol
		else:
			pass
			#params["st"] = vertTexUV
			
		if 1 == 2: 
			print "nfaces: ", nfaces
			print "nverts: ", nverts
			print "vertids: ", vertids
			print "ntags: ", len(tags)
			print "tags: ", tags
			print "nargs: ", nargs
			print "intargs: ", intargs
			print "floatargs: ", floatargs
			print "params: ", params
			
		
		# and now to build the call	
		if mesh.faceUV == 1:
			ri.RiDeclare("st", "facevarying float[2]") # declare ST just in case
			
		subdiv = ri.RiSubdivisionMesh("catmull-clark", nverts, vertids, tags, nargs, intargs, floatargs, params)
class ObjectUI:
	""" Object editor panel """
	protocols.advise(instancesProvide=[IObjectUI], asAdapterForTypes=[BtoRLattice, BtoRArmature, BtoRBasicObject, BtoREmpty, BtoRWave])
	options = {
		"Blank": ["Blank Property", False]
	}
	def __init__(self, obj):
		dict = globals()	
		self.settings = dict["instBtoRSettings"]
		self.evt_manager = dict["instBtoREvtManager"]
		self.scene = dict["instBtoRSceneSettings"]
		self.materials = dict["instBtoRMaterials"]
		self.objecteditor = dict["instBtoRObjects"]
		self.grouplist = dict["instBtoRGroupList"]
		self.helpwindow = dict["instBtoRHelp"]
		self.mat_selector = self.materials.getSelector()
		
		self.editorPanel = ui.Panel(4, 65, 491,  320, "Empty Panel", "", None, False)
		self.editorPanel.titleColor = [65, 65, 65, 255]
		self.editorPanel.hasHeader = False
		self.editorPanel.cornermask = 0
		self.editorPanel.shadowed = False
		self.editorPanel.outlined = False

		self.scroller= ui.ScrollPane(235, 25, 240, 260, "Scroller", "Scroller", self.editorPanel, True)
		self.properties = {}
		self.editors = {}
		for option in self.options:
			propertyName = self.options[option][0]
			propertyValue = self.options[option][1]
			# generate a list of option panels here and allow editing
			# create a property for each option
			self.properties[option] = IProperty(propertyValue) # 1st item is the property name, second item is the property initializer
			self.properties[option].setName(propertyName)
			self.properties[option].setWidth(self.scroller.width - 15)			
			# takes up half the available space of the main pane
			self.editors[option] = IPropertyEditor(self.properties[option])
			self.scroller.addElement(self.editors[option].getEditor()) # and that should be that. When this is discarded, all those go away
			self.editors[option].setParent(self.scroller)	
			self.scroller.offset = 0



	def getEditor(self):
		""" get the object editor for this object. """
		return self.editorPanel
		


class MeshUI(ObjectUI):
	object_output_options = ["Mesh", "Renderman Primitive", "RA Proxy", "RA Procedural"]
	mesh_output_options = ["basic", "SubDiv"]
	protocols.advise(instancesProvide=[IObjectUI], asAdapterForTypes=[BtoRMesh])
	""" This is the object editor. Here you can assign materials, and set various options for the export. """
	options = { "AutoCrease" : ["Automatic Creasing?", True], 
			"RIBEntity" : ["Save as RIB Entity", False], 
			"IncludeMats" : ["Include Materials in RIB Entity", True],
			"DefineAsObj" : ["Define as RiObject", False],
			"InplaceInstance" : ["In-Place Instancing", False],
			"AutoRandomize" : ["Auto Randomize Material", False],
			"OutputOptions" : ["Object Output Options:", {"Mesh" : "mesh", "Renderman Primitive" : "primtive", "RA Proxy" : "proxy", "RA Procedural" : "procedural" }]}
				
	def __init__(self, obj):
		ObjectUI.__init__(self, obj)
		
		self.editorPanel.title = "Mesh Export Settings:"			
		self.editorPanel.addElement(ui.Label(10, 25, "Assigned Material", "Assigned Material:", self.editorPanel, False))
		
		self.materialButton = ui.Button(10, 45, 180, 65, "Material", "None Selected", 'large', self.editorPanel, True)
		self.materialButton.registerCallback("release", self.showMaterialSelector)
		self.materialButton.textlocation = 1
		
		self.modifyShaderParams = ui.CheckBox(10, 120, "Auto-Randomize Material?", "Auto-Randomize Material?", False, self.editorPanel, True)
		self.modifyShaderParmsButton = ui.Button(10, 145, 150, 25, "Select Parameters", "Select Parameters", 'normal', self.editorPanel, True)
		self.modifyShaderParmsButton.registerCallback("release", self.showParameterModifier)
	
		# self.editorPanel.addElement(ui.Label(10, 185, "Object Group:", "Object Group:", self.editorPanel, True))
		# self.objectGroupMenu = ui.Menu(25, 215, "Object Group Menu", "Object Groups", self.scene.object_groups.keys(), self.editorPanel, True) 					
		# self.objectGroupButton = ui.Button(95, 185, 145, 25, "Object Group", "None Selected", 'normal', self.editorPanel, True)
		# self.objectGroupButton.registerCallback("release", self.showGroupSelector)
		
		self.exportButton = ui.Button(self.editorPanel.width - 185, self.editorPanel.height - 25, 180, 25, "Export", "Export Object", 'normal', self.editorPanel, True)
		self.exportButton.registerCallback("release", self.showExport)
		
		#self.helpButton = ui.Button(10, self.editorPanel.height - 25, 180, 25, "Help", "Help", 'normal', self.editorPanel, True)
		# create a help callback
		#self.helpButton.registerCallback("release", self.showHelp)
		
		self.exportSettings = btor.BtoRMain.ExportSettings()
		self.exportSettings.export_functions.append(self.objecteditor.exportSingleObject) # this should do the trick, but should apply to every object that's exportable standalone
		
		self.helpText = """ This is a test. 
		Here are some lines.
		And more lines.
		and yet more! """
		
	def showParameterModifier(self, button):
		pass
	def showGroupSelector(self, button):
		self.evt_manager.addElement(self.grouplist.getEditor())
	def showHelp(self, button):
		print "Here is my docstring!", self.__doc__
		self.helpwindow.setText(self.helpText)
		self.evt_manager.addElement(self.helpwindow.getEditor())
	def setImage(self, image):
		buttonImage = ui.Image(120, 5, 56, 56, image, self.materialButton, False)
		self.materialButton.image = buttonImage

		
	def selectMeshOutputType(self, obj):
		""" selector method for output menu """
		if self.mesh_output_menu.getSelectedIndex == 0:
			self.mesh_output_type_label.isVisible = True
			self.mesh_output_type_menu.isVisible = True
		else:
			self.mesh_output_type_label.isVisible = False
			self.mesh_output_type_menu.isVisible = False		
	
	def showMaterialSelector(self, obj):
		""" Display a material selector window. """
		# I should have loaded materials here, so let's do this.
		self.evt_manager.addElement(self.mat_selector)

	def selectMaterial(self, obj, matName = None):
		""" material selection callback """
		self.evt_manager.removeElement(self.mat_selector)
		# the button returned has the material name!
		# So all i need to do now is...
		if matName != None:
			self.material = matName
			self.materialButton.setTitle(matName)
			self.materialButton.image = self.materials.getMaterial(matName).image
		else:
			self.material = obj.title
			# self.scene.object_data[self.objectName.getValue()]["material"] = obj.title # Assign the material to the object adapter
			# print "Assigned material ", self.scene.object_data[self.objectName.getValue()]["material"], " to ", self.objectName.getValue()
			self.materialButton.title = obj.title			
			self.materialButton.image = ui.Image(120, 5, 56, 56,  obj.image.image, self.materialButton, False)
		
	def showExport(self, obj):
		""" Display the mesh object export dialog. """
		self.evt_manager.addElement(self.exportSettings.getEditor())
		
	def setArchive(self, obj):
		if obj.getValue():
			self.includeMaterials.isVisible = True
		else:
			self.includeMaterials.isVisible = False
	def setDefine(self, obj):
		if obj.getValue():
			self.defineObjectVisGroup.show()
		else:
			self.defineObjectVisGroup.hide()
			
		
		

class LampUI(ObjectUI):
	light_output_options = ["LightSource", "AreaLightSource"]
	protocols.advise(instancesProvide=[IObjectUI], asAdapterForTypes=[BtoRLamp])
	options = {"IncludeWithAO" : ["Include light with AO?", False], 
			"GenShadowMap" : ["Generate Shadow Maps", True], 
			"ShadowMapSize" : ["Shadow Map Size", { "256" : 256, "512" : 512, "1024" : 1024, "2048" : 2048 }]}
	def __init__(self, obj):
		
		ObjectUI.__init__(self, obj)
		self.editorPanel.title = "Light Export Settings:"		
		self.editorPanel.addElement(ui.Label(10, 25, "Light Type:", "Light Type:", self.editorPanel, False))
		self.light_output_menu = ui.Menu(15 + self.editorPanel.get_string_width("Light Type:", 'normal'), 25, 150, 25, "Light Menu", self.light_output_options, self.editorPanel, True)
		self.editorPanel.addElement(ui.Label(10, 65, "Light Shader:", "Light Shader:", self.editorPanel, False))
		self.shaderButton = ui.Button(15 + self.editorPanel.get_string_width("Light Shader:", 'normal'), 65, 125, 25, "None Selected", "None Selected", 'normal', self.editorPanel, True)
		shadow = self.settings.useShadowMaps.getValue()

			
	def setShader(self, shader):
		self.shaderButton.removeCallbacks("release")
		self.shader = shader
		self.shaderButton.registerCallback("release", self.shader.showEditor)
			
		
class MBallUI(ObjectUI):
	protocols.advise(instancesProvide=[IObjectUI], asAdapterForTypes=[BtoRMBall])
	def __init__(self, obj):
		ObjectUI.__init__(self, obj)
		self.editorPanel.title = "Metaball Export Settings:"

		
class CurveUI(ObjectUI):
	""" A UI for the curve type """
	protocols.advise(instancesProvide=[IObjectUI], asAdapterForTypes=[BtoRCurve])
	def __init__(self,obj):
		ObjectUI.__init__(self, obj)
		self.editorPanel.title = "Curve Export Settings: "

class SurfaceUI(ObjectUI):
	""" A UI for the surface type """
	protocols.advise(instancesProvide=[IObjectUI], asAdapterForTypes=[BtoRSurf])
	def __init__(self, obj):
		ObjectUI.__init__(self, obj)
		self.editorPanel.title = "Surface Export Settings:"

		
class CameraUI(ObjectUI):
	""" A UI for the camera type """
	protocols.advise(instancesProvide=[IObjectUI], asAdapterForTypes=[BtoRCamera])
	options = { "DOF" : ["Use Depth of Field?", True], 
			"fstop" : ["F-stop", 22], 
			"focallength" : ["Focal Length", 45],
			"focaldistance" : ["Focal Distance:", 10] }
	def __init__(self, obj):
		ObjectUI.__init__(self, obj)		
		self.editorPanel.title = "Camera Export Settings:"
		
		self.editorPanel.addElement(ui.Label(10, 30, "Imager Shader:", "Imager Shader:", self.editorPanel, False))
		self.shaderButton = ui.Button(self.editorPanel.get_string_width("Imager Shader:", 'normal') + 15, 30, 125, 25, "Imager Shader", "None Selected", 'normal', self.editorPanel, True)		
		# self.shaderButton.registerCallback("release", self.shader.showEditor)
		
	def setShader(self, shader):
		self.shader = shader
		self.shaderButton.registerCallback("release", self.shader.showEditor)
		
		
class ShaderParamUI:
	protocols.advise(instancesProvide=[IShaderParamUI], asAdapterForTypes=[BtoRStringParam, BtoRArrayParam, BtoRMatrixParam])
	def __init__(self, obj):
		self.materialName = obj.getMaterialName()
		self.shader = obj.getShader()
		self.parameter = obj.getParameter()
		self.type = getType()
		self.editorPanel = ui.Panel(0, 0, 100, 300, "Parameter: %s" % parameter, "Parameter: %s" % parameter, None, False)		
		if type in ["String", "Matrix", "Array"]:			
			self.editorPanel.addElement(ui.Label(10, 25, "None", "This parameter type has no editor defined yet.", self.editorPanel, False))
			
	def getEditor(self):
		return self.editorPanel
	def getVariance(self, index):
		pass
	def calcRange(self, button):
		pass
	def setRangeLength(self, length):
		pass
		
class FloatShaderParamUI(ShaderParamUI):
	protocols.advise(instancesProvide=[IShaderParamUI], asAdapterForTypes=[BtoRFloatParam])
	def __init__(self, obj):
		ShaderParamUI.__init__(self, obj)
		self.materialName = obj.getMaterial()
		self.shader = obj.getShader()
		self.parameter = obj.getParameter()
		self.type = getType()
		self.rangeLength = rangeLength
		self.editorPanel.addElement(ui.Label(10, 25, "Range start:", "Range start:", self.editorPanel, False))		
		self.rangeStart = ui.TextField(85, 25, 85, 25, "RangeStart", 0.0, self.editorPanel, True)
		self.rangeStart.registerCallback("update", self.calcRange)
		self.editorPanel.addElement(ui.Label(10, 60, "Range end:", "Range end:", self.editorPanel, False))
		self.rangeEnd = ui.TextField(85, 60, 85, 25, "RangeEnd", 1.0, self.editorPanel, True)
		self.rangeEnd.registerCallback("update", self.calcRange)
		self.linearDist = ui.CheckBox(10, 85, "Linear Values", "Linear Values", True, self.editorPanel, True)
		self.randomDist = ui.CheckBox(10, 110, "Random Distribution", "Random values", False, self.editorPanel, True)
		self.editorPanel.addElement(ui.Label(10, 125, "Random Seed:", "Random Seed:", self.editorPanel, False))
		self.rSeed = ui.TextField(85, 125, 85, 25, "Random seed", 1, self.editorPanel, True)
		self.editorPanel.addElement(ui.Label(10, 140, "Increment:", "Increment", self.editorPanel, False))
		self.increment = ui.TextField(85, 140, 85, 25, "Increment", 0.5, self.editorPanel, True)		
		
		self.vRange = range(rangeLength)
		
	def randomize(self):
		# randomize the value for this item
		return random.uniform(self.rangeStart, self.rangeEnd)
		
	def getVariance(self, index):
		return self.vRange(index)
		
	def calcRange(self, button):
		start = float(self.rangeStart.getValue())
		end = float(self.rangeEnd.getValue())
		length = end - start
		inc = length / len(self.vRange)
		val = start
		if self.linearDist.getValue():
			for idx in range(len(self.vRange)):
				self.vRange[idx] = val
				val = val + inc
		else:
			# initialize the seed value. I need this so I can always get back the same information after a material's been saved and restored
			random.seed(float(self.rSeed.getValue()))
			for idx in range(len(self.vRange)):
				self.vRange[idx] = random.uniform(start, end)
			
	def setRangeLength(self, length):
		self.rangeLength = length
		self.vRange = range(length)
		
class VecShaderParamUI(ShaderParamUI):
	protocols.advise(instancesProvide=[IShaderParamUI], asAdapterForTypes=[BtoRColorParam, BtoRPointParam, BtoRVectorParam, BtoRNormalParam])
	def __init__(self, obj):
		ShaderParamUI.__init__(self, obj)
		
	def setup(self, materialName, shader, type, parameter, rangeLength):
		width = 45
		
		if type == "color":
			a = "R:"
			b = "G:"
			c = "B:"
		else:
			a = "X:"
			b = "Y:"
			c = "Z:"
			
		self.editorPanel.addElement(ui.Label(25, 0, "X Range start:", "Range start:", self.editorPanel, False))	
		self.editorPanel.addElement(ui.Label(90, 80, "X Range end:", "Range end:", self.editorPanel, False))	
		
		self.editorPanel.addElement(ui.Label(10, 25, "X", a, self.editorPanel, False))		
		self.xRangeStart = ui.TextField(25, 25, width, 25, "X RangeStart", 0.0, self.editorPanel, True)
		self.xRangeStart.registerCallback("update", self.calcRange)		
		
		self.xRangeEnd = ui.TextField(90, 25,width, 25, "X RangeEnd", 1.0, self.editorPanel, True)
		self.xRangeEnd.registerCallback("update", self.calcRange)
		self.xDelta = ui.CheckBox(110, 25, "Delta", "Delta", False, self.editorPanel, True)
		
		self.editorPanel.addElement(ui.Label(10, 60, "Y", b, self.editorPanel, True))
		self.yRangeStart = ui.TextField(25, 60, width, 25, "Y RangeStart", 0.0, self.editorPanel, True)
		self.yRangeStart.registerCallback("update", self.calcRange)
		
		self.yRangeEnd = ui.TextField(90, 60, width, 25, "Y RangeEnd", 1.0, self.editorPanel, True)
		self.yRangeEnd.registerCallback("update", self.calcRange)
		self.yDelta = ui.CheckBox(110, 60, "Delta", "Delta", False, self.editorPanel, True)
		
		self.editorPanel.addElement(ui.Label(10, 95, "Z:", c, self.editorPanel, True))
		self.zRangeStart = ui.TextField(25, 95, width, 25, "X RangeStart", 0.0, self.editorPanel, True)
		self.zRangeStart.registerCallback("update", self.calcRange)
		
		self.zRangeEnd = ui.TextField(90, 95, width, 25, "X RangeEnd", 1.0, self.editorPanel, True)
		self.zRangeEnd.registerCallback("update", self.calcRange)
		self.zDelta = ui.CheckBox(110, 95, "Delta", "Delta", False, self.editorPanel, True)
		
		self.linearDist = ui.CheckBox(10, 115, "Linear Values", "Linear Values", True, self.editorPanel, True)
		self.randomDist = ui.CheckBox(10, 140, "Random Distribution", "Random values", False, self.editorPanel, True)
		self.editorPanel.addElement(ui.Label(10, 155, "Random Seed:", "Random Seed:", self.editorPanel, False))
		self.rSeed = ui.TextField(85, 155, width, 25, "Random seed", 1, self.editorPanel, True)
		
		self.vRange = range(rangeLength)
		for idx in range(rangeLength):
			self.vRange[idx] = [] # should I return a vec3 here?
			
	def getVariance(self, index):
		return self.vRange(index)
		
	def calcRange(self, button):
		start = float(self.rangeStart.getValue())
		end = float(self.rangeEnd.getValue())
		length = end - start
		inc = length / len(self.vRange)
		val = start
		
		if self.linearDist.getValue():
			for idx in range(len(self.vRange)):
				self.vRange[idx] = val
				val = val + inc
				
		else:
			# initialize the seed value. I need this so I can always get back the same information after a material's been saved and restored
			random.seed(float(self.rSeed.getValue()))
			for idx in range(len(self.vRange)):
				vec= []
				vec.append(randon.uniform(start, end))
				vec.append(random.uniform(start, end))
				vec.append(random.uniform(start, end))
				self.vRange[idx] 
			
	def setRangeLength(self, length):
		self.vRange = range(length)
		
		# BtoR-Specific objects
class ShaderParamEditor(ui.UIElement):
	protocols.advise(instancesProvide=[IShaderParamEditor])
	def __init__(self, bObj):
		self.obj = bObj # this is my incoming BtoRType
		self.value = bObj.value
		size = bObj.size
		self.name = bObj.name		
		self.parent = bObj.parent
		self.param_name = bObj.name
		ui.UIElement.__init__(self, size[0], size[1], size[2], self.height, self.name, '', bObj.parent, False)				
		self.addElement(ui.Label(5, 5, "Parameter: " + self.name, "Parameter: " + self.name, self, False))
		
	def getValue(self):
		return self.value
	def setValue(self, value):
		self.value = value
class ColorParamEditor(ShaderParamEditor):
	height = 60
	ColorSpaces = ['rgb', 'hsv', 'hsl', 'YIQ', 'xyz', 'xyY']
	paramtype = "color"
	protocols.advise(instancesProvide=[IShaderParamEditor], asAdapterForTypes=[BtoRColorParam])
	def __init__(self, bObj):		
		ShaderParamEditor.__init__(self, bObj)
		iVal = self.value
		self.value = []
		self.value.append(iVal[0])
		self.value.append(iVal[1])
		self.value.append(iVal[2]) # convert to an array I suppose
		
		self.Red = ui.TextField(25, 25, 35, 20, 'Red', self.value[0], self, True)
		self.Green = ui.TextField(85, 25, 35, 20, 'Green', self.value[1], self, True)
		self.Blue = ui.TextField(145, 25, 35, 20, 'Blue', self.value[2], self, True)
		#self.addElement(self.Red)
		#self.addElement(self.Green)
		#self.addElement(self.Blue)
		self.label_A = ui.Label(5, 25, "Red", "R:", self, True)
		self.label_B = ui.Label(65, 25, "Green", "G:", self, True)
		self.label_C = ui.Label(125, 25, "Blue", "B:", self, True)		
		#self.addElement(self.label_A)
		#self.addElement(self.label_B)
		#self.addElement(self.label_C)
		rgbValue = [self.value[0] * 255, self.value[1] * 255, self.value[2] * 255]
		self.colorButton = ui.ColorButton(200, 26, 45, 20, 'Picker', rgbValue, self, True)
		# self.addElement(self.colorButton)
		
		self.Red.registerCallback("update", self.updateColor)
		self.Green.registerCallback("update", self.updateColor)
		self.Blue.registerCallback("update", self.updateColor)
		
		# self.registerCallback("click", self.stat)
		self.bordered = True
		self.colorButton.picker.ok_functions.append(self.updateFields)
		self.colorButton.outlined = True
	
	def setValue(self, value):
		print "Set color: ", value
		self.Red.setValue(value[0])
		self.Green.setValue(value[1])
		self.Blue.setValue(value[2])
		self.updateColor(self.Blue)
	
		
	
	def getValue(self):		
		value = self.colorButton.getValue()
		r = float(float(value[0]) / 255)
		g = float(float(value[1]) / 255)
		b = float(float(value[2]) / 255)
		
		return [r, g, b]
		
	def updateFields(self, obj):
		# get the color of the button
		color = self.colorButton.getValue()
		
		# color values in the text boxes are assigned via ye olde renderman method, i.e. 0-1
		self.Red.setValue(float(float(color[0]) / 255))
		self.Green.setValue(float(float(color[1]) / 255))
		self.Blue.setValue(float(float(color[2]) / 255))
		
		# this is an update function to assign the color value of the button to the text editors. Fix it.
		
	def updateColor(self, obj):
		
		if obj.isEditing == False:
			# this function is called when any of the 3 text fields are updated
			# so the color button can be updated with the latest color
			# I probably need a *lot* more checking here
			r_s = float(self.Red.getValue())
			g_s = float(self.Green.getValue())
			b_s = float(self.Blue.getValue())
			if float(r_s) > 1:			
				r = int(r_s) 
			else:
				r = float(r_s) * 255			
				
			if float(g_s) > 1:
				g = float(g_s) 
			else:
				g = float(g_s) * 255
							
			if float(b_s) > 1:
				b = float(b_s) 
			else:
				b = float(b_s) * 255			
				
			rgb = [r, g, b, 255]
			self.colorButton.setValue(rgb) # set the value of the color button here
			
	def hit_test(self):		
		if self.colorButton.picking: # if the color picker is active, test for a hit
			rVal = True # this hits all the time to prevent any object from getting in front of it
			#rVal = self.colorButton.hit_test() 
		else: # otherwise, just my normal hit_test
			rVal = ui.UIElement.hit_test(self)
		return rVal
	
	def changeModel(self, obj):
		value = obj.getValue()
		self.label_A.setText(value[0] + ":")
		self.label_B.setText(value[1] + ":")
		self.label_C.setText(value[2] + ":")
		

class CoordinateEditor(ShaderParamEditor):
	height = 75
	paramtype = "coordinate"
	coordinateSpaces = ['current', 'object', 'shader', 'world', 'camera', 'screen', 'raster', 'NDC']
	protocols.advise(instancesProvide=[IShaderParamEditor], asAdapterForTypes=[BtoRPointParam, BtoRVectorParam, BtoRNormalParam])
	def __init__(self, bObj):
		ShaderParamEditor.__init__(self, bObj)
		width = self.get_string_width("Coordinate Space:", 'normal') + 5
		self.x_val = ui.TextField(width + 30, 25, 30, 20, "coord_x", self.value[0], self, True)
		self.y_val = ui.TextField(width + 85, 25, 30, 20, "coord_y", self.value[1], self, True)
		self.z_val = ui.TextField(width + 140, 25, 30, 20, "coord_z", self.value[2], self, True)
		#self.addElement(self.x_val)
		self.addElement(ui.Label(width + 10, 28, "X:", "X:", self, False))
		#self.addElement(self.y_val)
		self.addElement(ui.Label(width + 65, 28, "Y:", "Y:", self, False))
		#self.addElement(self.z_val)		
		self.addElement(ui.Label(width + 120, 28, "Z:", "Z:", self, False))
		
		# self.spaceMenu = Menu(width + 10, 25, 85, 25, "Coordinate Space:", self.coordinateSpaces, self, True)
		#self.addElement(self.spaceMenu)
		self.addElement(ui.Label(5, 25, "SpaceLabel", "Coordinate Space: ", self, False))
		# the rest of this should take care of itself, all I need is a getValue()
		self.bordered = True
		
	def getValue(self):
		value = []
		# value.append(self.spaceMenu.Value())
		value.append(float(self.x_val.getValue()))
		value.append(float(self.y_val.getValue()))
		value.append(float(self.z_val.getValue()))
		return value
	def setValue(self, value):
		self.x_val.setValue(value[0])
		self.y_val.setValue(value[1])
		self.z_val.setValue(value[2])
		
		
class FloatEditor(ShaderParamEditor):
	height = 50
	paramtype = "float"
	# for numeric values
	protocols.advise(instancesProvide=[IShaderParamEditor], asAdapterForTypes=[BtoRFloatParam])
	def __init__(self, bObj):
		ShaderParamEditor.__init__(self, bObj)				
		self.text = ui.TextField(15, 25, 40, 20, self.name, self.value, self, True)
		
		self.bordered = True
	
	def getValue(self):
		return float(self.text.getValue()) # return a double from here all the time
	def setValue(self, value):
		self.text.setValue(value)
		
class StringEditor(ShaderParamEditor):
	height = 50
	paramtype = "string"
	protocols.advise(instancesProvide=[IShaderParamEditor], asAdapterForTypes=[BtoRStringParam])
	def __init__(self, bObj):
		ShaderParamEditor.__init__(self, bObj)	
		width = self.get_string_width(self.name, 'normal') + 10
		self.text = ui.TextField(15, 25, 200, 20, self.name, self.value, self, True)		
		self.bordered = True
		
	def getValue(self):
		return self.text.getValue()
	def setValue(self, value):
		self.text.setValue(value)
			
		
class MatrixEditor(ShaderParamEditor):
	height = 140
	paramtype = "matrix"
	def __init__(self, bObj):
		ShaderParamEditor.__init__(self, bObj)	
		self.param_name = self.name
		column1 = []
		column2 = []
		column3 = []
		column4 = []
		for val in value: # presume that this array is constructed row-wise as in  [0, 0, 0, 0] being row 1
			# convert to a column format
			column1.append(val[0])
			column2.append(val[1])
			column3.append(val[2])
			column4.append(val[3])
			
		data = ui.Table(26, 20, 200, 80, 'Matrix', [column1, column2, column3, column4], self, True)
		
	def getValue(self):
		value = []
		for a in range(4):
			row = []
			for b in range(4):
				row.append(self.data.getValueAt(a, b))
			value.append(row)
		return value
	def setValue(self, value):
		for a in range(4):
			for b in range(4):
				self.data.setValueAt(a, b, value)

		
# the following 4 objects are for guessed object types, each includes a type override button
# to force a fallback to a string value
class SpaceEditor(ShaderParamEditor):
	height = 35
	paramtype = "string"
	Spaces = ['current', 'object', 'shader', 'world', 'camera', 'screen', 'raster', 'NDC']
	protocols.advise(instancesProvide=[IShaderParamEditor], asAdapterForTypes=[BtoRSpaceParam])
	def __init__(self,bObj):		
		ShaderParamEditor.__init__(self, bObj)	
		width = self.get_string_width(self.name, 'normal') + 5
		self.spaceMenu = ui.Menu(width + 15, 5, 100, 25, 'Coordinate Space:', self.Spaces, self, True)
		#self.addElement(self.spaceMenu)
		self.overrideButton = ui.Button(width + 120, 5, 60, 25, 'Override', 'Override', 'normal', self, True)
		#self.addElement(self.overrideButton)
		self.overrideButton.registerCallback("release", self.override_type)
		self.overridden = False
		
	def override_type(self, button):
		dispVal = self.spaceMenu.getValue()
		self.elements = []
		self.z_stack = []
		self.draw_stack = []
		self.addElement(ui.Label(5, 5, self.name, self.name, self, False))
		width = self.get_string_width(self.name, 'normal') + 5
		self.addElement(ui.TextField(width + 10, 5, 250, 20, self.name, dispVal, self, False))
		self.value = dispVal
		self.overridden = True
		
	def getValue(self):
		if self.overridden:
			value = self.elements[1].value
		else:
			value = self.spaceMenu.getValue()
		return value	
		
	def setValue(self, value):
		if self.overriden:
			self.elements[1].setValue(value)
		else:
			self.spaceMenu.setValueString(value)
		

class ProjectionEditor(ShaderParamEditor):
	height = 40
	paramtype = "string"
	Projections = ['st', 'planar', 'perspective', 'spherical', 'cylindrical']
	protocols.advise(instancesProvide=[IShaderParamEditor], asAdapterForTypes=[BtoRProjectionParam])
	def __init__(self, bObj):
		ShaderParamEditor.__init__(self, bObj)	
		width = self.get_string_width(self.name, 'normal') + 5
		self.projectionMenu = ui.Menu(width + 15, 5, 100, 25, 'Projection Type:', self.Projections, self, True)
		#self.addElement(self.projectionMenu)
		self.overrideButton = ui.Button(width + 120, 5, 60, 25, 'Override', 'Override', 'normal', self, True)
		#self.addElement(self.overrideButton)
		self.elements[2].registerCallback("release", self.override_type)
		self.overridden = False
		
	def override_type(self, button):
		dispVal = self.projectionMenu.getValue()
		self.elements = []
		self.z_stack = []
		self.draw_stack = []
		self.addElement(ui.Label(5, 5, self.name, self.name, self, False))
		width = self.get_string_width(self.name, 'normal') + 5
		self.addElement(ui.TextField(width + 10, 5, 250, 20, self.name, dispVal, self, False))
		self.value = dispVal
		self.overridden = True
		
	def getValue(self):
		if self.overridden:
			value = self.elements[1].value
		else:
			value = self.projectionMenu.getValue()
		return value	
		
	def setValue(self, value):
		if self.overridden:
			value = self.elements[1].setValue(value)
		else:
			self.projectionMenu.setValue(value)
		
	
class ColorSpaceEditor(ShaderParamEditor):
	height = 40
	paramtype = "string"
	ColorSpaces = ['rgb', 'hsv', 'hsl', 'YIQ', 'xyz', 'xyY']
	protocols.advise(instancesProvide=[IShaderParamEditor], asAdapterForTypes=[BtoRColorSpaceParam])
	def __init__(self, bObj):
		ShaderParamEditor.__init__(self, bObj)	
		width = self.get_string_width(self.name, 'normal') + 5
		self.colorSpaceMenu = ui.Menu(width + 15, 5, 100, 25, 'Color Space:', self.ColorSpaces, self, True)
		#self.addElement(self.colorSpaceMenu)
		self.overrideButton = ui.Button(width + 120, 5, 60, 25, 'Override', 'Override', 'normal', self, True)
		#self.addElement(self.overrideButton)
		self.overrideButton.registerCallback("release", self.override_type)
		self.overridden = False
		
	def override_type(self, button):
		dispVal = self.colorSpaceMenu.getValue()
		self.elements = []
		self.z_stack = []
		self.draw_stack = []
		self.addElement(ui.Label(5, 5, self.name, self.name, self, False))
		width = self.get_string_width(self.name, 'normal') + 5
		self.addElement(ui.TextField(width + 10, 5, 250, 20, self.name, dispVal, self, False))
		self.value = dispVal
		self.overridden = True
		
	def getValue(self):
		if self.overridden:
			value = self.elements[1].value
		else:
			value = self.colorSpaceMenu.getValue()
		return value	

	def setValue(self, value):
		if self.overridden:
			value = self.elements[1].setValue(value)
		else:
			self.colorSpaceMenu.setValue(value)
		


class FileEditor(ShaderParamEditor):
	height = 55
	paramtype = "string"
	protocols.advise(instancesProvide=[IShaderParamEditor], asAdapterForTypes=[BtoRFileParam])
	def __init__(self, bObj):
		ShaderParamEditor.__init__(self, bObj)	
		self.param_name = name
		self.label = ui.Label(5, 5, name, name, self, True)
		self.filename = ui.TextField(15, 25, 150, 20, name, value, self, True)
		self.browseButton = ui.Button(168, 24, 60, 20, "Browse", "Browse", 'normal', self, True)
		self.browseButton.registerCallback("release", self.openBrowseWindow)
		self.overrideButton = ui.Button(235, 24, 60, 20, "Override", "Override", 'normal', self, True)
		self.overrideButton.registerCallback("release", self.override_type)
		self.overridden = False
		#self.addElement(self.label)
		#self.addElement(self.filename)
		#self.addElement(self.browseButton)
		#self.addElement(self.overrideButton)
		
	def override_type(self, button):
		dispVal = self.filename.getValue()
		self.elements = []
		self.z_stack = []
		self.draw_stack = []
		self.addElement(ui.Label(5, 5, self.name, self.name, self, False))
		width = self.get_string_width(self.name, 'normal') + 5
		self.addElement(ui.TextField(width + 10, 5, 250, 20, self.name, dispVal, self, False))
		self.value = dispVal
		self.overridden = True
	
	def openBrowseWindow(self, button):
		Blender.Window.FileSelector(self.select, 'Choose a file')
		
	def select(self, file):
		#self.filename.value = file
		self.filename.setValue(file)
		
	def getValue(self):
		if self.overridden:
			value = self.elements[1].value			
		else:
			value = self.filename.getValue()
		return value
		
	def setValue(self, value):
		self.filename = value
		
class ArrayEditor(ShaderParamEditor):
	paramtype = "array"
	protocols.advise(instancesProvide=[IShaderParamEditor], asAdapterForTypes=[BtoRArrayParam])
	def __init__(self, bObj):
		screen = Blender.Window.GetAreaSize()		
		ShaderParamEditor.__init__(self, bObj)	
		self.values = value
		self.editorPanel = ui.Panel((screen[0] / 2) - 200, (screen[1] / 2) + 300, 400, 600, "ParameterEditor", "Array Parameter: %s" % name, None, False) # The container for the array value itself
		self.editorPanel.dialog = True
		self.editing = False
		idx = 0
		#self.closeButton = Button(10, 560, 50, 25, "Cancel", "Cancel", 'normal', self.editorPanel, True)
		# self.closeButton.registerCallback("release", self.closeEditor)
		self.okButton = ui.Button(self.editorPanel.width - 60, 560, 50,  25, "OK", "Close", 'normal', self.editorPanel, True)
		self.okButton.registerCallback("release", self.closeEditor)
		self.scrollPane = ui.ScrollPane(5, 27, 390, 530, "scrollPane", "scrollPane", self.editorPanel, True)
		self.scrollPane.normalColor = [185, 185, 185, 255]
		#self.editorPane.addElement(self.scrollPane)
		self.launchButton = ui.Button(0, 25, width - 25, height - 28, "Button", "Launch Editor for array parameter '%s'" % self.name, 'normal', self, True)
		self.launchButton.registerCallback("release", self.showEditor)
		
		if isinstance(values, list):
			for val in self.values: 
				size = [0, 0, 370, 0]
				bParm = BtoRTypes.__dict__["BtoR" + p_type.capitalize + "Param"](param = "Array index %d" % idx, value = val, size=size, parent = self.scrollPane) # just init a basic type
				editor = BtoRAdapterClasses.IShaderParamEditor(bParm) # that should do the trick

				#self.scrollPane.addElement(target_class(0, 0, 370, target_class.height, "Array index %d" % idx, value, self.scrollPane, False))
				#self.scrollPane.addElement(editor)  
				idx = idx + 1
		else:
			for idx in range(0, length):
				size = [0, 0, 370, 0]
				bParm = BtoRTypes.__dict__["BtoR" + p_type.capitalize + "Param"](param = "Array index %d" % idx, value = self.value, size=size, parent = self.scrollPane) # just init a basic type
				editor = BtoRAdapterClasses.IShaderParamEditor(bParm) # that should do the trick

				#self.scrollPane.addElement(target_class(0, 0, 370, target_class.height, "Array index %d" % idx, value, self.scrollPane, False))
				#self.scrollPane.addElement(editor)  
				idx = idx + 1
			
		# the only special use case I have
		sdict = globals()
		self.evt_manager = sdict["instBtoREvtManager"]
		# that should be that.
		
	def getEditorButton(self):
		return self.launchButton
		
	def getValue(self):
		value = []
		for element in self.scrollPane.elements:
			value.append(element.getValue()) # get the value in question and return it
		return value

	def showEditor(self, button):
		# show the editor
		self.editing = True
		# don't know that the above is stricly neccessary....and it's not
		# so
		self.evt_manager.addElement(self.editorPanel)
		self.evt_manager.raiseElement(self.editorPanel)
		
		
	def closeEditor(self, button):
		self.evt_manager.removeElement(self.editorPanel)
			
		
	def setValue(self, value):
		self.value = value
		# do nothing else
		
		

	
		
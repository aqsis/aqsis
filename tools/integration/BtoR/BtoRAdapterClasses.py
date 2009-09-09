import os
import btor
from btor import BtoRGUIClasses as ui
from btor.BtoRTypes import *
import cgkit
import cgkit.rmshader
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
import md5
import traceback
import re
  
# interfaces
class IProperty(protocols.Interface):
	def getValue():
		pass
	def setValue():
		pass

class IPropertyEditor(protocols.Interface):
	def getValue():
		"""" get the value of the property """
	def setValue():
		""" set the value of the property """
		
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
		
class Property:
	protocols.advise(instancesProvide=[IProperty])
	height = 27 # should work for most
	def __init__(self, value, xml = None):
		self.value = value
		self.saveable = True
		self.labelWidth = 0
		self.editorWidth = 0
		self.isRenderable = False
	def setHeight(self, height):
		self.height = height
	def setWidth(self, width):
		self.width = width
	def setCustomWidth(self, width):
		self.editorWidth = width[0]
		self.labelWidth = width[1]
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
			
	def toXML(self, xml):
		xmlProp = xml.createElement("property")
		xmlProp.setAttribute("type", type(self.getValue()).__name__)
		xmlProp.setAttribute("value", str(self.getValue()))
		#print "set property of type ", type(self.value), " to ", type(str(self.value).__name__)
		return xmlProp		
	# interface for complex properties
	def getEditor(self):
		return self.value.obj.getEditor()
	def getStrValue(self):
		return self.value.obj.getStrValue()
	def registerCallback(self, signal, function): # this is a pass-through to maintain abstraction
		#if self.value.__dict__.has_key("registerCallback"): # this will bypass problems with missing values, till I fix my implementation to full provide an interface
		print "setting callback for shader!"
		self.value.registerCallback(signal, function) 

# Properties
class StringProperty(Property): 
	protocols.advise(instancesProvide=[IProperty], asAdapterForTypes=[str])
	pass

class IntProperty(Property):
	protocols.advise(instancesProvide=[IProperty], asAdapterForTypes=[int])
	pass
	
class FloatProperty(Property): 
	protocols.advise(instancesProvide=[IProperty], asAdapterForTypes=[float])
	pass

class ColorProperty(Property): 
	protocols.advise(instancesProvide=[IProperty], asAdapterForTypes=[list])	
	def toXML(self, xml):
		xmlProp = xml.createElement("property")
		xmlProp.setAttribute("type", type(self.getValue()).__name__)
		xmlProp.setAttribute("red", str(self.getValue()[0]))
		xmlProp.setAttribute("green", str(self.getValue()[1]))
		xmlProp.setAttribute("blue", str(self.getValue()[2]))
		#print "set property of type ", type(self.value), " to ", type(str(self.value).__name__)
		print self.getValue()
		return xmlProp		

class DictProperty(Property):
	protocols.advise(instancesProvide=[IProperty], asAdapterForTypes=[dict])
	def __init__(self, value):
		self.labelWidth = 0
		self.editorWidth = 0
		self.saveable = True
		# what do I do here? I want this to actually be converted to a menu
		# thus
		# sort the array keys
		self.valueDict = value
		self.keyList = value.keys()		
		self.value = value[self.keyList[0]] # get the first value in the dictionary
		# sort the key list, but decide how to sort it based on the type of the values provided.
		if isinstance(self.value, int):
			newList = []
			for key in self.keyList:
				newList.append(int(key))
			newList.sort()
			self.keyList = []
			for key in newList:
				self.keyList.append(str(key))
				
		elif isinstance(self.value, float):
			newList = []
			for key in keyList:
				newList.append(float(key))
			newList.sort()
			self.keyList = []
			for key in newList:
				self.keyLIst.append(str(key))
		else:
			self.keyList.sort() #only strings, so sort accordingly
			
	def getKeys(self):
		return self.keyList # keylist may or may not be sorted
	def getValueByKey(self, key):
		return self.valueDict[key]	
		
class BooleanProperty(Property): 
	protocols.advise(instancesProvide=[IProperty], asAdapterForTypes=[bool])
	pass
	
class ShaderProperty(Property):
	protocols.advise(instancesProvide=[IProperty], asAdapterForTypes=[BtoRShaderType])
	def __init__(self, value, xml = None):
		sdict = globals()
		self.settings = sdict["instBtoRSettings"]
		Property.__init__(self, value, xml = xml)
		self.saveable = False
	
	def initShader(self, useXML = False, xml = None, shaderName = None, parmList = None, shaderFileName = None):
		# this should always have a shader, so
		
		s_type = self.value.getObject().s_type
		#print "Initializing a ", s_type, " shader"
		
		try:
			if self.settings.use_slparams:
				shaderPath = self.settings.getShaderSearchPaths()[0]
				
				initialized = True								
				if useXML:
					shader = cgkit.rmshader.RMShader(xml.getAttribute("path").encode("ascii"))	# this would actually be a problem. I'm not always 100% that I'll have a shader source file and so much rely 
					# also on the shader name to get this in cases where a custom shader was entered.
					parms = xml.getElementsByTagName("Param")
					self.populateShaderParams(parms, shader, initialized)
				else:
					shader = cgkit.rmshader.RMShader(shaderFileName)
					self.populateShaderParamsList(parmList, shader, initialized)		
			else:
				initialized = False
				shader = cgkit.rmshader.RMShader(shaderName)
				self.populateShaderParamsList(parmList, shader, initialized)		
			shader = btor.BtoRMain.GenericShader(shader, s_type, self)
		except:
			traceback.print_exc()
			shader = btor.BtoRMain.GenericShader(None, s_type, self)
			
		self.value = BtoRShaderType(shader)
		self.editor.setValue(shader.getStrValue())
		# I should probably update the property editor here
		
		
	def populateShaderParams(self, parms, shader, initialized):
		print parms
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
				parm_value = parm.getAttribute("value").encode("ascii")
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
			# print "Assigning parameter value ", p_name, " = ", parm_value
			# and set the value 
			setattr(shader, p_name, parm_value)
	
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
	def showEditor(self):
		# print self.value.getObject()
		self.value.getObject().showEditor()
class MaterialProperty(Property):
	protocols.advise(instancesProvide=[IProperty], asAdapterForTypes=[BtoRMaterialType])
	height = 65
	pass
	def getValue(self):
		return self.editor.value.getValue()
class VectorProperty(Property):
	protocols.advise(instancesProvide=[IProperty], asAdapterForTypes=[cgkit.cgtypes.vec3])
	pass

class RotationProperty(Property):
	protocols.advise(instancesProvide=[IProperty], asAdapterForTypes=[BtoRRotationType])
	pass
	
	
class MatrixProperty(Property):
	protocols.advise(instancesProvide=[IProperty], asAdapterForTypes=[cgkit.cgtypes.mat4])
	height = 95
	pass

class CustomRIBProperty(Property):
	protocols.advise(instancesProvide=[IProperty], asAdapterForTypes=[BtoRCustomRIB])
	height = 20
	pass

	
# Property Editors
class PropertyEditor: # this needs no interface declaration, since all this is doing is providing a baseclass
	fontsize = 'small'
	def __init__(self, property, suppressLabel = False):		
		self.property = property		
		self.height = self.property.height		
		if self.property.labelWidth > 0:
			pWidth = self.property.editorWidth
			lWidth = self.property.labelWidth
		else:
			pWidth = self.property.width
		self.editor = ui.Panel(0, 0, pWidth, self.height, "", "", None, False)
		self.editor.hasHeader = False
		self.editor.shadowed = False
		self.editor.normaColor = [128, 128, 128, 0]
		self.editor.hoverColor = [128, 128, 128, 0]
		self.editor.outlined = True
		self.editor.cornermask = 0
		self.editor.outlined = True
		self.editor.cornermask = 0
		if not suppressLabel:
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
		if type(obj) == ui.TextField:
			if obj.type == "int":
				self.property.setValue(int(obj.getValue()))
			elif obj.type == "float":
				self.property.setValue(float(obj.getValue()))
		else:
			self.property.setValue(obj.getValue())
		if self.func != None:
			self.func() # invoke the update function for this property

	def getEditor(self):
		return self.editor
		
class BasicPropertyEditor(PropertyEditor):
	protocols.advise(instancesProvide=[IPropertyEditor], asAdapterForTypes=[IntProperty, FloatProperty, BtoRFloatParam])
	""" A basic property, a label and a text box """
	def __init__(self, property):
		PropertyEditor.__init__(self, property)
		width = property.width
		height = property.height
		self.value = ui.TextField(width / 2, 0, width / 2, height, self.property.getName(), self.property.getValue(), self.editor, True, fontsize = self.fontsize)
		self.value.registerCallback("update", self.updateValue)
		
	# protocols.declareAdapter(BasicPropertyEditor, [IPropertyEditor], forTypes=[StringProperty, IntProperty, FloatProperty])
class StringPropertyEditor(PropertyEditor):
	protocols.advise(instancesProvide=[IPropertyEditor], asAdapterForTypes=[StringProperty, BtoRStringParam], factoryMethod="routeProperty")
	""" A basic property, a label and a text box """
	def __init__(self, property):
		sdict = globals()
		# self.scene = sdict["instBtoRSceneSettings"]
		
		PropertyEditor.__init__(self, property)
		width = property.width
		height = property.height
		self.value = ui.TextField(width / 2, 0, width / 2, height, self.property.getName(), self.property.getValue(), self.editor, True, fontsize = self.fontsize)
		self.value.registerCallback("update", self.updateValue)
		self.value.registerCallback("right_click", self.showAssets)
	
	def showAssets(self, obj):
		#self.scene.showAssets(self)
		print "yo!"
	
	@classmethod
	def routeProperty(cls, obj):		
		if isinstance(obj, StringProperty):
			#print "Returning a normal string property!"
			# init self, I'm a string
			return cls(obj)
		else:
			name = obj.getName()
			value = obj.getValue()			
			#print "Finding editor for ", name
			# color space or space property most likely, react accordingly
			spaceMatch = re.compile("space|proj", re.I)
			fileMatch = re.compile("tex|map|name|refl", re.I)
			if spaceMatch.search(name): # extend this to provide a nice list
				if value in ColorSpacePropertyEditor.ColorSpaces:				
					return ColorSpacePropertyEditor(obj)
				elif value in SpacePropertyEditor.Spaces:
					return SpacePropertyEditor(obj)
				elif value in ProjectionPropertyEditor.Projections:
					return ProjectionPropertyEditor(obj)
			if fileMatch.search(name):
				return FilePropertyEditor(obj)
			else:
				return cls(obj)
					
class FilePropertyEditor(PropertyEditor):
	# file property editor should have access to the global asset list. 
	# environment & shadow maps should apply to the *assigned* object
	# lightsources are slightly weird because they need to be both local and global (for AO purposes)
	# environment maps are actually global namely because of multiple assignment
	protocols.advise(instancesProvide=[IPropertyEditor])
	def __init__(self, property):		# remember to add an override button to all custom types!
		sdict = globals()
		self.evt_manager = sdict["instBtoREvtManager"]
		PropertyEditor.__init__(self, property)
		width = self.property.width
		height = self.property.height
		self.value = ui.TextField(width / 2, 0, width / 2 - height, height, self.property.getName(), self.property.getStrValue(), self.editor, True, fontsize = self.fontsize)
		# self.value.Enabled = False
		butX = self.value.x + self.value.width + 1
		self.triggerButton = ui.Button(butX, 0, height, height, "...", "...", 'small', self.editor, True)
		self.triggerButton.shadowed = False
		self.triggerButton.registerCallback("release", self.browse)
		# if isinstance(property, StringProperty):
		self.value.registerCallback("update", self.updateValue)
		# for (this( property editor, I should set a back reference so I can update the text field with the shader when it's initialized
		self.property.editor = self
		self.value.registerCallback("right_click", self.showAssets)
	
	def showAssets(self, obj):
		#self.scene.showAssets(self)
		print "yo!"
		
	def browse(self, button):
		""" Browser """

class PathPropertyEditor(PropertyEditor):
	""" Directory Browser """
	def __init__(self, property):
		sdict = globals()
		self.evt_manager = sdict["instBtoREvtManager"]
		width = self.property.width
		height = self.property.height
		self.value = ui.TextField(0, 0, width - 25, height, self.property.getName(), self.property.getStrValue(), self.editor, True, fontsize = self.fontsize)
		self.triggerButton = ui.Button(width - 25, 0, 25, height, "...", "...", 'small', self.editor, True)
		self.triggerButton.shadowed = False
		self.triggerButton.registerCallback("release", self.browsePath)
		self.value.registerCallback("update", self.updateValue)
		self.property.editor = self
		
	def browsePath(self, button):		
		Blender.Window.FileSelector(self.select, 'Choose any file')
		
	def select(self, file):
		path = os.path.dirname(file)
		self.value.setValue(path)
		self.updateValue(path)
		
class SpacePropertyEditor(PropertyEditor):
	Spaces = ['current', 'object', 'shader', 'world', 'camera', 'screen', 'raster', 'NDC']
	def __init__(self, property):
		PropertyEditor.__init__(self, property)
		width = property.width
		height = property.height
		self.value = ui.Menu(width / 2, 2, width / 2, height - 4, self.property.getName(), self.Spaces, self.editor, True, fontsize = self.fontsize)		
		self.value.registerCallback("select", self.updateValue)
		self.value.setShadowed(False)
	def setValue(self, value):
		# menu editors need to be slightly different
		self.property.setValue(value) 
		self.value.setValueString(value)
		
	def renameMenuItem(self, idx, name):
		self.value.renameElement(idx, name)
		
class ColorSpacePropertyEditor(PropertyEditor):
	ColorSpaces = ['rgb', 'hsv', 'hsl', 'YIQ', 'xyz', 'xyY']
	def __init__(self, property):
		PropertyEditor.__init__(self, property)
		width = property.width
		height = property.height
		self.value = ui.Menu(width / 2, 2, width / 2, height - 4, self.property.getName(), self.ColorSpaces, self.editor, True, fontsize = self.fontsize)		
		self.value.setShadowed(False)
		
		self.value.registerCallback("select", self.updateValue)
		
	def setValue(self, value):
		# menu editors need to be slightly different
		self.property.setValue(value) 
		self.value.setValueString(value)
		
	def renameMenuItem(self, idx, name):
		self.value.renameElement(idx, name)
		
class ProjectionPropertyEditor(PropertyEditor):
	Projections = ['st', 'planar', 'perspective', 'spherical', 'cylindrical']
	def __init__(self, property):
		PropertyEditor.__init__(self, property)
		width = property.width
		height = property.height
		self.value = ui.Menu(width / 2, 2, width / 2, height - 4, self.property.getName(), self.Projections, self.editor, True, fontsize = self.fontsize)		
		self.value.registerCallback("select", self.updateValue)
		self.value.setShadowed(False)
	def setValue(self, value):
		# menu editors need to be slightly different
		self.property.setValue(value) 
		self.value.setValueString(value)
			
	def renameMenuItem(self, idx, name):
		self.value.renameElement(idx, name)
	
class MenuPropertyEditor(PropertyEditor):
	protocols.advise(instancesProvide=[IPropertyEditor], asAdapterForTypes=[DictProperty])
	def __init__(self, property):
		PropertyEditor.__init__(self, property)
		width = self.property.width
		height = self.property.height
		menu = self.property.getKeys()	
		defVal = None
		if "btor:default" in menu:
			defVal = self.property.getValueByKey("btor:default")
			# strip the default option out of the menu now
			menu.remove("btor:default")			
		self.value = ui.Menu(width / 2, 2, width / 2, height - 4, self.property.getName(), menu, self.editor, True, fontsize = self.fontsize)		
		self.value.registerCallback("select", self.updateValue)
		self.value.setShadowed(False)
		self.property.setValue(self.value.getValue())
		if defVal != None:
			self.value.setValueString(defVal) # assigns the default value
		
	def setValue(self, value):
		# menu editors need to be slightly different
		self.property.setValue(value) 
		self.value.setValueString(value) 
	
	def updateMenu(self, menu):
		# reinit the menu, but keep the selected indesx
		index = self.value.getSelectedIndex()
		self.value.re_init(menu)
		if len(self.value.elements) > index:
			self.value.setValue(0)
		else:
			self.value.setValue(index)
		
	def renameMenuItem(self, idx, name):
		self.value.renameElement(idx, name)
		
	#protocols.declareAdapter(MenuPropertyEditor, [IPropertyEditor], forTypes=[DictProperty])

class ColorPropertyEditor(PropertyEditor):
	
	protocols.advise(instancesProvide=[IPropertyEditor], asAdapterForTypes=[ColorProperty, BtoRColorParam])
	def __init__(self, property):
		PropertyEditor.__init__(self, property)
		width = self.property.width
		height = self.property.height
		color = self.property.getValue()
		print color
		# I need 3 RGB values
		inc = (width / 2) / 4
		self.R = ui.TextField((width / 2), 0, inc -1, height, "Red", color[0], self.editor, True)
		self.G = ui.TextField((width / 2) + inc, 0, inc -1, height, "Green", color[1], self.editor, True)
		self.B = ui.TextField((width / 2) + (inc * 2), 0, inc -1, height, "Blue", color[2], self.editor, True)		
		self.colorButton = ui.ColorButton((width / 2) + (inc * 3), 0, inc - 4, height - 2, "Color", color, self.editor, True)
		self.colorButton.outlined = True		
		self.R.registerCallback("update", self.updateColor)
		self.G.registerCallback("update", self.updateColor)
		self.B.registerCallback("update", self.updateColor)
		self.updateColor(None)
		self.colorButton.picker.registerCallback("ok", self.updateFields)
		self.value = self.colorButton
		
	def updateFields(self, color):
		self.R.setValue(float(float(color.value[0]) / 255))
		self.G.setValue(float(float(color.value[1]) / 255))
		self.B.setValue(float(float(color.value[2]) / 255))
		self.property.setValue([float(float(color.value[0])/255), float(float(color.value[1]) / 255), float(float(color.value[2]) / 255)])
	
	def updateColor(self, obj):		
		# convert to RGB 255
		r_s = float(self.R.getValue())
		g_s = float(self.G.getValue())
		b_s = float(self.B.getValue())
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
		self.colorButton.setValue(rgb) 
		self.property.setValue([self.R.getValue(), self.G.getValue(), self.B.getValue()])
		
	def setValue(self, color):
		print "Setting color", color
		self.R.setValue(color[0])
		self.G.setValue(color[1])
		self.B.setValue(color[2])
		self.updateColor(None)
		# self.property.setValue([float(float(color.value[0])/255), float(float(color.value[1]) / 255), float(float(color.value[2]) / 255)])
		
	#protocols.declareAdapter(ColorPropertyEditor, [IPropertyEditor], forTypes=[ColorProperty])

class BooleanPropertyEditor(PropertyEditor):
	protocols.advise(instancesProvide=[IPropertyEditor], asAdapterForTypes=[BooleanProperty])
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
		
	#protocols.declareAdapter(BooleanPropertyEditor, [IPropertyEditor], forTypes=[BooleanProperty])
class ShaderPropertyEditor(PropertyEditor):
	protocols.advise(instancesProvide=[IPropertyEditor], asAdapterForTypes=[ShaderProperty])
	def __init__(self, property):
		sdict = globals()
		self.evt_manager = sdict["instBtoREvtManager"]
		PropertyEditor.__init__(self, property)
		width = self.property.width
		height = self.property.height
		self.value = ui.TextField(width / 2, 0, width / 2 - height, height, self.property.getName(), self.property.getStrValue(), self.editor, True, fontsize = self.fontsize)
		self.value.Enabled = False
		butX = self.value.x + self.value.width + 1
		self.triggerButton = ui.Button(butX, 0, height, height, "...", "...", 'small', self.editor, True)
		self.triggerButton.shadowed = False
		self.triggerButton.registerCallback("release", self.showPropertyEditor)
		self.property.registerCallback("update", self.updateValue)
		# for (this( property editor, I should set a back reference so I can update the text field with the shader when it's initialized
		self.property.editor = self
		# now I need to set a callback to get the shader value when it's updated		
		
	def showPropertyEditor(self, obj):
		# instead of doing this, I need to do		
		self.property.showEditor()
	
	def updateValue(self, obj): 	
		print "Updating property!"
		self.setValue(obj.getShaderName())
		
	def setValue(self, value):
		# self.property.setValue(value)
		self.value.setValue(value)	
		
class MaterialPropertyEditor(PropertyEditor):
	protocols.advise(instancesProvide=[IPropertyEditor], asAdapterForTypes=[MaterialProperty])
	def __init__(self, property):
		sdict = globals()
		self.evt_manager = sdict["instBtoREvtManager"]
		self.materials = sdict["instBtoRMaterials"]		
		self.property = property
		width = self.property.width
		self.height = self.property.height		
		self.editor = ui.Button(0, 0, width, self.height, "Assigned Material:", "Assigned Material:", 'normal', None, False)
		self.editor.registerCallback("release", self.showMaterialSelector)
		self.editor.textlocation = 1
		self.editor.shadowed = False
		#self.editor.normalColor = [128, 128, 128, 0]
		#self.editor.hoverColor = [128, 128, 128, 0]
		self.editor.outlined = True
		self.editor.cornermask = 0
		self.editor.outlined = True
		self.editor.cornermask = 0
		self.value = ui.Label(2, 25, "None Assigned:", "None Assigned:", self.editor, True, fontsize = self.fontsize)
		self.value.fontsize = 'small'
		self.value.transparent = True
		self.func = None
		width = self.property.width
		height = self.property.height
		self.property.editor = self
		
	def showPropertyEditor(self, obj):
		self.evt_manager.addElement(self.property.getEditor())
		
	def setValue(self, material):
		#self.evt_manager.removeElement(self.mat_selector)
		# the button returned has the material name!
		# So all i need to do now is...
		
		if material != None and material != "None Assigned:":
			self.value.setValue(material)
			self.material = self.materials.getMaterial(material) # this should be the material name!
			self.editor.setTitle(material)
			self.editor.image = self.materials.getMaterial(material).image
			
	def updateValue(self, obj):
		if material != None:
			self.material = material
			if preInit == False:
				self.objData["material"] = material.name
			self.objEditor.materialButton.setTitle(material.name)
			self.objEditor.setImage(material.image)
			
	def setMaterial(self, material):
		if material != None:
			self.value.setValue(material.material.name)
			self.property.setValue(material.material.name)			
			if self.editor.image == None:
				self.editor.image = ui.Image(150, 5, 56, 56, material.image, self.editor, False)
			else:
				self.editor.image = ui.Image(150, 5, 56, 56, material.image, self.editor, False)
	
	
	def showMaterialSelector(self, obj):
		""" Display a material selector window. """
		# I should have loaded materials here, so let's do this.
		if self.materials.getMaterialCount() < 1:
			self.evt_manager.showConfirmDialog("No materials defined!", "You haven't defined any materials!", None, False)
		else:
			self.evt_manager.addElement(self.materials.getSelector())

	def setImage(self, image):
		buttonImage = ui.Image(120, 5, 56, 56, image, self.materialButton, False)
		self.editor.image = buttonImage
		
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
			
class VectorPropertyEditor(PropertyEditor):
	protocols.advise(instancesProvide=[IPropertyEditor], asAdapterForTypes=[VectorProperty, BtoRPointParam, BtoRVectorParam, BtoRNormalParam])
	def __init__(self, property):
		PropertyEditor.__init__(self, property)
		width = property.width
		height = property.height
		start = width / 2
		inc = (width / 2) / 3 - 1
		
		self.x = ui.TextField(start, 0, inc - 1, height, "X", self.property.getValue()[0], self.editor, True, fontsize = self.fontsize)
		self.x.registerCallback("update", self.updateValue)
		start = start + inc
		
		self.y = ui.TextField(start, 0, inc - 1, height, "Y", self.property.getValue()[1], self.editor, True, fontsize = self.fontsize)
		self.y.registerCallback("update", self.updateValue)
		start = start + inc
		
		self.z = ui.TextField(start, 0, inc - 1, height, "Z", self.property.getValue()[2], self.editor, True, fontsize = self.fontsize)
		self.z.registerCallback("update", self.updateValue)
		
	def updateValue(self, obj):		
		self.property.value = cgkit.cgtypes.vec3(float(self.x.getValue()), float(self.y.getValue()), float(self.z.getValue()))
	def setValue(self, value):
		self.x.setValue(value[0])
		self.y.setValue(value[1])
		self.z.setValue(value[2])
		self.property.setValue(value)
class RotationPropertyEditor(PropertyEditor):
	protocols.advise(instancesProvide=[IPropertyEditor], asAdapterForTypes=[RotationProperty])
	def __init__(self, property):
		PropertyEditor.__init__(self, property)
		width = property.width
		height = property.height
		start = width / 2
		inc = (width / 2) / 4 - 1
		
		self.angle = ui.TextField(start, 0, inc - 1, height, "angle", self.property.getValue().getObject()[0], self.editor, True, fontsize = self.fontsize)
		self.angle.registerCallback("update", self.updateValue)
		start = start + inc
		
		self.x = ui.CheckBox(start + 1, 3, "X", "X",self.property.getValue().getObject()[1], self.editor, True, fontsize = self.fontsize)
		self.x.registerCallback("release", self.updateValue)
		start = start + inc
		self.x.elements[0].x = 12
		
		self.y = ui.CheckBox(start + 1, 3, "Y", "Y", self.property.getValue().getObject()[2], self.editor, True, fontsize = self.fontsize)
		self.y.registerCallback("release", self.updateValue)
		start = start + inc
		self.y.elements[0].x = 12
		
		self.z = ui.CheckBox(start + 1, 3, "Z", "Z", self.property.getValue().getObject()[3], self.editor, True, fontsize = self.fontsize)
		self.z.registerCallback("release", self.updateValue)
		self.z.elements[0].x = 12
		
	def updateValue(self, obj):		
		self.property.value.obj = [float(self.angle.getValue()), self.x.getValue(), self.y.getValue(), self.z.getValue()]
		
class MatrixPropertyEditor(PropertyEditor):
	protocols.advise(instancesProvide=[IPropertyEditor], asAdapterForTypes=[MatrixProperty, BtoRMatrixParam])
	def __init__(self, property):
		PropertyEditor.__init__(self, property)
		width = property.width
		height = property.height
		x = width / 2
		self.table = ui.Table(x, 0, x, 65, "table", property.getValue(), self.editor, True)
		
class CustomRIBPropertyEditor(PropertyEditor):
	protocols.advise(instancesProvide=[IPropertyEditor], asAdapterForTypes=[CustomRIBProperty])
	def __init__(self, property):
		PropertyEditor.__init__(self, property)
		width = property.width
		height = property.height
		x = width / 4
		self.value = ui.TextField(width / 2, 0, x, height, "", "No custom RIB", self.editor, True)
		self.value.Enabled = False
		self.trigger = ui.Button(x + 1, 0, x, height, "", "...", self.editor, True)
			

	
# Object Adapters
		
	

class ObjectAdapter: # baseclass for all Blender objects
	protocols.advise(instancesProvide=[IObjectAdapter], asAdapterForTypes=[BtoRLattice, BtoRArmature, BtoRBasicObject, BtoREmpty, BtoRWave])
	def __init__(self, obj):
		""" initialize an object adapter based on the incoming object """
		dict = globals()		
		self.settings = dict["instBtoRSettings"]
		self.evt_manager = dict["instBtoREvtManager"]
		self.scene = dict["instBtoRSceneSettings"]
		self.materials = dict["instBtoRMaterials"]
		self.lighting = dict["instBtoRLightManager"]
		self.object = obj.obj # this is a BtoR object instance
		# why can't I use the objectUI protocol to get an appropriate editor?
		# let's do that!
		self.objEditor = protocols.adapt(obj, IObjectUI)		
		self.properties = self.objEditor.properties
		self.editors = self.objEditor.editors
		self.rendererAtts = self.objEditor.rendererAtts
		self.initObjectData() # initializes object data 
		# self.objType = obj.obj.getType()
		self.changed = False
		
	def renderAttributes(self):
		# iterate renderer specific attributes
		attributes = self.settings.getRendererAttributes()		
		for attribute in attributes:
			render = False
			if attribute in self.rendererAtts: 
				attArray = attribute.split(":")
				val = self.rendererAtts[attribute].getValue()
				print "Value is:", val
				print "Default is:", attributes[attribute][2]				
				if val != attributes[attribute][2]: # not default value, render the attribute
					x = attArray[1].find("[")
					if x > -1:
						z = len(attArray[1] - 1)
						arrLen = int(attArray[1][x + 1:z])
						v = []
						sVal = val.split(" ")
						try:
							if "integer" in attArray[1] or "int" in attArray[1]:
								for vIdx in range(arrLen):
									v.append(int(sVal[vIdx]))
							elif "string" in attArray[1]:
								for vIdx in range(arrLen):
									v.append(sVal[vIdx])
							elif "float" in arrArray[1]:
									v.append(float(sVal[vIdx]))							
							val = v
							render = True
						except:
							render = False
					else:
						try:
							if attArray[1] == "integer" or attArray[1] == "int":
								val = [int(val)]
								render = True
							elif attArray[1] == "string":
								val = [str(val)]
								render = True
							elif attArray[1] == "float":
								val = [float(val)]
								render = True
							else:
								render = False
						except:
							render = False
			if render:	
				print "arrayVal: ", attArray[0], "arrayVal", attArray[1], " arrayVal: ", attArray[2], " Value:", val
				if attArray[0] == "" or attArray[0] == None or attArray[2] == "" or val == "" or val == None:
					print "Bad things! Attribute didn't work!"
				else:
					try:
						ri.RiAttribute(attArray[0], attArray[2], val)
					except:
						print "Attribute parse error!"
						attOut = "Attribute " + '"' + attArray[0] + '" "' + attArray[2] + '" "' + val[0]
						print attOut
						#ri._ribout.write(attOut)
				
	def temp(self):
		if len(self.rendererAtts) > 0:
			for att in self.rendererAtts:
				# for each attribute, get the value and render it
				attArray = att.split(":") # splits the option by : chars
				# propArray[0] = category
				# propArray[1] = type
				# propArray[2] = name
				render = True
				# get the attribute value				
				val = self.getProperty(property) # get the value
				print val
				print rproperties[property]
				if val != rproperties[property][2]:
					x = propArray[1].find("[")
					if x > -1:
						# array value
						# "integer[2]"
						print propArray
						z = len(propArray[1]) - 1
						arrLen = int(propArray[1][x + 1:z])
						v = []
						sVal = val.split(" ")
						if "integer" in propArray[1] or "int" in propArray[1]:
							for vIdx in range(arrLen):
								v.append(int(sVal[vIdx]))
						elif "string" in propArray[1]:
							for vIdx in range(arrLen):
								v.append(sVal[vIdx])
						elif "float" in propArray[1]:
							for vIdx in range(arrLen):
								v.append(float(sVal[vIdx]))
						val = v
					else:
						# not an array, lets' do this differently					
						if propArray[1] == "integer" or propArray[1] == "int":					
							val = [int(val)]
						elif propArray[1] == "float":
							val = [float(val)]
						elif propArray[1] == "string":
							val = [str(val)]
						else:
							render = False
					# here, val should be either an array or not
					# thus, eval
					# types and arrays should be ok now
					# thus, the option call
					if render:
						ri.RiOption(propArray[0], propArray[1] + " " + propArray[2], val)

		
	def genChecksum(self):
		pass
	
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
		
	def render(self, shadowPass = False):
		""" Generate Renderman data from this call. """
		# decide here what to do with the object's data. Do I render it using the normal adapter method, or do I do something special for cases involving ReadArchive and what-not?
		
		return True
	def renderAsCamera(self):
		return True
		
	def renderArchive(self):
		# this should be all that's neccessary 
		# I do need to handle dupliverts situations and animated curves, array modifer, all that crap.
		if self.__dict__.has_key("archive"):
			ri.RiBegin(archive)
			self.render()
			ri.RiEnd()
			
	def initObjectData(self):			
		""" Generate the object data for this  object. """
		self.objData = {}
		self.objData["name"] = self.object.getName()
		self.objData["type"] = self.object.getType()
	
	def doCameraTransform(self, axis = None):			
		if axis != None:	
			# I still need to transform based on the light's matrix
			# get the inverse matrix first
			# if this is point light, I should probably set the rotation values to zero	
			# step 1, transform the world to left-handed
			ri.RiScale(-1, 1, 1)
			ri.RiRotate(180, 0, 1, 0)
			if axis == "px":		
				ri.RiRotate(90, 0, 1, 0)
			elif axis == "nx":	
				#ri.RiRotate(-90, 1, 0, 0)
				ri.RiRotate(-90, 0, 1, 0)				
			elif axis == "py":				
				ri.RiRotate(-90, 1, 0, 0)
			elif axis == "ny":
				ri.RiRotate(90, 1, 0, 0)				
			elif axis == "pz":				
				ri.RiRotate(180, 0, 1, 0) 	

			cmatrix = self.object.getInverseMatrix()
			#sMat = Blender.Mathutils.ScaleMatrix(-1, 4, vecX)		
			#rMat = Blender.Mathutils.RotationMatrix(180, 4, "y")
			#mat = cmatrix * sMat * rMat
			trans = cmatrix.translationPart()
			#print "\n"
			#print "At shadowmap generation for axis:", axis
			#print "Light translation is", trans
			#print "\n"
			ri.RiTranslate(trans)
			
	def getRenderDirections(self):
			return ["px", "py", "pz", "nx", "ny", "nz"]
			
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
					ptype = "matrix"
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
				ptype = "str"
				val = param			
			
				
			if not initialized:				
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
						
			else:
				print "No type found, this parameter was a ", p_type
				parm_value = None
			if initialized == False:
				print "Declaring a variable!"
				shader.declare(p_name, type=convtypes[p_type], default=parm_value)
				# shader.createSlot(p_name, convtypes[p_type], None, parm_value) # Here we set the default value to the parameters incoming value.
				
			
			# and set the value 
			if parm_value != None:
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

		if len(self.properties) > 0:
			# I've got at least one property
			for property in self.properties:
				
				if self.properties[property].saveable:
					xmlProp = self.properties[property].toXML(xml)
					xmlProp.setAttribute("name", property) # set the name attribute here for the moment, but later configure a property to have a name AND a title!
					objXml.appendChild(xmlProp)
		
			
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
		xmlTypes = { "int" : int, "str" : str, "list" : list, "dict": dict, "float" : float, "bool": bool }
		self.objData = {}
		atts = xml.attributes
		# setup the attributes first, since I will need the shader filename if I'm using slparams		
		for att in range(atts.length):
			xAtt = atts.item(att)
			self.objData[xAtt.name] = xAtt.value.encode("ascii")
		
		xmlProperties = xml.getElementsByTagName("property")
		# what I should really do is move the properties from the object's UI to the object adapter, since that's really where I control the data.
		for xmlProperty in xmlProperties:
			# each of these is a property element
			propertyName = xmlProperty.getAttribute("name").encode("ascii")		
			xmlType = xmlProperty.getAttribute("type").encode("ascii")			
			xmlValue= xmlProperty.getAttribute("value").encode("ascii")
			if xmlType == "bool":
				propertyValue = xmlTypes[xmlType](eval(xmlValue))
			elif xmlType == "list":
				propertyValue = [float(eval(xmlProperty.getAttribute("red").encode("ascii"))), 
							float(eval(xmlProperty.getAttribute("green").encode("ascii"))),
							float(eval(xmlProperty.getAttribute("blue").encode("ascii")))] # rebuild a pretty color!
				# print propertyValue
			else:
				propertyValue = xmlTypes[xmlType](xmlValue)
				# print "Value for property ", propertyName, " is ", propertyValue
			self.editors[propertyName].setValue(propertyValue) # 
			
			
		# setup the properties for this objects				
		self.initMaterial()
		
		# test for any shaders
		parms = xml.getElementsByTagName("shader")
		if len(parms) > 0:
			# I've got some shader or another
			# strip the params out and stuff them into the "shaderparams" key
			self.initShader(useXML = True, xml = parms)
		
		self.checkReset()
			
	def initShader(self, useXML = False, xml = None):		
		try:
			if self.settings.use_slparams:
				shaderPath = self.settings.getShaderSearchPaths()[0]
				
				# try to find the shader file in question. In truth, the default files should exist
				# get the first shader search path for pre-generated shaders				
				initialized = True
				
				shader = cgkit.rmshader.RMShader(self.objData["shaderfilename"])
				if useXML:
					self.populateShaderParams(xml, shader, initialized)
				else:
					if self.objData.has_key("shaderparms"):
						self.populateShaderParamsList(self.objData["shaderparms"], shader, initialized)		
				self.shader = btor.BtoRMain.GenericShader(shader, self.shader_type, self)
			else:				
				initialized = False
				
				shader = cgkit.rmshader.RMShader() # blank shader here
				# sine I'm not relying upon slparams to build the light shader params for me, I must instead do something like
				# file = os.path.basename(self.objData["shaderfilename"])
				# so here, let's see if the light shader I want is actually in the first shader path..that should be in the filename
				
				# path = self.settings.getPathForShader(self.objData["shaderfilename"], self.shader_type)# here I should have a path
				
				#if path == None:					
				#	self.evt_manager.showErrorDialog("Missing shader or shader path!", "Error: The default " + self.shader_type + " shader(s) could not be found! Check your setup.")
				#	raise ValueError
				#else:					
				self.shader = btor.BtoRMain.GenericShader(None, self.shader_type, self)
				#
				#if self.shader.searchPaths.getValue() != path:
				#	self.shader.setSearchPath(path) # set the path to the path where my light shader is. Should be the *first* path listed, if not, move on
					# hopefully this doesn't break stuff.
				#else: # with luck, this is always the case
				path = os.path.split(self.objData["shaderfilename"])
				
				self.shader.setSearchPath(path[0])					
				self.shader.setShader(path[1]) # this should update the controls
				# self.shader.shader.shadername = self.objData["shaderparms"]["shadername"]
				if self.shader.shader != None:
					self.shader.shader.filename = self.objData["shaderfilename"]
					self.populateShaderParamsList(self.objData["shaderparms"], self.shader.shader, True)	# so I can now setup the parameters
				
				

		except:
			traceback.print_exc()
			self.shader = btor.BtoRMain.GenericShader(None, self.shader_type, self)
		
		self.objEditor.setShader(self.shader)
		
		# self.objEditor.shaderButton.setTitle(self.shader.getShaderName())
	def setMaterial(self, material, preInit = False):	
		if material != None:
			self.material = material
			if preInit == False:
				self.objData["material"] = material.material.name
			self.editors["material"].setMaterial(material)
	def initMaterial(self):	
		if self.objData.has_key("material"):
			# print self.objData["material"], " was found"
			if self.objData["material"] != None and self.objData["material"] != "None":
				self.editors["material"].setMaterial(self.materials.getMaterial(self.objData["material"]))
			
	def renderMaterial(self):
		Bmaterial = self.materials.getMaterial(self.getProperty("material"))
		if Bmaterial == "None" or Bmaterial == None:
			# generate a default material - make sure to setup a default material button in the scene settings dialog
			print "rendering matte material"
			ri.RiColor(cgkit.cgtypes.vec3(1.0, 1.0, 1.0))
			ri.RiOpacity(cgkit.cgtypes.vec3(1.0, 1.0, 1.0))
			ri.RiSurface("matte", { "Ka" : 1.0, "Kd" : 1.0 })
		else:
			if Bmaterial != None:				
				# test for a transform 
				translated = False
				rotated = False
				scaled = False
				transform = False
				translation = Bmaterial.getProperty("Translation")
				rotation = Bmaterial.getProperty("Rotation").obj
				scale = Bmaterial.getProperty("Scale")
				print "material transform info is:"
				print translation
				print rotation
				print scale
				if translation[0] <> 0.0 or translation[1] <> 0.0 or translation[1] <> 0.0:
					translated = True
					transform = True
				if rotation[1] or rotation[2] or rotation[3]:
					rotated = True
					transform = True
				if scale[0] != 0.0 or scale[1] != 0.0 or scale[2] != 0.0:
					scaled = True
					transform = True
					
				if transform:
					ri.RiTransformBegin()
					if translated:
						ri.RiTranslate(translation)
					if rotated:
						# here I have to figure out the axis of rotation.
						x, y, z = 0, 0, 0
						if rotation[1]:
							x = 1
						if rotation[2]:
							y = 1
						if rotation[3]:
							z = 1
							
						ri.RiRotate(rotation[0], x, y, z)
					if scaled:
						print "material scale is ", scale
						ri.RiScale(scale[0], scale[1], scale[2])
						
			
				material = Bmaterial.material
				Bmaterial.getProperty("Surface").getObject().updateShaderParams()
				Bmaterial.getProperty("Displacement").getObject().updateShaderParams()
				Bmaterial.getProperty("Volume").getObject().updateShaderParams()
				
				ri.RiColor(Bmaterial.getProperty("color"))			
				ri.RiOpacity(Bmaterial.getProperty("opacity"))
				# thus
				if material.surfaceShaderName() != None:				
					ri.RiSurface(material.surfaceShaderName(), material.surfaceShaderParams(0))
				if material.displacementShaderName() != None:				
					ri.RiDisplacement(material.displacementShaderName(), material.displacementShaderParams(0))
				if material.interiorShaderName() != None:
					ri.RiAtmosphere(material.interiorShaderName(), material.interiorShaderParams(0))
				if transform:
					ri.RiTransformEnd()
				
class MeshAdapter(ObjectAdapter):
	""" BtoR mesh Adapter """
	protocols.advise(instancesProvide=[IObjectAdapter], asAdapterForTypes=[BtoRMesh])
	def __init__(self, object):
		""" Initialize a mesh export adapter """		
		ObjectAdapter.__init__(self, object)
			
			
	def render(self, shadowPass = False, envPass = False, renderLayers = []):
		render = False
		renderLayer = False
		illumList = []
		# test for layer visibility first
		if not self.scene.getProperty("RenderLayers"):
			for layer in self.object.layers:
				if layer in self.scene.layers:
					# object exists in at least one layer that's enabled
					renderLayer = True
					break
		else:
			renderLayer = True
				
		if renderLayer:
							
			if shadowPass:				
				if self.properties["RenderInShadowPass"].getValue():				
					if self.lighting.getProperty("layerLighting"): # if I'm using layerLighting, then shadowPass rendering is only neccessary if any lights are illuminating this object.
						for layer in self.object.layers:
							if layer in self.scene.currentLightLayers:
								render = True
								break # first one wins
					else: # if I'm not using layerLighting, then render always if RenderInShadowPass is true
						render = True
			elif envPass:
				if self.properties["RenderInEnvMaps"].getValue():
					render = True
			else:			
				if self.lighting.getProperty("layerLighting"):
					# if I'm using layerLighting, then I know that all of my lights are going to be OFF by default
					# I need to turn some lights on and off here				
					
					for layer in self.object.layers:
						for light in self.scene.lightingLayers[layer]: 
							if light not in illumList:
								illumList.append(light)

				render = True					
					
			if render:						
				if not shadowPass:
					if len(illumList) > 0:				
						ri.RiAttributeBegin() # push the graphics state. All lights should be off
						for light in illumList:
							ri.RiIlluminate(light, ri.RI_TRUE)
				
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
						if not shadowPass:
							self.renderAttributes()
						ri.RiTransform(obj[1]) # that should be the duplicated matrix and in fact should take into account rotations and scaling, all that
						ri.RiObjectInstance(objSequence)	
						ri.RiAttributeEnd()
						ri.RiTransformEnd()
						instance = instance + 1
					ri.RiAttributeEnd()
				else:
					ri.RiAttributeBegin()				
					ri.RiAttribute("identifier", "name", [self.objData["name"]]) # give it a name
					if not shadowPass:
						self.renderAttributes()
					ri.RiAttribute("displacementbound", "sphere", self.getProperty("DispBound"))
					srate = self.getProperty("ShadingRate")
					sceneRate = self.scene.getProperty("ShadingRate")
					if srate != sceneRate:
						ri.RiShadingRate(srate)
						
					ri.RiTransformBegin()	 
						
					if shadowPass and self.getProperty("ShadowMats"):					
						self.renderMaterial()
					elif not shadowPass and not self.getProperty("Matte"):
						self.renderMaterial()
							
					ri.RiTransform(self.object.matrix) # transform				
					# self.renderMeshData() # and render the mesh	
					archiveFile = self.objData["archive"] + self.objData["name"] + ".rib"
					if self.settings.renderer == "BMRT":
						archiveFile = archiveFile.replace("\\", "\\\\")
					ri.RiReadArchive(archiveFile) # this should read from the archive path
					ri.RiTransformEnd()
					ri.RiAttributeEnd()
					# aaaannnnnd done.
					# I rendered this object, so flag the 
					self.changed = False
				if len(illumList) > 0:
					ri.RiAttributeEnd() # pop the graphics state back out
					
		
	def renderMeshData(self):
		
		# check first for special case rendering options
		if self.getProperty("FacePatches"):
			self.renderFacePatches()
		else:
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
				
	def renderArchive(self):
		""" Write this object to an external archive file. """
		# add support here for detecting if materials should be exported to archives or not.
		# I should probably add that as an option in the export settings dialog.
		
		ri.RiBegin(self.objData["archive"] + self.objData["name"] + ".rib")  # this should be ok
		# this is pure geometry here
		ri.RiAttributeBegin()
		ri.RiTransformBegin()
		self.renderMeshData()
		ri.RiTransformEnd()
		ri.RiAttributeEnd()
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
		

	def setGroup(self, group):
		self.objData["group"] = group
		self.objEditor.objectGroupButton.setTitle(group)
	
	def renderPointsPolygons(self):
		""" Export Renderman PointsPolygons object. """
		params = {}
		mesh = self.object.getData(False, True)
		points = []
		normals = []
		fvNormals = []
		faceVarying_N = False
		autosmooth = False
		for v in mesh.verts:
			points.append(v.co)
			#if autosmooth:
			#	normals.append(vert[4])
			#else:
				# setup for faceVarying normals
				
				# if mesh.faces[idx].smooth == 1:
			#	if 1 == 2:
			#		faceVarying_N = True
			#		fvNormals.append(vert[4]) # append the normal
			#	else:					
					# fvNormals.append(mesh.faces[idx].no)
			#		pass
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
		if mesh.faceUV == 1 and self.getProperty("ExportSt"):
			ri.RiDeclare("st", "facevarying float[2]") # declare ST just in case
			params["st"] = st
		if mesh.vertexColors == 1 and self.getProperty("ExportCS"): 
			vCol = []
			for vertCol in Cs:
				vCol.append([vertCol.r / 256.0, vertCol.g / 256.0, vertCol.b / 256.0])			
			params["Cs"] = vCol
			
		# handle normals issues
		if faceVarying_N:
			ri.RiDeclare("N", "facevarying normal") # declare N to be facevarying
			params["N"] = normals
		elif autosmooth:
			params["N"] = normals
		params = {"P":points }
			
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
			
	def xrenderSubdivMesh(self):
		""" render a subdivision mesh 
		This version reorders everything (faces, verts) in the mesh so it's sorted by z, x, y
		"""
		getCs = False
		getSt = False
		mesh = self.object.getData(False, True)		
		vList = range(len(mesh.verts)) # vertex list
				
		# vertsByIndex = range(len(mesh.verts))		
		facesByZ = range(len(mesh.faces))
		zVerts = []
		
		if mesh.vertexColors == 1:
			getCs = True
		if mesh.faceUV == 1:
			getSt = True
		
		# create a list of faces sorted by least z,x,y by using the least z,x,y vert of the face as the sorting key
		
		for face in mesh.faces:				
			# build an array of z,x,y-least verts from this face, then use the least of that as an index to this face.
			zList = []
			for vert in face.verts:
				zList.append((vert.co[2], vert.co[0], vert.co[1], vert.index, face.index)) # ok, both vertex and face indexes
				vList[vert.index] = [vert.co[2], vert.co[0], vert.co[1], vert.index]
			zList.sort()			
			zVerts.append(zList[0])
			

		zVerts.sort() # this is now the list of *faces* by Z
		vList.sort() # this is the list of *verts* sorted by Z
		
		points = []
		crossRef = range(len(mesh.verts)) 		
		index = 0				
		
		# create the cross reference list that indexes:
		# new index (index) vs. blender index (vert[4]])
		# so that crossRef[index] results in original vert.index
		# face vert ID access is thus
		# fVertIdx = crossRef[face.vert[index].index]
		
		for vert in vList:	
			crossRef[vert[3]] = index # cross reference item 			
			index = index + 1
			points.append([vert[1], vert[2], vert[0]]) # append to the POINTS array
		
		
		
		# now generate a list of vertex IDs for each face by iterating zVerts
		# retrieving the face index, getting the face verts, and cross-referencing them with the 
		# new z,x,y sorted verts in points.
		
		vertIDs = []
		nVerts = []
		normals = []
		fvNormals = []
		
		uv = []
		autosmooth = False
		faceVarying_N = False		
		
		Cs = range(len(mesh.verts)) 
		st = range(len(mesh.verts))
		
		print facesByZ
		
		for zVert in zVerts:	
			print "Zvert is:", zVert
			idx = zVert[4]
			face = mesh.faces[idx]
			vertList = []
			for vert in face.verts:
				vertList.append((vert.co[2], vert.co[0], vert.co[1], vert.index, vert.no))						
				
			nVerts.append(len(vertList)) # number of verts
			vertList.sort() # sort the verts here			
			for vert in vertList:
				# step one,  point data				
				vertIDs.append(crossRef[vert[3]]) # vertex id for this vert
				if autosmooth:
					normals.append(vert[4])
				else:
					# setup for faceVarying normals
					if mesh.faces[idx].smooth == 1:
						faceVarying_N = True
						fvNormals.append(vert[4]) # append the normal
					else:
						fvNormals.append(mesh.faces[idx].no)
				if getCs:
					Cs[crossRef[vert[3]]] = face.col[vert[3]] 
				if getSt:
					# here is an issue
					# Do I want to test to see if this value exists already? or not? I know that I've already got a range here, so
					st[crossRef[vert[3]]] = [face.uv[vert[3]][0],1.0 - face.uv[vert[3]][1]]
		
		creases = {}
		# develop a list of creases based on crease value.
		for edge in mesh.edges:		
			if edge.crease > 0:
				if edge.crease not in creases:
					creases[edge.crease] = []
				creases[edge.crease].append([crossRef[edge.v1.index], crossRef[edge.v2.index]])
		
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
				
		tags = []
		nargs = []
		intargs = []
		floatargs = []
		
		for crease in creaselist:
			if type(crease) != type(1):
				tags.append("crease")		
				nargs.append(len(crease[1]))
				nargs.append(1)
				for item in crease[1]:
					intargs.append(item)
				
				val = (float(crease[0]) / 255) * 5.0
				floatargs.append(val) # normalized currently for the Aqsis renderer
				
		# I should make this a property
		if self.getProperty("interpolateBoundary"):
			tags.append("interpolateboundary")
			nargs.append(0)
			nargs.append(0)
		
		params = {"P":points }
		if autosmooth or faceVarying_N:
			params["N"] = normals
		
			
		if mesh.faceUV == 1 and self.getProperty("ExportSt"):
			ri.RiDeclare("st", "facevarying float[2]") # declare ST just in case
			params["st"] = st
		if mesh.vertexColors == 1 and self.getProperty("ExportCS"): 
			vCol = []
			for vertCol in Cs:
				vCol.append([vertCol.r / 256.0, vertCol.g / 256.0, vertCol.b / 256.0])			
			params["Cs"] = vCol
			
		# handle normals issues
		if faceVarying_N:
			ri.RiDeclare("N", "facevarying normal") # declare N to be facevarying
		ri.RiSubdivisionMesh("catmull-clark", nVerts, vertIDs, tags, nargs, intargs, floatargs, params)
		
		
	def sortFaceVerts(self, face):
		v_list = []
		for vert in face.verts:
			v_list = (vert.co[2], vert.co[1], vert.co[0], vert.index)
		
		v_list.sort()
		return v_list
		
		
		
		
	def obfuscate(self):
		# don't forget reference geometry. I need the base mesh before lattice/armature transforms are applied to it.
		# I can probably disable all modifiers except for decimate and gather that mesh, 
		# then turn them all back on (excepting subsurf) and gather *that* mesh
		# then I do
		# params["PRef"] =  refPoints
		# for all the other stuff, I also need
		# params["Cs"] = vertColors  # per vertex colors
		# params["Cs"] = vertCoors # or Face UV colors		
		# params["FModes"] = faceModes # face display modes to pass to custom shaders
		pass
		
		
		
	def renderSubdivMesh(self):
		""" Export Subdivision mesh. """
		mesh = self.object.getData(False, True)
		modifiers = self.object.modifiers
		
		points = []
		normals = []
		uv = []
		faceModes = []
		
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
		getCs = False
		getSt = False
		
		if mesh.vertexColors == 1:
			getCs = True
		if mesh.faceUV == 1:
			getSt = True
		
		edge_faces = self.edge_face_users(mesh)
		
		for face in mesh.faces:
			# print face.index
			nverts.append(len(face.v))
			vtuv = []
			if len(face.v) > 2:
				for vertIdx in range(len(face.v)):
					if getCs:
						Cs[face.v[vertIdx].index] = face.col[vertIdx]
					if getSt:
						# here is an issue
						# Do I want to test to see if this value exists already? or not? I know that I've already got a range here, so
						if st[face.v[vertIdx].index] == face.v[vertIdx].index:
							st[face.v[vertIdx].index] = [face.uv[vertIdx][0],1.0 - face.uv[vertIdx][1]]
					
			for vert in face.v:
				vertids.append(vert.index)
					
		if 1 == 2:
			# this has to be calculated per edge, so what I probably want to do first is build up a list of face index per edge
			# get the normals of each face in the edge_faces list
			creases = {}
			creaselist = range(len(mesh.edges))
			edgeIdx = 0
			maxAng = self.getProperty("MaxCreaseAngle")			
			for edge in edge_faces:
				
				if len(edge) > 1:
					a = edge[0]
					b = edge[1]
					ang = Blender.Mathutils.AngleBetweenVecs(a.no, b.no)
					# my angle range is -90 to 90 degrees
					# thus
					if ang < 0:
						ang = -ang
						
					if ang > maxAng:
						factor = 1.0
					else:
						# calculate the value for the crease using 0-1.0 range, as applied to the range 0-maxAng.
						factor = ang / maxAng
					
					creaselist[edgeIdx] = [factor, [mesh.edges[edgeIdx].v1.index, mesh.edges[edgeIdx].v2.index]] # this is 1:1 because of how I'm doing this					
					edgeIdx = edgeIdx + 1
					
		else:
			
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
			
		tags = []
		nargs = []
		intargs = []
		floatargs = []
		
		for crease in creaselist:
			if type(crease) != type(1):
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
			
		if mesh.faceUV == 1 and self.getProperty("ExportSt"):
			params["st"] = st
		if mesh.vertexColors == 1 and self.getProperty("ExportCS"): 
			vCol = []
			for vertCol in Cs:
				vCol.append([vertCol.r / 256.0, vertCol.g / 256.0, vertCol.b / 256.0])			
			params["Cs"] = vCol
			
		# and why can't I output both?
			
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
		if mesh.faceUV == 1 and self.getProperty("ExportST"):
			ri.RiDeclare("st", "facevarying float[2]") # declare ST just in case
			
		ri.RiSubdivisionMesh("catmull-clark", nverts, vertids, tags, nargs, intargs, floatargs, params)
				
	def find_set(self, val, set_list):
		""" Find a set in a set"""
		for set in set_list:
			if val in set:
				return set
				
	def merge_set(self, seta, setb):
		""" merge two sets """
		return seta.union(setb)
		
	def renderFacePatches(self):
		mesh = self.object.getData(False, True)
		for face in mesh.faces:
			# for all faces with four verts, replace the face with a patch
			v = face.v
			if len(v) == 4: # render a patch
				ri.RiPatch("bilinear", {"P":[v[0].co, v[1].co, v[3].co, v[2].co] }) 
			
	# from BpyMesh
	def sorted_edge_indicies(self, ed):
		i1= ed.v1.index
		i2= ed.v2.index
		if i1>i2:
			i1,i2= i2,i1
		return i1, i2

	def edge_face_users(self, me):
		''' 
		Takes a mesh and returns a list aligned with the meshes edges.
		Each item is a list of the faces that use the edge
		would be the equiv for having ed.face_users as a property
		'''
		
		face_edges_dict= dict([(self.sorted_edge_indicies(ed), (ed.index, [])) for ed in me.edges])
		for f in me.faces:
			fvi= [v.index for v in f.v]# face vert idx's
			for i in xrange(len(f)):
				i1= fvi[i]
				i2= fvi[i-1]
				
				if i1>i2:
					i1,i2= i2,i1
				
				face_edges_dict[i1,i2][1].append(f)
		
		face_edges= [None] * len(me.edges)
		for ed_index, ed_faces in face_edges_dict.itervalues():
			face_edges[ed_index]= ed_faces
		
		return face_edges
		
	def face_edges(self, me):
		'''
		Returns a list alligned to the meshes faces.
		each item is a list of lists: that is 
		face_edges -> face indicies
		face_edges[i] -> list referencs local faces v indicies 1,2,3 &| 4
		face_edges[i][j] -> list of faces that this edge uses.
		crap this is tricky to explain :/
		'''
		face_edges= [ [None] * len(f) for f in me.faces ]
		
		face_edges_dict= dict([(self.sorted_edge_indicies(ed), []) for ed in me.edges])
		for fidx, f in enumerate(me.faces):
			fvi= [v.index for v in f.v]# face vert idx's
			for i in xrange(len(f)):
				i1= fvi[i]
				i2= fvi[i-1]
				
				if i1>i2:
					i1,i2= i2,i1
				
				edge_face_users= face_edges_dict[i1,i2]
				edge_face_users.append(f)
				
				face_edges[fidx][i]= edge_face_users
				
		return face_edges
		
class LampAdapter(ObjectAdapter):
	""" BtoR Lamp Adapter object """
	protocols.advise(instancesProvide=[IObjectAdapter], asAdapterForTypes=[BtoRLamp])
	def __init__(self, object):
		""" Initialize a Lamp export adapter """
		ObjectAdapter.__init__(self, object)	
		self.isAnimated = False 
		self.shadowMaps = {} # indexed by direction_frame method
			
	def render(self): 
		""" Generate renderman data for this object. """
		# I should call checkReset here to make sure I have the latest/greatest up to date data for this lamp, no?
		# shader = self.getProperty("shader").getObject().shader
		self.checkReset()
		self.shader.updateShaderParams()
		
		# I should probably get the light handle here
		if self.getProperty("transformLight"):
			ri.RiTransformBegin()
			ri.RiTranform(self.object.getMatrix())
		lightHandle = ri.RiLightSource(self.shader.shader.shadername, self.shader.shader.params())
		
		if self.scene.getProperty("RenderLayers"):
			allLayers = True
		else:
			allLayers = False
		# I need lightHandle for turning lights on and off for layer support
		if self.lighting.getProperty("layerLighting"): # 
			# unless this light is flagged as global, turn it off in the current graphics state.
			if not self.getProperty("globalLight"):
				ri.RiIlluminate(lightHandle, ri.RI_FALSE)
			for layer in self.object.layers:
				if allLayers:
					self.scene.lightingLayers[layer].append(lightHandle)
				else:
					if layer in self.scene.layers: # if the current layer is ON in the global layer list, then illuminate this layer
						self.scene.lightingLayers[layer].append(lightHandle)
						
		if self.getProperty("transformLight"):
			ri.RiTransformEnd()
	
	def generateRSL(self):
		""" Create RSL for this light. """
		pass
	def getScreenWindow(self, camera):
		""" Figure out the mapping between screen space and world space for an orthographic transform """
		# get the camera's matrix.
		cMat = camera.getMatrix()
		# this gives me the camera's transform matrix.
		# now what I have to do is 
		
	def genCheckSum(self):
		# the only settings I care about are
		# scale/rot/trans, obviously. I can probably get the transform matrix and use that, or make vectors of the three and go from there
		# then I want the type
		# the color...energy...'only shadow'...maybe the textures.
		lamp = self.obj.getData()
		name = self.obj.getName()
		intensity  =  "%f" % lamp.intensity		
		loc = "%f_%f_%f" % (self.obj.LocX, self.obj.LocY, self.obj.LocZ)
		rot = "%f_%f_%f" % (self.obj.RotX, self.obj.RotY, self.obj.RotZ)
		scale = "%f_%f_%f" % (self.obj.ScaleX, self.obj.ScaleY, self.obj.ScaleZ)
		energy = "%f" % lamp.getEnergy()
		r = "%f" % lamp.R
		g = "%f" % lamp.G
		b = "%f" % lamp.B
		# lamp types 
		# 0 - point
		# 1 - distant/sun
		# 2 - spot
		# 3 - hemi
		# 4 - area
		seed = name + intensity + loc + rot + scale + r + g + b # and this covers the pointlight
		if self.obj.getType() in [1, 2]:
			# I'm interested in spotsi and bias stuff
			# I probably want to worry about falloff too
			bias = "%f" % lamp.bias
			
			samples =  "%f" % lamp.samples
			spotsize =  "%f" % lamp.spotSize
			spotblend =  "%f" %  lamp.spotBlend 
			seed = seed + bias + samples + spotsize + spotblend # and this covers spots
			
		# now I have to worry about shader parameters being changed, because in the case of non-standard shader params (you know, different light shader), I might need to regenerate the shadow map even if the 
		# light wasn't actually touched in blender.
		shader = self.getProperty("shader").getObject().shader
		shaderName = shader.shaderName() # should I worry about the filename too?
		params = shader.params().__repr__()
			
		
		hash = md5.new(seed)
		
		return hash
			
		
	def doCameraTransform(self, axis):

		if axis != "shadow":	
			
			# I still need to transform based on the light's matrix
			# get the inverse matrix first
			# if this is point light, I should probably set the rotation values to zero	
			ri.RiScale(-1, 1, 1)
						
			if axis == "px":		
				ri.RiRotate(180, 0, 1, 0)
				ri.RiRotate(90, 0, 1, 0)
			elif axis == "nx":	
				ri.RiRotate(180, 0, 1, 0)			
				ri.RiRotate(-90, 0, 1, 0)				
			elif axis == "py":				
				ri.RiRotate(-90, 1, 0, 0)
			elif axis == "ny":
				ri.RiRotate(90, 1, 0, 0)				
			elif axis == "nz":				
				ri.RiRotate(180, 0, 1, 0) 	
			
			ri.RiTranslate(-self.object.LocX, -self.object.LocY, -self.object.LocZ)
				
		else:		
			ri.RiScale(-1, 1, 1)
			#ri.RiRotate(180, 0, 1, 0)			
			cmatrix = self.object.getInverseMatrix()
			print cmatrix
			matrix = [[cmatrix[0][0], cmatrix[0][1], -cmatrix[0][2], cmatrix[0][3]],
				[cmatrix[1][0], cmatrix[1][1], -cmatrix[1][2], cmatrix[1][3]],
				[cmatrix[2][0], cmatrix[2][1], -cmatrix[2][2], cmatrix[2][3]],
				[cmatrix[3][0], cmatrix[3][1], -cmatrix[3][2], cmatrix[3][3]]]
			ri.RiTransform(matrix)

		self.genChecksum()
	
	def getClippingRange(self):
		light = self.object.getData()		
		return [light.getClipStart(), light.getClipEnd()]
			
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
		sparams = self.shader.shader.shaderparams
		
		#if shadername == "shadowpoint":			
		if sparams.has_key("sfpx"):				
			self.shader.setParamValue("sfpx", params["px"]["shadowName"])
			self.shader.setParamValue("sfpy", params["py"]["shadowName"])
			self.shader.setParamValue("sfpz", params["pz"]["shadowName"])
			self.shader.setParamValue("sfnx", params["nx"]["shadowName"])
			self.shader.setParamValue("sfny", params["ny"]["shadowName"])
			self.shader.setParamValue("sfnz", params["nz"]["shadowName"])
		elif sparams.has_key("px"):
			self.shader.setParamValue("px", params["px"]["shadowName"])
			self.shader.setParamValue("py", params["py"]["shadowName"])
			self.shader.setParamValue("pz", params["pz"]["shadowName"])
			self.shader.setParamValue("nx", params["nx"]["shadowName"])
			self.shader.setParamValue("ny", params["ny"]["shadowName"])
			self.shader.setParamValue("nz", params["nz"]["shadowName"])
		elif sparams.has_key("shadow"):
			if params.has_key("shadowName"):
				self.shader.setParamValue("shadow", params["shadow"]["shadowName"])
		elif sparams.has_key("shadowname"):
			print params
			if params.has_key("shadow"):
				self.shader.setParamValue("shadowname", params["shadow"]["shadowName"])
			
		#elif shadername == "shadowspot" or shadername == "shadowdistant" or shadername == "bml":
			# setattr(self.shader.shader, "shadowname", params["shadow"]["shadowName"])
		#	self.shader.setParamValue("shadowname", params["shadow"]["shadowName"])
			
		
	def getRenderDirections(self):
		print "Light Type is:", self.object.getData().getType()
		if self.object.getData().getType() == 0:
			return ["px", "py", "pz", "nx", "ny", "nz"]
		elif self.object.getData().getType() == 1:			
			# determine how to bring the shadow map back here
			return ["shadow"]
		elif self.object.getData().getType() == 2:
			# figure out how to render the direction this light is pointing
			return ["shadow"]
		else: 
			return []
		
		
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
		if self.settings.use_slparams:
			ext = ".sl"
		else:
			ext = "." + self.settings.renderers[self.settings.renderer][4]
		# am I going to worry about intensity and color? Maybe not, because that will change per shader I think.
		# perhaps I should gather the parameters for the bml shader and use that as my primary lighting shader.
		shaderParms["intensity"] = lamp.getEnergy()
		# I'm only worried at the moment about deriving the correct shader to use as a starting point for the lamp.
		# thus
		shaderParms["from"] = [x, y, z]
		shaderParms["lightcolor"] = [lamp.R, lamp.G, lamp.B]
		self.objData["lightcolor"] = [lamp.R, lamp.G, lamp.B]
		

		if lamp.type == 0 or lamp.type == 4:
			self.object.RotX = 0.0
			self.object.RotY = 0.0
			self.object.RotZ = 0.0
			self.objData["type"] = 0
			energyRatio = lamp.dist * negative
			# get the first selected shader path...and hope it's setup correctly
		
			shaderPath = self.settings.getShaderSearchPaths()[0]
			if self.lighting.getProperty("GenShadowMaps"):				
				sFilename = "shadowpoint" + ext
			else:
				sFilename = "pointlight" + ext
			self.objData["shaderfilename"] = os.path.normpath(shaderPath + os.sep + sFilename)
			# I'm only really concerned about this if I'm using sl params
				
			self.objData["shadername"] = "pointlight"			
			shaderParms["intensity"] = (energyRatio * lamp.energy) * self.getProperty("Multiplier")
			
			
		elif lamp.type == 1:

			self.objData["type"] = 1
			energyRatio = negative
			self.objData["shadername"] = "distantlight"			
		
			shaderPath = self.settings.getShaderSearchPaths()[0]
			if self.lighting.getProperty("GenShadowMaps"):
				sFilename = "shadowdistant" + ext
			else:
				sFilename = "distantlight" + ext
			self.objData["shaderfilename"] = os.path.normpath(shaderPath + os.sep + sFilename)
			shaderParms["to"] = [ tox, toy, toz]
			shaderParms["intensity"] = (energyRatio * lamp.energy) * self.getProperty("Multiplier")
			
		elif lamp.type == 2:
			self.objData["type"] = 2
			energyRatio = lamp.dist * negative
			shaderPath = self.settings.getShaderSearchPaths()[0]
			#if self.settings.useShadowMaps.getValue():
			#	sFilename = "shadowspot.sl"
			#else:
			if self.lighting.getProperty("GenShadowMaps"):				
				sFilename = "shadowspot" + ext
				self.objData["shadername"] = "shadowspot"
			else:
				self.objData["shadername"] = "spotlight"
				sFilename = "spotlight" + ext
				
			self.objData["shaderfilename"] = os.path.normpath(shaderPath + os.sep + sFilename)
			
			# shaderParms["shadowbias"] = lamp.bias
			#shaderParms["blur"] = 0.0
			#shaderParms["samples"] = lamp.samples
			shaderParms["coneangle"] = (lamp.spotSize * math.pi / 360)
			shaderParms["conedeltaangle"] = (lamp.spotBlend * (lamp.spotSize * math.pi / 360))			
			shaderParms["to"] = [tox, toy, toz]
			shaderParms["intensity"] = (energyRatio * lamp.energy) * self.lighting.getProperty("Multiplier")
			
			# This might need to be animated, so I need to add a function to deal with that
			if self.lighting.getProperty("GenShadowMaps"):
				shaderParms["shadowname"] = self.object.getName() + "shadow.tx"
			else:
				shaderParms["shadowname"] = None
		elif lamp.type == 3:
			self.objData["type"] = 3
			energyRatio = negative
			self.objData["shadername"] = "hemilight"
		
			shaderPath = self.settings.getShaderSearchPaths()[0]			
			self.objData["shaderfilename"] = os.path.normpath(shaderPath + os.sep + "hemilight" + ext)

			shaderParms["to"] = [tox, toy, toz]
			shaderParms["falloff"] = 0
			shaderParms["intensity"] = (energyRatio * lamp.energy) * self.getProperty("Multiplier")

				
			
		self.objData["shaderparms"] = shaderParms
		
		self.initShader() # initialize my light shader
				
	def checkReset(self):
		
		# here I want to check if the lamp settings need changing or not.
		# if the user has selected a shader type that doesn't match the lamp settings, all I want to affect in that case
		# is the light color.		
		shaderParms = self.objData["shaderparms"]
		
		lamp = self.object.getData()		
		
		# for the most part, follow the parameters for the given light object and stuff the values into the 
		# shader parms 
		if self.getProperty("autoLighting"):
			mat = self.object.getMatrix()
			
			x = self.object.matrix[3][0] / self.object.matrix[3][3]
			y = self.object.matrix[3][1] / self.object.matrix[3][3]
			z = self.object.matrix[3][2] / self.object.matrix[3][3]
			
			tox = -self.object.matrix[2][0] + self.object.matrix[3][0]
			toy = -self.object.matrix[2][1] + self.object.matrix[3][1]
			toz = -self.object.matrix[2][2] + self.object.matrix[3][2]
			
			shaderParms["intensity"] = lamp.getEnergy()
			
			if self.objData["type"] == lamp.type:
				
				if lamp.getMode() & lamp.Modes['Negative']:
					negative = -1
				else:
					negative = 1
					
				shaderParms["from"] = [x, y, z]
				shaderParms["lightcolor"] = [lamp.R, lamp.G, lamp.B]
				self.objData["lightcolor"] = [lamp.R, lamp.G, lamp.B]
				
				if (lamp.type == 0 or lamp.type == 4) and (self.shader.getShaderName() == "pointlight" or self.shader.getShaderName() == "shadowpoint"):
					self.object.RotX = 0.0
					self.object.RotY = 0.0
					self.object.RotZ = 0.0
					energyRatio = lamp.dist * negative
					# get the first selected shader path...and hope it's setup correctly
					shaderParms["intensity"] = (energyRatio * lamp.energy) *  self.lighting.getProperty("Multiplier")
					
				elif lamp.type == 1 and (self.shader.getShaderName() == "distantlight" or self.shader.getShaderName() == "shadowdistant"):
					energyRatio = negative
					# self.objData["shadername"] = "distantlight"				
					shaderParms["to"] = [ tox, toy, toz]
					
				elif lamp.type == 2 and (self.shader.getShaderName() == "shadowspot" or self.shader.getShaderName() == "spotlight"): 
					energyRatio = lamp.dist * negative
					# shaderParms["shadowbias"] = lamp.bias
					#if self.shader.getShaderName() == "shadowspot":
						# shaderParms["blur"] = 0.0	
					#	shaderParms["samples"] = lamp.samples
					shaderParms["coneangle"] = (lamp.spotSize * math.pi / 360)
					shaderParms["conedeltaangle"] = (lamp.spotBlend * (lamp.spotSize * math.pi / 360))			
					shaderParms["to"] = [tox, toy, toz]
					shaderParms["intensity"] = (energyRatio * lamp.energy) *  self.lighting.getProperty("Multiplier")				
					
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
		else:
			mat = self.object.getMatrix()
			
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
			params = self.shader.shader.shaderparams
			if params.has_key("intensity"):
				energyRatio = lamp.dist * negative
				shaderParms["intensity"] = (energyRatio * lamp.energy) * self.getProperty("Multiplier")
			if params.has_key("from"):
				shaderParms["from"] = [x, y, z]
			if params.has_key("to"):
				shaderParms["to"] = [tox, toy, toz]
			if params.has_key("lightcolor"):
				shaderParms["lightcolor"]  = [lamp.R, lamp.G, lamp.B] 
			for key in shaderParms:
				self.shader.setParamValue(key, shaderParms[key])
			self.objData["reset"] = False
		
			
	def getSelector(self):
		return self.objEditor.getSelector()

	
		
class MBallAdapter(ObjectAdapter):
	protocols.advise(instancesProvide=[IObjectAdapter], asAdapterForTypes=[BtoRMBall])
	def __init__(self, object):
		""" Initialize a metaball export adapter """
		ObjectAdapter.__init__(self, object)					
		
	def render(self):
		""" generate Renderman data for this object """
		# create RiBlobbies or convert to mesh depending upon renderer target and support
		
	def initObjectData(self):
		""" Initialize BtoR object data for this object """		
		print self.object
		
		
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
	
	def renderArchive(self):
		ri.RiBegin(self.objData["archive"] + self.objData["name"] + ".rib") 
		# this is pure geometry here
		#ri.RiAttributeBegin()
		#ri.RiTransformBegin()
		#self.renderCurveData()
		#ri.RiTransformEnd()
		#ri.RiAttributeEnd()
		#ri.RiEnd() 
		
	def renderCurveData(self):
		""" renders curve data to RiCurve objects """
		#curve = self.object.getData()
		#nVerts = len(curve.verts)
		
		
	def initObjectData(self):
		""" Initialize BtoR object data for this object """
		# do some mesh-related stuff
		# all I'm concerned with at the moment is whether or not the mesh in question is subsurfed or not. I don't care about levels and what-not,
		# since I can grab that from the blender object itself.
		self.objData = {}
		self.objData["name"] = self.object.getName()				
		self.objData["type"] = self.object.getType()
		#curveObject = self.object.getData()
		self.objData["material"] = "None"
		
		
		
class SurfaceAdapter(ObjectAdapter):
	protocols.advise(instancesProvide=[IObjectAdapter], asAdapterForTypes=[BtoRSurf])
	def __init__(self, object):		
		""" Iniitialize a surface export adapter """
		ObjectAdapter.__init__(self, object)					
		#self.editorPanel.title = "Surface Export Settings:"

	def render(self):
		""" generate Renderman data for this object """				
		
	def initObjectData(self):
		""" Initialize BtoR object data for this object """

			

class CameraAdapter(ObjectAdapter):
	protocols.advise(instancesProvide=[IObjectAdapter], asAdapterForTypes=[BtoRCamera])
	def __init__(self, object):		
		ObjectAdapter.__init__(self, object)					

	def render(self):
		""" generate Renderman data for this object """		
		shader = self.getProperty("shader").getObject()		
		# self.objEditor.shaderButton.title = self.imagerShader.shader_menu.getValue()
		if self.settings.renderer != "Pixie":
			if self.getProperty("autoImager"):				
				bWorld = Blender.World.GetCurrent()
				if bWorld != None: 
					if bWorld.hor != [0, 0, 0]:
						ri.RiDeclare("bgcolor", "uniform color")
						ri.RiDeclare("background", "uniform color")
						iparams = { "bgcolor" : [bWorld.hor[0], bWorld.hor[1], bWorld.hor[2]], "background" : [bWorld.hor[0], bWorld.hor[1], bWorld.hor[2]] }
						ri.RiImager( "background", iparams )
				
			elif shader.shader != None:
				if shader.getShaderName() != None and shader.getShaderName != "None Selected":				
					shader.updateShaderParams()
					ishader = shader.shader					
					if ishader.shadername != "" or ishader.shaderName() != None:
						ri.RiImager(ishader.shadername, ishader.params()) # and done
			
					
		scene = Blender.Scene.GetCurrent()
		render = scene.getRenderingContext()
		camera = self.object.getData()
		cType = camera.getType() 
		ri.RiFormat(render.imageSizeX(), render.imageSizeY(), 1) # leave aspect at 1 for the moment
		
		if cType == 0:	
			if render.imageSizeX() >= render.imageSizeY():
				factor = render.imageSizeY() / float(render.imageSizeX())
			else:
				factor = render.imageSizeX() / float(render.imageSizeY())
			
			fov = 360.0 * math.atan((16 * factor) / camera.lens) / math.pi
			print "Using a ", fov, " degree Field of View"
			ri.RiProjection("perspective", "fov", fov)
			# Depth of field			
			if self.properties["DOF"].getValue():
				ri.RiDepthOfField(self.properties["fstop"].getValue(), self.properties["focallength"].getValue(), self.properties["focaldistance"].getValue())
			# Depth of field done
			
		else:
			self.objData["scale"] = camera.getScale() 
			ri.RiProjection("orthographic") 
		# ri.RiFrameAspectRatio(factor)	
		# Camera clipping
		ri.RiClipping(camera.getClipStart(), camera.getClipEnd())		
		# Viewpoint transform
		vecX = Blender.Mathutils.Vector(1, 0, 0)
		cmatrix = self.object.getInverseMatrix()
		sMat = Blender.Mathutils.ScaleMatrix(-1, 4, vecX)		
		rMat = Blender.Mathutils.RotationMatrix(180, 4, "y")
		mat = cmatrix * sMat * rMat
		ri.RiTransform(mat)		
		#ri.RiTranslate(0, 0, 1)
		
	def initObjectData(self):
		# object initia.ize
		""" Initialize BtoR object data for this object """
		self.objData = {}
		self.objData["name"] = self.object.getName()
		self.objData["type"] = self.object.getType() 
		self.shader_type = "imager"
		shaderParams = {}
		shaderPath = self.settings.getShaderSearchPaths()[0]
		# this is a potential deficiency
		self.objData["shaderfilename"] = os.path.normpath(shaderPath + os.sep + "background.sl")
		bWorld = Blender.World.GetCurrent()
		shaderParams["bgcolor"] = [bWorld.hor[0], bWorld.hor[1], bWorld.hor[2]]
		shaderParams["background"] = [bWorld.hor[0], bWorld.hor[1], bWorld.hor[2]]
		self.objData["shaderparams"] = shaderParams
		self.initShader()
		# shader should be initialized, so then
		self.shader.setParamValue("bgcolor",shaderParams["bgcolor"])
		self.shader.setParamValue("background", shaderParams["bgcolor"])

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
		
# Object UIs
class ObjectUI:
	""" Object editor panel """
	protocols.advise(instancesProvide=[IObjectUI], asAdapterForTypes=[BtoRLattice, BtoRArmature, BtoRBasicObject, BtoREmpty, BtoRWave])

	def __init__(self, obj):
		dict = globals()	
		self.settings = dict["instBtoRSettings"]
		self.evt_manager = dict["instBtoREvtManager"]
		self.scene = dict["instBtoRSceneSettings"]
		self.materials = dict["instBtoRMaterials"]
		self.objecteditor = dict["instBtoRObjects"]
		self.grouplist = dict["instBtoRGroupList"]
		self.helpwindow = dict["instBtoRHelp"]
		self.lighting = dict["instBtoRLightManager"]
		self.mat_selector = self.materials.getSelector()
		
		self.editorPanel = ui.Panel(4, 70, 255,  320, "Empty Panel", "", None, False)
		self.editorPanel.titleColor = [255,255,255, 255]
		self.editorPanel.hasHeader = False
		self.editorPanel.cornermask = 0
		self.editorPanel.shadowed = False
		self.editorPanel.outlined = False
		self.editorPanel.addElement(ui.Label(10, 25, "Object Properties", "Object Properties:", self.editorPanel, True))
		self.scroller= ui.ScrollPane(10, 50, 240, 235, "Scroller", "Scroller", self.editorPanel, True)
		
		self.attButton = ui.Button(self.editorPanel.width - 90, 0, 80, 25, "Atts", "Attributes", 'small', self.editorPanel, True)
		self.attButton.registerCallback("release", self.showAttributes)
		
		# hovering panel on the right for renderer-specific attributes
		self.attributePanel = ui.Panel(self.editorPanel.width + 10, 0, 255, 320, "Atts", "  Renderer Specific Attributes", self.editorPanel, True, fontsize = 'small')
		self.attributePanel.isVisible = False
		self.attributePanel.hasHeader = False
		self.attributePanel.shadowed = True
		self.attributePanel.outlined = True
		
		self.attributeScroller = ui.ScrollPane(5, 25, 245, 295, "Scroller", "Scroller", self.attributePanel, True)
		
		self.properties = {}
		self.editors = {}
		
		# setup renderer-specific options
		if self.__dict__.has_key("optionOrder"):
			for option in self.optionOrder:
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
		
		self.setupAttributes()
		self.settings.rendererListeners.append(self)
			
	def setupAttributes(self):
		self.rendererAtts = {}
		self.rendererAttEditors = {}		
		
		# setup renderer attributes
		atts = self.settings.getRendererAttributes()
		for att in atts:
			self.rendererAtts[att] = IProperty(atts[att][1])
			self.rendererAtts[att].setName(atts[att][0])
			self.rendererAtts[att].setWidth(self.attributeScroller.width - 15)
			
			self.rendererAttEditors[att] = IPropertyEditor(self.rendererAtts[att])
			self.attributeScroller.addElement(self.rendererAttEditors[att].getEditor())
			self.rendererAttEditors[att].setParent(self.attributeScroller)
			self.attributeScroller.offset = 0
	
	def updateAttributes(self):
		self.attributeScroller.clearElements()
		self.setupAttributes()
			
	def getEditor(self):
		""" get the object editor for this object. """
		return self.editorPanel
		
	def reloadOptions(self):
		self.scroller.clearElements()
		# simple enough to reload everything from the options array
		for option in self.optionOrder:
			self.scroller.addElement(self.editors[option].getEditor())
			
	def showAttributes(self, button):
		if self.attributePanel.isVisible:
			self.attributePanel.hide()
		else:
			self.attributePanel.show()
		

class MeshUI(ObjectUI):
	object_output_options = ["Mesh", "Renderman Primitive", "RA Proxy", "RA Procedural"]
	mesh_output_options = ["basic", "SubDiv"]
	protocols.advise(instancesProvide=[IObjectUI], asAdapterForTypes=[BtoRMesh])
	""" This is the object editor. Here you can assign materials, and set various options for the export. """
	
	def __init__(self, obj):
		self.options = { "material":["material", BtoRMaterialType("None Selected")],
			"AutoCrease" : ["Automatic Creasing?", False], 
			"MaxCreaseAngle" : ["Max AutoCrease Angle", 90.0],
			"RIBEntity" : ["Save as RIB Entity", False], 
			"IncludeMats" : ["Include Materials in RIB Entity", True],
			"ShadowMats" : [ "Include Materials in Shadowmap", False],
			"RenderInShadowPass" : [ "Include object in Shadowmap", True],
			"GenEnvMaps" : ["Generate Environment Maps" , False],
			"MapType" : ["Environment Map Type:", { "Cubic": "cubic", "Spherical": "spherical"}],
			"RenderInEnvMaps" : ["Include object in Environment Maps", True],
			"EnvMapPixelFilter" : ["EnvMap Pixel Filter", {"box":"box", "triangle":"triangle", "catmull-rom":"catmull-rom", "sinc":"sinc", "gaussian":"gaussian"}],
			"EnvMapFilterX":["EnvMap Filter size X", 1],
			"EnvMapFilterY":["EnvMap Filter size Y", 1],
			"EnvMapSamplesX":["EnvMap Samples X", 1],
			"EnvMapSamplesY":["EnvMap Samples Y", 1],
			"EnvMapShadingRate" : ["EnvMapShadingRate", 1.0],
			"DefineAsObj" : ["Define as RiObject", False]	,
			"InplaceInstance" : ["In-Place Instancing", False],
			"InstanceCount" : ["Number of Instances", 1],
			"AutoRandomize" : ["Auto Randomize Material", False],
			"OutputOptions" : ["Object Output Options:", {"Mesh" : "mesh", "Renderman Primitive" : "primtive", "RA Proxy" : "proxy", "RA Procedural" : "procedural" }],
			"Matte" : ["Treat as Matte", False],
			"Ignore" : ["Don't Export Object:", False],
			"Sides" : ["Sides", 1],
			"ShadingRate" : ["Shading Rate", 1.0],
			"DispBound" : ["Displacement Bound", 2.0],
			"DispCoords" : ["Disp Bound Coord sys", {"Object" : "object", "Shader":"shader", "NDC": "ndc", "World": "world"}],
			"ExportCs" : ["Export Vertex Colors(Cs)", True], 
			"ExportSt" : ["Export Texture Coordinates(s/t)", True],
			"FacePatches":["Export Faces as Patches:", False],
			"interpolateBoundary":["Interpolate Subdiv Boundary:", True]			
			}
		self.optionOrder = ["material",
				"OutputOptions", 
				"AutoCrease",
				"MaxCreaseAngle",
				"RenderInShadowPass",
				"ShadowMats",
				"GenEnvMaps",
				"EnvMapPixelFilter",
				"EnvMapFilterX",
				"EnvMapFilterY",
				"EnvMapSamplesX",
				"EnvMapSamplesY",
				"EnvMapShadingRate",
				"RenderInEnvMaps",
				"Matte",
				"Ignore",
				"Sides",
				"ShadingRate",
				"DispBound",
				"ExportCs",
				"ExportSt",
				"FacePatches",
				"interpolateBoundary"]
		# preinitialize a material property
		ObjectUI.__init__(self, obj)
		
		# commented out temporarily. Will return to service as global object editor for *all* objects, not just meshes
		#self.exportButton = ui.Button(self.editorPanel.width - 185, self.editorPanel.height - 25, 180, 25, "Export", "Export Object", 'normal', self.editorPanel, True)
		#self.exportButton.registerCallback("release", self.showExport)
		
		#self.exportSettings = btor.BtoRMain.ExportSettings()
		#self.exportSettings.export_functions.append(self.objecteditor.exportSingleObject) # this should do the trick, but should apply to every object that's exportable standalone
		
		self.helpText = """ This is a test. 
		Here are some lines.
		And more lines.
		and yet more! """
		
	def showHelp(self, button):
		print "Here is my docstring!", self.__doc__
		self.helpwindow.setText(self.helpText)
		self.evt_manager.addElement(self.helpwindow.getEditor())
		
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
	protocols.advise(instancesProvide=[IObjectUI], asAdapterForTypes=[BtoRLamp])	
	def __init__(self, obj):
		self.options = { "globalLight":["Light is global:", False],
				"transformLight":["Use Light Transform:", False],
				"autoLighting":["Automatic Lighting:", True],
				"IncludeWithAO" : ["Include light with AO?", False],
				"Multiplier" : ["Multiplier", 1.0],
				"GenShadowMap" : ["Render Shadow Maps:", {"Lazy" : "lazy", "Always" : "always", "Never" : "never"}, "lazy"], 
				"ShadowMapSize" : ["Shadow Map Size", { "256" : 256, "512" : 512, "1024" : 1024, "2048" : 2048 }, "256"],
				"ShadowMapEyeSplits" : ["Max Eyesplits for Map", 5],
				"DepthFilter" : ["Midpoint Depthfilter?", True],
				"ShadowmapSamples" : ["ShadowMap Samples:", 1],
				"ShadowMapJitter" : ["ShadowMap Jitter:", 0.0],
				"ShadowMapWindow" : ["ScreenWindow Size:", {"5" : 5, "10" : 10, "15": 15, "20" : 20, "50" : 50, "100" : 100 }, "5"],
				"ShowZBuffer" : ["Show Z Buffer?", False],
				"Group" : ["Occlusion Group:", {"None Selected":"none Selected"}, "None Selected"]}
					
		self.optionOrder = ["globalLight",
					"autoLighting",
					"GenShadowMap",
					"ShadowMapSize",
					"ShadowMapJitter",
					"ShadowMapEyeSplits",
					"Multiplier", 
					"IncludeWithAO",					
					"transformLight",
					"DepthFilter",
					"ShadowMapWindow",
					"ShowZBuffer"]	
		# occlusion group property here
		# "Group",					
		ObjectUI.__init__(self, obj)
		# assign custom stuff to any properties that need it
		#self.lighting.occlListeners.append(self.editors["Group"])
		#self.editors["Group"].updateMenu(self.lighting.occlusion_menu) # to catch any stragglers
		

		
	def getSelector(self):
		return self.selector
	
	def showShader(self):
		if self.shaderPanel.isVisible:
			self.shaderPanel.isVisible = False
		else:
			self.shaderPanel.x = self.editorPanel.parent.width + 15			
			self.shaderPanel.isVisible = True
		
	def setShader(self, shader):
		self.options["shader"] = ["Light Shader:", BtoRShaderType(shader)]
		self.properties["shader"] = IProperty(self.options["shader"][1])
		self.properties["shader"].setWidth(self.scroller.width - 15)
		self.properties["shader"].setName("Light Shader:")
		self.optionOrder.insert(0, "shader")
		self.editors["shader"] = IPropertyEditor(self.properties["shader"])
		self.editors["shader"].setParent(self.scroller)	
		self.properties["shader"].getValue().getObject().obj_parent = self
		
		self.shaderPanel = self.properties["shader"].getValue().getObject().getEditor()
		self.shaderPanel.parent = self.editorPanel		
		self.editorPanel.addElement(self.shaderPanel)
		self.shaderPanel.isVisible = False
		self.shaderPanel.shadowed = True
		self.shaderPanel.outlined = True
		self.shaderPanel.hasHeader = False				
		self.shaderPanel.invalid = True
		self.shaderPanel.validate()
		
		self.reloadOptions()
			
		
class MBallUI(ObjectUI):
	protocols.advise(instancesProvide=[IObjectUI], asAdapterForTypes=[BtoRMBall])
	def __init__(self, obj):
		ObjectUI.__init__(self, obj)
		

		
class CurveUI(ObjectUI):
	""" A UI for the curve type """
	protocols.advise(instancesProvide=[IObjectUI], asAdapterForTypes=[BtoRCurve])
	def __init__(self,obj):
		self.options = {  "material":["material", BtoRMaterialType("None Selected")],
					"width" : ["Curve width:", 1.0],
					"wrap" : ["Wrap:", {"Periodic" : "periodic", "Non-Periodic" : "nonperiodic" }],
					"interp" : ["Interpolation:", { "Linear" : "linear", "Cubic" : "cubic" }] }
		self.optionOrder = ["material", "width", "wrap", "interp"]
					
		ObjectUI.__init__(self, obj)
		

class SurfaceUI(ObjectUI):
	""" A UI for the surface type """
	protocols.advise(instancesProvide=[IObjectUI], asAdapterForTypes=[BtoRSurf])
	def __init__(self, obj):
		ObjectUI.__init__(self, obj)
		
	
		
class CameraUI(ObjectUI):
	""" A UI for the camera type """
	protocols.advise(instancesProvide=[IObjectUI], asAdapterForTypes=[BtoRCamera])

	def __init__(self, obj):
		self.options = { "DOF" : ["Use Depth of Field?", False], 
			"autoImager" : ["Automatic Background:", True],
			"fstop" : ["F-stop", 22], 
			"focallength" : ["Focal Length", 45],
			"focaldistance" : ["Focal Distance:", 10] }
		self.optionOrder = ["autoImager", "DOF", "fstop", "focallength", "focaldistance"]
		ObjectUI.__init__(self, obj)				
		#self.editorPanel.addElement(ui.Label(10, 30, "Imager Shader:", "Imager Shader:", self.editorPanel, False))
		#self.shaderButton = ui.Button(self.editorPanel.get_string_width("Imager Shader:", 'normal') + 15, 30, 125, 25, "Imager Shader", "None Selected", 'normal', self.editorPanel, True)		
		# self.shaderButton.registerCallback("release", self.shader.showEditor)
		
	def setShader(self, shader):
		self.options["shader"] = ["Imager Shader:", BtoRShaderType(shader)]
		self.properties["shader"] = IProperty(self.options["shader"][1])
		self.properties["shader"].setWidth(self.scroller.width - 15)
		self.properties["shader"].setName("Imager Shader:")
		self.optionOrder.insert(0, "shader")
		self.editors["shader"] = IPropertyEditor(self.properties["shader"])
		self.editors["shader"].setParent(self.scroller)	
		self.properties["shader"].getValue().getObject().obj_parent = self
		
		# shader panel setup
		self.shaderPanel = self.properties["shader"].getValue().getObject().getEditor()					
		self.shaderPanel.parent = self.editorPanel		
		self.editorPanel.addElement(self.shaderPanel)
		self.shaderPanel.isVisible = False
		self.shaderPanel.shadowed = True
		self.shaderPanel.outlined = True
		self.shaderPanel.hasHeader = False				
		self.shaderPanel.invalid = True
		self.shaderPanel.validate()
		
		self.reloadOptions()
		
	def showShader(self):
		if self.shaderPanel.isVisible:
			self.shaderPanel.isVisible = False
		else:
			self.shaderPanel.x = self.editorPanel.parent.width + 15			
			self.shaderPanel.isVisible = True
		
		
# material derivative UIs
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

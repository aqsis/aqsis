#######################################################################
# GUI  - GUI for the material definition system
# Version 0.1a
# Author: Bobby Parker                                                                         
# This file provides the GUI for the export & materials system
# The shader parameter lister part of this GUI has functionality where it attempts to guess the intended use of 
# a parameter based on its default value or name, e.g. if a parameter has the name text or texture or map, 
# then it's assumed that the parameter is for an image, thus a filename selector is presentable.
# Note that guessed parameters can be overridden via an override button provided per editor.
#
#######################################################################
import os
from btor import BtoRGUIClasses as ui
from btor import BtoRAdapterClasses
from btor import BtoRTypes
reload(BtoRTypes)
reload(ui) # remove these when released for production use
reload(BtoRAdapterClasses)
import cgkit
import cgkit.rmshader
import cgkit.cgtypes
import cgkit.quadrics
from cgkit import ri as ri
from cgkit import ribexport as export
import Blender
import xml.dom.minidom
import new
import traceback
import math
from sets import Set
import sys
import StringIO	
import string
import re
import popen2


###### HEY!!!!!!!! Design MATERIAL DERIVATIVES ##############


handler = """# SPACEHANDLER.VIEW3D.EVENT
#!BPY

import Blender

evt = Blender.event

if evt == Blender.Draw.RIGHTMOUSE or Blender.Draw.LEFTMOUSE:
	# on a right mouse click assume I've selected an object or unselected it
	# simply set something in the registry to say "hey, something changed"
	changed = { "changed" : True }
	print "changed"
	Blender.Registry.SetKey("BtoR_Space", changed)
	WinID = Blender.Registry.GetKey("BtoR_WinID")
	ID = WinID["ID"][0]
	Blender.Window.QAdd(ID, Blender.Draw.REDRAW, 0, 0)
"""

class BtoRObject:
	def getProperty(self, property):
		if self.properties.has_key(property):
			return self.properties[property].getValue()
		else:
			return False
		
	def setProperty(self, property, value):
		if self.properties.has_key(property):
			self.properties[property].setValue(value)
	
	def setupProperties(self):
		self.properties = {}
		self.editors = {}
		for option in self.optionOrder:
			propertyName = self.options[option][0]
			propertyValue = self.options[option][1]
			# generate a list of option panels here and allow editing
			# create a property for each option
			self.properties[option] = BtoRAdapterClasses.IProperty(propertyValue) # 1st item is the property name, second item is the property initializer
			self.properties[option].setName(propertyName)
			self.properties[option].setWidth(self.scroller.width - 15)			
			# takes up half the available space of the main pane
			self.editors[option] = BtoRAdapterClasses.IPropertyEditor(self.properties[option])
			self.scroller.addElement(self.editors[option].getEditor()) # and that should be that. When this is discarded, all those go away
			self.editors[option].setParent(self.scroller)	
			self.scroller.offset = 0
			self.properties[option].optionKey = option
	
		
	def setupLightingProperties(self):
		pass
	
			
	def saveProperties(self, doc, xml):
		
		if len(self.properties) > 0:			
			# I've got at least one property
			for property in self.properties:
				
				if self.properties[property].saveable:
					xmlProp = self.properties[property].toXML(doc)
					xmlProp.setAttribute("name", property) # set the name attribute here for the moment, but later configure a property to have a name AND a title!
					xml.appendChild(xmlProp)		
		return xml
		
	def loadProperties(self, xml):
		xmlTypes = { "int" : int, "str" : str, "list" : list, "dict": dict, "float" : float, "bool": bool }
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
				# print "Property Name: ", propertyName
				propertyValue = [float(eval(xmlProperty.getAttribute("red").encode("ascii"))), 
							float(eval(xmlProperty.getAttribute("green").encode("ascii"))),
							float(eval(xmlProperty.getAttribute("blue").encode("ascii")))] # rebuild a pretty color!
				# print "Property Value: ", propertyValue
			else:
				propertyValue = xmlTypes[xmlType](xmlValue)
			
			self.editors[propertyName].setValue(propertyValue) 

class BtoRSettings(BtoRObject): # an instance of this class should be passed to every base-level BtoR object (objects that go into the root event manager.
	# someone with 3Delight, Prman, entropy etc can add further renderer information here
	# standard setup: Renderer name: [render binary, sltell binary, texture binary, shader extension, [home env vars], [shader env vars], [texture env vars], [ display env vars],
	#				            [ procedural env vars ], [plugin env vars], [archive env vars]] 
	# assign all lists and strings to None if there's no available info or that particular renderer has no equivalent value (see Aqsis for ex. of home env vars)
	renderers = {  "AQSIS" : ["aqsis", "aqsl", "aqsltell", "teqser", "slx", None, "AQSIS_SHADER_PATH", "AQSIS_TEXTURE_PATH", "AQSIS_DISPLAY_PATH", "AQSIS_PROCEDURAL_PATH", "AQSIS_PLUGIN_PATH", "AQSIS_ARCHIVE_PATH"],
			     "BMRT" : ["rendrib", "slc", "slctell","mkmip","BMRTHOME", "SHADERS", "TEXTURES",None,None,None,None], 
			     "Pixie" : ["rndr", "sdr", "sdrinfo", "texmake", "PIXIEHOME", "SHADERS", None, None, None, None, None],
			     "3Delight":["renderdl", "shaderdl", "shaderinfo", "tdlmake", "DELIGHT",  "DL_SHADERS", None, "DL_DISPLAYS", None, None, None],
			     "RenderDotC":["wrendrdc", "shaderdc", "soinfo", "texdc", "RDCROOT", "SHADERS", "MAPS", "DISPLAYS", "PROCEDURALS", None, "ARCHIVES"] }
	cpp = "cpp"
	
	# what else does BtoRSettings need to know? How about a projects directory? MRUs? TBD later I suppose
	# renderer option arrays are placed in a dict structure for each renderer.
	# thus, each given option/attribute needs:
	# the property NAME (shows up in the property panel)
	# the property value ( array integer values cannot be guessed from a string, but maybe I *can* figure out a formatting method?)
	# the default value (used for testing if an option or attribute needs to be rendered)
	
	# aqsis
	aqsis_options = {
		"limits:integer[2]:bucketsize" : ["Bucketsize:", "16 16", "16 16"],
		"limits:integer:bucketmodulo" : ["Bucketmodulo", -1, -1],
		"limits:integer:eyesplits":["Max eyesplits:", 10, 10],
		"limits:integer:gridsize":["Gridsize:", 256, 256],
		"limits:integer:texturememory":["Texture Memory:", 8192, 8192],
		"limits:color:zthreshhold":["Z-Threshhold:", [1, 1, 1], [1, 1, 1]],
		"texture:integer:lerp":["Bilinear mipmapping?:", False, False],
		"shadow:float:bias":["Shadow Bias:", 0.0, 0.0],
		"shadow:float:bias0":["Shadow Bias Low:", 0.0, 0.0],
		"shadow:float:bias1":["Shadow Bias High:", 0.0, 0.0],
		"render:integer:multipass":["Multipass:", False, False],
		"render:string:bucketorder":["Bucket Order:", {"btor:default":"horizontal", "horizontal":"horizontal", "vertical":"vertical", "zigzag":"zigzag", "circle":"circle", "random":"random"}, "horizontal"]		
	}
	
	aqsis_attributes = {
		"autoshadows:integer:res:lamp":["Autoshadows Res:", 300, 300],
		"autoshadows:string:shadowmapname:lamp":["Autoshadow MapName:", "shadow", "shadow"]
	}	
	
	# Pixie
	rndr_options = {
		"limits:int:gridsize" : ["Gridsize:", 512, 512],
		"limits:int:inheritattributes" : ["Inherit Attributes:", False, False],
		"limits:int:texturememory" : ["Texture Memory:", 20000, 20000],
		"limits:int:shadercache" : ["Shader Cache:", 1000, 1000],
		"limits:int:hiearchydepth" : ["Hiearchy Depth:", 40, 40],
		"limits:int[2]:bucketsize" : ["Bucket Size:", "32 32", "32 32"],
		"limits:int:eyesplits" : ["Eyesplits:", 10, 10],
		"statistics:int:endofframe" : ["End of Frame:", 0, 0],
		"statistics:string:filelog" : ["File Log:", "", ""],
		"trace:int:maxdepth" : ["Max Depth:", 5, 5]
	}
	
	rndr_attributes = {
		"dice:int:minsubdivision" : ["Min subdivision:", 2, 2],
		"dice:int:maxsubdivision" : ["Max subdivision:", 2, 2],
		"dice:int[2]:numprobes" : ["Num Probes:", "3 3", "3 3"],
		"dice:int:minsplits" : ["Min splits:", 2, 2],
		"dice:float:boundexpand" : ["Bound expand:", 0.5, 0.5],
		"dice:int:binary" : ["Binary Dice:", False, False],
		"visbility:int:camera" : ["Camera Visibility:", 1, 1],
		"visibility:int:trace" : ["Trace Visbility:", 1, 1],
		"visibility:int:photon" : ["Photon Visibility:", 1, 1],
		"visibility:string:transmission" : ["Transmission:",  {"btor:default" : "transparent", "transparent" : "transparent", "opaque" : "opaque", "shader" : "shader", "Os" : "Os" }, "transparent"],
		"trace:int:displacements" : ["Trace Displacements:", 0, 0],
		"trace:float:bias" : ["Trace Bias:", 0.01, 0.01],
		"trace:int:maxdiffusedepth" : ["Max Diffuse Depth:", 1, 1],
		"trace:int:maxspeculardepth" : ["Max Specular Depth:", 2, 2],
		"irradiance:string:handle" : ["Irradiance Handle:", "", ""],
		"irradiance:string:filemode" : ["Irradiance Filemode:", {"btor:default" : "None", "None":"None",  "Read":"r", "Write":"w"}, "None"],
		"irradiance:float:maxerror" : ["Irradiance Max error:", 0.5, 0.5],
		"photon:string:globalmap" : ["Global Photon Map:", "", ""],
		"photon:string:causticmap" : ["Caustic Map:", "", ""],
		"photon:string:shadingmodel" : ["Shading Model:", {"btor:default" : "matte", "matte" : "matte", "glass" : "glass", "water" : "water", "chrome" : "chrome", "transparent" : "transparent" }, "matte"],
		"photon:int:estimator" : ["Photon Estimator:", 100, 100]
	}
	
	# BMRT
	bmrt_options = {
		"render:integer:max_raylevel":["Max Raylevel", 4, 4],
		"render:float:minshadowbias":["Minimum Shadow Bias:", 0.01, 0.01],
		"statistics:integer:endofframe":["Print Statistics:", False, False],
		"statistics:string:filename":["Stats file:", "", ""],
		"radiosity:integer:steps":["Radiosity Steps:", 0, 0],
		"radiosity:integer:minpatchsamples":["Radiosity Samples:", 1, 1],
		"limits:integer:texturememory:object":["Texture Memory:", 1000, 1000],
		"limits:integer:derivmemory:object":["Derivative Memory:", 2, 2],			
		"runtime:string:verbosity:object":["Verbosity:", {"btor:default":"normal", "normal":"normal", "stats":"stats", "debug":"debug", "silent":"silent"}, "normal"]
	}
	
	bmrt_attributes = {
		"render:integer:visbility:none":["Visibility:", 7],
		"render:string:casts_shadows:none":["Casts Shadows:", {"btor:default":"none","none":"none", "opaque":"opaque", "Os":"Os", "shade":"shade"}, "shade"],
		"render:integer:truedisplacement:none":["True Displacement:", False, False],
		"render:float:patch_multiplier:none":["Patch Multiplier:", 1.0, 1.0],
		"render:float:patch_maxlevel:none":["Patch Max Level:", 256, 256],
		"render:float:patch_minlevel:none":["Patch Min Level:", 1, 1],
		"trimcurve:string:sense:none":["Inside Curve Sense:", True, True],
		"render:integer:use_shading_rate:none":["Use Shading Rate:", True, True],
		"light:string:shadows:lamp":["Lamp Casts Shadows:", {"btor:default":"on", "on":"on", "off":"off"}, "on"],
		"light:integer:nsamples:lamp":["Samples:", 1, 1],
		"radiosity:color:averagecolor:none":["Radiosity Avg. Color:", [1, 1, 1], [1, 1, 1]],
		"radiosity:color:emissioncolor:none":["Radio. Emission Color:", [1, 1, 1], [1, 1, 1]],
		"radiosity:float:patchsize:none":["Radio patchsize:", 4, 4],
		"radiosity:float:elemsize:none":["Radio elemsize:", 2, 2],
		"radiosity:float:minsize:none":["Radio minsize:", 1, 1],
		"radiosity:string:zonal:object":["Zonal:", {"btor:default":"fully_zonal", "fully_zonal":"fully_zonal", "zonal_receives":"zonal_receives", "zonal_shoots":"zonal_shoots", "none":"none"}, "fully_zonal"],
		"indirect:float:maxerror:object":["GI Max error:", 0.25, 0.25],
		"indirect:float:maxpixeldist:object":["GI max pixel dist:", 20, 20],
		"indirect:integer:nsamples:object":["GI Samples:", 256, 256],
		"indirect:string:savefile:object":["GI Savefile:", "indirect.dat", "indirect.dat"],
		"indirect:string:seedfile:object":["GI Seedfile:", "indirect.dat", "indirect.dat"],
		"caustic:float:maxpixeldist:object":["Caustics Max Pixel Dist:", 16, 16],
		"caustic:float:integer:ngather:object":["Caustics Max Photons:", 75, 75],
		"light:float:integer:nphotons:lamp":["Light Max Photons:", 0, 0],
		"caustic:color:refractioncolor:object":["Cautic Refraction Color:", [0, 0, 0], [0, 0, 0]],
		"caustic:color:specularcolor:object":["Caustic Specular Color:", [0, 0, 0], [0, 0, 0]],
		"caustic:float:refractionindex:object":["Caustic Refract Index:", 1, 1]
	} 
	
	# 3Delight 
	renderdl_options = { # 3delight options
	
	}
	
	renderdl_attributes = {
	
	}
	
	# RenderDotC
	wrendrc_options = { 
	
	}
	
	wrendrc_attributes = {
	
	}
	
	def __init__(self):		
		self.options = {"Renderer":["Renderer:", { "Aqsis":"aqsis", "BMRT":"bmrt", "Pixie":"rndr", "3Delight":"renderdl", "RenderDotC":"wrendrdc" }, "aqsis"],
					"UseSlParams":["Use CGKIT Slparams?", True, True],
					"RenderMatsStartup":["Render Mats on Startup?", True, True]}
						
		sdict = globals()

		if sdict.has_key("instBtoREvtManager"):
			self.evt_manager = sdict["instBtoREvtManager"]
		setattr(BtoRAdapterClasses, "instBtoRSettings", self)
		setattr(BtoRAdapterClasses, "instBtoREvtManager", self.evt_manager)
		self.scene = None
		self.getSettings(None)
		screen = Blender.Window.GetAreaSize()
		self.editor = ui.Panel(10, screen[1] - 30, 300, 210, "GlobalSettings", "Global Neqsus/BtoR Settings:", None, False)
		self.editor.addElement(ui.Label(5, 30, "Renderer:", "Renderer:", self.editor, False))
		rendererList = self.renderers.keys()
		
		self.rendererMenu = ui.Menu(self.editor.get_string_width("Renderer:", 'normal') + 12, 30, 100, 20, "Renderer:", rendererList, self.editor, True, custom = False)				
		self.rendererMenu.setValue(rendererList.index(self.renderer))
		self.rendererMenu.registerCallback("release", self.selectRenderer)
		
		self.useSlParams = ui.CheckBox(5, 65,"Use CGKit for Shader Parameters?", "Use CGKit for Shader Parameters?", True, self.editor, True)
		self.useSlParams.setValue(self.use_slparams)		
		
		width = self.editor.get_string_width("Procedurals:", 'normal') + 30
		
		self.renderMatsOnStartup = ui.CheckBox(5, 90, "Render material previews on startup?", "Render material previews on startup?", False, self.editor, True)
		
		# shader search paths
		# self.editor.addElement(ui.Label(5, 140, "Shaders:", "Shader Search Paths:", self.editor, False))	
		self.showSearchPaths = ui.Button(self.editor.width - 90, 30, 70, 25, "Search Paths:", "Search Paths:", 'small', self.editor, True)
		self.showSearchPaths.registerCallback("release", self.togglePaths)
		
		#self.shaderpaths = ui.TextField(5,165, self.editor.width - 45 , 20, "ShaderPathText", self.shaderPathList, self.editor, True)		
		#self.shaderBrowse = ui.Button(self.shaderpaths.x + self.shaderpaths.width + 2, 165, 30, 20, "ShaderBrowse", "...", 'normal', self.editor, True)		
		#self.shaderBrowse.registerCallback("release", self.browse)
		
		# output path
		self.editor.addElement(ui.Label(5, 110, "Output Path:", "Output Path:", self.editor, False))
		self.outputpath = ui.TextField(5, 130, self.editor.width - 45, 20, "OutputPathText", self.outputPath, self.editor, True)
		self.outputBrowse = ui.Button(self.outputpath.x + self.outputpath.width + 2, 130, 30, 20, "OutputBrowse", "...", 'normal', self.editor, True)
		self.outputBrowse.registerCallback("release", self.browse)
		
		self.cancelButton = ui.Button(5, self.editor.height - 45, 60, 20, "Cancel", "Cancel", 'normal', self.editor, True)		
		self.okButton = ui.Button(self.editor.width - (60 + 15), self.editor.height - 45, 60, 20, "OK", "OK", 'normal', self.editor, True)
		
		self.okButton.registerCallback("release", self.saveSettings)
		self.cancelButton.registerCallback("release", self.getSettings)
		self.cancelButton.registerCallback("release", self.update)
		self.okButton.registerCallback("release", self.close)
		self.cancelButton.registerCallback("release", self.close)
		
		# get the first shader path in the search list
		# paths = self.shaderpaths.getValue().split(";")
		
		self.shadersSurface = {}
		self.shadersDisplacement = {}
		self.shadersImager = {}
		self.shadersVolume = {}
		self.shadersLight = {}
		self.surfaceFiles = {}
		self.dispFiles = {}
		self.volumeFiles = {}
		self.imagerFiles = {}
		self.lightFiles = {}
		self.assets = {} # indexed by object name
		# self.paths = {"shaders":[], "textures":[], "shadows":[] }
			
		self.shadows = os.sep + "shadows" + os.sep
		self.images = os.sep + "images" + os.sep
		self.textures = os.sep + "textures" + os.sep
		self.archives = os.sep + "archives" + os.sep
		self.maps = os.sep + "maps" + os.sep
		self.shaders = os.sep + "shaders" + os.sep
		self.materialPreviews = os.sep + "matPreviews" + os.sep # this should use the temp directory that's defined, or maybe I should make that another user option?
		
		self.pathsPanel = ui.Panel(305, 0, 300, 250, "", "", self.editor, True)
		self.pathsPanel.isVisible = False
		self.pathsPanel.shadowed = True
		self.pathsPanel.outlined = True
		self.pathsPanel.hasHeader = False
		
		self.pathsLabel = ui.Label(5, 3, "Paths", "Shader Search Paths:", self.pathsPanel, True)
		self.pathsLabel.transparent = True
		
		self.pathsContainer = ui.Panel(5, 80, 292, 150, "Shader Search Paths:", " ", self.pathsPanel, True)	
		
		self.pathsContainer.shadowed = False
		self.pathsContainer.outlined = False
		self.pathsContainer.hasHeader = False
		# self.pathsContainer.addElement(ui.Label(20, 5, "Shader Path:", "Shader Path:", self.pathsContainer, False, fontsize = 'small'))
		self.pathsScroller = ui.ScrollPane(0, 0, 290, 150, "Scroller", "Scroller", self.pathsContainer, True)
		
		self.addPass = ui.Button(5, 27, 105, 25, "Add Search Path", "Add Path", 'small', self.pathsPanel, True)
		self.addPass.registerCallback("release", self.addPath)
		self.deletePass = ui.Button(115, 27, 145, 25, "Delete Selected Path(s)", "Delete Selected Path(s)", 'small', self.pathsPanel, True)
		self.deletePass.registerCallback("release", self.deletePath)
		self.pathsPanel.addElement(ui.Label(220, 60, "Recurse", "Recurse", self.pathsPanel, False, fontsize = 'small'))
		self.pathsPanel.addElement(ui.Label(25, 60, "Path", "Path", self.pathsPanel, False, fontsize = 'small'))
		
		self.paths = []
		
		for path in self.shaderPathList[self.renderer]:			
			panel = self.addPath(None, False)			
			panel.pathName.setValue(path[0])
			panel.recurse.setValue(path[1])
			
		if len(self.paths) > 0:
			if self.use_slparams:
				self.getShaderSourceList(self.paths[0].pathName.getValue()) 
			else:
				self.getCompiledShaderList(self.paths[0].pathName.getValue())
		self.rendererListeners = []
			
	def togglePaths(self, button):
		""" toggle the panel """
		if self.pathsPanel.isVisible:
			self.pathsPanel.hide()
		else:
			self.pathsPanel.show()
		
	def addPath(self, button, browse = True):
		pathPanel = ui.Panel(0, 0, 274, 22, "RenderPass", "", self.pathsScroller, True)		
		pathPanel.outlined = True
		pathPanel.hasHeader = False		
		pathPanel.cornermask = 0
		pathPanel.shadowed = False
		pathPanel.selector = ui.CheckBox(2, 3, " ", " ",  False, pathPanel, True) 
		pathPanel.selector.transparent = True
		pathPanel.selector.elements[0].transparent = True
		pathPanel.pathName = ui.TextField(15, 0, 208, 20, "PathName", "path", pathPanel, True)
		pathPanel.recurse = ui.CheckBox(227, 3, "", "", False, pathPanel, True, fontsize = 'tiny')
		pathPanel.browse = ui.Button(253, 0, 20, 20, "shader", "...", 'small', pathPanel, True)
		pathPanel.browse.shadowed = False
		pathPanel.browse.registerCallback("release", self.browsePath)
		self.pathsScroller.addElement(pathPanel)
		# eventually extend so I can figure out which path type this is (shader, archive, etc)
		self.paths.append(pathPanel)		
		# self.log.appendText("Added an AOV pass\n")
		if browse:
			self.browsePath(pathPanel.browse)
		return pathPanel
		
	def deletePath(self, button):
		selected = []
		for element in self.paths:
			if element.selector.getValue():
				self.pathsScroller.removeElement(element)
				selected.append(element)
		for element in selected:
			self.paths.remove(element)
		
	def browsePath(self, button):		
		self.target = button
		Blender.Window.FileSelector(self.fselect, 'Choose any file')
		
	def fselect(self, file):					
		path = os.path.dirname(file)
		self.target.parent.pathName.setValue(path) # should do the job		
		
	def getAssets(self, name):
		return self.assets[name]
		
	def setAsset(self, name, value):
		self.assets[name] = value
	def getRendererOptions(self):
		if self.renderer == "AQSIS":
			return self.aqsis_options
		elif self.renderer == "BMRT":
			return self.bmrt_options
		elif self.renderer == "Pixie":
			return self.rndr_options
		elif self.renderer == "3Delight":
			return self.renderdl_options
		
	def getRendererAttributes(self):
		if self.renderer == "AQSIS":
			return self.aqsis_attributes
		elif self.renderer == "BMRT":
			return self.bmrt_attributes
		elif self.renderer == "Pixie":
			return self.rndr_attributes
		elif self.renderer == "3Delight":
			return self.renderdl_attributes

		
	def getShaderSearchPaths(self):
		pathlist = []
		for path in self.paths:
			if path.recurse.getValue():
				# iterate this path, returning it, and all folders
				dirPath = path.pathName.getValue()
				pathlist.append(dirPath)
				for root, dir, files in os.walk(dirPath):
					pathlist.append(root)
					#if len(dir) > 0:
					#	for xPath in dir:
					#		print "adding XPath: ", xPath
					#		if os.path.exists(xPath):
					#			pathlist.append(xPath)
				
				# see if there are more folders on this path
			else:
				pathlist.append(path.pathName.getValue())
		# pathList = self.shaderpaths.getValue().split(";")
		
		# let's make this more dynamic and test for wonderful things like..directory recursion
		
		return pathlist
			
	def selectRenderer(self, button):
		self.renderer = self.rendererMenu.getValue() 
		# setup the search paths
		self.paths = []
		self.pathsScroller.clearElements()
		if self.scene == None:
			sdict = globals()
			if sdict.has_key("instBtoRSceneSettings"):
				self.scene = sdict["instBtoRSceneSettings"]
		if self.scene != None:
			self.scene.resetProperties()
		
		for path in self.shaderPathList[self.renderer]:			
			panel = self.addPath(None, False)		
			
			panel.pathName.setValue(path[0])
			panel.recurse.setValue(path[1])
		# update all the rendererChangeListeners
		for listener in self.rendererListeners:
			listener.updateAttributes()
	
		
	def getShaderList(self, path):
		
		if self.use_slparams:
			self.getShaderSourceList(path) 
		else:
			self.getCompiledShaderList(path) 
			
	def getShaderParams(self, path, shader):	
		# run the shader params 
		# let's assume that if I'm here, I have a valid path		
		shaderparms = []
		#paths = self.shaderpaths.getValue()
		# sPaths = paths.split(";")
		# maxpath = os.path.join(sPaths[0], shader)			
		isdefvalue = False
		havetype = False	
		shaderFile = path + os.sep + shader
		type = None
		# the shader name is already passed in		
		if (os.name != "posix"): # must deal with linux here at some point
			command = '%s "%s"'%(self.renderers[self.renderer][2], shaderFile)	
			fromchild, tochild = popen2.popen2(command)		
			paramdata = fromchild.readlines()
			# print paramdata
			tochild.close()
			fromchild.close()	
			typex = ""
			for line in paramdata:	
				sdefvalue = ""
				if not line.strip():			
					# nothing here
					ztemp = 0
				else:					
					# for each shader, we want to process the avilable parameters.
					# the first line should contain the type of shader it is
					linedata = line.strip().replace('"', '')
					linex = linedata.split()[0]						
					# print "Shader type: ", linex
					if linex == "surface" or linex == "light" or linex == "volume" or linex == "displacement" or linex == "imager":	 # test for shader type declaration										
						typex = linex	
						# shaderparms.append(typex)
					else: # not the first item
						# find out where we are		
						parameter = []
						if linedata.find("Default value") > -1:
							isdefvalue = True
						paramline = linedata.split()						
						if not isdefvalue:
							# parse the parameter name and type, ignoring AOV for the moment, and figure out if something is an array or not as well							
							parametername = paramline[0]																				
							if (paramline[1] == "parameter"):
								# for aqsis & 3delight sltells								
								storageclass = paramline[2]																											
								storagetype = paramline[3]
							else:
								storageclass = paramline[1]
								storagetype = paramline[2]								
						else: 	
							if storagetype.find("[") > -1: # i have an array
									index = storagetype.index("[")
									# get the array length
									arrayLen = storagetype[index + 1:len(storagetype) - 1]
									# now trim the array Length off of the string.
									storagetype = storagetype[:index]
							else: 
								arrayLen = None
								
							# Modify parameter returns to be in line with how CgKit.slparams returns them so I only have to worry about one type of data structure
							if storagetype == "float":									
								# on the float value type
								fdefvalue = float(paramline[2])
								# output specifier is always blank here, leave it to the user to configure this								
								parmset = ('', storageclass, storagetype, arrayLen, parametername, None, fdefvalue) 
								shaderparms.append(parmset)	
							
							elif storagetype == "color" or storagetype == "vector" or storagetype == "normal" or storagetype == "point":
								# the default value for colors, vectors, points, or normals will be two values, so expand this section 
								odefvalue = [paramline[2].strip(""), float(paramline[3].strip("[")), float(paramline[4]), float(paramline[5].strip("]"))] 
								# pulls the coordinate space (or color space) and values
								parmset = ('', storageclass, storagetype, arrayLen, parametername, odefvalue[0], [odefvalue[1], odefvalue[2], odefvalue[3]])
								# parmset = [parametername, storageclass, storagetype, odefvalue]							
								shaderparms.append(parmset)	
								
							elif storagetype == "string":	
								if len(paramline) > 2:
									if len(paramline[2]) > 0:
										sdefvalue = paramline[2].strip("")
										if sdefvalue == "":
											sdefvalue = None

								else:
									sdefvalue = None
								# sdefvalue = None
								# sdefvalue = paramline[2]
								parmset = ('', storageclass, storagetype, arrayLen, parametername, None, sdefvalue) 
								# parmset = [parametername, storageclass, storagetype, sdefvalue]	
								shaderparms.append(parmset)
								
							elif storagetype == "matrix":
								# how would the default value be formatted?
								mdefvalue = [] # this is a nice array, row-major
								for x in range(2, len(paramline)):
									mdefvalue.append(paramline[x])
									
								parmset = ('', storageclass, storagetype, arrayLen, parametername, None, sdefvalue) 
								# parmset = [parametername, storageclass, storagetype, mdefvalue]
								shaderparms.append(parmset)							
									
							isdefvalue = False 
		
		shaderdata = (typex, shader, shaderparms) # this tuple should be exactly like what cgkit.slparams returns now		
		return shaderdata
			
	def getShaderSourceParams(self, file):
		
		try:
			parms = cgkit.slparams.slparams(file) 
		except cgkit.slparams.SyntaxError:
			# the shader's source-code was incomplete, don't include it.
			parms = None 
		return parms
			
	def getCompiledShaderList(self, shaderPath):
		# reset the shaders lists, since I'm looking at a new path	
		path = shaderPath.strip()		
		self.shadersSurface[shaderPath] = []
		self.shadersLight[shaderPath] = []
		self.shadersVolume[shaderPath] = []
		self.shadersDisplacement[shaderPath] = []
		self.shadersImager[shaderPath] = []	
		currentshaderlist = []		
		if os.path.exists(shaderPath):
			files = os.listdir(shaderPath)		
			for file in files:
				
				if (file.endswith(self.renderers[self.renderer][4])):		
					outx = os.path.splitext(file)					
					currentshaderlist.append(outx[0]) 					
					parms = self.getShaderParams(shaderPath, file)
					# let's do the sorting here			
					if (parms[0] == "surface"):
						self.shadersSurface[shaderPath].append(parms)
					elif (parms[0] == "imager"):
						self.shadersImager[shaderPath].append(parms)
					elif (parms[0] == "displacement"):
						self.shadersDisplacement[shaderPath].append(parms)
					elif (parms[0] == "volume"):
						self.shadersVolume[shaderPath].append(parms)
					elif (parms[0] == "light"):
						self.shadersLight[shaderPath].append(parms)	
		else:
			self.evt_manager.showErrorDialog("Invalid shader path specified!", "Invalid shader path specified! Check your settings.")
			
	def getPathForShader(self, shader, type):
		print shader
		if type == "surface":
			shaderList = self.shadersSurface
		elif type == "imager":
			shaderList = self.shadersImager
		elif type == "displacement":
			shaderList = self.shadersDisplacement
		elif type == "volume":
			shaderList = self.shadersVolume
		elif type == "light":
			shaderList = self.shadersLight
		pathlist = self.shaderpaths.getValue().split(";")
		for path in pathlist:			
			if shaderList.has_key(path):
				shaders = shaderList[path]
				for parms in shaders:
					if shader == parms[1]:
						return path # this gives back the path I'm on, bailing out immediately
		return None # otherwise, return none
		
	def getShaderSourceList(self, shaderPath): 
		# maybe add some iterative thing here that pops up a progress bar
		# reset the shader lists
		print "iterating shaders!"
		path = shaderPath.strip()
		# test for a valid path, and bail if not valid
		if os.path.exists(path):

			# reset the global shaders lists
			self.shadersSurface[shaderPath] = []
			self.surfaceFiles[shaderPath] = []
			self.shadersLight[shaderPath] = []
			self.lightFiles[shaderPath] = []
			self.shadersVolume[shaderPath] = []
			self.volumeFiles[shaderPath] = []
			self.shadersDisplacement[shaderPath] = []
			self.dispFiles[shaderPath] = []
			self.shadersImager[shaderPath] = []		
			self.imagerFiles[shaderPath] = []

			files = os.listdir(path)
			# determine the renderer	
			for file in files:
				if (file.endswith(".sl")):
					
					parms = self.getShaderSourceParams(path + os.sep + file)					
					if parms == None:
						print "Error getting parameters for shader ", file
					else:
		
						if (parms[0][0] == "surface"):
							self.shadersSurface[shaderPath].append(parms[0])
							self.surfaceFiles[shaderPath].append(file)
						elif (parms[0][0] == "imager"):
							self.shadersImager[shaderPath].append(parms[0])
							self.imagerFiles[shaderPath].append(file)
						elif (parms[0][0] == "displacement"):
							self.shadersDisplacement[shaderPath].append(parms[0])
							self.dispFiles[shaderPath].append(file)
						elif (parms[0][0] == "volume"):
							self.shadersVolume[shaderPath].append(parms[0])
							self.volumeFiles[shaderPath].append(file)
						elif (parms[0][0] == "light"):
							self.shadersLight[shaderPath].append(parms[0])
							self.lightFiles[shaderPath].append(file)
				# self.makeSourceMenus() # is this even neccessary?
				
		else:
			# display error dialog
			self.evt_manager.showErrorDialog("Invalid shader path specified!", "Invalid shader path specified! Check your settings.")
			
		#if len(self.shadersSurface) > 0:
			#print "Surface Shaders:"
			#print self.shadersSurface
		#if len(self.shadersImager) > 0:
			#print "Imager Shaders:"
			#print self.shadersImager
		#if len(self.shadersDisplacement) > 0:
			#print "Displacement Shaders:"
			#print self.shadersDisplacement
		#if len(self.shadersLight) > 0:
			#print "Light Shaders:"
			#print self.shadersLight
		#if len(self.shadersVolume) > 0:
			#print "Volume shaders:"
			#print self.shadersVolume
			
	def getSettings(self, button):
		# let's try the Registry!
		try:
			settings = Blender.Registry.GetKey("BtoR", True) # check everywhere for BtoR settings.
		except:
			settings = False
			
		if settings: # settings were found, btor was setup previously, let's get the information
			self.haveSetup = True
			# I've got the information
			self.renderer = settings["renderer"] # string			
			self.shaderPathList = settings["Shaders"] # string			
			self.outputPath = settings["outputPath"] # string
			self.use_slparams = settings["use_slparams"] # boolean		
				
		else:			
			self.haveSetup = False
			# no info		
			self.renderer = "AQSIS" # this should eventually change to use settings stored in either the blender registry, or something else
			# default to aqsis, since we've got no actual information
			# if os.name <> "posix" or os.name <> "mac":
			if sys.platform == "win32":
				defShaderPath = r"C:\Program Files\Aqsis\Shaders" 
				self.outputPath = r'C:\temp'
			else:
				defShaderPath = r"/usr/aqsis/shaders/"
				self.outputPath = r"/tmp/"
			self.use_slparams = True # set this to true by default
			
			self.shaderPathList = {"AQSIS":[[defShaderPath, False]], "BMRT":[], "Pixie":[], "3Delight":[], "RenderDotC":[]}
			
			# surf the environment and get the shader paths from the env vars for this renderer
			# step one, test the $HOME/shaders thing
			# news flash: Set this up on failure of the settings load				
			if self.renderers[self.renderer][5] <> None:			
				# the renderer root should have no need of multiple search paths
				if os.getenv(self.renderers[self.renderer][5]) <> None:
					self.shaderPathList = self.shaderPathList + (os.getenv(self.renderers[self.renderer][5]) + os.sep + "shaders" + os.sep) + os.pathsep
			if self.renderers[self.renderer][6] <> None:
				if os.getenv(self.renderers[self.renderer][6]) <> None:
					#if self.renderers[self.renderer][6].find
					self.shaderPathList = self.shaderPathList + os.getenv(self.renderers[self.renderer][6]) # that should be that.
						
		
	
	def saveSettings(self, button):
		# save the settings to the registry here. Mostly it's simple stuff, will probably expand, 
		# but for a first release, this should work OK
		#print self.rendererMenu.getValue()		
		pathList = []
		for path in self.paths:
			pathItem = [path.pathName.getValue(), path.recurse.getValue()]
			pathList.append(pathItem)
		self.shaderPathList[self.rendererMenu.getValue()] = pathList
		# update current path list, including whether or not I recurse
		
		settings = { "renderer" : self.rendererMenu.getValue(), 
				"Shaders" : self.shaderPathList,
				"outputPath" : self.outputpath.getValue(),
				"use_slparams" : self.useSlParams.getValue() }
		print "PathList = ", self.shaderPathList
		
		if self.haveSetup:
			Blender.Registry.RemoveKey("BtoR", True)
		Blender.Registry.SetKey("BtoR", settings, True)
		self.haveSetup = True
		if self.scene == None:
			sdict = globals()
			if sdict.has_key("instBtoRSceneSettings"):
				self.scene = sdict["instBtoRSceneSettings"]
		if self.scene != None:
			self.scene.resetProperties()
		
	def update(self, obj):
		self.rendererMenu.setValue(self.rendererMenu.menu.index(self.renderer))
		self.useSlParams.setValue(self.use_slparams)		
		self.outputpath.setValue(self.outputPath)
		self.paths = []
		self.pathsScroller.clearElements()
		for path in self.paths:			
			panel = self.addPath(None, False)
			# print panel
			panel.pathName.setValue(path)

	
	def cancel(self, button):
		self.getSettings() # recover the stuff from the registry
		self.update(None)
		
	def browse(self, button):
		self.activeButton = button
		Blender.Window.FileSelector(self.select, 'Choose any file')
	
	def close(self, button):
		self.evt_manager.removeElement(self.editor) # remove myself from the event manager
		
	def select(self, file):
		path = os.path.dirname(file)
		print path
		if not os.path.exists(path):
			self.evt_manager.showErrorDialog("That path does not exist!", "Error! That path does not exist! Please choose one that does.")
		else:
			self.outputpath.setValue(path)
			
		
	def getEditor(self):
		return self.editor	
	
		
class SceneSettings(BtoRObject):
	filter_menu = ["None Selected", "box", "triangle", "catmull-rom", "sinc", "gaussian"]
	def __init__(self):
		self.base_options = {
			"ShadingRate" : ["Shading Rate:", 1.0],
			"PixelSamplesX":["Pixel Samples X:", 2],
			"PixelSamplesY":["Pixel Samples Y:", 2],
			"Gain":["Gain:", 1.0],
			"Gamma":["Gamma:", 1.0],
			"PixelFilter":["Pixel Filter:", {"Box":"box", "Triangle":"triangle", "Catmull-Rom":"catmull-rom", "Sinc":"sinc","Gaussian":"gaussian"}],
			"FilterSamplesX":["Filter Samples X:", 2],
			"FilterSamplesY":["Filter Samples Y:", 2],
			"DefaultMaterial":["Default Material:", {"DefaultSurface":"rendDefault", "Matte":"matte", "Custom Material":"custom" }],
			"Framebuffer": ["Render to Framebuffer:", True],
			"RenderLayers": ["Render all Layers:", True]}
			# "ErrHandler" : ["Error Handler:", {"Ignore":"ignore", "Print":"print", "Abort":"abort"}]
		
		self.base_optionOrder = [
			"Framebuffer",
			"RenderLayers",
			"ShadingRate",
			"PixelSamplesX",
			"PixelSamplesY",
			"PixelFilter",
			"FilterSamplesX",
			"FilterSamplesY",
			"Gain",
			"Gamma"]
			
		self.options = {}
		self.optionOrder = []
		for option in self.base_optionOrder:
			self.optionOrder.append(option)
		for key in self.base_options:
			self.options[key] = self.base_options[key]
		
		sdict = globals()
		self.settings = sdict["instBtoRSettings"]
		self.evt_manager = sdict["instBtoREvtManager"]
		self.lighting = sdict["instBtoRLightManager"]
		self.log = sdict["instBtoRLog"]
		
		screen = Blender.Window.GetAreaSize()		
		self.editorPanel = ui.Panel(25, screen[1] - 50, 400, 180, "Export Settings:", "Render Settings:", None, False)
		
		self.close_button = ui.Button(self.editorPanel.width - 25, 5, 14, 14, "X", "X", 'normal', self.editorPanel, True)
		self.close_button.shadowed = False
		self.close_button.cornermask = 15
		self.close_button.radius = 1.5
				
		# close_func = self.close_button.callFactory(self.evt_manager.removeElement, self.editor)
		self.close_button.registerCallback("release", self.close)
		
		self.object_data = {} # fetch what will now be adapter objects for a given scene.
		self.camera_data = {} # I doubt this is neccessary, since the shader will now be saved per camera
		self.light_data = {} # ditto
		self.object_groups = {} # this *is* neccessary for instancing support
		self.occlusion_map = []
		
		self.renderContext = Blender.Scene.GetCurrent().getRenderingContext()
		
		self.editorPanel.addElement(ui.Label(5, 27, "Export Target:", "Export Target:", self.editorPanel, False))		
		target_menu = ["Selected Renderer...", "Files...", "Pipeline Command"]
		self.exportTarget = ui.Menu(87,27, 150, 25, "Export Target:", target_menu, self.editorPanel, True, custom = True, customPrompt = "Custom Command")
		
		self.showSettingsButt = ui.Button(273, 27, 120, 25, ">>", "Render Settings >>", 'normal', self.editorPanel, True)
		self.showSettingsButt.registerCallback("release", self.toggleSettings)
		
		self.showPassesButt = ui.Button(273, 60, 120, 25, ">>", "AOV Settings >>", 'normal', self.editorPanel, True)
		self.showPassesButt.registerCallback("release", self.togglePasses)
		
		self.showExtrasButt = ui.Button(273, 95, 120, 25, ">>", "Extra Options >>", 'normal', self.editorPanel, True)
		self.showExtrasButt.registerCallback("release", self.toggleExtras)
		
		# export path
		self.editorPanel.addElement(ui.Label(5, 63, "Export Path:", "Export Path:", self.editorPanel, False))
		path = self.settings.outputpath.getValue() + os.sep
		self.exportPath = ui.TextField(85, 60, 150, 25, "Export Path:", path, self.editorPanel, True)
		self.exportBrowse = ui.Button(237, 60, 25, 25, "...", "...", 'normal', self.editorPanel, True)
		self.exportBrowse.registerCallback("release", self.browsePath)
		
		# scene name - export prefix
		self.editorPanel.addElement(ui.Label(5, 98, "Scene Name:", "Scene Name:", self.editorPanel, True))
		self.exportPrefix = ui.TextField(85, 95, 150, 25, "Scene Name:", Blender.Scene.GetCurrent().getName(), self.editorPanel, True)
		self.autoName = ui.Button(237, 95, 25, 25, "**", "**", 'normal', self.editorPanel, True)
		self.autoName.registerCallback("release", self.setSceneName)
		
		self.purgeButton = ui.Button(15, self.editorPanel.height - 35, 150, 25, "Purge", "Purge export path", 'normal', self.editorPanel, True)
		self.purgeButton.registerCallback("release", self.purge)
		
		self.exportButton = ui.Button(self.editorPanel.width - 165, self.editorPanel.height - 35, 150, 25, "Export:", "Render!", 'normal', self.editorPanel, True)
		self.exportButton.registerCallback("release", self.preRenderCheck)
		
		self.suzanne = BtoRAdapterClasses.IObjectAdapter(BtoRTypes.BtoRPreview(Blender.Mesh.Primitives.Monkey()))
		
		self.settingsPanel = ui.Panel(405, 0, 250, 300, "", "", self.editorPanel, True)
		self.settingsPanel.isVisible = False
		self.settingsPanel.shadowed = True
		self.settingsPanel.outlined = True
		self.settingsPanel.hasHeader = False
		
		
		self.settingsLabel = ui.Label(5, 3, "Render Settings:", "Render Properties:", self.settingsPanel, True)
		self.settingsLabel.transparent = True
		self.scroller= ui.ScrollPane(5, 30, 240, 260, "Scroller", "Scroller", self.settingsPanel, True)
				
		self.setupRendererProperties()
		
		# render pass panel
		self.passesPanel = ui.Panel(405, 0, 300, 250, "", "", self.editorPanel, True)
		self.passesPanel.isVisible = False
		self.passesPanel.shadowed = True
		self.passesPanel.outlined = True
		self.passesPanel.hasHeader = False
		
		self.passesLabel = ui.Label(5, 3, "AOV Settings", "AOV Settings:", self.passesPanel, True)
		self.passesLabel.transparent = True
		
		self.passesContainer = ui.Panel(5, 65, 290, 150, "", "", self.passesPanel, True)
		self.passesContainer.shadowed = False
		self.passesContainer.outlined = False
		self.passesContainer.hasHeader = False
		self.passesContainer.addElement(ui.Label(20, 5, "Variable Name:", "Variable Name:", self.passesContainer, False, fontsize = 'small'))
		self.passesContainer.addElement(ui.Label(145, 5, "Mode:", "Mode:", self.passesContainer, False, fontsize = 'small'))
		self.passesContainer.addElement(ui.Label(200, 5, "Quantize:", "Quantize:", self.passesContainer, False, fontsize = 'small'))
		self.passesScroller = ui.ScrollPane(0, 30, 290, 150, "Scroller", "Scroller", self.passesContainer, True)
		
		# add / delete buttons
		self.addPass = ui.Button(5, 27, 105, 25, "Add Render Pass", "Add New Variable", 'small', self.passesPanel, True)
		self.addPass.registerCallback("release", self.addRenderPass)
		self.deletePass = ui.Button(115, 27, 145, 25, "Delete Selected Pass(es)", "Delete Selected Variable(s)", 'small', self.passesPanel, True)
		self.deletePass.registerCallback("release", self.deletePasses)
		self.passes = []
		
		# extra attributes panel
		self.extrasPanel = ui.Panel(405, 0, 350, 250, "", "", self.editorPanel, True)
		self.extrasPanel.isVisible = False
		self.extrasPanel.shadowed = True
		self.extrasPanel.outlined = True
		self.extrasPanel.hasHeader = False
		
		self.extrasLabel = ui.Label(5, 3, "Extra Options:", "Extra Options:", self.extrasPanel, True)
		self.extrasLabel.transparent = True
		
		self.extrasContainer = ui.Panel(5, 65, 340, 150, "", "", self.extrasPanel, True)
		self.extrasContainer.shadowed = False
		self.extrasContainer.outlined = False
		self.extrasContainer.hasHeader = False
		
		keyLabel = ui.Label(15, 0, "Keyword", "Name:", self.extrasContainer, True)
		keyLabel.fontsize = 'small'
		nameLabel = ui.Label(93, 0, "Name", "Variable:", self.extrasContainer, True)
		nameLabel.fontsize = 	'small'
		valueLabel = ui.Label(226, 0, "Value", "Value:", self.extrasContainer, True)
		valueLabel.fontsize = 'small'
		
		self.extrasContainer.addElement(keyLabel)
		self.extrasContainer.addElement(nameLabel)
		self.extrasContainer.addElement(valueLabel)
		
		self.extrasScroller = ui.ScrollPane(0, 27, 301, 150, "Scroller", "Scroller", self.extrasContainer, True)
		
		self.addExtraButt = ui.Button(5, 27, 135, 25, "Add Render Pass", "Add New Option", 'small', self.extrasPanel, True)
		self.addExtraButt.registerCallback("release", self.addExtra)
		self.deleteExtraButt = ui.Button(145, 27, 145, 25, "Delete Selected Pass(es)", "Delete Selected Option(s)", 'small', self.extrasPanel, True)
		self.deleteExtraButt.registerCallback("release", self.deleteExtras)
		self.extras = []			
		
		self.renderAnimation = False
		# setup the spacehandler
		self.setupSpaceHandler()		
		
		
		
	def setupRendererProperties(self, xml = None, sysGlobal = None):
		
		rproperties = self.settings.getRendererOptions()
		if rproperties != None and len(rproperties) > 0:
			for prop in rproperties:
				self.options[prop] = rproperties[prop] 
				# I probably don't care about ordering for the render properties, but their keys will keep them grouped anyway since dicts appear to index alphabetically
				self.optionOrder.append(prop)
		self.setupProperties()		
	
	def resetProperties(self):
		self.scroller.clearElements()
		self.properties = {}
		self.editors = {}
		self.options = {}
		self.optionOrder = []
		for option in self.base_optionOrder:
			self.optionOrder.append(option)
		for key in self.base_options:
			self.options[key] = self.base_options[key]
		self.setupRendererProperties()

	def deprecated(self): # now that I have the properties object
		xmlDefValues = {}
		if xml != None:
			# xml should be a nodelist of renderer settings
			for node in xml:
				# node should be a property element, thus
				name = node.getAttribute("name").encode("ascii")
				value = node.getAttribute("value").encode("ascii")
				
		
		for property in properties:
			# now I have a key
			propertyName = self.properties[property][0]
			propertyValue = self.properties[property][1]
			default = self.properties[property][2]
			self.properties[property] = BtoRAdapterClasses.IProperty(propertValue)
			self.properties[property].setName(propertyName)
			self.properties[property].setWidth(self.scroller.width - 15)
			self.editors[property] = BtoRAdapterClasses.IPropertyEditor(self.properties[option])
			self.scroller.addElement(self.editors[option].getEditor())
			self.editors[option].setParent(self.scroller)
			
		self.scroller.offset = 0
			
			
		
	def addExtra(self, button):
		extraPanel = ui.Panel(0, 0, 300, 22, "Extra", "", self.extrasScroller, True)
		extraPanel.outlined = True
		extraPanel.hasHeader = False
		extraPanel.cornermask = 0
		extraPanel.shadowed = False
		extraPanel.selector = ui.CheckBox(2, 3, " ", " ", False, extraPanel, True)
		extraPanel.selector.transparent = True
		extraPanel.selector.elements[0].transparent = True
		extraPanel.extraName = ui.TextField(15, 0, 75, 20, "ExtraName", "Name", extraPanel, True)
		extraPanel.extraVariable = ui.TextField(93, 0, 130, 20, "ExtraVariable", "Variable", extraPanel, True)
		extraPanel.extraValue = ui.TextField(226, 0, 75, 20, "ExtraValue", "Value", extraPanel, True)
		self.extrasScroller.addElement(extraPanel)
		self.extras.append(extraPanel)
		self.log.appendText("Added an extra option\n")
		return extraPanel
		
	def addRenderPass(self, button):
		passPanel = ui.Panel(0, 0, 274, 22, "RenderPass", "", self.passesScroller, True)		
		passPanel.outlined = True
		passPanel.hasHeader = False		
		passPanel.cornermask = 0
		passPanel.shadowed = False
		passPanel.selector = ui.CheckBox(2, 3, " ", " ",  False, passPanel, True) 
		passPanel.selector.transparent = True
		passPanel.selector.elements[0].transparent = True
		passPanel.passName = ui.TextField(15, 0, 128, 20, "PassName", self.exportPrefix.getValue() +  "_Cs", passPanel, True)
		passPanel.passMode = ui.Menu(143, 0, 49, 20, "Pass Mode:", ["Cs", "Os", "P", "dPdu", "dPdv", "N", "Ng", "u", "v", "s", "t", "du", "dv"], passPanel, True, custom = True, customPrompt = "Custom", shadowed = False)		
		passPanel.passQuant = ui.TextField(196, 0, 75, 20, "Quantize", "0 255 0 255", passPanel, True)
		passPanel.passMode.registerCallback("select", self.selectPassType)
		self.passesScroller.addElement(passPanel)
		self.passes.append(passPanel)
		self.log.appendText("Added an AOV pass\n")
		return passPanel
		
	def deleteExtras(self, button):
		selected = []
		for element in self.extras:
			if element.selector.getValue():
				self.extrasScroller.removeElement(element)
				selected.append(element)
		for element in selected:
			self.extras.remove(element)
		self.log.appendText("Deleting extra option...\n")
	
	def deletePasses(self, button):
		selected = []
		for element in self.passes:
			if element.selector.getValue():
				self.passesScroller.removeElement(element)
				selected.append(element)
		for element in selected:
			self.passes.remove(element)
		self.log.appendText("Deleting pass...\n")
		
	def selectPassType(self, menu):
		panel = menu.parent
		panel.passName.setValue(self.exportPrefix.getValue() + "_" + panel.passMode.getValue())
		
	def preRenderCheck(self, button):
		if self.exportPrefix.getValue() == "":
			self.evt_manager.showErrorDialog("Error: No scene name specified.", "Error! You must specify a name to export to.")
			self.log.appendText("Render attempt failed pre-render check!\n")
		else:
			# spawn the render
			self.renderScene()
	def setSceneName(self, button):
		self.exportPrefix.setValue(Blender.Scene.GetCurrent().getName())
	
	def browsePath(self, button):
		if button == self.exportBrowse:
			self.activeControl = self.exportPath
		Blender.Window.FileSelector(self.select, 'Choose any file')
		
	def select(self, file):
		path = os.path.dirname(file)
		self.activeControl.setValue(path + os.sep)
		
	def toggleExtras(self, button):
		if self.extrasPanel.isVisible:
			self.extrasPanel.isVisible = False
		else:
			self.extrasPanel.isVisible = True
			self.passesPanel.isVisible = False
			self.settingsPanel.isVisible = False
			
	def togglePasses(self, button):
		if self.passesPanel.isVisible:
			self.passesPanel.isVisible = False
		else:
			self.settingsPanel.isVisible = False
			self.extrasPanel.isVisible = False
			self.passesPanel.isVisible = True
			
	def toggleSettings(self, button):
		if self.settingsPanel.isVisible:
			self.settingsPanel.isVisible = False			
		else:
			self.settingsPanel.isVisible = True
			self.passesPanel.isVisible = False
			self.extrasPanel.isVisible = False
		self.editorPanel.invalid = True
		
	def render(self):
		# generate Ri calls for the scene
		# scene = Blender.Scene.GetCurrent()
		# render = scene.getRenderingContext(
		paths = self.settings.getShaderSearchPaths()
		outputPath = self.exportPath.getValue()
		# search path options
		for path in paths:
			ri.RiOption("searchpath", "shader", path + ":&")
		ri.RiOption("searchpath", "texture", outputPath + self.settings.shadows + ":&")
		ri.RiOption("searchpath", "texture", outputPath + self.settings.textures + ":&")
		ri.RiOption("searchpath", "texture", outputPath + self.settings.maps + ":&")
		ri.RiOption("searchpath", "archive", outputPath + self.settings.archives + ":&")
		
		
		
		ri.RiPixelSamples(self.getProperty("PixelSamplesX"), self.getProperty("PixelSamplesY"))
		ri.RiShadingRate(self.getProperty("ShadingRate"))
		ri.RiExposure(self.getProperty("Gain"), self.getProperty("Gamma"))
		ri.RiPixelFilter(self.getProperty("PixelFilter"), self.getProperty("FilterSamplesX"), self.getProperty("FilterSamplesY"))
		self.renderOptions()
		
	def renderOptions(self):
		# iterate renderer options
		rproperties = self.settings.getRendererOptions()
		if len(rproperties) > 0:
			for property in rproperties:
				propArray = property.split(":") # splits the option by : chars
				# propArray[0] = category
				# propArray[1] = type
				# propArray[2] = name
				render = True
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
						if "integer" in propArray[1] or "integer" in propArray[1]:
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
						# similiar method needs to be applied to per-renderer Attributes
			
			#if self.getProperty("MaxEyesplits") != self.options["MaxEyesplits"][2]:
			#	ri.RiOption("limits", "eyesplits", self.getProperty("MaxEyesplits"))
		

	def close(self, button):
		if self.settingsPanel.isVisible:
			self.settingsPanel.isVisible = False
			
		self.evt_manager.removeElement(self.editorPanel)
	def purge(self, button):
		# get the export path, and clear all of the archives and textures that have been generated
		path = self.exportPath.getValue()
		for item in ["shaders", "shadows", "images", "textures", "archives", "maps"]:
			p_path = path + self.settings.__dict__[item]
			# is it easer to list the files here, or should I make an os call?
			
	def renderScene(self):
		""" export/render the whole scene """		
		paths = self.settings.getShaderSearchPaths()
		self.lighting = globals()["instBtoRLightManager"]
		
		outputPath = self.exportPath.getValue() # get the value from *here*, THIS object
		if not os.path.exists(outputPath):
			os.mkdir(outputPath)
		if not os.path.exists(outputPath + self.settings.shadows):
			os.mkdir(outputPath + self.settings.shadows)
		if not os.path.exists(outputPath + self.settings.archives):
			os.mkdir(outputPath + self.settings.archives)
		if not os.path.exists(outputPath + self.settings.images):
			os.mkdir(outputPath + self.settings.images)
		if not os.path.exists(outputPath + self.settings.textures):
			os.mkdir(outputPath + self.settings.textures)
		if not os.path.exists(outputPath + self.settings.maps):
			os.mkdir(outputPath + self.settings.maps)
		# get required precursor info
		bScene = Blender.Scene.GetCurrent()
		rend = bScene.getRenderingContext()
		startFrame = rend.startFrame()
		endFrame = rend.endFrame()
		self.frames = range(startFrame, endFrame)
		
		self.toFile = False
		self.toRender = False
		self.toCustom = False
		if self.exportTarget.getSelectedIndex() == 1:
			self.toFile = True			
			filename = outputPath + os.sep + self.exportPrefix.getValue() # adding os.sep, just in case 
			# print "rendering to file"
		elif self.exportTarget.getSelectedIndex() ==  0:
			self.toRender = True			
			filename = outputPath + os.sep + self.exportPrefix.getValue()
		elif self.exportTarget.getSelectedIndex() == 3:
			self.toCustom = True
			filename = outputPath + os.sep + self.exportPrefix.getValue()
		else:
			self.toFile = True # not adding this in just yet
			filename = outputPath + os.sep + self.exportPrefix.getValue()
			
		# step 1, sort out the scene objects
		self.cameras = [] # temporary storage
		self.lights = []
		self.objects = []
		self.shadows = {}
		self.envmaps = {}		
		
		# lighting layer tracking
		self.layers = Blender.Scene.GetCurrent().getLayers()
		self.lightingLayers = [[] for x in range(20)]
		
		objs = Blender.Scene.GetCurrent().getChildren()
		
		# print len(objs), " objects found."
		# sort out the objects in this scene by type		
		objSequence = 1
		for obj in objs:
			if self.object_data.has_key(obj.getName()):
				adapter = self.object_data[obj.getName()]
			else:
				bObj = BtoRTypes.__dict__["BtoR" + obj.getType()](obj)
				self.object_data[obj.getName()] = BtoRAdapterClasses.IObjectAdapter(bObj) # add to the master object list for possible use later
				adapter = self.object_data[obj.getName()]
			if obj.getType() == "Camera":						
				self.cameras.append(adapter)
			elif obj.getType() == "Lamp":
				self.lights.append(adapter)
			else:
				self.objects.append(adapter)				
				
			adapter.objData["archive"] = outputPath + os.sep + self.settings.archives + os.sep # set the output path for each object to export it's own geometry				
			adapter.sequence = objSequence
			objSequence = objSequence + 1

		if self.renderAnimation == False:
			self.frames = [1] # reset the frame list to one frame - simple fix eh?
		
		# create object archives
		self.exportObjects() # remember to add animation support to this
		
		self.generateShadowMaps() # these need to be moved inside of the frame loop when I start adding support for animated lights
		self.generateEnvironmentMaps()
		outputPath = os.path.split(filename)[1]
		outputname = os.path.splitext(filename)[0] 				
		filename = outputname + ".rib"
		
		# getting started, I need to setup my error handler...this is optional, I may never re-enable this
		# ri.RiErrorHandler(self.options["ErrHandler"][1][self.getProperty("ErrHandler")])
		# print "Handler state is ", self.options["ErrHandler"][1][self.getProperty("ErrHandler")]
		
		if self.toFile:			
			ri.RiBegin(filename)
		elif self.toRender:
			ri.RiBegin(self.settings.renderers[self.settings.renderer][0]) # replace this with render flags
		elif self.toCustom:
			ri.RiBegin(self.exportTarget.getValue())
		else:
			ri.RiBegin()	
		
		for frame in self.frames:	
			self.frame = frame
			# frame block
			#Blender.Set("curfame", frame) 			
			cam = 1
			for camera in self.cameras:
				# split the filename off			
				imgFile = outputname + "_cam_%d_frame_%d" % (cam, frame) + ".tif" 
				
				# Main Render Start - Sounds just like Nasa doesn't it?

				print "Occlusion Maps"
				# feature masking will ignore this for renderers that don't support it.
				# Occlusion Map generation
				if self.lighting.getProperty("GenOcclusion") and self.lighting.getProperty("GenShadowMaps"): # yes, they *both* have to be on for SHADOWMAPPED occlusion
					occlMap = self.lighting.getProperty("AOShader")
					if occlMap.getObject().shader != None:
						# I have a shader, now
						map = getattr(occlMap.getObject().shader, "occlfile")
						occlString = "MakeOcclusion ["
						for smap in self.occlusion_maps:
							occlString = occlString + '"' + smap + '"'
						occlString = occlString +"] " + '"' + map + '"' + "\n"
						ri._ribout.write(occlString)
					else:
						self.lighting.getProperty("GenOcclusion").setValue(True)
						
					if 1 == 2:
						for group in self.lighting.occlGroups:
							groupName = group.groupName.getValue()
							if self.occlusion_maps.has_key(groupName):
								mapList = self.occlusion_maps[groupName]
								if len(mapList) > 0:							
									maps = "["
									for map in mapList:
										maps = maps + '"' + map + '" '
									maps = maps + "]"
									ri._ribout.write("MakeOcclusion " + maps + " " + '"' + groupName + ".sm" + '"' + "\n")
					
								
				# environment map generation
				print "Environment Maps"
				for obj in self.objects:
					if obj.getProperty("GenEnvMaps"):
						# this object created some environtment maps for this frame.
						# so charge on
						envmaps = self.envmaps[obj.objData["name"] + ("_%d" % frame)]
						maps = ""
						for dir in ["px", "nx", "py", "ny", "pz", "nz"]:
							maps = maps + '"' + envmaps[dir]["envFile"] + '" '							
						
						envString = "MakeCubeFaceEnvironment " + maps + '"' + obj.objData["name"] + '.tx" 92 "' + obj.getProperty("EnvMapPixelFilter") + '" '
						envString = envString + " %d %d" %(obj.getProperty("EnvMapFilterX") , obj.getProperty("EnvMapFilterY"))
						envString = envString + "\n"
						ri._ribout.write(envString) # environment map creation.
						
				print "Extra Options"
				# Custom extra RiOptions
				for extra in self.extras:
					found = True
					if "string" in extra.extraVariable.getValue():
						val = extra.extraValue.getValue()
					elif "integer" in extra.extraVariable.getValue():
						if "[" in extra.extraVariable.getValue():
							vals = extra.extraValue.getValue().split()
							val = []
							for aval in vals:
								val.append(int(aval))
						else:
							val = int(extra.extraValue.getValue())
					elif "float" in extra.extraVariable.getValue():
						if "[" in extra.extraVariable.getValue():
							vals = extra.extraValue.getValue()
							val = []
							for aval in vals:
								val.append(float(sval))
						else:
							val = float(extra.extraValue.getValue())
						
					else:
						found = False
					if found:
						variable = '"' + extra.extraVariable.getValue() + '"'
						name = extra.extraName.getValue()						
						ri.RiOption(name, variable, val) # charge on!
					#else:
					#	self.evt_manager.showErrorDialog("Bad Variable Type", "Your custom option has an incorrect variable type. It should be 'string', 'integer' or 'float'.")
				
				print "Frame Begin"				
				ri.RiFrameBegin(frame)
				
				# main render here
				for path in paths:
					if self.settings.renderer == "BMRT":
						path = path.replace("\\", "\\\\")
					ri.RiOption("searchpath", "shader", path + ":&")
				
				# Global scene options				
				self.render()
				if self.settings.renderer == "BMRT":
					imgFile = imgFile.replace("\\", "\\\\")
				ri.RiDisplay(imgFile, "file", "rgb")
				if self.getProperty("Framebuffer"):
					ri.RiDisplay("+" + imgFile, "framebuffer", "rgb")
				
				print "AOV passes"
				# AOV passes
				for variable in self.passes: # each variable has a name, a mode, and a quantize value
					vals = variable.passQuant.getValue().split()
					iVal = []
					for val in vals:
						iVal.append(int(val))
					params = { "quantize" : iVal }
					
					ri.RiDisplay("+" + variable.passName.getValue() + "_cam_%d" % cam + "_frame_%d" % frame + ".tif", "file", variable.passMode.getValue(), params)
				
				# self.renderFrame(exportSettings)		
				print "Renderer-specific commands"
				if self.settings.renderer == "Pixie":
					if self.lighting.properties.has_key("Hider"):
						hiderCommand = "Hider " + '"' +  self.lighting.getProperty("Hider") + '"' + "\n"
						ri._ribout.write(hiderCommand)
				
				print "Camera transform"
				# camera transform
				camera.render()	
				
				print "Render World"
				# world block
				self.renderWorld()
				
				cam = cam + 1
				ri.RiFrameEnd()			
				print "Frame complete!"
		ri.RiEnd()
		
		print "Render complete!"
		self.log.appendText("Render complete!\n")
		
	def generateShadowMaps(self):
		self.shadows = {}
		if self.lighting.getProperty("GenShadowMaps"):
			print "Shadow Maps"
			self.occlusion_maps = [] # each light knows whether it belongs to an occlusion group or not, so stuff shadow names into each key
			shadowPath = self.settings.outputpath.getValue() + self.settings.shadows
			
			for frame in self.frames:
				# if I'm rendering an animation, then I need to set the current frame
				# otherwise, not, and the above for loop will only happen once
				if self.renderAnimation:
					Blender.Set("curfame", frame) 
				lightIdx = 0
				for light in self.lights:			
					# update the light's data
					light.checkReset()
					lightIdx = lightIdx + 1
					doMap = light.getProperty("GenShadowMap")										
					render = False
					# create a shadowmap RIB		
					shadowList = {}
					for direction in light.getRenderDirections():						
						if doMap == "Lazy" and not light.changed: # this test first, since if the map doesn't exist, I'll have to create it.
							if light.shadowMaps.has_key(direction):
								if light.isAnimated:						
									# I should test for the existence of the file as well I suppose
									if os.path.exists(light.shadowMaps[direction + "_%d" % frame]["shadowName"]):
										self.shadows[light.objData["name"]] = light.shadowMaps[direction + "_%d" % frame]	
										render = False
										print "Found a shadowmap for " + light.objData["name"] + direction + ("_%d" % frame) + ", not rendering"
									else: # the shadowmap doesn't exist, but will be needed, so render it
										render = True
										print "Rendering shadowmap for " + light.objData["name"] + direction + ("_%d" % frame)
								else:
									if os.path.exists(light.shadowMaps[direction + "_%d" % frame]["shadowName"]):
										self.shadows[light.objData["name"]] = light.shadowMaps[direction] 
										render = False
										print "Found a shadowmap for " + light.objData["name"] + direction + ", not rendering"
									else:
										render = True
										print "Rendering shadowmap for " + light.objData["name"] + direction 
							else:
								# if there's no key, I have to render it.
								render = True
						elif (doMap == "Always") or (doMap == "Lazy" and light.changed):
							render = True
						
						# print "light render status is: ", render
						if render:
							if light.isAnimated: # the light is animated, so iterate all of the frames													
								shadow = shadowPath + light.objData["name"] + direction  + "_%d" % frame 
								shadowKey = direction + "_%d" % frame
							else:			
								shadow = shadowPath + light.objData["name"] + direction
								shadowKey = direction
								
							shadowName = shadow + ".tx"
							shadowFile = shadow + ".z"
							shadowRIB = shadow + ".rib"
							if self.settings.renderer == "BMRT":
								shadowFile = shadowFile.replace("\\", "\\\\")
								shadowName = shadowName.replace("\\", "\\\\")
							self.currentLightLayers = light.object.layers
							self.renderShadowMap(light, direction, shadowRIB, shadowName, shadowFile) # render the map here.
							shadowList[direction] = {"rib" : shadowRIB, "shadowName" : shadowName, "shadowFile" : shadowFile}
							light.shadowMaps[shadowKey] = shadowList
									
						self.shadows[light.objData["name"]] = shadowList # this is the *global* shadow list that the scene maintains. This can be regenerated on each round
				
	def generateEnvironmentMaps(self):
		self.envMaps = {}		
		envPath = self.settings.outputpath.getValue() + self.settings.maps
		for obj in self.objects:
			if obj.getProperty("GenEnvMaps"):
				envList = {}
				for direction in ["px", "nx", "py", "ny", "pz", "nz"]:
					for frame in self.frames:
						Blender.Set("curframe", frame)
						env = envPath + obj.objData["name"] + "_" + direction + ("_frame_%d" % frame)
						envFile = env + ".tiff"
						envRIB = env + ".rib"
						envName = env + ".tx"						
						self.renderEnvMap(obj, direction, envRIB, envName, envFile)
						envList[direction] = {"rib":envRIB, "envName":envName, "envFile":envFile}
				self.envmaps[obj.objData["name"] + ("_%d" % frame)] = envList
			# the map creation call here goes at the beginning of the rib file for the frame.
		self.log.appendText("Generated environment maps...\n")
	
	def renderEnvMap(self, obj, direction, envRIB, envName, envFile):
		if self.toFile:
			ri.RiBegin(envRIB)
		elif self.toRender:
			ri.RiBegin(self.settings.renderers[self.settings.renderer][0])
		elif self.toCustom:
			ri.RiBegin(self.exportTarget.getValue())
		else:
			ri.RiBegin()	# for pipeline command later
			
		paths = self.settings.getShaderSearchPaths()
		for path in paths:
			ri.RiOption("searchpath", "shader", path + ":&")	
		
		# these options should be overriden by object level options for env-map generation.
		ri.RiPixelSamples(obj.getProperty("EnvMapSamplesX"), obj.getProperty("EnvMapSamplesY"))
		ri.RiPixelFilter(obj.getProperty("EnvMapPixelFilter"), obj.getProperty("EnvMapFilterX"), obj.getProperty("EnvMapFilterY"))
		if self.settings.renderer == "BMRT":
			envFile = envFile.replace("\\", "\\\\")
			
		ri.RiDisplay(envFile, "file", "rgb")
		ri.RiProjection("perspective", "fov", 92)
		ri.RiShadingRate(obj.getProperty("EnvMapShadingRate"))
		obj.doCameraTransform(direction)
		ri.RiWorldBegin()
		params = { "uniform float intensity" : 1.0, "uniform point from": [0, 0, 0] }
		ri.RiLightSource("pointlight", params)
		self.renderLights()
		self.renderObjects(envObj = obj)
		ri.RiWorldEnd()
		
		ri.RiEnd() # see above RiBegin statement
		# add a texture make command here. in other words, get the texture tool from the main settings object		
				
	def renderShadowMap(self, light, direction, shadowRIB, shadowName, shadowFile):	
		
		print "Rendering shadowmap for ", light.objData["name"], "direction ", direction
		self.lightDebug = False
		if self.toFile:
			ri.RiBegin(shadowRIB)
		elif self.toRender:
			ri.RiBegin(self.settings.renderers[self.settings.renderer][0])
		elif self.toCustom:
			ri.RiBegin(self.exportTarget.getValue())
		else:
			ri.RiBegin()	# for pipeline command later

		paths = self.settings.getShaderSearchPaths()
		for path in paths:
			if self.settings.renderer == "BMRT":
				path = path.replace("\\", "\\\\")
			ri.RiOption("searchpath", "shader", path + ":&")
		eyeSplits = light.getProperty("ShadowMapEyeSplits")
		ri.RiOption("limits", "integer eyesplits", eyeSplits)
		size = light.getProperty("ShadowMapSize")
		ri.RiFormat(size, size, 1)
		ri.RiPixelSamples(1, 1)
		ri.RiPixelFilter("box", 1, 1)			
		ri.RiHider("hidden", {"uniform float jitter" : [0], "uniform string depthfilter" : "midpoint"})
		
		ri.RiDisplay(shadowFile, "zfile", "z")			
		if light.getProperty("ShowZBuffer"):
			ri.RiDisplay("+view_from_light " + shadowName, "zframebuffer", "z")
		projection = light.getRenderProjection()			
		if projection == "orthographic":
			ri.RiProjection(projection)
			# setup the clipping ratio
			xrange = light.getClippingRange()
			#ri.RiClipping(0.1, 1000)
			ri.RiClipping(xrange[0], xrange[1])
			screenVal = int(light.getProperty("ShadowMapWindow"))
			ri.RiScreenWindow(0 - screenVal, screenVal, 0 - screenVal, screenVal)
		else:
			ri.RiProjection(projection, "fov", 92) # 92 degrees projection
		ri.RiShadingRate(4.0)
		light.doCameraTransform(direction)
		ri.RiWorldBegin()
		# ri.RiReverseOrientation()		
		self.renderObjects(shadowPass = True)
		ri.RiWorldEnd()
		ri.RiMakeShadow(shadowFile, shadowName)
		
		ri.RiEnd()
		
		if self.lighting.getProperty("GenOcclusion"):
			#occlProp = light.getProperty("Group")
			#if occlProp != "None Selected":z
			#	if not self.occlusion_maps.has_key(occlProp):
			#		self.occlusion_maps[occlProp] = []
			self.occlusion_maps.append(shadowFile) # append to the occlusion map for this group
		self.log.appendText("Generated shadow maps...\n")
		
	def renderWorld(self):
		ri.RiWorldBegin()		
		# if we're shadowmapping, make sure to generate shadowmaps each of these - of course the question becomes how...
		self.renderLights()		
		#ri.RiIdentity()
		self.renderObjects()
		ri.RiWorldEnd()
		
	def renderWorldArchive(self):
		""" render the folowing objects to archives """
		for obj in self.objects:
			if not obj.getProperty("Ignore"):
				obj.renderArchive()

	def renderLights(self):
		sdict = globals()
		lm = sdict["instBtoRLightManager"]
		# occlusion setup here		
		if lm.getProperty("GenOcclusion"):
			# using occlusion
			#for group in lm.occlGroups: # render the AO ambient lights
			#	shader = group.aoShaderProperty.getValue().getObject()		
			shader = lm.getProperty("AOShader").getObject()
			shader.updateShaderParams()				
			ri.RiLightSource(shader.shader.shadername, shader.shader.params())

			if lm.getProperty("UseAmbient"):
				params = { "uniform float intensity" : lm.getProperty("AmbIntensity"), "lightcolor" : lm.getProperty("AmbColor") }
				ri.RiLightSource("ambient", params)
				
			for light in self.lights:
				if light.getProperty("IncludeWithAO"):
					if self.lighting.getProperty("GenShadowMaps"):
						if light.getProperty("GenShadowMap"):
							# print light.object.getName()
							#print dir(light)
							if self.shadows.has_key(light.objData["name"]):
								shadows = self.shadows[light.objData["name"]]								
								light.setShadowParms(shadows)
					light.render()
		else:
			if lm.getProperty("UseAmbient"):
				params = { "uniform float intensity" : lm.getProperty("AmbIntensity"), "lightcolor" : lm.getProperty("AmbColor") }
				ri.RiLightSource("ambient", params)
			for light in self.lights:
				# get the fame info
				frame = Blender.Get("curframe")
				if self.lighting.getProperty("GenShadowMaps"):
					if light.getProperty("GenShadowMap"):
						if self.shadows.has_key(light.objData["name"]):
							shadows = self.shadows[light.objData["name"]]			
							light.setShadowParms(shadows)	
				light.render()
				
					
		
		
	def renderObjects(self, shadowPass = False, envObj = None):
		for obj in self.objects:
			# get all the objects in the scene that aren't lights or cameras			
			if obj != envObj or shadowPass or not obj.getProperty("Ignore"):
				obj.render(shadowPass = shadowPass)
		
	def exportObjects(self):
		for obj in self.objects:
			obj.renderArchive()
		
	def getEditor(self):
		return self.editorPanel
		

		
	def getSceneXMLData(self):
		
		# get the material list
		dict = globals()		
		keys = dict.keys()
		for key in keys:		
			if isinstance(dict[key], MaterialList):
				self.materials = dict[key]
				# material list
		newdoc =xml.dom.minidom.Document()
		root = newdoc.createElement("BtoR")
		newdoc.appendChild(root)
		
		# scene properties
		scene = newdoc.createElement("Scene")		
		scene = self.saveProperties(newdoc, scene )
		
		# now to worry about extra options and AOV variables. I only care about the variable TYPE, really, because I can regen everything from the export paths and scene name.
		for aovPass in self.passes:
			mode = aovPass.passMode.getValue()
			quant = aovPass.passQuant.getValue()
			aovXML = newdoc.createElement("AOVPass")
			aovXML.setAttribute("PassMode", mode)
			aovXML.setAttribute("Quantize", quant)
			scene.appendChild(aovXML)
		
		for extra in self.extras:
			extraXML = newdoc.createElement("ExtraOption")
			extraXML.setAttribute("ExtraName", extra.extraName.getValue())
			extraXML.setAttribute("ExtraVariable", extra.extraVariable.getValue())
			extraXML.setAttribute("ExtraValue", extra.extraValue.getValue())
			scene.appendChild(extraXML)			
		
		root.appendChild(scene)

		lighting = newdoc.createElement("Lighting")
		lighting = self.lighting.saveProperties(newdoc, lighting)
				
		for group in self.lighting.occlGroups:
			occlXML = newdoc.createElement("OcclusionGroup")
			occlXML.setAttribute("Name", group.groupName.getValue())
			shader = group.aoShaderProperty.getValue().getObject()
			# print shader
			if shader.getStrValue() != "None":
				shaderXML = shader.getShaderXML(newdoc)
			occlXML.appendChild(shaderXML) # annnnnd done.			
			lighting.appendChild(occlXML)
		root.appendChild(lighting)
		
		mat = self.materials.saveMaterials(newdoc)
		root.appendChild(mat)
		
		obj = self.saveObjects(newdoc)
		
		root.appendChild(obj)
		# self.saveSceneSettings(root_element)
		return root
		
	def writeToFile(self, xml, filename):
		try:
			file = open(filename, 'w')
			file.write(xml.toprettyxml())
			file.close()
		except: 
			traceback.print_exc()
			self.evt_manager.showErrorDialog("There was an error saving the file.", "There was an error saving the file.")
			self.log.appendText("Error saving XML file.\n")
	def loadFromFile(self, filename):
		
		try:
			file = open(filename, 'r')
			xmlfile = file.readlines()
			xmlStr = ""
			for line in xmlfile:
				xmlStr = xmlStr + line
			found = True
		except:
			self.evt_manager.showErrorDialog("Error parsing XML!", "There was an error in the XML file or it was not found.")	
			self.log.appendText("Error loading file...\n")
			found = False
			return None
		if found:
			self.parseXML(xmlStr)
		
	def writeToBlenderText(self, xml):
		try:
			text = Blender.Text.Get("BtoRXML")
			found = True
		except NameError: # didn't find one...and I should add functionality to BtoRSettings to make sure this never happens
			# not found, create a new one
			text = Blender.Text.New("BtoRXML")
			found = False
		
		try:
			xmlOut = xml.toprettyxml()
			text.clear()
			# doesn't matter, it's getting written
			text.write(xmlOut) # that should be that.
		except:
			# not found, spawn a dialog
			traceback.print_exc()
			self.evt_manager.showErrorDialog("Error!", "Something went wrong. Look at the console!")
			self.log.appendText("Error saving to blender text!\n")
	def setupSpaceHandler(self):
		global handler
		try:
			text = Blender.Text.Get("BtoRSpaceHandler")
			found = True
		except:
			handlerText = Blender.Text.New("BtoRSpaceHandler")
			handlerText.write(handler)
			
			
	def loadSceneData(self):
		try:
			text = Blender.Text.Get("BtoRXML")
			lines = text.asLines()
			xmlData = ""
			for line in lines:
				xmlData = xmlData + line
			# do something fun here
			found = True
		except: 
			traceback.print_exc()
			# self.evt_manager.showErrorDialog("Error parsing XML!", "There was an error in the BtoRXML text file or it was not found.")
			return None
		if found:
			self.parseXML(xmlData)

	def parseXML(self, xmlData):
		self.object_data = {} # clear the object data cache
		try:
			xmlData = xml.dom.minidom.parseString(xmlData)
			hasXMLData = True
		except xml.parsers.expat.ExpatError:
			hasXMLData = False	
			# pop up an error dialog!
			traceback.print_exc()
			self.evt_manager.showConfirmDialog("Error parsing XML!", "There was an error in the BtoRXML text file!", None, False)
			self.log.appendText("Error parsing XML!")
			return None
		if hasXMLData:
			# deal with each in turn.
			 
			xmlScene = xmlData.getElementsByTagName("Scene")[0] # there should be and had *better* be only one of thse
			
			xmlLighting = xmlData.getElementsByTagName("Lighting")[0] # and this too
			xmlObjects = xmlData.getElementsByTagName("Object")
			
			self.loadProperties(xmlScene)
			
			xmlAOV = xmlScene.getElementsByTagName("AOVPass")
			for aov in xmlAOV:
				aovPanel = self.addRenderPass(None)
				aovPanel.passMode.setValueString(aov.getAttribute("PassMode"))
				aovPanel.passQuant.setValue(aov.getAttribute("Quantize"))
				
				self.selectPassType(aovPanel.passMode)
				
			xmlExtras = xmlScene.getElementsByTagName("ExtraOption")
			
			for extra in xmlExtras:
				extraPanel = self.addExtra(None)
				extraPanel.extraName.setValue(extra.getAttribute("ExtraName"))
				extraPanel.extraVariable.setValue(extra.getAttribute("ExtraVariable"))
				extraPanel.extraValue.setValue(extra.getAttribute("ExtraValue"))
			
			
			self.lighting.loadProperties(xmlLighting)
			
			occlGroups = xmlLighting.getElementsByTagName("OcclusionGroup")
			
			for group in occlGroups:
				occl = self.lighting.addOccl(None)
				print "Setting up an occlusion group!"
				occl.groupName.setValue(group.getAttribute("Name"))
				shaderXML = group.getElementsByTagName("shader")
				print "setting up a shader from ", shaderXML
				if len(shaderXML) > 0:
					occl.aoShaderProperty.initShader( useXML = True, xml = shaderXML[0])
				
			
			if len(xmlObjects) > 0:
				for xmlObject in xmlObjects:
					# get the name of the object in question. They should all have names.
					objName = xmlObject.getAttribute("name")
					# find the object in blender
					obj = Blender.Object.Get(objName)
					# load the target object...
					bObj = BtoRTypes.__dict__["BtoR" + obj.getType()](obj)
					objData = BtoRAdapterClasses.IObjectAdapter(bObj) # get an adapter
					# test to ensure that I've got the same object type...
					if obj.getType() != xmlObject.getAttribute("type"):
						# just in case something went wrong here, discard most of the object data and re-initialize with just a material def
						if xmlObject.getAttribute("material") != "":
							objData.objData["material"] = xmlObject.getAttribute("material")
					else:
						# otherwise, load up the full data from the object definition
						objData.loadData(xmlObject)
					self.object_data[objName] = objData # stuff the adapter component into the object data
		# return some statistics here 
		self.log.appendText("%d objects were successfully loaded..." % len(xmlObjects))
		#self.evt_manager.showConfirmDialog("Scene Data Loaded!", "%d objects were successfully loaded." % len(xmlObjects), None, False)
			
	def saveObjects(self, xml):
		
		#create a root object for the XML
		objectRoot = xml.createElement("Object_Defs")
		
		# object save routines for each adapter
		# get the current list of objects in the scene
		scene = Blender.Scene.GetCurrent()
		
		objList = scene.getChildren()
		for obj in objList:
			if self.object_data.has_key(obj.getName()):
				objXml = self.object_data[obj.getName()].saveData(xml)
				print "XML received for ", obj.getName(), "of type ", obj.getType(), "..."
			else:
				# create a new adapter in case there's no data defined for it.
				bObj = BtoRTypes.__dict__["BtoR" + obj.getType()](obj)
				adapter = BtoRAdapterClasses.IObjectAdapter(bObj)	
				objXml = adapter.saveData(xml)
				self.object_data[obj.getName()] = adapter
				print "New adapter generated for ", obj.getName(), " of type ", obj.getType(), "..."
			objectRoot.appendChild(objXml)	# that should be successful one way or another.		
		
		return objectRoot
					
	def saveSimpleObject(self, objData, xml):
		objXml = xml.createElement("Object")
		for key in objData:
			objXml.setAttribute(key, objData[key])		
		return objXml
		
	def saveObject(self, objData, xml):
		objXml = xml.createElement("object")
		for key in objData:			
			if key == "shaderparms":
				shaderparms = objData["shaderparms"]
				# I need to spawn an RMShader to deal with this effectively.
				# Before I can do that, the restore needs to be in place to shove the data back into the shader 
				for skey in shaderparms:
					#print type(shaderparms[skey])
					shader.setAttribute(skey, shaderparms[skey])
					
				objXml.appendChild(shader)
			else:
				objXml.setAttribute(key, objData[key])
		return objXml				
			
		
	def saveShaderParms(self, shader, xml): # I only need to pass the shader in here
		shaderNode = xml.createElement("Shader")				
		shaderNode.setAttribute("name", mat.surfaceShaderName())
		if shader.filename != None:
			shaderNode.setAttribute("path", os.path.normpath(shader.filename))
		else:
			shaderNode.setAttribute("path", "None")
		
		for parm in shader.shaderparams:
			# get the param and stuff into my dict				
			# create the node
			parmNode = xml.createElement("Param")
			value = getattr(mat.surface, parm[1])
			# create an XML element for this value.
			s_type = parm[0].split()[1]
			# setup as much of the node element as I can here
			parmNode.setAttribute("name", parm[1])
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
						parmNode.setAttribute(sep.join(["value", index]), '%f' % value[x, y])
						index = index + 1
			
			# now commit this node to the shader node
			shaderNode.appendChild(parmNode)
		return shaderNode
			

class ObjectEditor(BtoRObject):
	def __init__(self):
		
		sdict = globals()
		self.settings = sdict["instBtoRSettings"]
		self.evt_manager = sdict["instBtoREvtManager"]
		self.scene = sdict["instBtoRSceneSettings"]
		self.materials = sdict["instBtoRMaterials"]
		self.groupList = sdict["instBtoRGroupList"]
		self.log = sdict["instBtoRLog"]		
		setattr(BtoRAdapterClasses, "instBtoREvtManager", sdict["instBtoREvtManager"])
		setattr(BtoRAdapterClasses, "instBtoRSceneSettings", sdict["instBtoRSceneSettings"])
		setattr(BtoRAdapterClasses, "instBtoRMaterials", sdict["instBtoRMaterials"])
		setattr(BtoRAdapterClasses, "instBtoRHelp", sdict["instBtoRHelp"])
		setattr(BtoRAdapterClasses, "instBtoRObjects", self)
		
		self.debug = False
				
		self.selected_objects = []
		
		# master panel
		screen = Blender.Window.GetAreaSize()	
		self.editorPanel = ui.Panel(10, screen[1] - 30, 260, 400, "Object Editor:", "Object Properties:", None, False)
		self.editorPanel.addElement(ui.Label(10, 30, "Object Name:", "Object Name:", self.editorPanel, False))
		
		self.objectName = ui.Label(15 + self.editorPanel.get_string_width("Object Name:", 'normal'), 30, "None Selected", "None Selected", self.editorPanel, True)
		self.objectMenu = ui.Menu(self.objectName.x, 30, 150, 25, "Object menu", ["None Selected"], self.editorPanel, True)
		self.objectMenu.isVisible = False
		self.objectMenu.registerCallback("release", self.selectObject_menu)
		
		containerOffset = 110
		containerHeight = 280
								
		self.close_button = ui.Button(self.editorPanel.width - 20, 5, 14, 14, "X", "X", 'normal', self.editorPanel, True)
		self.close_button.shadowed = False
		self.close_button.cornermask = 15
		self.close_button.radius = 1.5
		# close_func = self.close_button.callFactory(self.evt_manager.removeElement, self.editor)
		self.close_button.registerCallback("release", self.close)
		
		self.reset_button = ui.Button(15, 50, 130, 15, "Reset", "Reset Object Data", 'small', self.editorPanel, True)
		self.reset_button.registerCallback("release", self.resetObject)
		
		self.objEditorPanel = ui.Panel(4, 70, 491,  280, "Empty Panel", "", None, False) # keep this one invisible
		self.objEditorPanel.isVisible = False
		self.objEditorPanel.hasHeader = False
		self.objEditorPanel.cornermask = 0
		self.objEditorPanel.shadowed = False
		self.objEditorPanel.outlined = False
		
		self.editorPanel.addElement(self.objEditorPanel) 
		
		self.export_funcs = []
		#ok, this is now unneccessary
		# objFunc = self.editorPanel.callFactory(self.exportObject, "hi!")
		
		# set up the listener for the spacehandler 
		self.evt_manager.registerCallback("draw", self.getSelected)
		
		# self.infoButton = ui.Button(250, 60, 150, 25, "Show object info", "Show object info", 'normal', self.editorPanel, True)
		# self.infoButton.registerCallback("release", self.showObjData)
		# material selector setup		
		self.materials.select_functions.append(self.selectMaterial)
		
		# group selector
		self.groupList.select_functions.append(self.selectGroup)

		self.materials.loadMaterials()
		self.scene.loadSceneData()
		
	def getEditor(self):
		return self.editorPanel

	
	def exportScene(self):
		pass
		
	def selectionChanged(self):
		try:
			changed = Blender.Registry.GetKey("BtoR_Space", False)
			selection = changed["changed"]
			changed["changed"] = False			
		except:
			selection = False
		return selection
		
	def selectObject_menu(self, obj):
		object = self.objects[self.objectMenu.getSelectedIndex()]
		self.selectObject(object)
		
	def selectObject(self, obj):
		""" An object's been selected, make the magic perform. """		
		name = obj.getName()		
		self.objectName.setValue(name)
		objType = obj.getType()	
		self.log.appendText("Selected object " + name)	
		# print self.scene.object_data
		# so instead I simply retrieve the object in question
		if self.scene.object_data.has_key(obj.getName()): 
			try:
				self.objData = self.scene.object_data[obj.getName()] # fetch the adapter 
			except:
				traceback.print_exc()
				self.log.appendText("Failed to fetch object data for " + name)
			# flag the object as modified						
		else:			
			# no adapter found in the scene settings dictionary, so
			# create a new one based on a returned BtoR type
			try:
				bObj = BtoRTypes.__dict__["BtoR" + obj.getType()](obj)				
				self.objData = BtoRAdapterClasses.IObjectAdapter(bObj)
				self.scene.object_data[obj.getName()] = self.objData # assign the new adapter to the scene settings dict
			except:
				traceback.print_exc()
				self.log.appendText("Failed to allocate object adapter for " + name)
		
		# editor panel setup
		self.objEditor = self.objData.getEditor()
		try:			
			self.editorPanel.removeElement(self.objEditorPanel)
		except:
			self.log.appendText("Failed to remove non-existent object")
		self.objEditorPanel = self.objEditor.getEditor()
		self.objEditorPanel.parent = self.editorPanel
		self.objEditorPanel.invalid = True
		self.editorPanel.addElement(self.objEditorPanel)
		
		# and finally, object checks/resets for interested objects
		self.objData.checkReset()
		
	def resetObject(self, button):
		""" reset this object's data """
		if self.objData != None:
			try:			
				self.editorPanel.removeElement(self.objEditorPanel)	
			except: 
				print "No panel found"
			try:
				objName = self.objData.object.getName() 
				self.scene.object_data.pop(objName) #removes the reference from objectData
				self.objData = None
				self.objEditorPanel = None
				print "reselecting ", objName
				self.selectObject(Blender.Object.Get(objName))
			except:
				print "Bad things happened"
				traceback.print_exc()
		
	def selectMaterial(self, button):
		# set the material in the current object adapter
		self.evt_manager.removeElement(self.materials.getSelector())
		matName = button.title
		mat = self.materials.getMaterial(matName)
		self.objData.setMaterial(mat)
		
	def selectGroup(self, button):
		self.evt_manager.removeElement(self.groupList.getEditor())
		groupName = button.title
		self.objData.setGroup(groupName)
		
		
	def showObjData(self, button):
		self.objData.getInfo()
		
	def exportSingleObject(self):
		""" export a single object """
		# ok so here, I get the scene settings from the scene object and react to those
		exportSettings = self.objData.objEditor.exportSettings
		# get the filename and export path first
		if exportSettings.camera_menu.getSelectedIndex() == 0:
				makeFullScene = True
				# but which camera do I get?
				# for now, support only one camera, that being the first one I find.
				
		toFile = False
		toRender = False
		if exportSettings.export_menu.getSelectedIndex() == 0:
			toFile = True			
			filename = exportSettings.filename.getValue()
		elif exportSettings.export_menu.getSelectedIndex() == 1:
			toRender = True			
			filename = exportSettings.filename.getValue()
		else:
			toFile = False
			filename = exportSettings.textName.getValue()
		
		# ok, so the scene settings
		if makeFullScene:
			if toFile:
				ri.RiBegin(filename)
			elif toRender:
				ri.RiBegin("aqsis")
			else:
				ri.RiBegin()
			ri.RiFrameBegin(1) # just one frame for now		
			
			# Renderman scene stuff, exposure, samples, shading rate, etc.
			self.scene.render()
			imgFile = os.path.splitext(filename)[0] + ".tif "
			ri.RiDisplay(imgFile, "file", "rgb")
			
			# the blender camera (or somehow find the current 3D view, not sure how to do that yet.
			blenderScene = Blender.Scene.GetCurrent()
			
			camera = blenderScene.getCurrentCamera()
			if self.scene.object_data.has_key(camera.getName()):
				camAdapter = self.scene.object_data[camera.getName()] # fetch the camera adapter for this object
			else:
				# create a new camera adapter.
				bObj = BtoRTypes.__dict__["BtoR" + camera.getType()](camera)
				camAdapter = BtoRAdapterClasses.IObjectAdapter(bObj)		
				self.scene.object_data[camera.getName()] = camAdapter # assign the new adapter to the scene settings dict
			
			
			camAdapter.render()
			ri.RiWorldBegin()
			# grab all the lights	
			objs = blenderScene.getChildren()
			for obj in objs:
				if obj.getType() == "Lamp":
					if self.scene.object_data.has_key(obj.getName()):
						self.scene.object_data[obj.getName()].render()
					else:
						bObj = BtoRTypes.__dict__["BtoR" + obj.getType()](obj)
						self.scene.object_data[obj.getName()] = BtoRAdapterClasses.IObjectAdapter(bObj)
						self.scene.object_data[obj.getName()].render() # move this redundant crap to it's own func
		
		ri.RiIdentity() # normalize the transform matrix here.
		
		ri.RiAttributeBegin()
		ri.RiTransformBegin()
		
		# if making a full scene here, include transform data
		if makeFullScene:
			# objMatrix = cgkit.cgtypes.mat4(self.objData.object.matrix)

			# so how to compensate for the weird Z axis thing?
			ri.RiTransform(self.objData.object.matrix)
		self.objData.render()
		
		ri.RiTransformEnd()
		ri.RiAttributeEnd()
		
		if makeFullScene:
			ri.RiWorldEnd()
			# add possible support for exporting object animation stuff, but that should really be in the main scene rib
			# need to find out exactly how to do that.
			ri.RiFrameEnd()
			ri.RiEnd()
	
	def noPanel(self):
		if self.objEditorPanel in self.editorPanel.elements:
			self.editorPanel.removeElement(self.objEditorPanel)
		
	def getSelected(self):
		if self.editorPanel in self.evt_manager.elements: #only if this instance is up and running.
			if self.selectionChanged():
				try:
					objects = Blender.Object.GetSelected()
				
					# got nothing
					# get the first object
					if len(objects) < 1:
						self.objectName.isVisible = True
						self.objectMenu.isVisible = False

						self.objectName.setValue("None Selected")						
						self.noPanel()
						
					elif len(objects) > 1:
						self.objectName.isVisible = False
						self.objectMenu.isVisible = True
						self.obj_menu = [] # reset the menu
						self.obj_types = []
						for obj in objects:
							self.obj_menu.append(obj.getName())
							
						self.objects = objects
						self.objectMenu.re_init(self.obj_menu)
						self.objectMenu.setValue(1)
						self.selectObject_menu(None)
					else:
						self.objectName.isVisible = True
						self.objectMenu.isVisible = False
						obj = objects[0]
						self.selectObject(obj)
				except:
					traceback.print_exc()
					# there was a failure, dammit, this is where it's dying!
					self.objectName.isVisible = True
					self.objectMenu.isVisible = False					
					self.objectName.setValue("None Selected")
					self.noPanel()			
					
								
	def initializeBtoRObjectData(self): # this changes to support generation of BtoR export adapters
		# get everything in the current scene.
		# I should probably get the scene name here as well for reference, since BtoR *should* be able to store seperate scene setups in a .blend 
		# with multiple scenes in it.
		objList = Blender.Scene.getChildren()
		objectID = 0 # this ensures that all objects have a unique handle

		for object in objList:			
			self.scene.objData[object.getName()] = protocols.adapt(object.getType(), IObjectAdapter)	
			
	def close(self, button):
		self.evt_manager.removeElement(self.editorPanel)
	
class Material(BtoRObject):	
	def __init__(self, material, parent, selector):	

		# material should be an RMMaterial			
		sdict = globals()		
		self.settings = sdict["instBtoRSettings"]
		self.evt_manager = sdict["instBtoREvtManager"]
		self.scene = sdict["instBtoRSceneSettings"]
		self.materials = sdict["instBtoRMaterials"]
				
		if material <> None:
			self.material = material
			render = True
		else:			
			self.material = cgkit.rmshader.RMMaterial()
			render = False
		# panel init		
		screen = Blender.Window.GetAreaSize()	
		self.editorPanel = ui.Panel(225, screen[1] - 40, 400, 260, "shaders", "Material Settings:", None, False)		
		
		self.previewTypeMenu = ui.Menu(15, 35, 128, 25, "Preview Type", ["Bicubic Sphere", "Cube", "Suzanne - Mesh", "Suzanne - Subdiv", "Teapot"], self.editorPanel, True)
		self.preview_button = ui.Button(15, 200, 128, 25, "Preview Material:", "Preview Material:", 'normal', self.editorPanel, True)
		self.preview_button.registerCallback("release", self.renderPreview)
		
		try:
			self.image = Blender.Image.Get(self.material.name)
		except:
			self.image = Blender.Image.New(self.material.name, 128, 128, 24)
			
		self.editorPanel.addElement(ui.Image(15, 65, 128, 128, self.image, self.editorPanel, False))
		
		self.close_button = ui.Button(self.editorPanel.width - 20, 5, 14, 14, "X", "X", 'normal', self.editorPanel, True)
		self.close_button.shadowed = False
		self.close_button.cornermask = 15
		self.close_button.radius = 1.5
		self.close_button.registerCallback("release", self.close)
		
		self.matListPanel = ui.Panel(0, 0, 184, 52, "", "", parent, True)
		self.matListPanel.cornermask = 0
		self.matListPanel.radius = 1
		self.matListPanel.shadowed = False
		self.matListPanel.hasHeader = False
		self.matListPanel.bordered = False
		self.matListPanel.mat = self
		
		# editor button that displays the material editor
		self.editorButton = ui.Button(28, 0, 155, 52, self.material.name, self.material.name, 'normal', self.matListPanel, True)
		self.editorButton.shadowed = False
		self.editorButton.outlined = True
		self.editorButton.textlocation = 1
		self.editorButton.bordered = False
		self.editorButton.mat = self
		
		self.editorButton.registerCallback("release", self.showEditor)		
		self.editorButton.image = ui.Image(109, 1, 48, 48,  self.image, self.editorButton, False)
		
		self.selector = ui.CheckBox(6, 20, "", "", False, self.matListPanel, True)
		self.selector.transparent = True
		self.matListPanel.selector = self.selector
		
		self.selectorButton = ui.ToggleButton(0, 0, 180, 65, self.material.name, self.material.name, 'normal', selector, True)
		self.selectorButton.shadowed = False
		self.selectorButton.outlined = True
		self.selectorButton.textlocation = 1
		self.selectorButton.image = ui.Image(120, 5, 56, 56, self.image, self.selectorButton, False)
		self.selectorButton.title = self.editorButton.title
		self.selectorButton.mat = self
					
		self.select_functions = []
			
		self.options = { "MaterialName" : ["Material Name:", self.material.name],
					"color" : ["Color:", [1.0, 1.0, 1.0]],
					"opacity" : ["Opacity:", [1.0, 1.0, 1.0]],
					"DispBound" : ["Displacement Bound:" , 2.0],
					"Surface" : ["Surface Shader:", BtoRTypes.BtoRShaderType(Shader(self.material.surface, "surface", self))],
					"Displacement" : ["Displacement Shader:", BtoRTypes.BtoRShaderType(Shader(self.material.displacement, "displacement", self))],
					"Volume": ["Volume Shader", BtoRTypes.BtoRShaderType(Shader(self.material.interior, "volume", self))],
					"Translation":["Translation:", cgkit.cgtypes.vec3(0, 0, 0)],
					"Rotation":["Rotation:", BtoRTypes.BtoRRotationType([0.0, False, False, False])],
					"Scale":["Scale:", cgkit.cgtypes.vec3(0, 0, 0)]
		}
		
		self.optionOrder = [ "MaterialName",
						"Surface",
						"Displacement",
						"Volume",
						"color",
						"opacity",
						"DispBound",
						"Translation",
						"Rotation",
						"Scale"]
						
		self.editorPanel.addElement(ui.Label(155, 27, "Material Properties:", "Material Properties:", self.editorPanel, False))
		self.scroller= ui.ScrollPane(155, 45, 240, 200, "Scroller", "Scroller", self.editorPanel, True)
		self.setupProperties()
		
		# initialize the color and opacity properties from the material if I have one
		self.editors["color"].setValue(self.material.color())
		self.editors["opacity"].setValue(self.material.opacity())		
		
		self.surfPanel = self.properties["Surface"].getValue().getObject().getEditor()		
		#self.properties["Surface"].getValue().getObject().registerCallback("update", self.updateShaderName)
		self.editorPanel.addElement(self.surfPanel)
		self.surfPanel.isVisible = False
		self.surfPanel.shadowed = True
		self.surfPanel.outlined = True
		self.surfPanel.hasHeader = False				
		self.surfPanel.invalid = True
		self.surfPanel.validate()
				
		self.dispPanel = self.properties["Displacement"].getValue().getObject().getEditor()
		#self.properties["Displacement"].getValue().getObject().registerCallback("update", self.updateShaderName)
		self.editorPanel.addElement(self.dispPanel)
		self.dispPanel.isVisible = False
		self.dispPanel.shadowed = True
		self.dispPanel.outlined = True
		self.dispPanel.hasHeader = False
			
		self.volumePanel = self.properties["Volume"].getValue().getObject().getEditor()		
		self.editorPanel.addElement(self.volumePanel)
		self.volumePanel.isVisible = False
		self.volumePanel.shadowed = True
		self.volumePanel.outlined = True
		self.volumePanel.hasHeader = False
		
		if render:
			self.renderPreview(None)
	
	def showShader(self, type):
		if type == "surface":
			if self.surfPanel.isVisible == True:
				self.surfPanel.isVisible = False
			else:
				self.surfPanel.isVisible = True
				self.dispPanel.isVisible = False
				self.volumePanel.isVisible = False
		elif type == "displacement":
			if self.dispPanel.isVisible == True:
				self.dispPanel.isVisible = False
			else:
				self.surfPanel.isVisible = False
				self.dispPanel.isVisible = True
				self.volumePanel.isVisible = False
		else:
			if self.volumePanel.isVisible == True:
				self.volumePanel.isVisible = False
			else:
				self.surfPanel.isVisible = False
				self.dispPanel.isVisible = False
				self.volumePanel.isVisible = True
			
	def setImage(self, image):
		self.image = image		
		self.materialButton.image.bindImage()
		self.selectorButton.image.bindImage()
			
	def close(self, button):
		self.evt_manager.removeElement(self.editorPanel)
		
	def rename(self, objl):
		# add in redundancy checking here to ensure that I don't have two materials of the same name.
		# see if he name exists in the material list already
		
		for button in self.material_list.scroller.elments:
			if self.button.title == self.materialName.getValue():
				duplicate_name = True
				break
		if duplicate_name:
			self.materialName.setValue
			self.evt_manager.showConfirmDialog("Material name Already Exists!", "That material name already exists. Please choose another.", None, False)
			
		else:
			self.editorButton.title = self.materialName.getValue()
			# also on a material rename, I should probably search the scene data to figure out if any objects reference this material and change the referenc to match.
			# thus			
			for object in self.scene.object_data.itervalues():
				try:
					if object["material"] == self.name:
						object["material"] = self.materialName.getValue()
				except:
					print "Some kind of error occured"
			self.name = self.materialName.getValue()
									
		
	def getEditor(self):
		return self.editorPanel
		
	def getMaterialButton(self):
		return self.editorButton
		
	def getSelectorButton(self):
		return self.selectorButton
	
	def selectMaterial(self):
		for func in self.select_functions:
			func(self)
		
	def showEditor(self, button):
		self.evt_manager.addElement(self.editorPanel)		
		
	def draw(self):
		self.editorPanel.draw()
		
	def updateColor(self, obj):
		setattr(self.material, "_color", cgkit.cgtypes.vec3(float(self.color_red.getValue()), float(self.color_green.getValue()), float(self.color_blue.getValue())))
			# self.material._color = (float(self.color_red.getValue()), float(self.color_green.getValue()), float(self.color_blue.getValue()))
			
	def updateOpacity(self, obj):
		setattr(self.material, "_opacity", cgkit.cgtypes.vec3(float(self.opacity_red.getValue()), float(self.opacity_green.getValue()), float(self.opacity_blue.getValue())))
			# self.material._opacity = (float(self.opacity_red.getValue()), float(self.opacity_green.getValue()), float(self.opacity_blue.getValue()))
	
	def renderPreview(self, button):
		# here, render a preview image using the current material
		# step 1 is to find a place to put this, so consult the BtoR settings to see where that is.
		filename = os.path.normpath(self.settings.outputPath + os.sep + "material_" + self.material.name + ".tif")
		if self.settings.renderer == "BMRT":
			filename = filename.replace("\\", "\\\\")
		# print "Selected renderer: ", self.settings.renderers[self.settings.renderer][0]
		ri.RiBegin(self.settings.renderers[self.settings.renderer][0]) # replace this with render flags		
		# ri.RiBegin(os.path.normpath(self.settings.outputPath + os.sep + "material_" + self.material.name + ".rib"))
		# get the shader options
		paths = self.settings.getShaderSearchPaths()
		for path in paths:
			if self.settings.renderer == "BMRT":
				ri.RiOption("searchpath", "shader", os.path.normpath(path).replace("\\", "\\\\") + ":&")
				
			else:
				ri.RiOption("searchpath", "shader", os.path.normpath(path) + ":&")
		print "Using render command: ", self.settings.renderers[self.settings.renderer][0]
		ri.RiFormat(128, 128, 1)
		# ri.RiProjection("perspective", "fov", 22.5)
		ri.RiAttribute("displacementbound", "coordinatesystem", "shader", "sphere", 1.5)
		ri.RiDisplay(filename, "file", "rgb")
		ri.RiTranslate(0, 0, 2.7)
		ri.RiWorldBegin()
		# ri.RiRotate(45, 1, 1, 0)
		ri.RiAttributeBegin()
		ri.RiCoordinateSystem("world")
		ri.RiAttributeEnd()
		self.renderLights()
		ri.RiAttributeBegin() 
		# ok here I need to get the material settings out of the material object somehow.
		# check the value of the surface shader
		surfShader = self.getProperty("Surface").getObject()
		surface = surfShader.shader				
		if surface.shadername != None:				
			print "Surface Shader parms:", surface.shaderparams
			surfShader.updateShaderParams()
			parms = dict()
			for parm in surface.shaderparams:
				# get the param and stuff into my dict				
				parms[parm] = getattr(surface, parm)
				ri.RiDeclare(parm, surface.shaderparams[parm])
			# now	
			ri.RiSurface(surface.shadername, parms)
		dispShader = self.getProperty("Displacement").getObject()
		displacement = dispShader.shader
		if displacement.shadername != None:				
			dispShader.updateShaderParams()
			parms = dict()
			for parm in displacement.shaderparams:
				parms[parm] = getattr(displacement, parm)
				ri.RiDeclare(parm, displacement.shaderparams[parm])
				
			ri.RiDisplacement(displacement.shadername, parms)
		volShader = self.getProperty("Volume").getObject()
		volume = volShader.shader 
		if volume.shadername != None:			
			parms = dict()
			volShader.updateShaderParams() # this is the BtoRShader instance remember!
			for parm in volume.shaderparams:
				parms[parm] = getattr(volume, parm)
				ri.RiDeclare(parm,volume.shaderparams[parm])				
			ri.RiAtmosphere(volume.shadername, parms)
		color = self.getProperty("color")
		opacity = self.getProperty("opacity")
		ri.RiTransformBegin()
		ri.RiScale(1, 1, 1)
		ri.RiColor([color[0], color[1], color[2]])		
		ri.RiOpacity([opacity[0], opacity[1], opacity[2]])

		previewType = self.previewTypeMenu.getValue()
		if previewType == "Bicubic Sphere":
			self.renderBicubicSphere()
		elif previewType == "Cube":
			self.renderUnitCube()
		elif previewType == "Suzanne - Mesh":
			self.renderSuzanneMesh()
		elif previewType == "Suzanne - Subdiv":
			self.renderSuzanneSubdiv()
		elif previewType == "Teapot":
			self.renderTeapot()
		else:
			self.renderBicubicSphere()
		ri.RiTransformEnd()
		ri.RiAttributeEnd()
		# self.renderBackground()
		ri.RiWorldEnd()
		ri.RiEnd()
		
		# here I want to load the image I just rendered into the material and draw it.		
		self.image.setFilename(filename)
		self.image.reload()
		self.image.glLoad()
		self.editorButton.image.bindImage()
		self.selectorButton.image.bindImage()
		
	def renderBicubicSphere(self):
		ri.RiTransformBegin()
		ri.RiRotate(90, 1, 0, 0)
		ri.RiSphere(0.75, -0.75, 0.75, 360)
		ri.RiTransformEnd()
		
	def renderSuzanneMesh(self):
		self.scene.suzanne.isSubdiv = False
		ri.RiRotate(180, 0, 1, 0)
		ri.RiRotate(27, 1, 0, 0)
		ri.RiRotate(-30, 0, 1, 0)
		ri.RiScale(.74, .74, .74)
		self.scene.suzanne.render()
		
	def renderSuzanneSubdiv(self):
		ri.RiRotate(180, 0, 1, 0)
		ri.RiRotate(27, 1, 0, 0)
		ri.RiRotate(-30, 0, 1, 0)
		ri.RiScale(.74, .74, .74)
		self.scene.suzanne.isSubdiv = True
		self.scene.suzanne.render()
	
	def renderTeapot(self):
		ri.RiTransformBegin()
		ri.RiTranslate(0, -.25, .5)
		ri.RiRotate(180, 0, 0, 1)
		ri.RiRotate(125, 1, 0, 0)
		ri.RiRotate(-45, 0, 0, 1)
		#ri.RiRotate(45, 0, 0, 1)
		ri.RiScale(0.35, 0.35, 0.35)
		teapotRIB = """
		AttributeBegin
		Basis "bezier" 3 "bezier" 3
		PatchMesh "bicubic" 13 "nonperiodic" 10 "nonperiodic" "P" [1.5 0 0 1.5 0.828427 0 0.828427 1.5 0 0 1.5 0 -0.828427 1.5 0 -1.5 0.828427 0 -1.5 0 0 -1.5 -0.828427 0 -0.828427 -1.5 0 0 -1.5 0 0.828427 -1.5 0 1.5 -0.828427 0 1.5 0 0 1.5 0 0.075 1.5 0.828427 0.075 0.828427 1.5 0.075 0 1.5 0.075 -0.828427 1.5 0.075 -1.5 0.828427 0.075 -1.5 0 0.075 -1.5 -0.828427 0.075 -0.828427 -1.5 0.075 0 -1.5 0.075 0.828427 -1.5 0.075 1.5 -0.828427 0.075 1.5 0 0.075 2 0 0.3 2 1.10457 0.3 1.10457 2 0.3 0 2 0.3 -1.10457 2 0.3 -2 1.10457 0.3 -2 0 0.3 -2 -1.10457 0.3 -1.10457 -2 0.3 0 -2 0.3 1.10457 -2 0.3 2 -1.10457 0.3 2 0 0.3 2 0 0.75 2 1.10457 0.75 1.10457 2 0.75 0 2 0.75 -1.10457 2 0.75 -2 1.10457 0.75 -2 0 0.75 -2 -1.10457 0.75 -1.10457 -2 0.75 0 -2 0.75 1.10457 -2 0.75 2 -1.10457 0.75 2 0 0.75 2 0 1.2 2 1.10457 1.2 1.10457 2 1.2 0 2 1.2 -1.10457 2 1.2 -2 1.10457 1.2 -2 0 1.2 -2 -1.10457 1.2 -1.10457 -2 1.2 0 -2 1.2 1.10457 -2 1.2 2 -1.10457 1.2 2 0 1.2 1.75 0 1.725 1.75 0.966498 1.725 0.966498 1.75 1.725 0 1.75 1.725 -0.966498 1.75 1.725 -1.75 0.966498 1.725 -1.75 0 1.725 -1.75 -0.966498 1.725 -0.966498 -1.75 1.725 0 -1.75 1.725 0.966498 -1.75 1.725 1.75 -0.966498 1.725 1.75 0 1.725 1.5 0 2.25 1.5 0.828427 2.25 0.828427 1.5 2.25 0 1.5 2.25 -0.828427 1.5 2.25 -1.5 0.828427 2.25 -1.5 0 2.25 -1.5 -0.828427 2.25 -0.828427 -1.5 2.25 0 -1.5 2.25 0.828427 -1.5 2.25 1.5 -0.828427 2.25 1.5 0 2.25 1.4375 0 2.38125 1.4375 0.793909 2.38125 0.793909 1.4375 2.38125 0 1.4375 2.38125 -0.793909 1.4375 2.38125 -1.4375 0.793909 2.38125 -1.4375 0 2.38125 -1.4375 -0.793909 2.38125 -0.793909 -1.4375 2.38125 0 -1.4375 2.38125 0.793909 -1.4375 2.38125 1.4375 -0.793909 2.38125 1.4375 0 2.38125 1.3375 0 2.38125 1.3375 0.738681 2.38125 0.738681 1.3375 2.38125 0 1.3375 2.38125 -0.738681 1.3375 2.38125 -1.3375 0.738681 2.38125 -1.3375 0 2.38125 -1.3375 -0.738681 2.38125 -0.738681 -1.3375 2.38125 0 -1.3375 2.38125 0.738681 -1.3375 2.38125 1.3375 -0.738681 2.38125 1.3375 0 2.38125 1.4 0 2.25 1.4 0.773198 2.25 0.773198 1.4 2.25 0 1.4 2.25 -0.773198 1.4 2.25 -1.4 0.773198 2.25 -1.4 0 2.25 -1.4 -0.773198 2.25 -0.773198 -1.4 2.25 0 -1.4 2.25 0.773198 -1.4 2.25 1.4 -0.773198 2.25 1.4 0 2.25 ]
		Basis "bezier" 3 "bezier" 3
		PatchMesh "bicubic" 13 "nonperiodic" 7 "nonperiodic" "P" [1.3 0 2.25 1.3 0.71797 2.25 0.71797 1.3 2.25 0 1.3 2.25 -0.71797 1.3 2.25 -1.3 0.71797 2.25 -1.3 0 2.25 -1.3 -0.71797 2.25 -0.71797 -1.3 2.25 0 -1.3 2.25 0.71797 -1.3 2.25 1.3 -0.71797 2.25 1.3 0 2.25 1.3 0 2.4 1.3 0.71797 2.4 0.71797 1.3 2.4 0 1.3 2.4 -0.71797 1.3 2.4 -1.3 0.71797 2.4 -1.3 0 2.4 -1.3 -0.71797 2.4 -0.71797 -1.3 2.4 0 -1.3 2.4 0.71797 -1.3 2.4 1.3 -0.71797 2.4 1.3 0 2.4 0.4 0 2.4 0.4 0.220914 2.4 0.220914 0.4 2.4 0 0.4 2.4 -0.220914 0.4 2.4 -0.4 0.220914 2.4 -0.4 0 2.4 -0.4 -0.220914 2.4 -0.220914 -0.4 2.4 0 -0.4 2.4 0.220914 -0.4 2.4 0.4 -0.220914 2.4 0.4 0 2.4 0.2 0 2.55 0.2 0.110457 2.55 0.110457 0.2 2.55 0 0.2 2.55 -0.110457 0.2 2.55 -0.2 0.110457 2.55 -0.2 0 2.55 -0.2 -0.110457 2.55 -0.110457 -0.2 2.55 0 -0.2 2.55 0.110457 -0.2 2.55 0.2 -0.110457 2.55 0.2 0 2.55 0 0 2.7 0 0 2.7 0 0 2.7 0 0 2.7 0 0 2.7 0 0 2.7 0 0 2.7 0 0 2.7 0 0 2.7 0 0 2.7 0 0 2.7 0 0 2.7 0 0 2.7 0.8 0 3 0.8 0.441828 3 0.441828 0.8 3 0 0.8 3 -0.441828 0.8 3 -0.8 0.441828 3 -0.8 0 3 -0.8 -0.441828 3 -0.441828 -0.8 3 0 -0.8 3 0.441828 -0.8 3 0.8 -0.441828 3 0.8 0 3 0 0 3 0 0 3 0 0 3 0 0 3 0 0 3 0 0 3 0 0 3 0 0 3 0 0 3 0 0 3 0 0 3 0 0 3 0 0 3 ]
		Basis "bezier" 3 "bezier" 3
		PatchMesh "bicubic" 4 "nonperiodic" 7 "nonperiodic" "P" [-2 0 0.75 -2 0.3 0.75 -1.9 0.3 0.45 -1.9 0 0.45 -2.5 0 0.975 -2.5 0.3 0.975 -2.65 0.3 0.7875 -2.65 0 0.7875 -2.7 0 1.425 -2.7 0.3 1.425 -3 0.3 1.2 -3 0 1.2 -2.7 0 1.65 -2.7 0.3 1.65 -3 0.3 1.65 -3 0 1.65 -2.7 0 1.875 -2.7 0.3 1.875 -3 0.3 2.1 -3 0 2.1 -2.3 0 1.875 -2.3 0.3 1.875 -2.5 0.3 2.1 -2.5 0 2.1 -1.6 0 1.875 -1.6 0.3 1.875 -1.5 0.3 2.1 -1.5 0 2.1 ]
		PatchMesh "bicubic" 4 "nonperiodic" 7 "nonperiodic" "P" [2.8 0 2.25 2.8 0.15 2.25 3.2 0.15 2.25 3.2 0 2.25 2.9 0 2.325 2.9 0.25 2.325 3.45 0.15 2.3625 3.45 0 2.3625 2.8 0 2.325 2.8 0.25 2.325 3.525 0.25 2.34375 3.525 0 2.34375 2.7 0 2.25 2.7 0.25 2.25 3.3 0.25 2.25 3.3 0 2.25 2.3 0 1.95 2.3 0.25 1.95 2.4 0.25 1.875 2.4 0 1.875 2.6 0 1.275 2.6 0.66 1.275 3.1 0.66 0.675 3.1 0 0.675 1.7 0 1.275 1.7 0.66 1.275 1.7 0.66 0.45 1.7 0 0.45 ]
		PatchMesh "bicubic" 4 "nonperiodic" 7 "nonperiodic" "P" [-1.9 0 0.45 -1.9 -0.3 0.45 -2 -0.3 0.75 -2 0 0.75 -2.65 0 0.7875 -2.65 -0.3 0.7875 -2.5 -0.3 0.975 -2.5 0 0.975 -3 0 1.2 -3 -0.3 1.2 -2.7 -0.3 1.425 -2.7 0 1.425 -3 0 1.65 -3 -0.3 1.65 -2.7 -0.3 1.65 -2.7 0 1.65 -3 0 2.1 -3 -0.3 2.1 -2.7 -0.3 1.875 -2.7 0 1.875 -2.5 0 2.1 -2.5 -0.3 2.1 -2.3 -0.3 1.875 -2.3 0 1.875 -1.5 0 2.1 -1.5 -0.3 2.1 -1.6 -0.3 1.875 -1.6 0 1.875 ]
		PatchMesh "bicubic" 4 "nonperiodic" 7 "nonperiodic" "P" [3.2 0 2.25 3.2 -0.15 2.25 2.8 -0.15 2.25 2.8 0 2.25 3.45 0 2.3625 3.45 -0.15 2.3625 2.9 -0.25 2.325 2.9 0 2.325 3.525 0 2.34375 3.525 -0.25 2.34375 2.8 -0.25 2.325 2.8 0 2.325 3.3 0 2.25 3.3 -0.25 2.25 2.7 -0.25 2.25 2.7 0 2.25 2.4 0 1.875 2.4 -0.25 1.875 2.3 -0.25 1.95 2.3 0 1.95 3.1 0 0.675 3.1 -0.66 0.675 2.6 -0.66 1.275 2.6 0 1.275 1.7 0 0.45 1.7 -0.66 0.45 1.7 -0.66 1.275 1.7 0 1.275 ]
		AttributeEnd """
		ri._ribout.write(teapotRIB)
		ri.RiTransformEnd()

		
	def renderUnitCube(self):	
		ri.RiRotate(63, 1, 0, 0)
		ri.RiRotate(45, 0, 0, 1)
		
		#ri.RiScale(.70, .70, .70)
		# Far
		unitsize = .65	
		ri.RiTransformBegin()
		ri.RiPolygon("P", [unitsize, unitsize, unitsize, -unitsize, unitsize, unitsize, -unitsize, -unitsize, unitsize, unitsize, -unitsize, unitsize])
		ri.RiRotate(90, 0, 1, 0)
		# right 
		ri.RiPolygon("P", [unitsize, unitsize, unitsize, -unitsize, unitsize, unitsize, -unitsize, -unitsize, unitsize, unitsize, -unitsize, unitsize])
		ri.RiRotate(90, 0, 1, 0)
		# near
		ri.RiPolygon("P", [unitsize, unitsize, unitsize, -unitsize, unitsize, unitsize, -unitsize, -unitsize, unitsize, unitsize, -unitsize, unitsize])
		ri.RiRotate(90, 0, 1, 0)
		# left
		ri.RiPolygon("P", [unitsize, unitsize, unitsize, -unitsize, unitsize, unitsize, -unitsize, -unitsize, unitsize, unitsize, -unitsize, unitsize])
		ri.RiTransformEnd()
		ri.RiTransformBegin()
		ri.RiRotate(90, 1, 0, 0)
		# bottom
		ri.RiPolygon("P", [unitsize, unitsize, unitsize, -unitsize, unitsize, unitsize, -unitsize, -unitsize, unitsize, unitsize, -unitsize, unitsize])
		ri.RiTransformEnd()
		ri.RiTransformBegin()
		ri.RiRotate(90, 1, 0, 0)
		# top
		ri.RiPolygon("P", [unitsize, unitsize, unitsize, -unitsize, unitsize, unitsize, -unitsize, -unitsize, unitsize, unitsize, -unitsize, unitsize])
		ri.RiTransformEnd()
	
	def renderLights(self):
		ri.RiTransformBegin()
		# case checking needed here for lighting information
		ri.RiLightSource("ambientlight", "lightcolor", [0.151, 0.151, 0.151])
		ri.RiLightSource("distantlight", "lightcolor", [1, 1, 1], "from", [1, 1.5, -1], "to", [0, 0, 0], "intensity", 1)
		ri.RiLightSource("distantlight", "lightcolor", [0.7, 0.7, 0.7], "from", [-1.3, -1.2, -1.0], "to", [0, 0, 0], "intensity", 1)
		# ri.RiLightSource("distantlight", "lightcolor", [1, 1, 1], "from", [-1, -1.5, -1], "to", [0, 0, 0], "intensity", 1)
		# ri.RiLightSource("distantlight", "lightcolor", [1, 1, 1], "from", [-1, 1.5, -1], "to", [0, 0, 0], "intensity", 1)
		ri.RiTransformEnd()
		
	def renderBackground(self):
		ri.RiAttributeBegin()
		ri.RiTranslate(0, 0, 0.5)
		ri.RiRotate(90, 0, 1, 1)

		ri.RiColor([1, 1, 1])		
		ri.RiDeclare("frequency", "uniform float")
		ri.RiSurface("checker", "frequency", 1)
		ri.RiTransformBegin()
		# Far
		unitsize = 1.5
		ri.RiRotate(45, 1, 1, 1)
		ri.RiPolygon("P", [unitsize, unitsize, unitsize, -unitsize, unitsize, unitsize, -unitsize, -unitsize, unitsize, unitsize, -unitsize, unitsize])
		ri.RiRotate(90, 0, 1, 0)
		# right 
		ri.RiPolygon("P", [unitsize, unitsize, unitsize, -unitsize, unitsize, unitsize, -unitsize, -unitsize, unitsize, unitsize, -unitsize, unitsize])
		ri.RiRotate(90, 0, 1, 0)
		# near
		ri.RiPolygon("P", [unitsize, unitsize, unitsize, -unitsize, unitsize, unitsize, -unitsize, -unitsize, unitsize, unitsize, -unitsize, unitsize])
		ri.RiRotate(90, 0, 1, 0)
		# left
		ri.RiPolygon("P", [unitsize, unitsize, unitsize, -unitsize, unitsize, unitsize, -unitsize, -unitsize, unitsize, unitsize, -unitsize, unitsize])
		ri.RiTransformEnd()
		ri.RiTransformBegin()
		ri.RiRotate(90, 1, 0, 0)
		# bottom
		ri.RiPolygon("P", [unitsize, unitsize, unitsize, -unitsize, unitsize, unitsize, -unitsize, -unitsize, unitsize, unitsize, -unitsize, unitsize])
		ri.RiTransformEnd()
		ri.RiTransformBegin()
		ri.RiRotate(90, 1, 0, 0)
		# top
		ri.RiPolygon("P", [unitsize, unitsize, unitsize, -unitsize, unitsize, unitsize, -unitsize, -unitsize, unitsize, unitsize, -unitsize, unitsize])
		# ri.RiPatch("bilinear", "P", [-2, 2, 0, 2, 2, 0, -2, -2, 0, 2 -2, 0])
		ri.RiTransformEnd()
		ri.RiAttributeEnd()
	
class Shader(BtoRObject):
	# what does this need to know?
	# the shader path for one thing...and the shader type...and the 
	# current shader itself
	# the shader, as an RMShader object doesn't know its own type probably, so I have to supply it
	# this needs to at some point construct the set of parameters that belongs to the selected shader.
	def __init__(self, shader, stype, material):
		
		sdict = globals()		
		self.settings = sdict["instBtoRSettings"]
		self.evt_manager = sdict["instBtoREvtManager"]
		self.log = sdict["instBtoRLog"]
		
		if shader <> None:
			self.shader = shader
		else:
			self.shader = None # no selection, we'll be assigning it as we go.
		
		self.searchpaths = self.settings.getShaderSearchPaths()
		self.stype = stype
		self.material = material # parent material
		
		# initialize the panel
		# find the center of the screen...again
		screen = Blender.Window.GetAreaSize()
		
		self.editorPanel = ui.Panel(410, 0, 300, 400, "ShaderEditor", "  " + stype.capitalize() + "Shader Settings:", material.getEditor(), False)
		self.editorPanel.outlined = True
		#self.editorPanel.hasFooter = True
		#self.editorPanel.y_resize = True

		self.editorPanel.addElement(ui.Label(5, 27, "SearchPathLabel", "Shader Search Path:", self.editorPanel, True))		
		self.searchPaths = ui.Menu(5, 50, 210, 20, "SearchPath",  self.searchpaths, self.editorPanel, True)
		self.searchPaths.registerCallback("release", self.listShaders)
		
		# next we need the shader type, so I know what I'm working with.
		
		self.editorPanel.addElement(ui.Label(5, 75, "Shader", "Shader:", self.editorPanel, False))
		
		# the starting shader type is Surface
		self.shaders = []
		self.makeShaderMenu()
		
		self.shader_menu = ui.Menu(self.editorPanel.get_string_width("Shader:", 'normal') + 12, 75, 150, 20, "Shaders", self.shadersMenu, self.editorPanel, True)
		self.shader_menu.registerCallback("release", self.selectShader)
		# now that THAT nastiness is done...
		self.scroller = ui.ScrollPane(5, 115, self.editorPanel.width - 10, self.editorPanel.height - 125, "Param Scroller", "parmScroller", self.editorPanel, True)
		#self.scroller.normalColor = [45, 45, 45, 255]
		
		#self.close_button = ui.Button(self.editorPanel.width - 20, 5, 14, 14, "X", "X", 'normal', self.editorPanel, True)
		#self.close_button.shadowed = False
		#self.close_button.cornermask = 15
		#self.close_button.radius = 1.5
		#close_func = self.close_button.callFactory(self.evt_manager.removeElement, self.editorPanel)
		#self.close_button.registerCallback("release", self.close)
		
		if self.shader.shaderparams != None:
			self.setupParamProperties()
			# self.setupParamEditors()
		
		if self.shader.shaderName() != None:
			# set the shader name to the correct shader name in the menu
			# first, discover the path in use
			path = os.path.split(self.shader.filename)[0]
			# if the path isn't the same as the selected path, generate the shader list for it and set everything up
			spath = self.searchPaths.getValue()
			spath = os.path.normpath(spath)
			path = os.path.normpath(path)
			
			if path != spath:
				found = False
				for apath in self.searchpaths:					
					if path == os.path.normpath(apath):
						pIndex = self.searchpaths.index(apath)
						found = True
						break
				if found:
					self.searchPaths.setValue(pIndex)
					# print "Setting search path"
				else:
					# print "Appending search path: ", path
					self.searchpaths.append(path)
					self.searchPaths.re_init(self.searchpaths)
					self.searchPaths.setValue(self.searchpaths.index(path))
				self.listShaders(None)
			sIndex = self.shadersMenu.index(self.shader.shaderName())
			self.shader_menu.setValue(sIndex)
		#else:
		#	self.selectShader(None)
		# otherwise, I'll be selecting a shader at the outset...
		self.value_tables = {} # vts are referenced per shader parameter
		self.hasValueTable = False
		self.update_functions = []
		 

	def showEditor(self):
		self.material.showShader(self.stype)
		
	def getStrValue(self):
		return self.getShaderName()
		
	def getValueTable(self, parameter):
		return value_tables[parameter]
		
	def listShaders(self, obj):
		print "listing shaders!"
		if self.searchPaths.getValue() != "":
			# test here to see if I've got the searchpath key
			if not self.settings.shadersSurface.has_key(self.searchPaths.getValue().strip()):
				self.settings.getShaderList(self.searchPaths.getValue())
			# reset the shader list menu
			self.shaders = []
			self.makeShaderMenu()
			self.shader_menu.re_init(self.shadersMenu)
		
	def getShaderName(self):
		if self.shader == None:
			return "None Selected"
		elif self.shader.shadername == None:
			return "None Selected"
		else:
			return self.shader.shadername
	
	def getShaderFilename(self):
		return self.shader.shaderfilename
		
		
	def close(self, button):		
		self.evt_manager.removeElement(self.editorPanel)
		self.material.renderPreview(None)
		
	def getEditor(self):		
		return self.editorPanel
	
	#def showEditor(self, button):
	#	self.evt_manager.addElement(self.editorPanel)
		
	def makeShaderMenu(self):
		# get the current shader path
		print "Fetching shader data for ", self.stype, " shaders on path ", self.searchPaths.getValue()
		path = self.searchPaths.getValue()

		self.shadersMenu = []
		self.shadersMenu.append("None Selected")
		
		if self.stype == "surface":
			if not self.settings.shadersSurface.has_key(path):
				found = False
			elif len(self.settings.shadersSurface[path]) < 1:
				found = False
			else:
				self.shaders = self.settings.shadersSurface[path]
				for shader in self.settings.shadersSurface[path]:
					self.shadersMenu.append(shader[1])
				found = True
		elif self.stype == "displacement":
			if not self.settings.shadersDisplacement.has_key(path):
				found = False
			elif len(self.settings.shadersDisplacement[path]) < 1:
				found = False
			else:
				self.shaders = self.settings.shadersDisplacement[path]
				for shader in self.settings.shadersDisplacement[path]:				
					self.shadersMenu.append(shader[1])
				found = True
		elif self.stype == "volume":
			if not self.settings.shadersVolume.has_key(path):
				found = False
			elif len(self.settings.shadersVolume[path]) < 1:
				found = False
			else:
				self.shaders = self.settings.shadersVolume[path]
				for shader in self.settings.shadersVolume[path]:
					self.shadersMenu.append(shader[1])
				found = True
		if not found:
			self.log.appendText("No " + self.stype + " shaders were found on the selected shader path")
			#self.evt_manager.showConfirmDialog("No " + self.stype + " shaders found!", "There were no " + self.stype + "shaders found on the selected shader path!", None, False)

				
	def selectShader(self, button):
		# select the shader in question and then setup the params for it.
		# ditch the current parameter editors	
		path = self.searchPaths.getValue()
		self.scroller.clearElements()	
		self.shader = None	# clear all the old data.
		if self.shader_menu.getSelectedIndex() > 0:			
			shaderID = self.shader_menu.getSelectedIndex() - 1
			if self.stype == "surface":
				self.shaderParms = self.settings.shadersSurface[path][shaderID]
			elif self.stype == "displacement":
				self.shaderParms = self.settings.shadersDisplacement[path][shaderID]
			elif self.stype == "volume":
				self.shaderParms = self.settings.shadersVolume[path][shaderID]
			elif self.stype == "light":
				self.shaderParms = self.settings.shadersLight[path][shaderID]
			elif self.stype == "imager":
				self.shaderParms = self.settings.shadersImager[path][shaderID]				
				
			# initialize a new RMShader object
			if self.settings.use_slparams: 
				if self.stype == "surface":
					file = self.settings.surfaceFiles[path][shaderID]
				elif self.stype == "displacement":				
					file = self.settings.dispFiles[path][shaderID]
				elif self.stype == "volume":
					file = self.settings.volumeFiles[path][shaderID]
				elif self.stype == "light":
					file = self.settings.lightFiles[path][shaderID]
				elif self.stype == "imager":
					file = self.settings.imagerFiles[path][shaderID]
					
				# get the search path from the settings object, and use that to find the shader source
				# hopefully it's the same as the shader name
				path = self.settings.shaderPathList
				self.shader = cgkit.rmshader.RMShader(self.searchPaths.getValue() + os.sep + file)
				self.setupParamProperties()
				# self.setupParamEditors()
			else:
				self.shader = cgkit.rmshader.RMShader()
				self.initCompiledShaderParams(self.shaderParms, self.shader)				
				# self.setupParamEditors()
				self.setupParamProperties()
		
		if self.shader == None:
			self.shader = cgkit.rmshader.RMShader() # blank shader
				
		if self.material != None:
			if self.stype == "surface":				
				self.material.material.surface = self.shader
			elif self.stype == "volume":				
				self.material.material.interior = self.shader
			elif self.stype == "displacement":
				self.material.material.displacement = self.shader
		
		for func in self.update_functions:
			print "Running function!"
			func(self) # pass myself back for querying purposes.
			
	def registerCallback(self, event, callback):
		# blackmagic and v00d00
		# At some point, rewrite this a bit to be more interface driven so the object in question has certain signals per interface
		self.__dict__[event + "_functions"].append(callback)
	
	def removeCallbacks(self, event):
		self.__dict__[event + "_functions"] = []
		
	def setupProperties(self):
		self.properties = {}		
		self.editors = {}
		for option in self.optionOrder:
			propertyName = self.options[option][0]
			propertyValue = self.options[option][1]
			
			# generate a list of option panels here and allow editing
			# create a property for each option
			self.properties[option] = propertyValue # 1st item is the property name, second item is the property initializer
			self.properties[option].setName(propertyName)
			self.properties[option].setWidth(self.scroller.width - 15)			
			
			# takes up half the available space of the main pane
			self.editors[option] = BtoRAdapterClasses.IPropertyEditor(propertyValue)
			self.scroller.addElement(self.editors[option].getEditor()) # and that should be that. When this is discarded, all those go away
			self.editors[option].setParent(self.scroller)	
			self.scroller.offset = 0
		
	def setupParamProperties(self):
		# iterate the current shader and setup the parameter editors for it.
		# here I'm using the rmshader object so react accordingly
		# iterate the slots in the RMShader object
		# delete the current set of elements in the scroller object	
		self.options = {}
		self.optionOrder = []
		errors = []
		count = 0
		for param in self.shader.shaderparams:
			# test here for an array value
			normalColor = [216, 214, 220, 255]
			#if count % 2:
			#	normalColor = [190, 190, 190, 255]			
			p_type = self.shader.shaderparams[param].split()[1]
			
			if "[" in p_type:
				# this is an array value, react accordingly
				# perhaps I can simply look at the _slot in the shader and decide from there?
				arrayLen = self.shader.__dict__[param + "_slot"].size() # this should be there if everything was initialized correctly!
				# and the type name
				idx = p_type.index("[")
				isArray = True
				p_type = p_type[:idx]
				# if of type array, setup 
			else:
				isArray = False
			# get the default value from the shader			
			defValue = getattr(self.shader, param)
			size = [0, 0, self.scroller.width - 30, 0]
			
			try:
				if isArray:
					bParm = BtoRTypes.__dict__["BtoRArrayParam"](param = param, name = param, p_type = p_type, value = defValue, size=size, parent = self.scroller) # just init a basic type
				else:					
					bParm = BtoRTypes.__dict__["BtoR" + p_type.capitalize() + "Param"](param = param, p_type = p_type, name = param, value = defValue, size=size, parent = self.scroller) # just init a basic type
				self.options[param] = [param, bParm] # so properties instantiantion works! Whee!
				self.optionOrder.append(param)
				
				# so the two arrays will make sure that my properties are correctly instantiated
				# I really need to make sure that I configure a baseClass for properties instantiation
				
				#editor = BtoRAdapterClasses.IShaderParamEditor(bParm) # that should do the trick
				#self.scroller.addElement(editor)
			except:
				traceback.print_exc()
				errors.append(param)
			count = count + 1
		#self.scroller.invalid = True
		#self.scroller.currentItem = 1
		self.setupProperties()
		if len(errors) > 0:
			self.evt_manager.showErrorDialog("Shader Parameter Error", "%d shader parameter(s) could not construct an editor." % len(errors))
		# set the scrollers up
		

	def setupParamEditorsX(self):
		# iterate the current shader and setup the parameter editors for it.
		# here I'm using the rmshader object so react accordingly
		# iterate the slots in the RMShader object
		# delete the current set of elements in the scroller object		
		errors = []
		count = 0
		for param in self.shader.shaderparams:
			# test here for an array value
			normalColor = [216, 214, 220, 255]
			#if count % 2:
			#	normalColor = [190, 190, 190, 255]			
			p_type = self.shader.shaderparams[param].split()[1]
			
			if "[" in p_type:
				# this is an array value, react accordingly
				# perhaps I can simply look at the _slot in the shader and decide from there?
				arrayLen = self.shader.__dict__[param + "_slot"].size() # this should be there if everything was initialized correctly!
				# and the type name
				idx = p_type.index("[")
				isArray = True
				p_type = p_type[:idx]
				# if of type array, setup 
			else:
				isArray = False
			# get the default value from the shader
			iParams = self.shader.params()			
			defValue = getattr(self.shader, param)
			size = [0, 0, self.scroller.width - 30, 0]
			try:
				if isArray:
					bParm = BtoRTypes.__dict__["BtoRArrayParam"](param = param, name = param, value = defValue, size=size, parent = self.scroller) # just init a basic type
				else:
					bParm = BtoRTypes.__dict__["BtoR" + p_type.capitalize() + "Param"](param = param, name = param, value = defValue, size=size, parent = self.scroller) # just init a basic type
				editor = BtoRAdapterClasses.IShaderParamEditor(bParm) # that should do the trick
				self.scroller.addElement(editor)
			except:
				traceback.print_exc()
				errors.append(param)
			count = count + 1
		self.scroller.invalid = True
		self.scroller.currentItem = 1
		if len(errors) > 0:
			self.evt_manager.showErrorDialog("Shader Parameter Error", "%d shader parameter(s) could not construct an editor." % len(errors))
	
	def declareParams(self):
		for parm in self.shaderparams:
			ri.RiDeclare(parm[0], parm[1])
			
	def initCompiledShaderParams(self, params, shader):
		convtypes = {"float":"double",
				"string":"string",
				"color":"vec3",
				"point":"vec3",
				"vector":"vec3",
				"normal":"vec3",
				"matrix":"mat4"}
		
		paramList = params[2]
		shaderName = os.path.splitext(params[1])[0]
		shader.shadername = shaderName
		# get the current shader path		
		shader.filename = self.searchPaths.getValue() + os.sep + params[1]
		for param in paramList:
			if param[2] == "color" or param[2] == "point" or param[2] == "normal" or param[2] == "vector":
				defVal = (cgkit.cgtypes.vec3(param[6][0], param[6][1], param[6][2]))
			else:
				defVal = param[6]			
			if param[3] != None:
				shader.declare("%s %s[%d] %s" %(param[1], param[2], param[3], param[4]))
			else:
				shader.declare("%s %s %s" %(param[1], param[2], param[4]))
			if defVal != None and defVal != "None":
				setattr(shader, param[4], defVal)
				
	def initShaderParams(self, params, shader):		
		convtypes = {"float":"double",
				"string":"string",
				"color":"vec3",
				"point":"vec3",
				"vector":"vec3",
				"normal":"vec3",
				"matrix":"mat4"}
		for param in params:
			atts = param.attributes
			switch = atts["type"].nodeValue
			if switch == "color":
				val = cgkit.cgtypes.vec3(float(atts["value_a"].nodeValue), float(atts["value_b"].nodeValue), float(atts["value_c"].nodeValue))
				# no color space support and most blender people won't have a clue what this means anyway
			elif switch == "float":						
				val = cgkit.cgtypes.double(float(atts["value"].nodeValue))
			elif switch == "string":
				val = atts["value"].nodeValue
			elif switch == "matrix":
				val = cgkit.cgtypes.mat4(float(atts["value_0"].nodeValue), 
										float(atts["value_1"].nodeValue), 
										float(atts["value_2"].nodeValue), 
										float(atts["value_3"].nodeValue), 
										float(atts["value_4"].nodeValue), 
										float(atts["value_5"].nodeValue), 
										float(atts["value_6"].nodeValue), 
										float(atts["value_7"].nodeValue), 
										float(atts["value_8"].nodeValue), 
										float(atts["value_9"].nodeValue), 
										float(atts["value_10"].nodeValue), 
										float(atts["value_11"].nodeValue), 
										float(atts["value_12"].nodeValue), 
										float(atts["value_13"].nodeValue), 
										float(atts["value_14"].nodeValue), 
										float(atts["value_15"].nodeValue))				
			
			elif switch == "point" or switch == "vector" or switch == "normal":
				val = cgkit.cgtypes.vec3(float(atts["value_a"].nodeValue), float(atts["value_b"].nodeValue), float(atts["value_c"].nodeValue))
			# slot this attribute into the shader
			shader.declare(atts["name"], type=convtypes[switch], default = val)
			# shader.createSlot(atts["name"].nodeValue, convtypes[switch], None, val) # array  length is none for now, array behaviour is uncertain here		
	
	
	def updateShaderParamsX(self):
		# here I need to cobble all the parameter values together and push them into the shader itself
		# so the material can then be exported.
		index = 0
		for element in self.scroller.elements:
			p_type = element.paramtype
			name = element.param_name
			if p_type == "float" or p_type == "string": # lo, all my single-variable types
				setattr(self.shader, name, self.scroller.elements[index].getValue()) 
				
			elif p_type == "color" or p_type == "coordinate": # all vec3 types	
				val = self.scroller.elements[index].getValue()
				vec = cgkit.cgtypes.vec3(val[0], val[1], val[2])
				setattr(self.shader, name, vec)
				
			elif p_type == "matrix": # matrix types				
				val = self.scroller.elements[index].getValue()
				matrix = cgkit.cgtypes.mat4(val[0][0], val[0][1], val[0][2], val[0][3], val[1][0], val[1][1], val[1][2], val[1][3], val[2][0], val[2][1], val[2][2], val[2][3], val[3][0], val[3][1], val[3][2], val[3][3])				
				setattr(self.shader, name, matrix)
			
			index = index + 1
			
	def updateShaderParams(self):
		# here I need to cobble all the parameter values together and push them into the shader itself
		# so the material can then be exported.
		index = 0
		properties = self.properties
		for propName in properties:
			property = properties[propName]
			#	print "incoming value: ", propName, " ", property.getValue()
			p_type = property.getType()
			name =property.getName()
			#print p_type
			if p_type == "float":
				setattr(self.shader, propName, float(property.getValue()))
			elif p_type == "string": 
				setattr(self.shader, propName, property.getValue())
			elif p_type == "color" or p_type == "coordinate": # all vec3 types	
				val = property.getValue()
				vec = cgkit.cgtypes.vec3(val[0], val[1], val[2])
				setattr(self.shader, propName, vec)
				
			elif p_type == "matrix": # matrix types				
				val = property.getValue()
				matrix = cgkit.cgtypes.mat4(val[0][0], val[0][1], val[0][2], val[0][3], val[1][0], val[1][1], val[1][2], val[1][3], val[2][0], val[2][1], val[2][2], val[2][3], val[3][0], val[3][1], val[3][2], val[3][3])				
				setattr(self.shader, propName, matrix)
			# print "Set value: ", getattr(self.shader, name)
			index = index + 1

class GenericShader(BtoRObject):
	def __init__(self, shader, s_type, parent, passSelf = False):
		self.parent = parent
		sdict = globals()		
		self.settings = sdict["instBtoRSettings"]
		self.evt_manager = sdict["instBtoREvtManager"]
		self.log = sdict["instBtoRLog"]
				
		if shader != None:
			self.shader = shader
		else:
			self.shader = cgkit.rmshader.RMShader()
			
		searchpaths = self.settings.getShaderSearchPaths()
		self.s_type = s_type
		
		# initialize the panel
		# find the center of the screen...again
		screen = Blender.Window.GetAreaSize()
		
		self.editorPanel = ui.Panel(410, 0, 300, 400, "ShaderEditor", "  " + s_type.capitalize() + "Shader Settings:", None, False)
		
		# search path label
		self.editorPanel.addElement(ui.Label(5, 30, "SearchPathLabel", "Shader Search Path:", self.editorPanel, True))		
		
		self.searchPaths = ui.Menu(5, 50, 210, 20, "SearchPath",  searchpaths, self.editorPanel, True)
		self.searchPaths.registerCallback("release", self.listShaders)
		
		
		self.editorPanel.addElement(ui.Label(5, 80, "Shader", "Shader:", self.editorPanel, False))
		
		self.shaders = []
		self.makeShaderMenu()
				
		self.shader_menu = ui.Menu(self.editorPanel.get_string_width("Shader:", 'normal') + 12, 80, 150, 20, "Shaders", self.shadersMenu, self.editorPanel, True)
		self.shader_menu.registerCallback("release", self.selectShader)
		# now that THAT nastiness is done...
		self.scroller = ui.ScrollPane(5, 115, self.editorPanel.width - 10, self.editorPanel.height - 125, "Param Scroller", "parmScroller", self.editorPanel, True)
		#self.scroller.normalColor = [45, 45, 45, 255]
		
		self.close_button = ui.Button(self.editorPanel.width - 20, 5, 14, 14, "X", "X", 'normal', self.editorPanel, True)
		self.close_button.shadowed = False
		self.close_button.cornermask = 15
		self.close_button.radius = 1.5
		#close_func = self.close_button.callFactory(self.evt_manager.removeElement, self.editorPanel)
		self.close_button.registerCallback("release", self.close)
		
		if self.shader != None:
			if self.shader.shaderparams != None:
				self.setupParamProperties()
		
			if self.shader.shaderName() != None:
				# set the shader name to the correct shader name in the menu
				sIndex = self.shadersMenu.index(self.shader.shaderName())
				self.shader_menu.setValue(sIndex)
		else:
			self.shader = rmshader.RMShader()
			
		self.update_functions = []
		self.obj_parent = None 
		self.passSelf = passSelf
		#else:
		#	self.selectShader(None)
		# otherwise, I'll be selecting a shader at the outset...
	def registerCallback(self, event, callback):
		# blackmagic and v00d00
		# At some point, rewrite this a bit to be more interface driven so the object in question has certain signals per interface
		self.__dict__[event + "_functions"].append(callback)
	
	def removeCallbacks(self, event):
		self.__dict__[event + "_functions"] = []
		
	def listShaders(self, obj):
		self.settings.getShaderList(self.searchPaths.getValue())
		# reset the shader list menu
		self.shaders = []
		self.makeShaderMenu()
		self.shader_menu.re_init(self.shadersMenu)
		
	def getShaderName(self):
		if self.shader == None:
			return "None Selected"
		elif self.shader.shadername == None:
			return "None Selected"
		else:
			return self.shader.shadername
		
	def getShaderFilename(self):
		return self.shader.shaderfilename
	
	def setupProperties(self):
		self.properties = {}		
		self.editors = {}
		for option in self.optionOrder:
			propertyName = self.options[option][0]
			propertyValue = self.options[option][1]
			# generate a list of option panels here and allow editing
			# create a property for each option
			self.properties[option] = propertyValue # 1st item is the property name, second item is the property initializer
			self.properties[option].setName(propertyName)
			self.properties[option].setWidth(self.scroller.width - 15)			
			# takes up half the available space of the main pane
			self.editors[option] = BtoRAdapterClasses.IPropertyEditor(propertyValue)
			self.scroller.addElement(self.editors[option].getEditor()) # and that should be that. When this is discarded, all those go away
			self.editors[option].setParent(self.scroller)	
			self.scroller.offset = 0
			
	def setSearchPath(self, path):
		self.searchPaths.setValueString(path)
		
	def close(self, button):		
		self.updateShaderParams()
		self.evt_manager.removeElement(self.editorPanel)
		# no preview neccessar here
		
	def getEditor(self):		
		return self.editorPanel
	def showEditor(self):
		if self.passSelf:
			self.obj_parent.showShader(self)
		else:
			self.obj_parent.showShader()
		
	def setParamValue(self, parameter, value):
		if self.properties.has_key(parameter):
			self.properties[parameter].setValue(value)
			self.editors[parameter].setValue(value)		
		# set it in the shader too, for good measure, but...why do I need to this if I'm calling updateShaderparams()?
		# setattr(self.shader, parameter, value)

	def makeShaderMenu(self):
		self.shadersMenu = []
		path = self.searchPaths.getValue()
		self.shadersMenu.append("None Selected")
		found = False
		if self.s_type == "light":
			if not self.settings.shadersLight.has_key(path):
				found = False
				print "setting found to False at key test"
			elif len(self.settings.shadersLight[path]) < 1:
				found = False
				print "setting found to False at length test"
			else:
				found = True
				self.shaders = self.settings.shadersLight[path]
				for shader in self.settings.shadersLight[path]:
					self.shadersMenu.append(shader[1])
				
		elif self.s_type == "imager":
			if not self.settings.shadersImager.has_key(path):
				found = False
				print "setting found to False at key test"
			elif len(self.settings.shadersImager[path]) < 1:
				found = False
				print "setting found to False at length test"
			else:
				found = True
				self.shaders = self.settings.shadersImager[path]
				for shader in self.settings.shadersImager[path]:
					self.shadersMenu.append(shader[1])
		if not found:
			# find out what happened here
			self.log.appendText("No " + self.s_type + " shaders were found on the selected shader path")
			
			# self.evt_manager.showConfirmDialog("No " + self.s_type + " shaders found!", "There were no " + self.s_type + "shaders found on the selected shader path!", None, False)
	def setShader(self, shader):
		# doh!
		index = self.shader_menu.getValueIndex(shader) # I don't want to use setValueString here because that might result in an index that doesn't exist in the shader list
		if self.shader_menu.getValueIndex(shader) > -1:
			self.shader_menu.setValue(index)
			self.selectShader(None)
			
	def selectShader(self, button):
		# select the shader in question and then setup the params for it.
		# ditch the current parameter editors	
		path = self.searchPaths.getValue()
		self.scroller.clearElements()	
		self.shader = None	# clear all the old data.
		if self.shader_menu.getSelectedIndex() > 0:			
			shaderID = self.shader_menu.getSelectedIndex() - 1
			if self.s_type == "light":
				self.shaderParms = self.settings.shadersLight[path][shaderID]
			elif self.s_type == "imager":
				self.shaderParms = self.settings.shadersImager[path][shaderID]
			# initialize a new RMShader object
			if self.settings.use_slparams: 
				if self.s_type == "light":
					file = self.settings.lightFiles[path][shaderID]
				elif self.s_type == "imager":
					file = self.settings.imagerFiles[path][shaderID]
					
				# get the search path from the settings object, and use that to find the shader source
				# hopefully it's the same as the shader name
				self.shader = cgkit.rmshader.RMShader(self.searchPaths.getValue() + os.sep + file)
				self.setupParamProperties()
			else:
				self.shader = cgkit.rmshader.RMShader()
				self.initCompiledShaderParams(self.shaderParms, self.shader)				
				self.setupParamProperties()
		else:
			self.shader = cgkit.rmshader.RMShader()
			
		if self.shader == None:
			self.shader = cgkit.rmshader.RMShader() # blank shader
			
		#if self.parent != None: # this should be modified to support message passing instead of explicitly setting it.
		#	self.parent.getEditor().shaderButton.title = self.shader_menu.getValue()
		for func in self.update_functions:
			func(self) # pass myself back for querying purposes.
			
			
	def setupParamProperties(self):
		# iterate the current shader and setup the parameter editors for it.
		# here I'm using the rmshader object so react accordingly
		# iterate the slots in the RMShader object
		# delete the current set of elements in the scroller object	
		self.options = {}
		self.optionOrder = []
		errors = []
		count = 0
		print "Shader parameters are: ", self.shader.shaderparams
		for param in self.shader.shaderparams:
			# test here for an array value
			normalColor = [216, 214, 220, 255]
			#if count % 2:
			#	normalColor = [190, 190, 190, 255]			
			p_type = self.shader.shaderparams[param].split()[1]
			
			if "[" in p_type:
				# this is an array value, react accordingly
				# perhaps I can simply look at the _slot in the shader and decide from there?
				arrayLen = self.shader.__dict__[param + "_slot"].size() # this should be there if everything was initialized correctly!
				# and the type name
				idx = p_type.index("[")
				isArray = True
				p_type = p_type[:idx]
				# if of type array, setup 
			else:
				isArray = False
			# get the default value from the shader			
			defValue = getattr(self.shader, param)
			size = [0, 0, self.scroller.width - 30, 0]
			
			try:
				if isArray:
					bParm = BtoRTypes.__dict__["BtoRArrayParam"](param = param, p_type = p_type, name = param, value = defValue, size=size, parent = self.scroller) # just init a basic type
				else:					
					bParm = BtoRTypes.__dict__["BtoR" + p_type.capitalize() + "Param"](param = param, p_type = p_type, name = param, value = defValue, size=size, parent = self.scroller) # just init a basic type
				self.options[param] = [param, bParm] # so properties instantiantion works! Whee!
				self.optionOrder.append(param)
				
				# so the two arrays will make sure that my properties are correctly instantiated
				# I really need to make sure that I configure a baseClass for properties instantiation
				
				#editor = BtoRAdapterClasses.IShaderParamEditor(bParm) # that should do the trick
				#self.scroller.addElement(editor)
			except:
				traceback.print_exc()
				errors.append(param)
			count = count + 1

		self.setupProperties()
		if len(errors) > 0:
			self.evt_manager.showErrorDialog("Shader Parameter Error", "%d shader parameter(s) could not construct an editor." % len(errors))
		# set the scrollers up
		
	def setupParamEditorsX(self):
		# iterate the current shader and setup the parameter editors for it.
		# here I'm using the rmshader object so react accordingly
		# iterate the slots in the RMShader object
		# delete the current set of elements in the scroller object		

		count = 0
		for param in self.shader.shaderparams:
			normalColor = [216, 214, 220, 255]
			#if count % 2:
			#	normalColor = [190, 190, 190, 255]			
			p_type = self.shader.shaderparams[param].split()[1]
			# get the default value from the shader
			iParams = self.shader.params()			
			defValue = getattr(self.shader, param)
			if "[" in p_type:
				# this is an array value, react accordingly
				# perhaps I can simply look at the _slot in the shader and decide from there?
				arrayLen = self.shader.__dict__[param + "_slot"].size() # this should be there if everything was initialized correctly!
				# and the type name
				idx = p_type.index("[")
				isArray = True
				p_type = p_type[:idx]
				# if of type array, setup 
			else:
				isArray = False
			# I should be able to instantiate an editor like this
			size = [0, 0, self.scroller.width - 30, 0]
			if isArray:
				bParm = BtoRTypes.__dict__["BtoRArrayParam"](param = param, value = defValue, size=size, parent = self.scroller) # just init a basic type
			else:
				bParm = BtoRTypes.__dict__["BtoR" + p_type.capitalize() + "Param"](param = param, name = param, value = defValue, size=size, parent = self.scroller) # just init a basic type
			editor = BtoRAdapterClasses.IShaderParamEditor(bParm) # that should do the trick
			self.scroller.addElement(editor)
			count = count + 1
		self.scroller.invalid = True
		self.scroller.currentItem = 1
		
		
	def updateShaderParamsX(self):
		# here I need to cobble all the parameter values together and push them into the shader itself
		# so the material can then be exported.
		index = 0
		for element in self.scroller.elements:
			p_type = element.paramtype
			name = element.param_name
			# print "updating ", element.param_name, " to value: ", element.getValue()
			if p_type == "float" or p_type == "string": # lo, all my single-variable types
				setattr(self.shader, name, self.scroller.elements[index].getValue())
				
			elif p_type == "color" or p_type == "coordinate": # all vec3 types	
				val = self.scroller.elements[index].getValue()
				vec = cgkit.cgtypes.vec3(val[0], val[1], val[2])
				setattr(self.shader, name, vec)
				
			elif p_type == "matrix": # matrix types				
				val = self.scroller.elements[index].getValue()
				matrix = cgkit.cgtypes.mat4(val[0][0], val[0][1], val[0][2], val[0][3], val[1][0], val[1][1], val[1][2], val[1][3], val[2][0], val[2][1], val[2][2], val[2][3], val[3][0], val[3][1], val[3][2], val[3][3])				
				setattr(self.shader, name, matrix)
			print getattr(self.shader, name)
			index = index + 1
			
	def updateShaderParams(self):
		# here I need to cobble all the parameter values together and push them into the shader itself
		# so the material can then be exported.
		index = 0
		properties = self.properties
		for propName in properties:
			property = properties[propName]
			p_type = property.getType()
			name =property.getName()			
			if p_type == "float" or p_type == "string": # lo, all my single-variable types
				setattr(self.shader, name, property.getValue()) 
				
			elif p_type == "color" or p_type == "coordinate": # all vec3 types	
				val = property.getValue()
				vec = cgkit.cgtypes.vec3(val[0], val[1], val[2])
				setattr(self.shader, name, vec)
				
			elif p_type == "matrix": # matrix types				
				val = property.getValue()
				matrix = cgkit.cgtypes.mat4(val[0][0], val[0][1], val[0][2], val[0][3], val[1][0], val[1][1], val[1][2], val[1][3], val[2][0], val[2][1], val[2][2], val[2][3], val[3][0], val[3][1], val[3][2], val[3][3])				
				setattr(self.shader, name, matrix)
			
			index = index + 1
			
	def initCompiledShaderParams(self, params, shader):
		convtypes = {"float":"double",
				"string":"string",
				"color":"vec3",
				"point":"vec3",
				"vector":"vec3",
				"normal":"vec3",
				"matrix":"mat4"}
		
		paramList = params[2]
		shaderName = os.path.splitext(params[1])[0]
		shader.shadername = shaderName
		# get the current shader path		
		shader.filename = self.searchPaths.getValue() + os.sep + params[1]
		for param in paramList:
			if param[2] == "color" or param[2] == "point" or param[2] == "normal" or param[2] == "vector":
				defVal = (cgkit.cgtypes.vec3(param[6][0], param[6][1], param[6][2]))
			else:
				defVal = param[6]			
			if param[3] != None:
				shader.declare("%s %s[%d] %s" %(param[1], param[2], param[3], param[4]))
			else:
				shader.declare("%s %s %s" %(param[1], param[2], param[4]))
			if defVal != None and defVal != "None":
				setattr(shader, param[4], defVal)
				
	def initShaderParamsList(self, paramlist, shader):
		convtypes = {"float":"double",
				"str":"string",
				"color":"vec3",
				"point":"vec3",
				"vector":"vec3",
				"normal":"vec3",
				"matrix":"mat4"}
		for key in paramlist:
			param = paramlist[key]
		
			if type(param) == "list":
				ptype = param[0]
				if ptype == "point" or "vector" or "normal" or "color":
					val = cgkit.types.vec3(float(ptype[1]), float(ptype[2]), float(ptype[3]))
									
				elif ptype == "matrix":
					val = cgkit.types.mat4(float(param[1]),
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
							float(param[15]),
							float(param[16]))
			elif type(param) == "float" or type(param) == "int":
				ptype = "float"
				val = float(param)
			elif type(param) == "str":
				ptype = "string"
				val = param	
			shader.declare(key, type=convtypes[switch], default=val)
			# shader.createSlot(key + "_slot", convtypes[switch], None, val) # array  length is none for now, array behaviour is uncertain here	
			
	def initShaderParams(self, params, shader):		
		convtypes = {"float":"double",
				"string":"string",
				"color":"vec3",
				"point":"vec3",
				"vector":"vec3",
				"normal":"vec3",
				"matrix":"mat4"}
		for param in params:
			atts = param.attributes
			switch = atts["type"].nodeValue
			if switch == "color":
				val = cgkit.cgtypes.vec3(float(atts["value_a"].nodeValue), float(atts["value_b"].nodeValue), float(atts["value_c"].nodeValue))
				# no color space support and most blender people won't have a clue what this means anyway
			elif switch == "float":						
				val = cgkit.cgtypes.double(float(atts["value"].nodeValue))
			elif switch == "string":
				val = atts["value"].nodeValue
			elif switch == "matrix":
				val = cgkit.cgtypes.mat4(float(atts["value_0"].nodeValue), 
										float(atts["value_1"].nodeValue), 
										float(atts["value_2"].nodeValue), 
										float(atts["value_3"].nodeValue), 
										float(atts["value_4"].nodeValue), 
										float(atts["value_5"].nodeValue), 
										float(atts["value_6"].nodeValue), 
										float(atts["value_7"].nodeValue), 
										float(atts["value_8"].nodeValue), 
										float(atts["value_9"].nodeValue), 
										float(atts["value_10"].nodeValue), 
										float(atts["value_11"].nodeValue), 
										float(atts["value_12"].nodeValue), 
										float(atts["value_13"].nodeValue), 
										float(atts["value_14"].nodeValue), 
										float(atts["value_15"].nodeValue))				
			
			elif switch == "point" or switch == "vector" or switch == "normal":
				val = cgkit.cgtypes.vec3(float(atts["value_a"].nodeValue), float(atts["value_b"].nodeValue), float(atts["value_c"].nodeValue))
			# slot this attribute into the shader
			shader.declare(atts["name"], type=convtypes[switch], default=val)
			# shader.createSlot(atts["name"].nodeValue, convtypes[switch], None, val) # array  length is none for now, array behaviour is uncertain here	
	def getStrValue(self):
		return self.getShaderName()
		
	def getShaderXML(self, xml):
		
		shaderNode = xml.createElement("shader")	
		shader = self.shader
		# update all the stuff...
		self.updateShaderParams()						
		
		# get the shader information
		shaderNode.setAttribute("name", shader.shaderName())
		if shader.filename != None:
			shaderNode.setAttribute("path", os.path.normpath(shader.filename))
		else:
			shaderNode.setAttribute("path", "None")
		
			
		for parm in shader.shaderparams:
			# get the param and stuff into my dict				
			# create the node
			parmNode = xml.createElement("Param")
			value = getattr(shader, parm)	
			# create an XML element for this value.
			s_type = shader.shaderparams[parm].split()[1]
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
						parmNode.setAttribute(sep.join(["value", index]), '%f' % value[x, y])
						index = index + 1
			
			# now commit this node to the shader node
			shaderNode.appendChild(parmNode)
		
		return shaderNode
	
	def populateShaderParams(self):
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
				
class GeneticParamModifier(BtoRObject):
	def __init__(self):
		sdict = globals()
		self.settings = sdict["instBtoRSettings"]
		self.evt_manager = sdict["instBtoREvtManager"]
		self.scene = sdict["instBtoRSceneSettings"]
		self.materials = sdict["instBtoRMaterials"]
		screen = Blender.Window.GetAreaSize()
		self.editorPanel = ui.Panel(screen[0] - 200, screen[1] - 300, 400, 600, "Parameter modification:", "Parameter Modification:", None, False)
		self.shaderName = ui.Label(5, 27, "Shader Name: %s" % shader.shadername, "Shader Name: %s" % shader.shadername, self.editorPanel, True)
		self.parm_menu = []

		self.editorPanel.addElement(ui.Label(5, 40, "Parameter:", "Parameter:", self.editorPanel, False))
		self.parmMenu = ui.Menu(45, 40, "Parameters", self.parm_menu, self.editorPanel, True)
		self.addButton = ui.Button(self.parmMenu.width + 45, 40, "Add Parameter", "Add Parameter", self.editorPanel, True)
		self.addButton.registerCallback("release", self.addParm)
		self.scrollpane = ui.ScrollPane(2, 60, self.editorPanel.width - 4, self.editorPanel.height - 70, "Scroller", "Scroller", self.editorPanel, True)
		# I have nothing here.
		self.assigned_parms = []
	def init_load(self, parms):
		""" initialize the list of ranges """
		# remember to fix this
	def setShader(self, shader):
		self.assigned_parms = []
		self.shader = shader
		self.shaderName.setValue(shader.shaderName())
		# get the list of parameters
		for param in self.shader.shaderparams:
			self.parm_menu.append(param[0])
			self.parmMenu.reinit(self.parm_menu)
			
	def addParm(self, button):
		# somehow test for duplicates		
		parm = self.parmMenu.getValue()
		if parm in self.assigned_parms:
			self.evt_manager.showConfirmDialog("Error!", "You've already added that parameter!", None, False)
		else:
			parmType = self.shader.shaderparams[parm].split()[1].capitalize()
			if "[" in parmType:
				bObj = BtoRTypes.__dict__["BtoRArrayParam"](self.matName, self.shader, parm, "Array") # you know, there's no reason why this won't work...
			else:
				bObj = BtoRTypes.__dict__["BtoR" + parmType + "Param"](self.matName, self.shader, parm, parmType)			
			pObj = BtoRAdapterClasses.IShaderParamUI(bObj)
			editor = pObj.getEditor()
			containerPane = ui.Panel(0, 0, editor.width + 15, editor.height, "container", "container", self.scrollpane, False)
			containerPane.addElement(ui.CheckBox(0, 2, "", "", False, containerPane, False)) 
			iPane = ui.Panel(20, 0, editor.width, editor.height, "none", "none", self.containerPane, True)
			iPane.addElement(editor)
			self.scrollpane.addElement(containerPane) # should be all done			
			self.assigned_parms.append(parm)
		
	def removeSelectedParms(self, button):
		# somehow determine what the selected parameter is.
		# I suppose I have to embed all this stuff into a large toggle button?
		toRemove = []
		for element in self.scroller.elements:
			if element.elements[0].getValue():
				toRemove.append(element)
		for element in toRemove:
			self.scroller.removeElement(element)
	


class MainUI(BtoRObject):
	# this is the main UI object, the "main menu" of sorts from which all things spring.
	def __init__(self):
		# find my settings
		sdict = globals()
		self.settings = sdict["instBtoRSettings"]
		self.evt_manager = sdict["instBtoREvtManager"]
		self.scene = sdict["instBtoRSceneSettings"]
		self.materials = sdict["instBtoRMaterials"]
		self.objecteditor = sdict["instBtoRObjects"]
		self.lights = sdict["instBtoRLightManager"]
		self.log = sdict["instBtoRLog"]
		# setup the spaceHandler window ID
		reg = { "ID" : [Blender.Window.GetAreaID()] }
		Blender.Registry.SetKey("BtoR_WinID", reg)
		
		screen = Blender.Window.GetAreaSize()		
		self.editor = ui.MenuBar(None, False)
		#self.editor = ui.Panel(10, screen[1] - 10, 200, 205, "BtoR", "Blender to Renderman:", None, False)		
		self.editor.outlined = True
		p = self.editor
		offset = 10
		self.file_menu = ["Save to XML in .blend", "Save to XML file", "Load XML file", "Reload Blender XML", "Exit"]
		width = self.editor.get_string_width("File", 'normal') + 5
		
		self.fileMenu = ui.Menu(5, 5, 50, 18, "File", self.file_menu, self.editor, True, 
										enableArrowButton = False, 
										noSelect = True,
										baseButtonTitle = "File",
										shadowed = False,
										outlined = False)
				
		self.fileMenu.registerCallbackForElementIndex(0, "release", self.saveToXML)
		self.fileMenu.registerCallbackForElementIndex(1, "release", self.selectSaveFile)
		self.fileMenu.registerCallbackForElementIndex(2, "release", self.selectOpenFile)
		self.fileMenu.registerCallbackForElementIndex(3, "release", self.reloadXML)
		self.fileMenu.registerCallbackForElementIndex(4, "release", self.close_app)
		
		self.edit_menu = ["Edit Object Properties", "Lighting Manager"]
		self.editMenu = ui.Menu(60, 5, 80, 18, "Edit Objects", self.edit_menu, self.editor, True, 
										enableArrowButton = False,
										noSelect = True,
										baseButtonTitle = "Edit Objects",
										shadowed = False,
										outlined = False)
		self.editMenu.registerCallbackForElementIndex(0, "release", self.showObjectEditor)
		self.editMenu.registerCallbackForElementIndex(1, "release", self.showLightingManager)
				
		self.material_menu = ["Material List", "Import from XML", "Export to XML"]
		self.materialMenu = ui.Menu(150, 5, 80, 18, "Materials", self.material_menu, self.editor, True, 
										enableArrowButton = False,
										noSelect = True,
										baseButtonTitle = "Materials",
										shadowed = False,
										outlined = False)
										
		self.materialMenu.width = self.fileMenu.get_string_width("Material", 'normal') + 5
								
		self.materialMenu.registerCallbackForElementIndex(0, "release", self.showMaterials)
		self.materialMenu.registerCallbackForElementIndex(1, "release", self.selectMatLoadFile)
		self.materialMenu.registerCallbackForElementIndex(2, "release", self.selectMatSaveFile)
		
		self.settings_menu = ["Global Settings", "Log Window"]
		self.settingsMenu = ui.Menu(240, 5, 80, 18, "Settings", self.settings_menu, self.editor, True, enableArrowButton = False,
										noSelect = True,
										baseButtonTitle = "Settings",
										shadowed = False,
										outlined = False)
		self.settingsMenu.registerCallbackForElementIndex(0, "release", self.showGlobal)
		self.settingsMenu.registerCallbackForElementIndex(1, "release", self.showLog)
		
		self.exportButt = ui.Button(350, 5, 50, 18, "Export", "Export", 'normal', self.editor, True)
		
		self.exportButt.registerCallback("release", self.showExport)
		self.exportButt.shadowed = False
		
		
	def showLog(self, obj):
		editor = self.log.getEditor()
		self.evt_manager.addElement(editor)
		self.evt_manager.raiseElement(editor)
		
	def showLightingManager(self, button):
		editor = self.lights.getEditor()
		self.evt_manager.addElement(editor)
		self.evt_manager.raiseElement(editor)
	
	def saveToXML(self, button):
		xml = self.scene.getSceneXMLData()
		self.scene.writeToBlenderText(xml)
		
	def reloadXML(self, button):
		self.scene.loadSceneData()
		
	def close_app(self, button):
		self.evt_manager.showConfirmDialog("Save your changes?", "Would you like to save your changes?", self.exitBtoR, True)
		
	def exitBtoR(self, dialog):
		if dialog.getValue() == dialog.OK:
			# save the file locally...
			xml = self.scene.getSceneXMLData()
			self.scene.writeToBlenderText(xml)
			Blender.Draw.Exit()
			# exit out
		elif dialog.getValue() == dialog.DISCARD:
			Blender.Draw.Exit()
			# exit out
		elif dialog.getValue() == dialog.CANCEL:
			pass
	
	def selectSaveFile(self, button):		
		# get the filename first
		Blender.Window.FileSelector(self.saveFile, "Select XML File:", "BtoRData.xml")
		
	def selectMatSaveFile(self, button):
		Blender.Window.FileSelector(self.saveMaterials, "Select XML File:", "BtoRMaterials.xml")
		
	def saveFile(self, filename):
		xml = self.scene.getSceneXMLData()
		self.scene.writeToFile(xml, filename)
	
	def saveMaterials(self, filename):
		print "Saving materials to file: ", filename
		newdoc = xml.dom.minidom.Document()
		root = newdoc.createElement("BtoR")	
		matXml = self.materials.saveMaterials(newdoc)		
		root.appendChild(matXml)
		self.scene.writeToFile(root, filename)
		
		
	def selectOpenFile(self, button):
		Blender.Window.FileSelector(self.openFile, "Select XML file to load:", "BtoRData.xml")		
	def openFile(self, filename):
		self.scene.loadFromFile(filename)
	def selectMatLoadFile(self, button):
		Blender.Window.FileSelector(self.openMaterials, "Select XML file to load:", "BtoRMaterials.xml")	
	def openMaterials(self, filename):
		self.materials.loadFromFile(filename)
	def hideDialog(self, dialog):
		self.evt_manager.removeElement(dialog)
		
	def showEditor(self, editor):
		self.evt_manager.addElement(editor)
		self.evt_manager.raiseElement(editor)
		
	def showExport(self, button):
		editor = self.scene.getEditor()
		self.evt_manager.addElement(editor)
		self.evt_manager.raiseElement(editor)
		
	def showGlobal(self, button):
		editor = self.settings.getEditor()
		editor.dialog = True
		self.evt_manager.addElement(editor)
		self.evt_manager.raiseElement(editor)
		
	def showMaterials(self, button):
		editor = self.materials.getEditor()		
		self.evt_manager.addElement(editor)
		self.evt_manager.raiseElement(editor)
				
	def showObjectEditor(self, button):
		editor = self.objecteditor.getEditor()
		self.evt_manager.addElement(editor)
		self.evt_manager.raiseElement(editor)
		
	def showSceneSettings(self, button):
		editor = self.scene.getEditor()
		self.evt_manager.addElement(editor)
		self.evt_manager.raiseElement(editor)
	
	def getEditor(self):
		return self.editor
		
		
class MaterialList(BtoRObject):
	def __init__(self):
		sdict = globals()
		self.settings = sdict["instBtoRSettings"]
		self.evt_manager = sdict["instBtoREvtManager"]
		self.scene = sdict["instBtoRSceneSettings"]
		self.log = sdict["instBtoRLog"]
		screen = Blender.Window.GetAreaSize()
		
		# self.picker.x = self.absX
		self.editor = ui.Panel(10, screen[1] - 50, 200, 400, "Material Selector", "Material Editor:", None, False)
		self.selector = ui.Panel(225, screen[1] - 50, 200, 375, "Material Selector", "Material Selector:", None, False)
		
		self.add_button = ui.Button(0, 370, 99, 30, "Add New", "Add New", 'normal', self.editor, True)
		self.add_button.shadowed = False
		self.add_button.cornermask = 8
		self.add_button.radius = 10
		self.add_button.registerCallback("release", self.showAddDialog)
		
		self.delete_button = ui.Button(101, 370, 99, 30, "Delete", "Delete", 'normal', self.editor, True)
		self.delete_button.shadowed = False
		self.delete_button.cornermask = 4 
		self.delete_button.radius = 10
		self.delete_button.registerCallback("release", self.showDeleteDialog)
		
		self.scroller = ui.ScrollPane(0, 27, 200, 340, "MaterialScroller", "Scroller", self.editor, True)
		#self.scroller.normalColor = [43,43,43,255]
				
		self.sel_scroller = ui.ScrollPane(0, 27, 200, 340, "MaterialScroller", "Scroller", self.selector, True)
		#self.sel_scroller.normalColor = [43,43,43,255]
			
		self.close_button = ui.Button(self.editor.width - 20, 5, 14, 14, "X", "X", 'normal', self.editor, True)
		self.close_button.shadowed = False
		self.close_button.cornermask = 15
		self.close_button.radius = 1.5
		close_func = self.close_button.callFactory(self.evt_manager.removeElement, self.editor)
		self.close_button.registerCallback("release", self.close)

		self.materials = []
				
		self.sel_sticky = True
		self.select_functions = []
		
	def getMaterialCount(self):
		return len(self.materials)
		
	def loadMaterials(self):
		self.materials = self.getSavedMaterials()

		for material in self.materials:
			# create a button editor for each material			
			#self.scroller.elements.append(material.getMaterialButton())			
			material.selectorButton.registerCallback("release", self.select)
	
	def getMaterial(self, materialName):
		for material in self.materials:
			# print material.materialName.getValue()
			if material.material.name == materialName:
				return material
		return None
			
	def toggleOn(self, button):
		# sticky toggle keeps the button set, no matter what 		
		if 1 == 2:
			if self.stickyToggle:
				for element in self.scroller.elements:
					if element <> button:
						element.set_state(False)
				# simply set the state of this button and move on
				button.set_state(True)
			else:
				# print button.state
				if button.state == False:
					self.value = None # no value, since nothing's on
					# do nothing, the button has been turned off
				else: # the button was turned on, turn everything else off
					for element in self.scroller.elements:											
						if element <> button:						
							element.set_state(False)
					self.value = button.name 
	
	def select(self, button):				
		
		if self.sel_sticky:
			for element in self.sel_scroller.elements:
				if element <> button:
					element.set_state(False)
			# simply set the state of this button and move on
			button.set_state(True)
			self.value = button.title
		else:
			# print button.state
			if button.state == False:
				self.value = None # no value, since nothing's on
				# do nothing, the button has been turned off
			else: # the button was turned on, turn everything else off
				for element in self.sel_scroller.elements:											
					if element <> button:						
						element.set_state(False)
				self.value = button.title
		# now what do I do? Somehow get the selected material back to the object editor.
	
		for func in self.select_functions:
			func(button) # links back to whatever's interested in selections.
				
	def getEditor(self):
		return self.editor
	
	def getSelector(self):
		return self.selector
	def setSelected(self, ID):
		# print "selecting ", ID
		for element in self.sel_scroller.elements:
			if element.title == ID:				
				element.set_state(True)
			else:
				element.set_state(False)
	def getSelected(self):
		button = None
		for element in self.sel_scroller.elements:
			if element.getValue():
				button = element
				break
				
		return button
	
	def close(self, button):
		self.evt_manager.removeElement(self.editor)
		
	def showAddDialog(self, button): # coroutine, open the naming dialog
		self.evt_manager.showSimpleDialog("New Material:", "New Material Name:", "Material %d" % len(self.materials), self.addNew)
		
	def addNew(self, dialog):
		if dialog.state == dialog.OK:
			# assign the name
			# create the new RMMaterial
			rm_mat = cgkit.rmshader.RMMaterial(dialog.getValue())
			
			
			material = Material(rm_mat, self.scroller, self.sel_scroller)
			
			self.materials.append(material)			
			self.scroller.lastItem = 0
			self.sel_scroller.lastItem = 0
			
			material.selectorButton.registerCallback("release", self.select)			
			self.log.appendText("Created material " + dialog.getValue() + "...")
		# self.evt_manager.removeElement(dialog)
				
	def showDeleteDialog(self, button):
		self.evt_manager.showConfirmDialog("Delete Material?", "Delete selected material?",  self.deleteMaterial, False)
		
	def deleteMaterial(self, dialog):
		if dialog.state == dialog.OK:
			# find the selected material by chasing through the elements in the scroller and deleting the one that's found.
			selected = []
			for element in self.scroller.elements:
				if element.selector.getValue():					
					self.materials.remove(element.mat)
					selected.append(element)
							
			for element in selected:
				self.scroller.removeElement(element)
				self.sel_scroller.removeElement(element.mat.selectorButton)
				del element
			del selected
					
					
	def getSavedMaterials(self, xmldat = None):
		# this looks for XML stored in a blender text object by the name of BtoRXMLData
		# look to see if there's an XML object here
		materials = []
		if xmldat == None:			
			try:
				text = Blender.Text.Get("BtoRXML")
				found = True
			except NameError: # didn't find one...and I should add functionality to BtoRSettings to make sure this never happens
				found = False		
			if found:
				xmldat = ""
				lines = text.asLines()
				for line in lines:
					xmldat = xmldat + line
			else: # didn't find one...and I should add functionality to BtoRSettings to make sure this never happens
				# not found, spawn a dialog
				print "NO dialog!"
				#self.evt_manager.showErrorDialog("No materials were found!", "No saved materials were found in this document.")
				
		try:				
			# parse the settings tree and see what we have for materials
			xmlsettings = xml.dom.minidom.parseString(xmldat)
			hasXMLData = True
		except:
			hasXMLData = False			
	
		if hasXMLData:
			xmlMaterials = xmlsettings.getElementsByTagName("Material")				
			# now I have a DOM object that contains (hopefully) my materials list
			if len(xmlMaterials) > 0:	
				print "Found some materials"
				# iterate the materials and create the objects 
				for xmlMaterial in xmlMaterials:						
					# the RMMaterial gets created last						
					name = xmlMaterial.getAttribute("name")
					# print "Material name is ", name
					surface = xmlMaterial.getElementsByTagName("Surface")
					displacement = xmlMaterial.getElementsByTagName("Displacement")
					volume = xmlMaterial.getElementsByTagName("volume")
					
					colorElement = xmlMaterial.getElementsByTagName("Color")[0]
					matColor = [float(colorElement.getAttribute("red")), float(colorElement.getAttribute("green")), float(colorElement.getAttribute("blue"))]
					
					opacityElement = xmlMaterial.getElementsByTagName("Opacity")[0]
					matOpacity = [float(opacityElement.getAttribute("red")), float(opacityElement.getAttribute("green")), float(opacityElement.getAttribute("blue"))]
					
					if len(surface) > 0:
						# new CgKit surface shader
						
						if surface[0].getAttribute("filename").encode("ascii") == "None":
							# I don't know where the path is. Initialize a basic shader and set the slots up manually
							surfShader = cgkit.rmshader.RMShader()
							initialized = False
						else:
							if self.settings.use_slparams:
								surfShader = cgkit.rmshader.RMShader(os.path.normpath(surface[0].getAttribute("path").encode("ascii")))
							else:
								surfShader = cgkit.rmshader.RMShader(surface[0].getAttribute("path") + os.sep + surface[0].getAttribute("filename").encode("ascii") + self.settings.renderers[self.settings.renderer][4])
							initialized = True
							
						# setup the parameters
						
						parms = surface[0].getElementsByTagName("Param")																					
						self.populateShaderParams(parms, surfShader, initialized)
					else:
						surfShader = cgkit.rmshader.RMShader()
					
					if len(displacement) > 0:
						# new CgKit displacement Shader
						
						if displacement[0].getAttribute("filename").encode("ascii") == "None":
							dispShader = cgkit.rmshader.RMShader()
						else:
							if self.settings.use_slparams:
								dispShader = cgkit.rmshader.RMShader(os.path.normpath(displacement[0].getAttribute("path").encode("ascii")))
							else:
								dispShader = cgkit.rmshader.RMShader(displacement[0].getAttribute("path") + os.sep + displacement[0].getAttribute("filename").encode("ascii") + self.settings.renderers[self.settings.renderer][4])
			
							
						parms = displacement[0].getElementsByTagName("Param")
						self.populateShaderParams(parms, dispShader, initialized)
					else:
						dispShader = cgkit.rmshader.RMShader()
					
					if len(volume) > 0:
						
						if volume[0].getAttribute("filename").encode("ascii") == "None":
							volShader = cgkit.rmshader.RMShader()
						else:
							# new CgKit volume shader
							if self.settings.use_slparams:
								volumeShader = cgkit.rmshader.RMShader(os.path.normpath(volume[0].getAttribute("path").encode("ascii")))
							else:
								volumeShader = cgkit.rmshader.RMShader(volume[0].getAttribute("path") + os.sep + volume[0].getAttribute("name").encode("ascii") + self.settings.renderers[self.settings.renderer][4])
			
						parms = volume.getElementsByTagName("Param")			
						self.populateShaderParams(parms, volumeShader, initialized)
					else:
						volumeShader = cgkit.rmshader.RMShader()
						
					# base RMMaterial
					
					rmmaterial = cgkit.rmshader.RMMaterial(name = name.encode("ascii"), surface = surfShader, displacement = dispShader, 
												interior = volumeShader, color = matColor, opacity = matOpacity)
					# print "Color: ", matColor
					# B2RMaterial Editor here
					BtoRmat = Material(rmmaterial, self.scroller, self.sel_scroller)
					# set the title
					BtoRmat.editorButton.title = name
					BtoRmat.selectorButton.title = name
					BtoRmat.selectorButton.registerCallback("release", self.select)
					BtoRmat.editorButton.registerCallback("release", self.toggleOn)
					BtoRmat.setProperty("MaterialName", name)					
					materials.append(BtoRmat) 
					
					# that was easier than it seemed at first blush
		return materials
		
	def loadFromFile(self, filename):
		try:
			file = open(filename, 'r')
			xmlfile = file.readlines()
			xmlStr = ""
			for line in xmlfile:
				xmlStr = xmlStr + line
			found = True
		except:
			self.evt_manager.showConfirmDialog("Error parsing XML!", "There was an error in the XML file!", None, False)
			found = False
			return None
		if found:
			self.materials = self.getSavedMaterials(xmldat = xmlStr)
	
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
			
			# and set the value 	
			setattr(shader, p_name, parm_value)
				
		
	
	def saveMaterials(self, xmlDoc):
		
		# save materials to XML format.
		# the format for the materials section of the XML is thus		
		# <Material_Defs>
		#	<Material name="name">
		#		<Color red="1.0" green="1.0" blue="1.0"/>
		# 		<Opacity red="1.0" green="1.0" blue="1.0"/>
		#		<Surface shader="shadername">
		#			<Param type="float" name="whatever" value="whatever"/>
		#			<Param name="color" type="color" red="1.0" green="1.0" blue="1.0" />
		#			<Param name="matrix_name" type="matrix" value_1="1.0"/> # matrix values will increase in row-major order.
		#			<Param type="string" name="name" value="value"/>
		#			<Param name="name" type="point/normal/vector" x="1.0" y="1.0" z="1.0"/>
		#			<Param name="name" type="array">
		#				<some params>
		#			</Param>
		#

		matRoot = xmlDoc.createElement("Materials_Defs")
		# xmlDoc.appendChild(matRoot)
				
		for material in self.materials:

			mat = material.material	
			
			surface = mat.surface
			disp = mat.displacement
			volume = mat.interior
			color = material.getProperty("color")
			
			opacity = material.getProperty("opacity")
			
			dispBound = material.getProperty("DispBound")
			
			matNode = xmlDoc.createElement("Material")
			matNode.setAttribute("name", material.getProperty("MaterialName"))
			matNode.setAttribute("displacementBound", "%f" % material.getProperty("DispBound")) # that should be a string value here
			
			# color
			print color
			colorNode = xmlDoc.createElement("Color")
			colorNode.setAttribute("red", '%f' % float(color[0]))
			colorNode.setAttribute("green", '%f' % float(color[1]))
			colorNode.setAttribute("blue", '%f' % float(color[2]))
			
			#opacity
			print opacity
			opacityNode = xmlDoc.createElement("Opacity")
			opacityNode.setAttribute("red", '%f' % float(opacity[0]))
			opacityNode.setAttribute("green", '%f' % float(opacity[1]))
			opacityNode.setAttribute("blue", '%f' % float(opacity[2]))
			
			matNode.appendChild(colorNode)
			matNode.appendChild(opacityNode)
			print "saving surface ", mat.surfaceShaderName()

			if mat.surfaceShaderName() != None:
				
				surfNode = xmlDoc.createElement("Surface")				
				# update all the stuff...
				material.getProperty("Surface").getObject().updateShaderParams()						
				print "got the shader updated"
				# get the shader information
				surfNode.setAttribute("name", mat.surfaceShaderName())
				if mat.surface.filename != None:
					surfNode.setAttribute("path", os.path.normpath(mat.surface.filename))
				else:
					surfNode.setAttribute("path", "None")
					
				for parm in surface.shaderparams:
					# get the param and stuff into my dict				
					# create the node
					parmNode = xmlDoc.createElement("Param")
					value = getattr(mat.surface, parm)	
					# create an XML element for this value.
					s_type = surface.shaderparams[parm].split()[1]
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
								parmNode.setAttribute(sep.join(["value", index]), '%f' % value[x, y])
								index = index + 1
					
					# now commit this node to the shader node
					surfNode.appendChild(parmNode)
				
				# add to the material node
				matNode.appendChild(surfNode)
			print "Saving displacement..."
			
			if mat.displacementShaderName() != None:
				dispNode = xmlDoc.createElement("Displacement")
				# update all the stuff...
				material.getProperty("Displacement").getObject().updateShaderParams()						
				# get the shader information
				dispNode.setAttribute("name", mat.displacementShaderName())
				if mat.displacement.filename != None:
					dispNode.setAttribute("path", os.path.normpath(mat.displacement.filename))
					
				else:
					dispNode.setAttribute("path", "None")
					
				for parm in disp.shaderparams:
					# get the param and stuff into my dict				
					# create the node
					parmNode = xmlDoc.createElement("Param")
					value = getattr(disp, parm)
					# create an XML element for this value.
					s_type = disp.shaderparams[parm].split()[1]
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
					dispNode.appendChild(parmNode)
					
				# add this node to the material node
				matNode.appendChild(dispNode)
			print "Saving volume..."
			if mat.interiorShaderName() != None:
				volumeNode = xmlDoc.createElement("Volume")
				# update all the stuff...
				material.getProperty("Volume").getObject().updateShaderParams()						
				# get the shader information
				volumeNode.setAttribute("name", mat.interiorShaderName())
				
				if material.interior.filename != None:
					volumeNode.setAttribute("path", os.path.normpath(mat.interior.filename))
				else:
					volumeNode.setAttribute("path", "None")
					
				for parm in mat.interior.shaderparams:
					# get the param and stuff into my dict				
					# create the node
					parmNode = xmlDoc.createElement("Param")
					value = getattr(volume, parm)
					# create an XML element for this value.
					s_type = volume.shaderparams[parm].split()[1]
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
					volumeNode.appendChild(parmNode)

				# since I'm done, commit this to the material node
				matNode.appendChild(volumeNode)
				
			matRoot.appendChild(matNode)
		return matRoot
class GroupList(BtoRObject):
	def __init__(self):
		sdict = globals()
		self.settings = sdict["instBtoRSettings"]
		self.evt_manager = sdict["instBtoREvtManager"]
		self.scene = sdict["instBtoRSceneSettings"]
		setattr(BtoRAdapterClasses, "instBtoRGroupList", self)
		screen = Blender.Window.GetAreaSize()
		
		# self.picker.x = self.absX
		self.editor = ui.Panel((screen[0] / 2) - 200 , (screen[1] / 2) + 100, 200, 400, "Object Groups", "Object Groups:", None, False)

		self.add_button = ui.Button(0, 370, 99, 30, "Add New", "Add New", 'normal', self.editor, True)
		self.add_button.shadowed = False
		self.add_button.cornermask = 8
		self.add_button.radius = 10
		self.add_button.registerCallback("release", self.showAddDialog)
		
		self.delete_button = ui.Button(101, 370, 99, 30, "Delete", "Delete", 'normal', self.editor, True)
		self.delete_button.shadowed = False
		self.delete_button.cornermask = 4 
		self.delete_button.radius = 10
		self.delete_button.registerCallback("release", self.showDeleteDialog)
		
		self.sel_scroller = ui.ScrollPane(0, 27, 200, 340, "ObjectScroller", "Scroller", self.editor, True)
		self.sel_scroller.normalColor = [45,45,45,255]
				
		self.close_button = ui.Button(self.editor.width - 20, 5, 14, 14, "X", "X", 'normal', self.editor, True)
		self.close_button.shadowed = False
		self.close_button.cornermask = 15
		self.close_button.radius = 1.5
		close_func = self.close_button.callFactory(self.evt_manager.removeElement, self.editor)
		self.close_button.registerCallback("release", self.close)
		
		self.stickyToggle = False
		self.sel_sticky = True
		
		self.select_functions = []
		
	def getEditor(self):
		return self.editor
		
	def close(self, button):
		self.evt_manager.removeElement(self.editor)
		
	def select(self, button):				
		
		if self.sel_sticky:
			for element in self.sel_scroller.elements:
				if element <> button:
					element.set_state(False)
			# simply set the state of this button and move on
			button.set_state(True)
			self.value = button.title
		else:
			# print button.state
			if button.state == False:
				self.value = None # no value, since nothing's on
				# do nothing, the button has been turned off
			else: # the button was turned on, turn everything else off
				for element in self.sel_scroller.elements:											
					if element <> button:						
						element.set_state(False)
				self.value = button.title
		# now what do I do? Somehow get the selected material back to the object editor.
	
		for func in self.select_functions:
			func(button) # links back to whatever's interested in selections.
			
	def toggleOn(self, button):
		# sticky toggle keeps the button set, no matter what 		
		if self.stickyToggle:
			for element in self.sel_scroller.elements:
				if element <> button:
					element.set_state(False)
			# simply set the state of this button and move on
			button.set_state(True)
		else:
			# print button.state
			if button.state == False:
				self.value = None # no value, since nothing's on
				# do nothing, the button has been turned off
			else: # the button was turned on, turn everything else off
				for element in self.sel_scroller.elements:											
					if element <> button:						
						element.set_state(False)
				self.value = button.name 
				
	def showAddDialog(self, button): # coroutine, open the naming dialog
		self.evt_manager.showSimpleDialog("New Object Group:", "New Object Group Name:", "ObjGroup %d" % len(self.scene.object_groups.keys()), self.addNew)
		
	def addNew(self, dialog):
		self.scene.object_groups[dialog.getValue()] = [] # make it empty for the time being.
		selectorButton = ui.ToggleButton(0, 0, 180, 35, dialog.getValue(), dialog.getValue(), 'normal', self.sel_scroller, True)
		selectorButton.shadowed = False
		selectorButton.outlined = True
		selectorButton.textlocation = 1		
		selectorButton.title = dialog.getValue()
		self.sel_scroller.addElement(selectorButton)
		selectorButton.registerCallback("release", self.toggleOn)
		selectorButton.registerCallback("release", self.select)
		
	def showDeleteDialog(self, button):
		self.evt_manager.showConfirmDialog("Delete Object Group?", "Delete selected object group?",  self.deleteGroup, False)
	def deleteGroup(self, button):
		if dialog.state == dialog.OK:
			# find the selected material by chasing through the elements in the scroller and deleting the one that's found.
			for element in self.sel_scroller.elements:
				if element.getValue() == True:
					groupName = element.getTitle()
					self.scene.object_groups.popitem(groupName)					
					self.sel_scroller.removeElement(element)
					break
	def initializeGroups(self):
		for key in self.scene.object_groups.keys():
			selectorButton = ui.ToggleButton(0, 0, 180, 35, dialog.getValue(), dialog.getValue(), 'normal', selector, True)
			selectorButton.shadowed = False
			selectorButton.outlined = True
			selectorButton.textlocation = 1		
			selectorButton.title = dialog.getValue()
			self.sel_scroller.addElement(selectorButton)
			
	def setSelected(self, ID):
		# print "selecting ", ID
		for element in self.sel_scroller.elements:
			if element.title == ID:				
				element.set_state(True)
			else:
				element.set_state(False)
				
	def getSelectedValue(self):
		button = None
		for element in self.sel_scroller.elements:
			if element.getValue():
				button = element
				break
				
		return button.title
	
	
		
		
class ExportSettings(BtoRObject):	
	def __init__(self):	
		sdict = globals()
		self.settings = sdict["instBtoRSettings"]
		self.evt_manager = sdict["instBtoREvtManager"]
		self.scene = sdict["instBtoRSceneSettings"]
		self.materials = sdict["instBtoRMaterials"]
			
		screen = Blender.Window.GetAreaSize()		
		self.editorPanel = ui.Panel((screen[0] / 2) - 225 , (screen[1] / 2) + 150 , 450, 250, "Export Object", "Export Selected Object:", None, False)
		self.editorPanel.dialog = True
		
		self.editorPanel.addElement(ui.Label(10, 28, "Export to:", "Export to:", self.editorPanel, False))
		export_options = ["File...", "To selected renderer...", "To a Blender Text"]
		self.export_menu = ui.Menu(self.editorPanel.get_string_width("Export to:", 'normal') + 20, 28, 150, 20, "Export To?", export_options, self.editorPanel, True)
		self.export_menu.registerCallback("release",self.selectExportType)
		
		self.textLabel = ui.Label(10, 63, "Text Name:", "Text Name:", self.editorPanel, False)
		self.textLabel.isvisible = False
		self.editorPanel.addElement(self.textLabel)		
		self.textName = ui.TextField(self.editorPanel.get_string_width("Text Name:", 'normal') + 20, 63, 300, 20, "Text Name", "RIBOutput.rib", self.editorPanel, True)
		self.textName.isvisible = False
				
		self.fileLabel = ui.Label(10, 63, "Export Filename:", "Export Filename:", self.editorPanel, False)
		self.editorPanel.addElement(self.fileLabel)
		self.filename = ui.TextField(20 + self.editorPanel.get_string_width("Export Filename:", 'normal'), 63, 250, 20, "Filename", self.settings.outputpath.getValue() + os.sep + "output.rib", self.editorPanel, True)
		# the filename should probably reflect the object name in question.
		self.browseButton = ui.Button(self.filename.x + self.filename.width + 5, 63, 60, 20, "Browse", "Browse", 'normal', self.editorPanel, True)
		self.browseButton.registerCallback("release",self.openBrowseWindow)
		
		self.editorPanel.addElement(ui.Label(10, 95, "Export coordinates:", "Export coordinates:", self.editorPanel, False))
		coordinate_options = ["Use world coordinates", "Use object coordinates"]
		self.coordinate_menu = ui.Menu(self.editorPanel.get_string_width("Export coordinates:", 'normal') + 20, 95, 250, 20, "Coordinates to use:", coordinate_options, self.editorPanel, True)
		self.coordinate_menu.registerCallback("release",self.selectCoords)
		
		self.editorPanel.addElement(ui.Label(10, 135, "Export Materials?", "Export Materials?", self.editorPanel, False))
		self.material_menu = ui.Menu(self.editorPanel.get_string_width("Export Materials?", 'normal') + 20, 135, 80, 20, "Export Materials?", ["Yes", "No"], self.editorPanel, True)
		
		self.camera_label = ui.Label(10, 165, "Create renderable file?", "Include camera settings? (Creates renderable file)", self.editorPanel, True)
		self.camera_menu = ui.Menu(self.editorPanel.get_string_width("Include camera settings? (Creates renderable file)", 'normal') + 20, 165, 80, 20, "Include Camera?", ["No", "Current Camera", "Current 3D View"], self.editorPanel, True)
		
				
		# self.editorPanel.addElement(ui.Label(10, 160, "Duplicate Object by:", "Duplicate Object By:", self.editorPanel, False))
		# additive transform matrix, (or subtractive, multiplicative, divisive)
		# per-vertex of some other object
		# some kind of random script
		
		
		self.closeButton = ui.Button(10, self.editorPanel.height - 40, 100, 25, "Close", "Cancel", 'normal', self.editorPanel, True)
		self.closeButton.registerCallback("release",self.close)
		
		# the export button should be the last thing on the panel
		self.exportButton = ui.Button(self.editorPanel.width - 110, self.editorPanel.height - 40, 100, 25, "Export Object", "Export Object", 'normal',  self.editorPanel, True)
		self.exportButton.registerCallback("release",self.doExport)
				

		self.export_functions = []
		self.selectExportType(None)
		
	def getEditor(self):
		return self.editorPanel
		
	def selectCoords(self, button):
		if self.coordinate_menu.getSelectedIndex() == 0:
			self.camera_label.isVisible = True
			self.camera_menu.isVisible = True
		else:
			self.camera_label.isVisible = False
			self.camera_menu.isVisible = False
			
	def selectExportType(self, button):
		if self.export_menu.getSelectedIndex() == 0:
			self.fileLabel.isVisible = True			
			self.browseButton.isVisible = True
			self.filename.isVisible = True			
			self.textLabel.isVisible = False
			self.textName.isVisible = False
		elif self.export_menu.getSelectedIndex() == 1:
			self.textLabel.isVisible = False
			self.textName.isVisible = False
			self.fileLabel.isVisible = False
			self.browseButton.isVisible = False
			self.filename.isVisible = False			
		else:
			self.fileLabel.isVisible = False
			self.browseButton.isVisible = False
			self.filename.isVisible = False
			self.textLabel.isVisible = True
			self.textName.isVisible = True
		
	def doExport(self, button):
		self.evt_manager.removeElement(self.editorPanel)
		for func in self.export_functions:
			func()
	
	def openBrowseWindow(self, button):
		Blender.Window.FileSelector(self.select, 'Choose a file')
		
	def select(self, file):
		#self.filename.value = file
		self.filename.setValue(file)
			
	def close(self, button):
		self.evt_manager.removeElement(self.editorPanel)
	
	def getValue(self):
		return self.filename.getValue()

class LightManager(BtoRObject):
	""" This is the light manager. It provides direct access to lights in the scene without having to select them in the blender window. 
	
	     this will allow me to load light setups/looks
	"""
		# "UseGI" : "Use Global Illuminator", False],
		# "SkyDOme" : ["Create Sky Dome", False],
		# 
	def __init__(self):
		# center the panel
		sdict = globals()
		self.settings = sdict["instBtoRSettings"]
		self.evt_manager = sdict["instBtoREvtManager"]		
		setattr(BtoRAdapterClasses, "instBtoRLightManager", self)
		
		self.options = { "GenOcclusion" : ["Generate Occlusion Maps:", False],
			"GenShadowMaps" : ["Create Shadow Maps:", True],
			"UseAmbient" : ["Default Ambient Light:", False],
			"AmbIntensity" : ["Ambient Intensity:", 1.0],
			"AmbColor" : ["Ambient Light Color:", [1.0, 1.0, 1.0]],
			"Multiplier" : ["Light Multiplier:", 1.0],
			"AOShader" : ["AO Shader:", BtoRTypes.BtoRShaderType(GenericShader(None, "light", self))],
			"layerLighting" : ["Enable lighting layers:", False]}
		
		self.optionOrder = ["GenOcclusion", "GenShadowMaps", "layerLighting", "AOShader", "UseAmbient", "AmbColor", "AmbIntensity", "Multiplier"]

		
		# insert my AO shader property into the options list
		
		screen = Blender.Window.GetAreaSize()		
		self.editorPanel = ui.Panel(25 , screen[1] - 35 , 245, 320, "Light Manager", "Light Manager:", None, False)
		
		self.editorPanel.addElement(ui.Label(5, 27, "Scene Lighting properties:", "Scene Lighting Properties", self.editorPanel, False))
		self.scroller= ui.ScrollPane(5, 50, 240, 260, "Scroller", "Scroller", self.editorPanel, True)
		
		self.AOPanel = self.options["AOShader"][1].getObject().getEditor()	
		self.AOPanel.isVisible = False
		self.AOPanel.shadowed = True
		self.AOPanel.outlined = True
		self.AOPanel.hasHeader = False				
		self.AOPanel.invalid = True
		self.AOPanel.validate()
		self.editorPanel.addElement(self.AOPanel)
		self.AOPanel.parent = self.editorPanel
		self.AOPanel.x = self.editorPanel.width + 10
		
		self.options["AOShader"][1].getObject().obj_parent = self
		
		self.close_button = ui.Button(self.editorPanel.width - 20, 5, 14, 14, "X", "X", 'normal', self.editorPanel, True)
		self.close_button.shadowed = False
		self.close_button.cornermask = 15
		self.close_button.radius = 1.5
		# close_func = self.close_button.callFactory(self.evt_manager.removeElement, self.editor)
		self.close_button.registerCallback("release", self.close)
		self.lights = {}		
		
		self.occlusion_menu = ["None Selected"]
		self.occlListeners = []
		if self.settings.renderer == "Pixie":
			hiderProp = ["Hider", {"btor:default" : "stochastic", "stochastic" : "stochastic", "zbuffer" : "zbuffer", "raytrace" : "raytrace", "photon" : "photon", "opengl" : "opengl" }, "stochastic"]
			self.options["Hider"] = hiderProp
			self.optionOrder.insert(0, "Hider")
			
		self.setupProperties()
		# the properties here should read in the lights that belong to which occlusion groups and what-not
		# as well as set the groups up..but all of this will be a special case, since my properties don't actually
		# support automatically setting variables outside of themselves
		
		#self.lightsButt = ui.Button(250, 27, 120, 25, "Lights", "Lights", 'normal', self.editorPanel, True)
		#self.lightsButt.registerCallback("release", self.toggleLights)
		
		# occlusion groups here!
		# self.occluButt = ui.Button(250, 27, 120, 25, "Occlusion", "Occlusion Groups", 'normal', self.editorPanel, True)
		# self.occluButt.registerCallback("release", self.toggleOccl)
		
		# lighting roles here!
		# self.roleButt = ui.Button(250, 27, 120, 25, "Roles", "Lighting Roles", 'normal', self.editorPanel, True)
		# self.roleButt.registerCallback("release", self.toggleRole)
		
		self.rolePanel = ui.Panel(390, 0, 310, 300, "", "", self.editorPanel, True)
		self.rolePanel.hasHeader = False
		self.rolePanel.isVisible = False
		self.rolePanel.outlined = True
		self.rolePanel.shadowed = True
		self.rolePanel.addElement(ui.Label(5, 5, "Lighting Roles", "Lighting Roles:", self.rolePanel, False))
		self.addRoleButt = ui.Button(5, 27, 75, 20, "Add", "Add Role", 'small', self.rolePanel, True)
		self.delRoleButt = ui.Button(90, 27, 74, 20, "Del", "Delete Light", 'small', self.rolePanel, True)
		self.addRoleButt.registerCallback("release", self.addRole)
		self.delRoleButt.registerCallback("release", self.delRole)
		
		self.roleScroller = ui.ScrollPane(5, 65, 300, 230, "", "", self.rolePanel, True)		
		
		# use getSelected to setup a lighting group.
		# use a reverse selection model to select a group when an occlusion group gets selected	
		self.lightsPanel = ui.Panel(390, 0, 295, 300, "", "", self.editorPanel, True)
		self.lightsPanel.hasHeader = False
		self.lightsPanel.isVisible = False
		self.lightsPanel.outlined = True
		self.lightsPanel.shadowed = True
		self.lightsPanel.addElement(ui.Label(5, 5, "Selected Lights", "Selected Lights:", self.lightsPanel, False))

		self.lightScroller = ui.ScrollPane(5, 35, 281, 250, "", "", self.lightsPanel, True)
		self.lights = {}
		
		self.occlPanel = ui.Panel(390, 0, 310, 300, "", "", self.editorPanel, True)
		self.occlPanel.hasHeader = False
		self.occlPanel.isVisible = False
		self.occlPanel.outlined = True
		self.occlPanel.shadowed = True
		self.occlPanel.addElement(ui.Label(5, 5, "Occlusion Groups", "Ambient Occlusion Lights/Groups:", self.occlPanel, False))
		self.occlAddGroup = ui.Button(5, 27, 75, 20, "Add", "Add Light", 'small', self.occlPanel, True)
		self.occlDelGroup = ui.Button(90, 27, 75, 20, "Del", "Delete Light", 'small', self.occlPanel, True)
		self.occlDelGroup.registerCallback("release", self.delOccl)
		self.occlAddGroup.registerCallback("release", self.addOccl)
		#self.occlDelGroup.registerCallback("release", self.delOccl)
		
		self.occlScroller = ui.ScrollPane(5, 65, 300, 230, "", "", self.occlPanel, True)		
		self.occlGroups = [] # e.g. "groupname" : [[properties],["light1", "light2", "light3"]]
		self.roles = []
		
		# add some lighting roles, like key, fill and shadow
		self.occlIndex = 0
		self.settings.rendererListeners.append(self)
		
	def showShader(self):
		if self.AOPanel.isVisible:
			self.AOPanel.hide()
		else:
			self.AOPanel.show()
		
		
	def getSelected(self):		
		if not self.__dict__.has_key("scene"):
			sdict = globals()
			self.scene = sdict["instBtoRSceneSettings"]
		# clear the scroller
		self.lightScroller.clearElements()
			
		if self.editorPanel in self.evt_manager.elements: #only if this instance is up and running.
			objects = Blender.Object.GetSelected()
			for obj in objects:
				if obj.getType() == "Lamp":
					# so what are we doing here?
					if not self.scene.object_data.has_key(obj.getName()): 
						# probably need to instantiate a light adapter and go from there
						bObj = BtoRTypes.__dict__["BtoR" + obj.getType()](obj)
						objData = BtoRAdapterClasses.IObjectAdapter(bObj)		
						self.scene.object_data[obj.getName()] = objData						
						# if the object doesn't have objData in the scene data list, instantiate an adapter
					if obj.getName() in self.lights:
						# no need to add it, simply display it
						self.lightScroller.addElement(self.lights[obj.getName()])
					else:
						self.addLight(obj.getName()) # the light will add itself
						
	def addRole(self, button, roleName = None):
		if roleName == None:
			roleName = " "
		rolePanel = ui.Panel(0, 0, 265, 20, "role", "", self.roleScroller, True)
		rolePanel.selector = ui.CheckBox(2, 3, " ", " ", False, rolePanel, True)
		rolePanel.outlined = True
		rolePanel.shadowed = False
		rolePanel.cornermask = 0
		rolePanel.hasHeader = False
		rolePanel.selector.transparent = True
		rolePanel.selector.elements[0].transparent = True
		rolePanel.roleName = ui.TextField(15, 0, 75, 20, "", roleName, rolePanel, True)			
		rolePanel.renderPass = ui.CheckBox(91, 3, " ", " ", False, rolePanel, True)
		
	def delRole(self, button):
		selected = []
		for role in self.roles:
			if role.selector.getValue():
				selected.append(role)
		for role in selected:
			self.roles.remove(role)
			self.roleScroller.removeElement(role)
			
	def updateAttributes(self):
		hiderProp = ["Hider", {"btor:default" : "stochastic", "stochastic" : "stochastic", "zbuffer" : "zbuffer", "raytrace" : "raytrace", "photon" : "photon", "opengl" : "opengl" }, "stochastic"]
		if self.settings.renderer == "Pixie":
			if not self.properties.has_key("Hider"):
				property = BtoRAdapterClasses.IProperty(hiderProp[1])
				property.setName("Hider")
				property.setWidth(self.scroller.width - 15)
				editor = BtoRAdapterClasses.IPropertyEditor(property)
				self.properties["Hider"] = property
				self.editors["Hider"] = editor
				editor.getEditor().parent = self.scroller
				self.scroller.addElement(editor.getEditor())
				self.scroller.raiseElement(editor.getEditor())
		else:
			if self.properties.has_key("Hider"):
				self.properties.pop("Hider")
				x = self.editors.pop("Hider")
				self.scroller.removeElement(x.getEditor())
			
	def addLight(self, light):
		lightPanel = ui.Panel(0, 0, 265, 20, light, "", self.lightScroller, True)
		lightPanel.selector = ui.CheckBox(2, 3, " ", " ", False, lightPanel, True)
		lightPanel.outlined = True
		lightPanel.shadowed = False
		lightPanel.cornermask = 0
		lightPanel.hasHeader = False
		lightPanel.selector.transparent = True
		lightPanel.selector.elements[0].transparent = True
		lightPanel.lightName = ui.TextField(15, 0, 75, 20, light, light, lightPanel, True)	
		lightPanel.lightName.Enabled = False
		role_menu = ["Normal", "Keylight", "Shadows Only", "Off"]
		
		lightPanel.lightRoleMenu = ui.Menu(91, 0, 75, 20, "Light Role", role_menu, lightPanel, True, shadowed = False, fontsize = 'small')
		lightPanel.lightRoleMenu.shadowed = False
		# the light render button
		lightPanel.lightOcclGroupMenu = ui.Menu(167, 0, 100, 20, "Occlusion Groups", self.occlusion_menu, lightPanel, True, shadowed = False, fontsize = 'small')
		lightPanel.lightOcclGroupMenu.shadowed = False
		self.occlListeners.append(lightPanel.lightOcclGroupMenu)
		
		#lightPanel.lightSoloButton = ui.Button(167, 0, 75, 20, light, "Solo Render", 'small', lightPanel, True) # make the name of this button the same as the light, so I can "solo" render the scene from
		#lightPanel.lightSoloButton.shadowed = False
		#lightPanel.lightSoloButton.registerCallback("release", self.soloRender) # trigger the solo render for the light.		
		self.lights[light] = lightPanel
		
		self.lightScroller.addElement(lightPanel)

		
	def addOccl(self, button):
		occlPanel = ui.Panel(0, 0, 95, 20, "occl%d" % self.occlIndex, "", self.occlScroller, True)
		occlPanel.outlined = True
		occlPanel.shadowed = False
		occlPanel.cornermask = 0
		occlPanel.hasHeader = False
		occlPanel.selector = ui.CheckBox(2, 4, " ", " ", False, occlPanel, True)
		occlPanel.selector.transparent = True
		occlPanel.selector.elements[0].transparent = True
		occlPanel.groupName = ui.TextField(15, 0, 75, 20, occlPanel.name, "Ambient%d" % self.occlIndex, occlPanel, True)
		occlPanel.groupName.registerCallback("update", self.updateOcclusionList)
		occlPanel.aoShaderProperty = BtoRAdapterClasses.IProperty(BtoRTypes.BtoRShaderType(GenericShader(None, "light", self, passSelf = True)))
		occlPanel.aoShaderProperty.value.obj_parent = self
		occlPanel.aoShaderProperty.width = 202
		occlPanel.aoShaderProperty.height = 20
		occlPanel.aoShaderProperty.name = "AO Shader:"
		occlPanel.aoShaderEditor = BtoRAdapterClasses.IPropertyEditor(occlPanel.aoShaderProperty)
		##occlPanel.aoShaderEditor.label.isVisible = False
		##occlPanel.aoShaderEditor.value.x = 0
		##occlPanel.aoShaderEditor.value.width = 100
		##occlPanel.aoShaderEditor.triggerButton.x = 101
		##occlPanel.aoShaderEditor.triggerButton.width = 20
		##occlPanel.aoShaderEditor.editor.width = 121
		occlPanel.aoShaderEditor.editor.x = 91	
		occlPanel.aoShaderEditor.editor.parent = occlPanel
		occlPanel.aoShaderEditor.editor.invalid = True
		occlPanel.addElement(occlPanel.aoShaderEditor.getEditor())
		self.occlIndex = self.occlIndex + 1
		# now I have to setup occlusion properties, like the occlusion shader, and the attached lights array		
		self.occlGroups.append(occlPanel)
		self.updateOcclusionList(None)
		return occlPanel
		
	def delOccl(self, obj):
		selected = []
		for occl in self.occlGroups:
			if occl.selector.getValue():
				selected.append(occl)
		for occl in selected:
			self.occlGroups.remove(occl)
			self.occlScroller.removeElement(occl)
			
	def updateOcclusionList(self, obj):
		self.occlusion_menu = ["None Selected"]		
		for occl in self.occlGroups:
			self.occlusion_menu.append(occl.groupName.getValue())
		for listener in self.occlListeners:			
			listener.updateMenu(self.occlusion_menu)
			
	def selectOcclusion(self, button):
		pass
	def soloRender(self, button):
		pass # button name will contain the name of the light in question.
		
	def checkLights(self):
		pass
		# iterate lights here and if I find one that's not in the list, add it
		# if I find a light in the list that's no longer in the scene, ditch it.
		
		
	def toggleLights(self, button):
		if self.lightsPanel.isVisible:
			self.lightsPanel.isVisible = False			
		else:
			self.lightsPanel.show()
			self.occlPanel.hide()
	def toggleOccl(self, button):
		if self.occlPanel.isVisible:
			self.occlPanel.hide()
		else:
			self.occlPanel.show()
			self.lightsPanel.hide()
			self.rolePanel.hide()
			
	def toggleRole(self, button):
		if self.rolePanel.isVisible:
			self.rolePanel.hide()
		else:
			self.rolePanel.show()
			self.occlPanel.hide()
			self.lightsPanel.hide()
			
		
	def getEditor(self):
		return self.editorPanel
		
	
	def close(self, button):
		self.evt_manager.removeElement(self.editorPanel)
		
		
class TextWindow(BtoRObject):
	def __init__(self):
		sdict = globals()
		
		self.evt_manager = sdict["instBtoREvtManager"]		
		screen = Blender.Window.GetAreaSize()	
		self.editorPanel = ui.Panel(10, 240, 300, 200, "Log", "BtoR Log:", None, False)
		self.logPanel = ui.Panel(0, 27, 300, 160, "", "", self.editorPanel, True)
		self.logPanel.shadowed = False
		self.logPanel.hasHeader = False
		self.logPanel.cornermask = 0
		self.logPanel.normalColor = [255, 255, 224, 255]
		self.valueText = ui.TextItem(3, 0, "", " ", self.logPanel, True, fontsize = 'normal', multiLine = True, ignoreWhitespace = True)
		self.valueText.textColor = [0, 0, 0, 255]
		
		self.close_button = ui.Button(self.editorPanel.width - 20, 5, 14, 14, "X", "X", 'normal', self.editorPanel, True)
		self.close_button.shadowed = False
		self.close_button.cornermask = 15
		self.close_button.radius = 1.5
		
		self.close_button.registerCallback("release", self.close)
		
	def close(self, button):
		self.evt_manager.removeElement(self.editorPanel)
		
	def appendText(self, value):
		self.valueText.appendLine(value)
		
	def getEditor(self):
		return self.editorPanel
	
	def setText(self, value):
		self.valueText.setValue(value)
		
class RibBox(BtoRObject):
	def __init__(self):
		sdict = globals()
		self.evt_manager = sdict["instBtoREvtManager"]
		self.settings = sdict["instBtoRSettings"]
		
		screen = Blender.Window.GetAreaSize()
		self.editor = ui.Panel(screen[0] / 2 - 150, (screen[1] / 2) + 150, 300, 300, "CustomRIB", "Custom RIB:", None, False)
		self.text = ui.TextBox(0, 27, 300, 250, "None", "Wheeee", self.editor, True)

		
	def getEditor(self):
		return self.editor
		
class LookMaker(BtoRObject):
	def __init__(self):
		# time for black magic & v00d00.
		# the look manager will build shaders on the fly using custom SL generation.
		
		sdict = globals()
		self.evt_manager = sdict["instBtoREvtManager"]
		self.settings = sdict["instBtoRSettings"]
		
		
		
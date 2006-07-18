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
	
	# convenience method to return the editor object for this adapter
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
		pass
		
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
		objectNode = xmlDoc.createElement("object")
		for key in self.objData:
			if key == "shaderparms":
				# I can pretty much assume that if I've got a shaderparms key, then a shader exists...
				shaderNode = xmlDoc.createElement("shader")					
				# update all the stuff...
				if self.__dict__.has_key("imagerShader"):
					gShader = self.imagerShader
				elif self.__dict__.has_key("lightShader"):
					gShader = self.lightShader
				# get the shader information
				# I should test to ensure that there IS a shader selected.
				shader = gShader.shader
				shaderNode.setAttribute("name", shader.shaderName)
				
				if shader.filename != None:
					shaderNode.setAttribute("path", os.path.normpath(shader.filename))
				else:
					shaderNode.setAttribute("path", "None")
					
				for parm in shader.shaderparams:
					# get the param and stuff into my dict				
					# create the node
					parmNode = xmlDoc.createElement("Param")
					value = shader.getattr(parm[1]).getValue()	
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
				objectNode.appendChild(shaderNode)
			else:
				# add the normal stuff here.
				objectNode.setAttribute(key, self.objData[key])
		return objectNode
				
	def loadData(self):
		""" Recreate this object from an XML representation of it. """
		pass
		
class MeshAdapter(ObjectAdapter):
	""" BtoR mesh Adapter """
	protocols.advise(instancesProvide=[IObjectAdapter], asAdapterForTypes=[BtoRMesh])
	def __init__(self, object):
		""" Initialize a mesh export adapter """		
		ObjectAdapter.__init__(self, object)				
	def render(self):
		""" Generate Renderman data for this object."""
		# print "exporting!"
		# let's assume that if I'm here, that RiBegin has already been called
		# start gathering data
		modifiers = self.object.modifiers
		subsurf = False
		
		ri.RiAttributeBegin()
		ri.RiTransformBegin()
		#print self.objData
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
				
		for modifier in modifiers:			
			if modifier.type == Blender.Modifier.Type["SUBSURF"]:
				subsurf = True				
			else:
				subsurf = False
		# Transform		
		ri.RiTransform(self.object.matrix)
		if subsurf:
			# print "Exporting a subdivision mesh"
			self.renderSubdivMesh()
		else:
			# print "Exporting a standard mesh"
			self.renderPointsPolygons()
		
		ri.RiTransformEnd()
		ri.RiAttributeEnd()
		
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
		
	def setMaterial(self, material):
		self.material = material
		self.objData["material"] = material.name
		self.objEditor.materialButton.setTitle(material.name)
		self.objEditor.setImage(material.image)
		
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
			
	def render(self): 
		""" Generate renderman data for this object. """
		# objData = self.getLampData(object)
		# now that I have the objData, get the shader stuff
		params = {}
		for param in self.lightShader.shader.shaderparams:
			# get the value of each
			params[param] = getattr(self.lightShader.shader, param)
		ri.RiLightSource(self.lightShader.shader.shadername, params)
		

	def initLightShader(self):			
		if self.settings.use_slparams:
			# try to find the shader file in question. In truth, the default files should exist
			# get the first shader search path for pre-generated shaders				
			initialized = True			
			shader = cgkit.rmshader.RMShader(self.objData["shaderfilename"])
			self.populateShaderParamsList(self.objData["shaderparms"], shader, initialized)
			self.lightShader = btor.BtoRMain.GenericShader(shader, "light", self) # how to initialize the shader from the stored filename?				
		else:
			initialized = False
			shader = cgkit.rmshader.RMShader(self.objData["shadername"])
			self.populateShaderParamsList(self.objData["shaderparms"], shader, initialized)
			self.lightShader = btor.BtoRMain.GenericShader(shader, "light", self) # blank shader, I will choose from there. 
		# adjust for light color changes
		# set the scene-level objects
		#self.scene.light_data[self.object.getName()] = self.lightShader
		# self.scene.object_data[self.object.getName()] = self.objData # ah-ha! this is already done for me in BtoRMain!
		self.objEditor.setShader(self.lightShader)
		self.objEditor.shaderButton.setTitle(self.lightShader.getShaderName())
		
	def initObjectData(self):
		self.objData = {}
		self.objData["name"] = self.object.getName()
		self.objData["type"] = self.object.getType()
		
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
		
		self.initLightShader() # initialize my light shader
				
	def checkReset(self):
		
		# here I simply want to check if the lamp settings need changing or not.
		# if the user has selected a shader type that doesn't match the lamp settings, all I want to affect in that case
		# is the light color.
		
		shaderParms = self.objData["shaderparms"]
		
		lamp = self.object.getData()		
		
		# for the most part, follow the parameters for the given light object and stuff the values into the 
		# shader parms 
		if self.objData["type"] == lamp.type:
			
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
			if (lamp.type == 0 or lamp.type == 4) and self.lightShader.shader.shadername == "pointlight":
				energyRatio = lamp.dist * negative
				# get the first selected shader path...and hope it's setup correctly
				shaderParms["intensity"] = (energyRatio * lamp.energy) * self.scene.lightMultiplier
				
				# I will have to deal with shadowmapping at some point
				#shaderParms["name"] = "shadowpoint"
			elif lamp.type == 1 and self.lightShader.shader.shadername == "distantlight":
				energyRatio = negative
				self.objData["shadername"] = "distantlight"			

				shaderParms["to"] = [ tox, toy, toz]
							
				# parms for shadowMapping
				#shaderParms["name"] = "distantshadow"
				#shaderParms["shadowname"] = "shadow"
				#shaderParms["shadowsamples"] = 1			
				#shaderParms["shadowblur"] = 0.0
				
			elif lamp.type == 2 and self.lightShader.shader.shadername == "bml":
				energyRatio = lamp.dist * negative
				shaderParms["shadowbias"] = 1
				shaderParms["blur"] = 0.0
				shaderParms["samples"] = 1
				shaderParms["coneangle"] = (lamp.spotSize * math.pi / 360)
				shaderParms["conedeltaangle"] = (lamp.spotBlend * (lamp.spotSize * math.pi / 360))			
				shaderParms["to"] = [tox, toy, toz]
				shaderParms["intensity"] = (energyRatio * lamp.energy) * self.scene.lightMultiplier			
				# shaderParms["shadowname"] = None # ignore the shadow name. if that's been set, I want to keep the value
			elif lamp.type == 3 and self.lightShader.shader.shadername == "hemilight":
				energyRatio = negative
				shaderParms["to"] = [tox, toy, toz]
				shaderParms["falloff"] = 0
				shaderParms["intensity"] = energyRatio * lamp.energy
			
			self.objData["shaderparms"] = shaderParms
			
			# and reset the light color.
			self.objData["lightcolor"] = [lamp.R, lamp.G, lamp.B]
			
			for key in shaderParms:				
				self.lightShader.setParamValue(key, shaderParms[key])
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
		ObjectAdapter.__init__(self)	
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
		if self.imagerShader.shader != None:
			if self.imagerShader.getShaderName() != None:
				self.imagerShader.updateShaderParams()
				ishader = self.imagerShader.shader
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
			dof = self.objEditor.getDoF()
			if dof != None:
				ri.RiDepthOfField(dof[0], dof[1], dof[2])
			# Depth of field done
			
		else:
			self.objData["scale"] = camera.getScale() # not sure what this does yet
			ri.RiProjection("orthographic") 
		ri.RiFrameAspectRatio(factor)	
		# Camera clipping
		ri.RiClipping(camera.getClipStart(), camera.getClipEnd())
		
		# Viewpoint transform
		cmatrix = self.object.getInverseMatrix()
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
		# that should take care of the camera
		
	def initObjectData(self):
		# object initia.ize
		""" Initialize BtoR object data for this object """
		self.objData = {}
		self.objData["name"] = self.object.getName()
		self.objData["type"] = self.object.getType() 
		self.initImagerShader()
		
	def initImagerShader(self):
		# initialize a camera 	
		bWorld = Blender.World.GetCurrent()
		
		shaderParams = {}
		shaderParams["bgcolor"] = [bWorld.hor[0], bWorld.hor[1], bWorld.hor[2]]
		try:
			if self.settings.use_slparams:
				shaderPath = self.settings.shaderpaths.getValue().split(";")[0]
				self.objData["shaderfilename"] = os.path.normpath(shaderPath + os.sep + "background.sl")
				# try to find the shader file in question. In truth, the default files should exist
				# get the first shader search path for pre-generated shaders				
				initialized = True
				
				shader = cgkit.rmshader.RMShader(self.objData["shaderfilename"])
				self.populateShaderParamsList(shaderParams, shader, initialized)			
				
			else:
				initialized = False
				shader = cgkit.rmshader.RMShader(self.objData["shadername"])
				self.populateShaderParamsList(shaderParams, shader, initialized)		
			self.imagerShader = btor.BtoRMain.GenericShader(shader, "imager", self)
		except:
				
			self.imagerShader = btor.BtoRMain.GenericShader(None, "imager", self)
		# adjust for light color changes
		# set the scene-level objects
		#self.scene.imager_data[self.object.getName()] = self.imagerShader

		self.objEditor.setShader(self.imagerShader)
		self.objEditor.shaderButton.setTitle(self.imagerShader.getShaderName())
		
		# initialize a shader with the background. This is for INITIALIZATION ONLY
			
		
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
		self.mat_selector = self.materials.getSelector()
		
		self.editorPanel = ui.Panel(4, 110, 491,  280, "Empty Panel", "", None, False)
		self.editorPanel.hasHeader = False
		self.editorPanel.cornermask = 0
		self.editorPanel.shadowed = False
		self.editorPanel.outlined = False

	def getEditor(self):
		""" get the object editor for this object. """
		return self.editorPanel

		
		

class MeshUI(ObjectUI):
	object_output_options = ["Mesh", "Renderman Primitive", "RA Proxy", "RA Procedural"]
	mesh_output_options = ["basic", "SubDiv"]
	protocols.advise(instancesProvide=[IObjectUI], asAdapterForTypes=[BtoRMesh])
	""" Mesh UI panel """
	def __init__(self, obj):
		ObjectUI.__init__(self, obj)
		
		self.editorPanel.title = "Mesh Export Settings:"			
		self.editorPanel.addElement(ui.Label(10, 25, "Assigned Material", "Assigned Material:", self.editorPanel, False))
		
		self.materialButton = ui.Button(10, 45, 180, 65, "Material", "None Selected", 'normal', self.editorPanel, True)
		self.materialButton.registerCallback("release", self.showMaterialSelector)
		self.materialButton.textlocation = 1
		
		self.editorPanel.addElement(ui.Label(220, 25, "Output Type:", "Output Type:", self.editorPanel, False))
		self.mesh_output_menu = ui.Menu(225 + self.editorPanel.get_string_width("Output Type:", 'normal'), 25, 150, 25, "Output Menu", self.object_output_options, self.editorPanel, True)
		
		self.mesh_output_type_label = ui.Label(220, 60, "Mesh Type:", "Mesh Type:",self.editorPanel, True)
		self.mesh_output_type_menu = ui.Menu(self.mesh_output_menu.x, 60, 150, 25, "Output Mesh Type:",  ["Subdivision Surface", "RiPointsPolygons"],  self.editorPanel, True)
		
		self.exportButton = ui.Button(self.editorPanel.width - 185, self.editorPanel.height - 25, 180, 25, "Export", "Export Object", 'normal', self.editorPanel, True)
		self.exportButton.registerCallback("release", self.showExport)
		
		self.exportSettings = btor.BtoRMain.ExportSettings()
		self.exportSettings.export_functions.append(self.objecteditor.exportSingleObject) # this should do the trick, but should apply to every object that's exportable standalone
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
		
	def selectMaterial(self, obj):
		""" material selection callback """
		self.evt_manager.removeElement(self.mat_selector)
		# the button returned has the material name!
		# So all i need to do now is...
		self.material = obj.title
		# self.scene.object_data[self.objectName.getValue()]["material"] = obj.title # Assign the material to the object adapter
		# print "Assigned material ", self.scene.object_data[self.objectName.getValue()]["material"], " to ", self.objectName.getValue()
		self.materialButton.title = obj.title
		self.materialButton.image = ui.Image(120, 5, 56, 56,  obj.image.image, self.materialButton, False)
		
	def showExport(self, obj):
		""" Display the mesh object export dialog. """
		self.evt_manager.addElement(self.exportSettings.getEditor())
		

		

class LampUI(ObjectUI):
	light_output_options = ["LightSource", "AreaLightSource"]
	protocols.advise(instancesProvide=[IObjectUI], asAdapterForTypes=[BtoRLamp])
	def __init__(self, obj):
		ObjectUI.__init__(self, obj)
		self.editorPanel.title = "Light Export Settings:"		
		self.editorPanel.addElement(ui.Label(10, 25, "Light Type:", "Light Type:", self.editorPanel, False))
		self.light_output_menu = ui.Menu(15 + self.editorPanel.get_string_width("Light Type:", 'normal'), 25, 150, 25, "Light Menu", self.light_output_options, self.editorPanel, True)
		self.editorPanel.addElement(ui.Label(10, 65, "Light Shader:", "Light Shader:", self.editorPanel, False))
		self.shaderButton = ui.Button(15 + self.editorPanel.get_string_width("Light Shader:", 'normal'), 65, 150, 25, "None Selected", "None Selected", 'normal', self.editorPanel, True)
		
		# self.shaderButton.registerCallback("release", self.showLightShader)
		
		
	def setShader(self, shader):
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
	def __init__(self, obj):
		ObjectUI.__init__(self, obj)		
		self.editorPanel.title = "Camera Export Settings:"
		
		self.editorPanel.addElement(ui.Label(10, 30, "Imager Shader:", "Imager Shader:", self.editorPanel, False))
		self.shaderButton = ui.Button(self.editorPanel.get_string_width("Imager Shader:", 'normal') + 15, 30, 150, 25, "Imager Shader", "None Selected", 'normal', self.editorPanel, True)		
		# self.shaderButton.registerCallback("release", self.shader.showEditor)
		self.editorPanel.addElement(ui.Label(10, 80, "Depth of Field:", "Depth of Field:", self.editorPanel, False))
		self.DoFbutton = ui.ToggleButton(10, 110, 80, 20, "Use DoF", "Use DoF", 'normal', self.editorPanel, True)
		self.DoFbutton.registerCallback("release", self.toggleDoF)
		# self.shader = GenericShader(cgkit.rmshader.RMShader(), "imager", self)
		
		# self.scene.imagers[obj.getName()] = self.imagerShader
		# self.scene.object_data[obj.getName()] = objData # not neccessary
		offset = self.DoFbutton.x + self.DoFbutton.width + 15
		self.fstop_label = ui.Label(offset, 110, "f-Stop:", "f-Stop:", self.editorPanel, True)
		self.fstop_label.isVisible = False
		
		offset = offset + self.editorPanel.get_string_width("f-Stop:", 'normal') + 10
		self.fstop = ui.TextField(offset, 110, 25, 20, "f-Stop", 22, self.editorPanel, True)		
		self.fstop.isVisible = False
		
		offset = offset +  30
		self.focallength_label = ui.Label(offset, 110, "Focal Length:", "Focal Length:", self.editorPanel, True)
		self.focallength_label.isVisible = False
		
		offset = offset + self.editorPanel.get_string_width("Focal Length:", 'normal') + 5
		self.focallength = ui.TextField(offset, 110, 25, 20, "Focal Length", 45, self.editorPanel, True)
		self.focallength.isVisible = False
		
		offset = offset + 30
		self.focaldistance_label = ui.Label(offset, 110, "Focal Distance:", "Focal Distance:", self.editorPanel, True)
		self.focaldistance_label.isVisible = False
		
		offset = offset + self.editorPanel.get_string_width("Focal Distance:", 'normal') + 5
		self.focaldistance = ui.TextField(offset, 110, 25, 20, "Focal Distance", 10, self.editorPanel, True)
		self.focaldistance.isVisible = False
		
	def setShader(self, shader):
		self.shader = shader
		self.shaderButton.registerCallback("release", self.shader.showEditor)
		
	def toggleDoF(self, button):		
		if button.getValue():
			self.fstop_label.isVisible = True
			self.fstop.isVisible = True
			self.focallength_label.isVisible = True
			self.focallength.isVisible = True
			self.focaldistance_label.isVisible = True
			self.focaldistance.isVisible = True
		else:
			self.fstop_label.isVisible = False
			self.fstop.isVisible = False
			self.focallength_label.isVisible = False
			self.focallength.isVisible = False
			self.focaldistance_label.isVisible = False
			self.focaldistance.isVisible = False
			
	def getDoF(self):
		if self.DoFbutton.getValue():
			dof = [self.fstop.getValue(), self.focallength.getValue(), self.focaldistance.getValue()]
			return dof
		else:
			return None
		
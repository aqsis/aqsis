####################################################################
# BtoR - Renderman Exporter for blenderman
# Author: Bobby Parker
# This is the main module for the BtoR, 
# that's responsible for loading data from 
# the environment, the blender Registry,
# and the XML data files for the BtoR
#####################################################################

import Blender
from Blender import Registry
from ShaderEditorGUI import *
import string
import os

# SCRIPT global state settings - (not to be confused with global settings for the renderer config)

# shader search path environment variable names
# someone who knows PRMan, AIR, etc. can add to this list
env_paths = ["SHADERS", "PIXIEHOME", "BMRTHOME", "AQSISHOME", "DELIGHT"] 

# script state - this steers the event processing so that I can easily determine which editor I should display
state = 3 # this is the "state control" variable, which steers the GUI generation code to the right screen
old_state = 1 # this is the control variable that's used by the save/discard/cancel dialog
new_state = 1 # this is another control variable that's used by the save/discard/cancel dialog
state_render = 1 # export and render
state_global = 2 # global settings, renderer choice, all that
state_shader = 3 # shader editor
state_object = 4 # object editor, assignment, etc.
state_scene = 5 # scene settings, resolution, lighting/gi
state_dialog = 6 # save/discard/cancel dialog (it's here, because it qualifies as a "screen"


# query system environment for all known renderer configuration info, e.g. SHADERS, PIXIEHOME, BMRTHOME, SHADERPATH, etc.
try:
	for x in env_paths:
		shaderspath = string.split(os.environ[x], ";")	
		shaderspath.append("Add New")
		globalshaderpathmenu = string.join(shaderspath, "|")
except:
	print "I failed"
# other startup tasks:
# load current configuration from the .blend file if it's there, 
# otherwise, try to load system-wide settings from somewhere I have yet to define
# probably some XML in the "script home" directory

# end startup tasks
				
#####################################################################
# master GUI routine
# This steers the GUI events around so that the correct GUI is drawn based on the values of the 
# state, new_state, and old_state variables. 
#####################################################################
def master_gui(): # master GUI constructor
	# create the persistent buttons		
	if state == state_render: # GUI state is set to render and export
		render_draw() # draw the render/export GUI
	if state == state_global: # state is the global settings screen
		global_draw() # draw the global settings GUI
	if state == state_shader: # state is the shader editor
		shader_draw() # draw the shader editor GUI
	if state == state_object: # state is the object editor
		object_draw() # draw the object editor GUI
	if state == state_scene: # state is the scene settings screen
		scene_draw() # draw the scene settings GUI	
	if state == state_dialog: # state is the confirm dialog
		dialog_draw() # draw the confirm dialog

######################################################################
# master event processor 
# This distributes the events received to the correct processor 
# processor routine based on the script's state variables
#
######################################################################
def master_event(evt, val): # master key/mouse event processor
	if state == state_render: # GUI state is set to render and export
		render_event(evt, val) # render event
	if state == state_global: # GUI state is set to global settings
		global_event(evt, val) # global settings event
	if state == state_shader: # GUI state is set to shader editor
		shader_event(evt, val) # shader editor event
	if state == state_object: # GUI state is set to object editor
		object_event(evt, val) # object editor event
	if state == state_scene: # GUI state is set to scene settings
		scene_event(evt, val) # scene settings event
	if state == state_dialog: # GUI state is set to confirm dialog
		dialog_event(evt, val) # confirm dialog event
		
def master_bevent(evt):
	if state == state_render: # GUI state is set to render and export
		render_bevent(evt) # render button event
	if state == state_global: # GUI state is set to global settings
		render_bevent(evt) # global settings button event
	if state == state_shader: # GUI state is set to shader editor
		shader_bevent(evt) # shader editor button event
	if state == state_object: # GUI state is set to object editor
		object_bevent(evt) # object editor button event
	if state == state_scene: # GUI state is set to scene settings
		scene_bevent(evt) # scene settings event
	if state == state_dialog: # GUI state is set to confirm dialog
		dialog_bevent(evt) # confirm dialog button event
		
Register(master_gui, master_event, master_bevent)		
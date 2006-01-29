####################################################################
# BtoR - Renderman Exporter for blenderman
# dialog.py module 
# Author: Bobby Parker
# Provides a common dialog for use
# by other modules
#####################################################################
import Blender
import Blender.Draw
import global_data

# message variables, this thing supports 5 lines of text
message_linea = "Do you want to save settings?"
message_lineb = ""
message_linec = ""
message_lined = ""
message_linee = ""

# save/discard/cancel dialog event constants
evt_save = 1
evt_discard = 2
evt_cancel = -1

def dialog_draw():
	
	glClearColor(0.502, 0.502, 0.502, 0.0)
	glClear(GL_COLOR_BUFFER_BIT)
	glColor3f(0.239, 0.329, 0.427)
	glRecti(64, 124, 455, 323)

	glColor3f(0.733, 0.737, 0.776)
	glRecti(72, 172, 447, 315)
	glColor3f(0.239, 0.329, 0.427)
	glRecti(64, 124, 455, 323)

	glColor3f(0.000, 0.000, 0.000)

	glRasterPos2i(88, 296)
	Text(message_linea)
	glRasterPos2i(88, 272)
	Text(message_lineb)
	glRasterPos2i(88, 248)
	Text(message_linec)
	glRasterPos2i(88, 224)
	Text(message_lined)		
	glRasterPos2i(88, 200)
	Text(message_linee)

	Button('Save', 1, 360, 140, 79, 23, '')
	Button('Discard', 2, 224, 140, 87, 23, '')
	Button('Cancel', 3, 96, 140, 87, 23, '')
	Register(master_draw, master_event, master_bevent)

def dialog_event(evt, val): # event dialog events
	global state, old_state
	if evt == ESCKEY:
		# CANCEL, do not exit from this screen. Esc is synonmymous with cancel button
		state = old_state	
	# is other processing neccessary?
	Register(master_draw, master_event, master_bevent) # re-register
	
def dialog_bevent(evt): # event dialog buttons.
	global state, old_state, state_shader, state_object, state_scene, state_global, new_state
	if evt == evt_save:
		# to add a new save state, add to this list of conditionals
		# check here to see what state we're actually in
		if old_state == state_global:
			saveGlobal()	
			state = new_state		
		if old_state == state_shader:
			saveShader()
			state = new_state
		if old_state == state_scene:			
			saveScene()
			state = new_state
		if old_state == state_object:
			saveObject() 			
			state = new_state			
	if evt == evt_discard:
		state = new_state		
		# discard and move to the new GUI state
	if evt == evt_cancel:
		# do nothing
		state = old_state		
		Redraw() # this should take care of the state stuff
		
# 
		
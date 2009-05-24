-- AqsisConfig.applescript
-- AqsisConfig

-- Aqsis
-- Copyright (C) 1997 - 2001, Paul C. Gregory
--
-- Contact: pgregory@aqsis.org
--
-- This library is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public
-- License as published by the Free Software Foundation; either
-- version 2 of the License, or (at your option) any later version.
--
-- This library is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
-- General Public License for more details.
--
-- You should have received a copy of the GNU General Public
-- License along with this library; if not, write to the Free Software
-- Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

(*
/** \file
		\brief Declares typedefs for the basic types.
		\author Paul C. Gregory (pgregory@aqsis.org)
*/
*)

--  Created by Tobias Sauerwein on 22.03.08.
--  Copyright 2008 Aqsis. All rights reserved.

global profile

on awake from nib theObject
	(*Get AQSIS_SHADERS_PATH from .bashrc*)
	set profile to "~/.profile"
	
	restore()
end awake from nib

on idle theObject
	(*Add your script here.*)
end idle

on should quit after last window closed theObject
	return true
end should quit after last window closed

on clicked theObject
	if the name of theObject is equal to "saveButton" then
		(*save profile settings*)
		
		tell application "Finder"
			set localProfile to (home as string) & ".profile"
			if not (exists file localProfile) then
				open for access file localProfile
				close access file localProfile
			end if
		end tell
		
		set aqsisPath to content of text field "aqsisPathField" of window "mainWindow"
		set shaderPath to content of text field "shaderPathField" of window "mainWindow"
		set displayPath to content of text field "displayPathField" of window "mainWindow"
		set systemPath to content of text field "systemPathField" of window "mainWindow"
		
		(*export $AQSIS path*)
		set search to do shell script "/usr/bin/sed -n 's/^export AQSIS=.* # added by Aqsis$/&/p'  " & profile
		if search is "" then
			set export to "/bin/echo 'export AQSIS=" & aqsisPath & " # added by Aqsis' >> " & profile
			do shell script export
		else
			set export to "/usr/bin/sed -i.backup 's%^" & search & "$%export AQSIS=" & aqsisPath & " # added by Aqsis%' " & profile
			do shell script export
		end if
		
		(*export $PATH*)
		set search to do shell script "/usr/bin/sed -n 's/^export PATH=.* # added by Aqsis$/&/p'  " & profile
		if search is "" then
			set export to "/bin/echo 'export PATH=" & systemPath & " # added by Aqsis' >> " & profile
			do shell script export
		else
			set export to "/usr/bin/sed -i.backup 's%^" & search & "$%export PATH=" & systemPath & " # added by Aqsis%' " & profile
			do shell script export
		end if
		
		(*export $AQSIS_SHADERS_PATH path*)
		set search to do shell script "/usr/bin/sed -n 's/^export AQSIS_SHADERS_PATH=.* # added by Aqsis$/&/p'  " & profile
		if search is "" then
			set export to "/bin/echo 'export AQSIS_SHADERS_PATH=" & shaderPath & " # added by Aqsis' >> " & profile
			do shell script export
		else
			set export to "/usr/bin/sed -i.backup 's%^" & search & "$%export AQSIS_SHADERS_PATH=" & shaderPath & " # added by Aqsis%' " & profile
			do shell script export
		end if
		
		(*export $AQSIS_DISPLAY_PATH path*)
		set search to do shell script "/usr/bin/sed -n 's/^export AQSIS_DISPLAY_PATH=.* # added by Aqsis$/&/p'  " & profile
		if search is "" then
			set export to "/bin/echo 'export AQSIS_DISPLAY_PATH=" & displayPath & " # added by Aqsis' >> " & profile
			do shell script export
		else
			set export to "/usr/bin/sed -i.backup 's%^" & search & "$%export AQSIS_DISPLAY_PATH=" & displayPath & " # added by Aqsis%' " & profile
			do shell script export
		end if
		
	else if the name of theObject is equal to "restoreButton" then
		(*restore settings*)
		restore()
		
	else if the name of theObject is equal to "defaultButton" then
		(*default settings*)
		set currentShaderPath to "$AQSIS/share/aqsis/shaders"
		set content of text field "shaderPathField" of window "mainWindow" to currentShaderPath
		
		set currentDisplayPath to "$AQSIS/lib"
		set content of text field "displayPathField" of window "mainWindow" to currentDisplayPath
		
	else if the name of theObject is equal to "cancelButton" then
		(*cancel*)
		quit
	end if
end clicked

on end editing theObject
	(*Add your script here.*)
end end editing

on restore()
	tell application "Finder"
		set localProfile to (home as string) & ".profile"
		if exists file localProfile then
			(*Get AQSIS_SHADERS_PATH from .profile*)
			set currentShaderPath to do shell script "/usr/bin/sed -n 's/^export AQSIS_SHADERS_PATH=//p' " & profile & " | /usr/bin/sed 's/# added by Aqsis$//'"
			(*Get AQSIS_DISPLAY_PATH from .profile*)
			set currentDisplayPath to do shell script "/usr/bin/sed -n 's/^export AQSIS_DISPLAY_PATH=//p' " & profile & " | /usr/bin/sed 's/# added by Aqsis$//'"
		else
			set currentShaderPath to ""
			set currentDisplayPath to ""
		end if
	end tell
	
	if currentShaderPath is "" then
		set currentShaderPath to "$AQSIS/shaders"
	end if
	set content of text field "shaderPathField" of window "mainWindow" to currentShaderPath
	
	if currentDisplayPath is "" then
		set currentDisplayPath to "$AQSIS/lib"
	end if
	set content of text field "displayPathField" of window "mainWindow" to currentDisplayPath
	
	(*Get AQSIS location*)
	tell application "Finder"
		set aqsisHome to (path to me as string) & "Contents:Resources:bin:aqsis"
		set currentAqsisPath to ""
		if exists file aqsisHome then
			set currentAqsisPath to POSIX path of (path to me) & "Contents/Resources"
		else
			display dialog "Aqsis could not be found. Please contact the Aqsis team."
			display dialog "This application will now quit."
			tell me
				quit
			end tell
		end if
	end tell
	
	set content of text field "aqsisPathField" of window "mainWindow" to currentAqsisPath
	
	(*Build PATH *)
	set content of text field "systemPathField" of window "mainWindow" to "$AQSIS/bin" & ":$PATH"
end restore
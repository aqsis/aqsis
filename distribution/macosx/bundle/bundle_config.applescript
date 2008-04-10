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
		\brief Exports the AQSISHOME to the users .profile.
		\author Tobias Sauerwein (tsauerwein@aqsis.org)
*/
*)

--  Created by Tobias Sauerwein on 09.04.08.
--  Copyright 2008 Aqsis. All rights reserved.


set profile to "~/.profile"

(*Get AQSIS location*)
tell application "Finder"
	set aqsisHome to (path to me as string) & "Contents:MacOS:aqsis"
	set aqsisPath to ""
	if exists file aqsisHome then
		set aqsisPath to quoted form of text 1 thru -2 of POSIX path of (path to me)
		tell application "Finder"
			set localProfile to (home as string) & ".profile"
			if not (exists file localProfile) then
				open for access file localProfile
				close access file localProfile
			end if
		end tell
		
		(*export $AQSISHOME path*)
		set search to do shell script "/usr/bin/sed -n 's/^export AQSISHOME=.* # added by Aqsis$/&/p'  " & profile
		if search is "" then
			set export to "/bin/echo 'export AQSISHOME=" & aqsisPath & " # added by Aqsis' >> " & profile
			do shell script export
		else
			set export to "/usr/bin/sed -i.backup 's%^" & search & "$%export AQSISHOME=" & aqsisPath & " # added by Aqsis%' " & profile
			do shell script export
		end if
		
		(*export $PATH*)
		set systemPath to "$AQSISHOME/Contents/MacOS:$PATH"
		set search to do shell script "/usr/bin/sed -n 's/^export PATH=.* # added by Aqsis$/&/p'  " & profile
		if search is "" then
			set export to "/bin/echo 'export PATH=" & systemPath & " # added by Aqsis' >> " & profile
			do shell script export
		else
			set export to "/usr/bin/sed -i.backup 's%^" & search & "$%export PATH=" & systemPath & " # added by Aqsis%' " & profile
			do shell script export
		end if
		
		ignoring application responses
			tell application "Finder"
				do shell script aqsisPath & "/Contents/MacOS/" & "eqsl"
			end tell
		end ignoring
		
	else
		display dialog "Aqsis could not be found. Please contact the Aqsis team."
		quit me
	end if
end tell



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
		\brief Exports the AQSISHOME to the users .bash_profile.
		\author Tobias Sauerwein (tsauerwein@aqsis.org)
*/
*)

--  Created by Tobias Sauerwein on 09.04.08.
--  Copyright 2008 Aqsis. All rights reserved.


set bash_profile to ".bash_profile"
set bashrc to ".bashrc"
set profile to ".profile"

set targetProfile to "~/"

set aqsisFound to false

(*Get AQSIS location*)
tell application "Finder"
	set aqsisHome to (path to me as string) & "Contents:MacOS:aqsis"
	set aqsisPath to ""
	if (exists file aqsisHome) then
		set aqsisPath to quoted form of text 1 thru -2 of POSIX path of (path to me)
		tell application "Finder"
			set localProfile to POSIX path of ((home as string) & bash_profile)
			set fileExists to my checkExistsFile(localProfile)
			if fileExists is not equal to 1 then
				set localProfile to POSIX path of ((home as string) & bashrc)
				set fileExists to my checkExistsFile(localProfile)
				if fileExists is not equal to 1 then
					set localProfile to POSIX path of ((home as string) & profile)
					set fileExists to my checkExistsFile(localProfile)
					if fileExists is not equal to 1 then
						do shell script ("touch " & localProfile)
					end if
				end if
			end if
			set targetProfile to localProfile
		end tell
		
		quit me
		
		(*export $AQSISHOME path*)
		set search to do shell script "/usr/bin/sed -n 's/^export AQSISHOME=.* # Entry managed by Aqsis Renderer$/&/p'  " & targetProfile
		if search is "" then
			set export to "/bin/echo 'export AQSISHOME=" & aqsisPath & " # Entry managed by Aqsis Renderer' >> " & targetProfile
			do shell script export
		else
			set export to "/usr/bin/sed -i.backup 's%^" & search & "$%export AQSISHOME=" & aqsisPath & " # Entry managed by Aqsis Renderer%' " & targetProfile
			do shell script export
		end if
		
		(*export $PATH*)
		set systemPath to "$AQSISHOME/Contents/MacOS:$PATH"
		set search to do shell script "/usr/bin/sed -n 's/^export PATH=.* # Entry managed by Aqsis Renderer$/&/p'  " & targetProfile
		if search is "" then
			set export to "/bin/echo 'export PATH=" & systemPath & " # Entry managed by Aqsis Renderer' >> " & targetProfile
			do shell script export
		else
			set export to "/usr/bin/sed -i.backup 's%^" & search & "$%export PATH=" & systemPath & " # Entry managed by Aqsis Renderer%' " & targetProfile
			do shell script export
		end if
		
		do shell script aqsisPath & "/Contents/MacOS/" & "eqsl &> /dev/null &"
		
	else
		display dialog "Aqsis Renderer could not be found. Please visit our website for further assistance or to report this issue.

 http://www.aqsis.org"
		quit me
	end if
end tell




on checkExistsFile(theFilename)
	set theTerminalCommand to "test -f " & theFilename & ";
testResult=$?;
if [[ $testResult = 0 ]]; then
echo '1'
elif [[ $testResult = 1 ]]; then
echo '0'
else
echo '-1'
fi"
	
	return (do shell script theTerminalCommand) as integer
end checkExistsFile
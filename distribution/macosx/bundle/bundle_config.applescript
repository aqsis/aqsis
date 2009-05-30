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
		\brief Aqsis launcher
		\author Tobias Sauerwein (tsauerwein@aqsis.org)
*/
*)

--  Created by Tobias Sauerwein on 29.04.09.
--  Copyright 2008 Aqsis. All rights reserved.

-- set up environment variables
set aqsisHome to (POSIX path of (path to me as string))
set aqsisPath to aqsisHome & "Contents/Resources/bin"

-- ask user how to use aqsis
set question to display dialog ("You are about to launch Aqsis. Do you want to use the shell or eqsl, our graphical frontend?") with icon alias (POSIX file aqsisHome & "Contents:Resources:Aqsis.icns" as string) buttons {"eqsl", "shell"} default button 2
set answer to button returned of question


if answer is "eqsl" then
	
	do shell script "PATH=\"" & aqsisPath & ":$PATH\" eqsl &> /dev/null &"
	
else if answer is "shell" then
	
	set AqsisVersion to (do shell script aqsisPath & "/aqsis -version | grep \"aqsis version\" | sed 's/.* \\([0-9][0-9]*.[0-9][0-9]*.[0-9][0-9]*\\) .*/\\1/'")
	
	tell application "Terminal"
		--activate
		do script with command "export PATH=\"" & aqsisPath & ":$PATH\"; export AQSISHOME=\"" & aqsisHome & "\"; export PS1=\"aqsis-" & AqsisVersion & " : \\W$ \";clear ; history -d $((HISTCMD-1))"
		
		tell window 1
			set frontmost to true
			set title displays custom title to true
			set custom title to "Aqsis"
		end tell
	end tell
	
end if

quit me
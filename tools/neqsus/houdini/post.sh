#!/bin/bash

# ***System paths***

echo "=== System Path(s) ==="
echo
while true
do
	if [ -e $HOUDINI_PATH/houdini/RIBtargets ]
	then
		cd $HOUDINI_PATH
		break
	else
		echo "Please enter the installation path of Houdini: "
	fi
	read $HOUDINI_PATH
done


# ***Compile shaders***

echo
echo
echo "=== Compiling Shader(s) ==="
echo
cd "houdini/ri_shaders"
aqsl *.sl

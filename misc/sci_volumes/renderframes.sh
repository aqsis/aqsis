#!/bin/bash
#
# This is a little utility script to render a rotating version of the data box,
# rotating about the vertical axis.  Quantities which need to be changed by the
# script are marked with ChangeMe_* tags in the source RIB file.

if [[ -z $1 ]] ; then
	echo "Usage: renderframes.sh inputFile.rib"
	exit 0
fi
inputFile=$1

for ((i=0;i<360;i+=1)) ; do
	frameName=$(printf 'frame%0.3d.tif' $i)
	sed -e '/ChangeMe_Phi/ s/Rotate [0-9]\+/Rotate '$i'/' \
	    -e '/ChangeMe_FrameName/ s/volume\.tif/'$frameName'/' \
		-e 's/Display.*piqsl.*$//' \
		$inputFile | aqsis -progress
	sleep 1
done

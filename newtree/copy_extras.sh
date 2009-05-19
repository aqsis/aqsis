#!/bin/bash

extrasLoc=$PWD/extras
sourceLoc=$PWD/../../newtree_src

cd $extrasLoc

for f in $(find -type f \! -name '*.swp') ; do 
	f=${f#./}
	if ln -s "$extrasLoc/$f" "$sourceLoc/$f" 2> /dev/null ; then
		echo ln "$sourceLoc/$f"
	fi
done

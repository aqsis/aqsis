rm -f *.tif
rm -f *.z
rm -f *.slx
rm -f *.map
for file in $(find shaders/ -name "*.sl" | sort); do ./aqsl $file; done
for file in $(find ribfiles/ -name "*.rib" | sort); do echo "Rendering $file ..."; ./aqsis $file; done

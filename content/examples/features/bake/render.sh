#!/bin/bash

# ***Compile shaders***

echo  "=== Compiling Shader(s) ==="
echo
aqsl "../../../shaders/imager/gradient.sl"
aqsl "../../../shaders/surface/expensive.sl"


# ***Render files***

echo 
echo
echo "=== Rendering File(s) ==="
echo
aqsis -progress "bakesphere.rib"


# ***Compile textures***

echo 
echo
echo "=== Compiling Texture(s) ==="
echo
teqser -wrap=periodic -filter=mitchell -width=2.0 -bake=128 "sphere.bake.bake" "sphere.bake.tex"


# ***Render files***

echo 
echo
echo "=== Rendering File(s) ==="
echo
aqsis -progress "sphere.rib"

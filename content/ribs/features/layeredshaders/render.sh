#!/bin/bash

# ***Compile textures***

echo "=== Compiling Texture(s) ==="
echo
teqser "grid.tif" "grid.tex"


# ***Compile shaders***

echo 
echo
echo  "=== Compiling Shader(s) ==="
echo
aqsl "texmap.sl"


# ***Render files***

echo 
echo
echo "=== Rendering File(s) ==="
echo
aqsis -progress "layered.rib"

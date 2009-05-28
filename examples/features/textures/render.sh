#!/bin/bash

# ***Compile textures***

echo "=== Compiling Texture(s) ==="
echo
teqser -wrap=periodic "grid.tif" "grid.tex"


# ***Compile shaders***

echo 
echo
echo  "=== Compiling Shader(s) ==="
echo
aqsl "../../../shaders/surface/sticky_texture.sl"


# ***Render files***

echo 
echo
echo "=== Rendering File(s) ==="
echo
aqsis -progress "sticky.rib"

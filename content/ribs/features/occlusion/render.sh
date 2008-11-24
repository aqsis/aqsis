#!/bin/bash

# ***Compile shaders***

echo  "=== Compiling Shader(s) ==="
echo
aqsl "envlight.sl"


# ***Render files***

echo 
echo
echo "=== Rendering File(s) ==="
echo
aqsis -progress "occlmap.rib"
aqsis -progress "simple.rib"
echo "=== Rendering using the envlight.ptc file ==="
aqsis -progress "simple_texture3d.rib"

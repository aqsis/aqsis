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

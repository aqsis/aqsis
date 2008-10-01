#!/bin/sh

# ***Compile shaders***

echo "=== Compiling Shader(s) ==="
echo
aqsl "fisheye_projection.sl"


# ***Render files***

echo
echo
echo "=== Rendering File(s) ==="
echo
aqsis -progress "envmap.rib"
aqsis -progress "fisheye.rib"
aqsis -progress "scene.rib"

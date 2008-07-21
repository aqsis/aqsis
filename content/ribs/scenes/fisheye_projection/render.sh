#!/bin/sh

echo "=== Compiling Shader(s) ==="
aqsl fisheye_projection.sl

echo
echo "=== Rendering File(s) ==="
echo "Rendering 6 cube environment faces ..."
aqsis -progress environment_faces.rib
echo "Rendering fisheye projection from centre ..."
aqsis -progress fisheye.rib
echo "Rendering external view of environment with reflective sphere ..."
aqsis -progress environment_scene.rib

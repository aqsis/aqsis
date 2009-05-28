#!/bin/bash

# ***Compile shaders***

echo  "=== Compiling Shader(s) ==="
echo
aqsl "../../../shaders/displacement/micro_bumps.sl"
aqsl "../../../shaders/surface/microscope.sl"


# ***Render files***

echo 
echo
echo "=== Rendering File(s) ==="
echo
aqsis -progress "microbe.rib"

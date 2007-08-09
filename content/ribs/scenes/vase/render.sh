#!/bin/bash

# ***Compile shaders***

echo "=== Compiling Shader(s) ==="
echo
aqsl "../../../shaders/displacement/dented.sl"
aqsl "../../../shaders/light/shadowspot.sl"


# ***Render files***

echo
echo
echo "=== Rendering File(s) ==="
echo
aqsis -progress "vase.rib"

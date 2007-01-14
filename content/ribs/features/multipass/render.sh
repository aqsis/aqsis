#!/bin/bash

# ***Compile shaders***

echo  "=== Compiling Shader(s) ==="
echo
aqsl "myval.sl"


# ***Render files***

echo 
echo
echo "=== Rendering File(s) ==="
echo
aqsis -progress "aov.rib"

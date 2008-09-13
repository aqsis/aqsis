#!/bin/bash

# ***Compile shaders***

echo "=== Compiling Shader(s) ==="
echo
cd `dirname $1`
aqsl $@

exit 0

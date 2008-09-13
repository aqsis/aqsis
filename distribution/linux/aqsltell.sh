#!/bin/bash

# ***Inspect shaders***

echo "=== Inspecting Shader(s) ==="
echo
cd `dirname $1`
aqsltell $@
echo
read -p "... done!"

exit 0

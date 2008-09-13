#!/bin/bash

# ***Render files***

echo "=== Rendering File(s) ==="
echo
cd `dirname $1`
aqsis -progress $@

exit 0

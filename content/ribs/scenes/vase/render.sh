#!/usr/bin/bash

echo 
echo
echo ***Render files***
echo
aqsis -progress --shaders=/usr/share/aqsis/shaders:/usr/share/aqsis/content/shaders/displacement:/usr/share/aqsis/content/shaders/light /usr/share/aqsis/content/ribs/scenes/vase/vase.rib


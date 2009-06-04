!insertmacro ConfigWrite

${StrRep} $R0 $INSTDIR "\" "/"
${ConfigWrite} "$INSTDIR\bin\aqsisrc" 'Option "defaultsearchpath" "string shader" ' '["$R0/shaders/displacement:$R0/shaders/imager:$R0/shaders/light:$R0/shaders/surface:$R0/shaders/volume"]' $R1
${ConfigWrite} "$INSTDIR\bin\aqsisrc" 'Option "defaultsearchpath" "string archive" ' '["$R0"]' $R2
${ConfigWrite} "$INSTDIR\bin\aqsisrc" 'Option "defaultsearchpath" "string texture" ' '["$R0"]' $R3
${ConfigWrite} "$INSTDIR\bin\aqsisrc" 'Option "defaultsearchpath" "string display" ' '["$R0/bin"]' $R4
${ConfigWrite} "$INSTDIR\bin\aqsisrc" 'Option "defaultsearchpath" "string procedural" ' '["$R0/bin"]' $R5
${ConfigWrite} "$INSTDIR\bin\aqsisrc" 'Option "defaultsearchpath" "string resource" ' '["$R0"]' $R6

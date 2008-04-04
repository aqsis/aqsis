#!/bin/sh

APPLICATION_NAME="Aqsis"
BUNDLE_NAME="$APPLICATION_NAME.app"
STAGING="${BUNDLEDIR}/staging"
BUNDLE="$STAGING/$BUNDLE_NAME"
CONTENTS="$BUNDLE/Contents"
RESOURCES="$CONTENTS/Resources"
FRAMEWORKS="$CONTENTS/Frameworks"
MACOS="$CONTENTS/MacOS"
SHADERDIR="$RESOURCES/shaders"
INCLUDEDIR="$RESOURCES/include"
CONTENTDIR="${CMAKE_SOURCE_DIR}/content"
SCRATCH="$STAGING/scratch"
DISK_IMAGE="$APPLICATION_NAME-${MAJOR}.${MINOR}.${BUILD}-${CMAKE_SYSTEM_PROCESSOR}.dmg"


### Purge files
echo "Purging old files..."
rm -rvf "$STAGING"
rm -vf "${BUNDLEDIR}/$DISK_IMAGE"

### Create structure
echo "Creating bundle structure..."
mkdir -p "$STAGING"
mkdir -p "$SCRATCH"
mkdir -p "$BUNDLE"
mkdir -p "$CONTENTS"
mkdir -p "$RESOURCES"
mkdir -p "$FRAMEWORKS"
mkdir -p "$MACOS"
mkdir -p "$SHADERDIR"
mkdir -p "$INCLUDEDIR"

### Copy bundle files
echo "Copying bundle files..."
touch "$CONTENTS/PkgInfo"
cp "${BUNDLEDIR}/Info.plist" "$CONTENTS/"
cp "${BUNDLEDIR}/"*.icns "$RESOURCES/"

### Copy aqsis files
echo "Copying aqsis files..."
/Developer/Tools/CpMac -r "${CMAKE_BINARY_DIR}/${BINDIR}" "$RESOURCES/${BINDIR}"
cp -r "${CMAKE_BINARY_DIR}/${LIBDIR}" "$RESOURCES/${LIBDIR}"
cp "${CMAKE_BINARY_DIR}/shaders/"*.slx "$SHADERDIR/"
cp "${CMAKE_SOURCE_DIR}/shaders/"*.sl "$SHADERDIR/"
cp "${CMAKE_SOURCE_DIR}/aqsistypes"/*.h "$INCLUDEDIR/"
cp "${CMAKE_SOURCE_DIR}/aqsistypes/posix"/*.h "$INCLUDEDIR/"
cp "${CMAKE_SOURCE_DIR}/renderer/ddmanager"/ndspy.h "$INCLUDEDIR/"
cp "${CMAKE_SOURCE_DIR}/rib/api"/ri.h "$INCLUDEDIR/"
cp "${CMAKE_BINARY_DIR}/rib/api"/ri.inl "$INCLUDEDIR/"
cp "${CMAKE_SOURCE_DIR}/shadercompiler/shadervm"/shadeop.h "$INCLUDEDIR/"
cp "${BUNDLEDIR}/aqsisrc" "$RESOURCES/${BINDIR}"

### Resolve external dependencies
echo "Resolving external dependencies..."
for folder in $( ls -R $RESOURCES | egrep '.+:$' | sed 's/\/\//\//' | sed 's/:/\//' ) "$FRAMEWORKS/"; do
	for file in $( ls $folder ); do
		#echo "Resolving dependencies for $file"
		for dep in $( otool -L $folder$file | grep dylib | grep opt | grep -v $BUNDLE_NAME | sed s/\(.*\)// ) $( otool -L $folder$file | grep dylib | grep fltk | grep -v $BUNDLE_NAME | sed s/\(.*\)// ); do
			bn=`basename $dep`
			#echo "  ==>>>  $file  needs  $bn  ( $dep )"
			if [ ! -e $FRAMEWORKS/$bn ]; then
				#echo "Processing $bn"
				cp -R $dep "$FRAMEWORKS"
				if [ -L $dep ]; then
				   	link=`ls -l $dep | cut -d">" -f2 | sed s/\ //`
				   	deref=`echo $dep | sed s/$bn/$link/`  		
				   	cp -R $deref "$FRAMEWORKS"
				fi
			fi
			install_name_tool -change $dep @executable_path/../../Frameworks/$bn $folder$file
		done
	done
done

### Update libs
echo "Updating lib names..."
for m in $( ls $FRAMEWORKS | grep dylib ); do
	#echo "Processing $m"
	install_name_tool -id @executable_path/../../Frameworks/$m $FRAMEWORKS/$m
done

for m in $( ls $RESOURCES/${LIBDIR} | grep dylib ); do
	#echo "Processing $m"
	install_name_tool -id @executable_path/../lib/$m $RESOURCES/${LIBDIR}/$m
done

### Resolving internal dependencies
echo "Resolving internal dependencies..."
for folder in $( ls -R $RESOURCES | egrep '.+:$' | sed 's/\/\//\//' | sed 's/:/\//' ); do
	PWD=`pwd`
	for file in $( ls $folder ); do
		#echo "Resolving dependencies for $file"
		for dep in $( otool -L $folder$file | grep dylib | grep ${CMAKE_BINARY_DIR} | sed 's/(.*)//' ); do
			bn=`basename $dep`
			install_name_tool -change $dep @executable_path/../lib/$bn $folder$file
			#echo "$dep      $bn" 
		done
	done
done

### Purge redundant content
echo "Purging redundant content..."
rm -rf $(find "$CONTENTS" -name ".svn")

### Create disk image (DMG)
echo "Creating disk image..."
SIZE=`expr 5 + \`du -s -k "$BUNDLE" | cut -f1\` + \`du -s -k "$CONTENTDIR" | cut -f1\` / 1000`

hdiutil create "$SCRATCH/$DISK_IMAGE" -volname "$APPLICATION_NAME" -megabytes $SIZE -type SPARSE -fs HFS+ 2>/dev/null >/dev/null

hdid "$SCRATCH/$DISK_IMAGE.sparseimage" 2>/dev/null >/dev/null

DEV=`mount | grep "Volumes/$APPLICATION_NAME" | cut -f1 -d" "`

ditto -rsrc $BUNDLE "/Volumes/$APPLICATION_NAME/$BUNDLE_NAME" #2>/dev/null >/dev/null
ditto -rsrc "$CONTENTDIR" "/Volumes/$APPLICATION_NAME/Examples" #2>/dev/null >/dev/null
ditto -rsrc "${CMAKE_SOURCE_DIR}"/README "/Volumes/$APPLICATION_NAME/README" #2>/dev/null >/dev/null

hdiutil detach "$DEV" 2>/dev/null >/dev/null

hdiutil convert "$SCRATCH/$DISK_IMAGE.sparseimage" -format UDZO -o "$DISK_IMAGE" -imagekey zlib-devel=9 2>/dev/null >/dev/null

if [ -e $DISK_IMAGE ]; then
	echo "$DISK_IMAGE successfully created!"
	exit
fi
echo "An error occurred!"

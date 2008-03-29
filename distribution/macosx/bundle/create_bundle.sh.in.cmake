#!/bin/sh

APPLICATION_NAME="Aqsis"
BUNDLE_NAME="$APPLICATION_NAME.app"
BUNDLE="${STAGING}/$BUNDLE_NAME"
CONTENTS="$BUNDLE/Contents"
RESOURCES="$CONTENTS/Resources"
FRAMEWORKS="$CONTENTS/Frameworks"
MACOS="$CONTENTS/MacOS"
SCRATCH="${STAGING}/scratch"
ARCH=`arch`
DISK_IMAGE="$APPLICATION_NAME-${MAJOR}.${MINOR}.${BUILD}-$ARCH.dmg"
AQSCONF="AqsisConfig"
SHADERDIR="shaders"


# Cleanup old files
echo "Cleaning up old files ..."
rm -rvf "${STAGING}"
rm -rvf "$DISK_IMAGE"
rm -rvf "$SCRATCH"


### Create structure
echo "Creating structure ..."
mkdir -p "${STAGING}"
mkdir -p "$SCRATCH"
mkdir -p "$BUNDLE"
mkdir -p "$CONTENTS"
mkdir -p "$RESOURCES"
mkdir -p "$FRAMEWORKS"
mkdir -p "$MACOS"
mkdir -p "$RESOURCES/$SHADERDIR"
mkdir -p "$RESOURCES/include"

### Create bundle files
echo "Creating bundle files ..."
touch "$CONTENTS/PkgInfo"
cp "Info.plist" "$CONTENTS/Info.plist" 
cp "Aqsis.icns" "$RESOURCES/Aqsis.icns"


### Copy aqsis files
echo "Copying aqsis files ..."
cp -r "${CMAKE_BINARY_DIR}/${BINDIR}" "$RESOURCES/${BINDIR}"
cp -r "${CMAKE_BINARY_DIR}/${LIBDIR}" "$RESOURCES/${LIBDIR}"
cp "${CMAKE_BINARY_DIR}/$SHADERDIR/"*.slx "${CMAKE_BINARY_DIR}/${BUNDLEDIR}/$RESOURCES/$SHADERDIR/"
cp "${CMAKE_SOURCE_DIR}/$SHADERDIR/"*.sl "${CMAKE_BINARY_DIR}/${BUNDLEDIR}/$RESOURCES/$SHADERDIR/"

### Copy includes
TARGET="${CMAKE_BINARY_DIR}/${BUNDLEDIR}/$RESOURCES/include/"
cp "${CMAKE_SOURCE_DIR}/aqsistypes"/aqsis_types.h 	$TARGET
cp "${CMAKE_SOURCE_DIR}/aqsistypes"/aqsis.h 			$TARGET
cp "${CMAKE_SOURCE_DIR}/aqsistypes"/aqsismath.h 		$TARGET
cp "${CMAKE_SOURCE_DIR}/aqsistypes"/bitvector.h 		$TARGET
cp "${CMAKE_SOURCE_DIR}/aqsistypes"/cellnoise.h 		$TARGET
cp "${CMAKE_SOURCE_DIR}/aqsistypes"/color.h 			$TARGET
cp "${CMAKE_SOURCE_DIR}/aqsistypes"/exception.h 		$TARGET
cp "${CMAKE_SOURCE_DIR}/aqsistypes"/file.h 			$TARGET
cp "${CMAKE_SOURCE_DIR}/aqsistypes"/list.h 			$TARGET
cp "${CMAKE_SOURCE_DIR}/aqsistypes"/logging_streambufs.h 	$TARGET
cp "${CMAKE_SOURCE_DIR}/aqsistypes"/logging.h 			$TARGET
cp "${CMAKE_SOURCE_DIR}/aqsistypes"/lowdiscrep.h 		$TARGET
cp "${CMAKE_SOURCE_DIR}/aqsistypes"/matrix.h 			$TARGET
cp "${CMAKE_SOURCE_DIR}/aqsistypes"/matrix2d.h 		$TARGET
cp "${CMAKE_SOURCE_DIR}/aqsistypes"/memorysentry.h 	$TARGET
cp "${CMAKE_SOURCE_DIR}/aqsistypes"/multitimer.h 		$TARGET
cp "${CMAKE_SOURCE_DIR}/aqsistypes"/noise.h 			$TARGET
cp "${CMAKE_SOURCE_DIR}/aqsistypes"/noise1234.h 		$TARGET
cp "${CMAKE_SOURCE_DIR}/aqsistypes"/plugins.h 		$TARGET
cp "${CMAKE_SOURCE_DIR}/aqsistypes"/pool.h 			$TARGET
cp "${CMAKE_SOURCE_DIR}/aqsistypes"/random.h 			$TARGET
cp "${CMAKE_SOURCE_DIR}/aqsistypes"/refcount.h 		$TARGET
cp "${CMAKE_SOURCE_DIR}/aqsistypes"/smartptr.h 		$TARGET
cp "${CMAKE_SOURCE_DIR}/aqsistypes"/socket.h 			$TARGET
cp "${CMAKE_SOURCE_DIR}/aqsistypes"/spline.h 			$TARGET
cp "${CMAKE_SOURCE_DIR}/aqsistypes"/sstring.h 		$TARGET
cp "${CMAKE_SOURCE_DIR}/aqsistypes"/vector2d.h 		$TARGET
cp "${CMAKE_SOURCE_DIR}/aqsistypes"/vector3d.h 		$TARGET
cp "${CMAKE_SOURCE_DIR}/aqsistypes"/vector4d.h 		$TARGET
cp "${CMAKE_SOURCE_DIR}/aqsistypes/posix"/aqsis_compiler.h 	$TARGET
cp "${CMAKE_SOURCE_DIR}/aqsistypes/posix"/multitimer_system.h 	$TARGET
cp "${CMAKE_SOURCE_DIR}/aqsistypes/posix"/socket_system.h 	$TARGET
cp "${CMAKE_SOURCE_DIR}/renderer/ddmanager"/ndspy.h 			$TARGET
cp "${CMAKE_SOURCE_DIR}/rib/api"/ri.h 				$TARGET
cp "${CMAKE_BINARY_DIR}/rib/api"/ri.inl 			$TARGET
cp "${CMAKE_SOURCE_DIR}/shadercompiler/shadervm"/shadeop.h 		$TARGET

### Resolve external dependencies
echo "Resolving external dependencies ..."
for folder in $( ls -R $RESOURCES | egrep '.+:$' | sed 's/\/\//\//' | sed 's/:/\//' ) "$FRAMEWORKS/"; do
	for file in $( ls $folder ); do
		#echo "Resolving dependencies for $file"
		for dep in $( otool -L $folder$file | grep dylib | grep opt | sed s/\(.*\)// ) $( otool -L $folder$file | grep dylib | grep fltk | sed 's/\(.*\)//' | sed 's/://' ); do
			bn=`basename $dep`
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
echo "Updating lib names ..."
for m in $( ls $FRAMEWORKS | grep dylib ); do
	#echo "Processing $m"
	FRMWRKDIR=`echo $FRAMEWORKS | sed "s/${STAGING}//" | sed s_/__`
	install_name_tool -id @executable_path/../../Frameworks/$m $FRAMEWORKS/$m
done

for m in $( ls $RESOURCES/lib | grep dylib ); do
	#echo "Processing $m"
	RESRCDIR=`echo $RESOURCES/lib | sed "s/${STAGING}//" | sed s_/__`
	install_name_tool -id @executable_path/../lib/$m $RESOURCES/lib/$m
done


### Resolving internal dependencies
echo "Resolving internal dependencies ..."
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


### Strip the binaries
echo "Stripping the binaries ..."
strip "$RESOURCES/bin/piqsl"
strip "$RESOURCES/bin/aqsis"
strip "$RESOURCES/bin/aqsl"
strip "$RESOURCES/bin/teqser"
strip "$RESOURCES/bin/miqser"
strip "$RESOURCES/bin/aqsltell"
strip "$RESOURCES/bin/eqsl"


### Update Piqsl and framebuffer
echo "Updating fltk resources ..."
#/Developer/Tools/CpMac "/opt/local/include/FL/mac.r" "${CMAKE_BINARY_DIR}/${BUNDLEDIR}/$FRAMEWORKS"
for folder in $( ls -R $RESOURCES | egrep '.+:$' | sed 's/\/\//\//' | sed 's/:/\//' ); do
	for file in $( ls $folder ); do
		if [ `otool -L $folder$file | grep dylib | grep fltk | sed 's/(.*)//'` ]; then
			/Developer/Tools/Rez -t APPL -o "$folder$file" /opt/local/include/FL/mac.r
		fi
	done
done
/Developer/Tools/Rez -t APPL -o "$RESOURCES/bin/aqsis" /opt/local/include/FL/mac.r


### Copy AqsisConfigurator
echo "Copying AqsisConfigurator ..."
TARGET="${CMAKE_BINARY_DIR}/${BUNDLEDIR}/$CONTENTS/"
SOURCE="${CMAKE_BINARY_DIR}/tools/AqsisConfig/Release/AqsisConfig.app/Contents/"
/Developer/Tools/CpMac -r "$SOURCE/Resources"/English.lproj 	"$TARGET/Resources/"
/Developer/Tools/CpMac -r "$SOURCE/Resources"/Scripts 			"$TARGET/Resources/"
/Developer/Tools/CpMac "$SOURCE/MacOS"/AqsisConfig 			"$TARGET/MacOS/AqsisConfig"


### Replace aqsisrc
echo "Adding aqsisrc ..."
cp "${CMAKE_CURRENT_SOURCE_DIR}/aqsisrc" "$RESOURCES/bin/aqsisrc"


# Get rid of those pesky .svn directories ...
echo "Cleaning up directories ..."
rm -rf $(find "$CONTENTS" -name ".svn")


### Create a nice DMG
echo "Creating disk image ..."
SIZE=`expr 5 + \`du -s -k "$BUNDLE" | cut -f1\` + \`du -s -k "${CMAKE_SOURCE_DIR}/content" | cut -f1\` / 1000`

hdiutil create "$SCRATCH/$DISK_IMAGE" -volname "$APPLICATION_NAME" -megabytes $SIZE -type SPARSE -fs HFS+ 2>/dev/null >/dev/null

hdid "$SCRATCH/$DISK_IMAGE.sparseimage" 2>/dev/null >/dev/null

DEV=`mount | grep "Volumes/$APPLICATION_NAME" | cut -f1 -d" "`

ditto -rsrc $BUNDLE "/Volumes/$APPLICATION_NAME/$BUNDLE_NAME" #2>/dev/null >/dev/null
ditto -rsrc "${CMAKE_SOURCE_DIR}"/content "/Volumes/$APPLICATION_NAME/Examples" #2>/dev/null >/dev/null
ditto -rsrc "${CMAKE_SOURCE_DIR}"/README "/Volumes/$APPLICATION_NAME/README" #2>/dev/null >/dev/null

hdiutil detach "$DEV" 2>/dev/null >/dev/null

hdiutil convert "$SCRATCH/$DISK_IMAGE.sparseimage" -format UDZO -o "$DISK_IMAGE" -imagekey zlib-devel=9 2>/dev/null >/dev/null

if [ -e $DISK_IMAGE ]; then
	echo "$DISK_IMAGE successfully created."
	exit
fi
echo "An error occurred."

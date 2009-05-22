#!/usr/bin/python

import os
import sys
import re
import shutil
import fileinput


class FileSourceGuesser:
	'''
	Guess the location of a source file
	'''
	def __init__(self, fileList):
		self.fileMap = self.buildFileNameMap(fileList)

	def buildFileNameMap(self, fileList):
		fileNameMap = dict()
		for file in fileList:
			baseName = os.path.basename(file)
			if fileNameMap.has_key(baseName):
				fileNameMap[baseName].append(file)
			else:
				fileNameMap[baseName] = [file]
		return fileNameMap

	def find(self, fileName, hintName):
		'''
		Guess the desired file from the source tree location.
		'''
		if hintName is None:
			baseName = os.path.basename(fileName)
		else:
			baseName = os.path.basename(hintName)
		if not self.fileMap.has_key(baseName):
			return None
		candidates = self.fileMap[baseName]
		if len(candidates) == 1:
			return candidates[0]
		else:
			if hintName is not None:
				hintDir = os.path.basename(os.path.dirname(hintName))
			else:
				hintDir = None
			if hintDir is None:
				parentDir = os.path.basename(os.path.dirname(fileName))
			else:
				parentDir = hintDir

			for sourceFile in candidates:
				if sourceFile.find(parentDir) != -1:
					return sourceFile
			return None



def findFiles(dirName, matcher = None, excludeDirs = None):
	'''
	Find all files files in the given directory for which pattern(fileName)
	returns true, excluding anything in a .git directory.
	'''
	files = []
	for (dir, dirNames, fileNames) in os.walk(dirName):
		if excludeDirs is not None:
			excludeNames = []
			for dirName in dirNames:
				if os.path.basename(dirName) in excludeDirs:
					excludeNames.append(dirName)
			for exDir in excludeNames:
				dirNames.remove(exDir)
		if matcher is None:
			files += [os.path.join(dir, f) for f in fileNames]
		else:
			files += filter(matcher, [os.path.join(dir, f) for f in fileNames])
	return files



def parseDestSpec(fileName):
	'''
	Parse the specification of the destination tree, and return a list of files
	and a list of directories to be created in the new tree.
	'''
	lineFormat = re.compile(r'( *)(\w[-~\w.]*)(/?) *(?:<-- *([-/\w.]+))?(?:#.*)?$')
	specFile = open(fileName)
	prevLevel = 0
	prevWasDir = False
	dirStack = []
	destFileList = []
	destDirList = []
	for line in specFile.readlines():
		m = lineFormat.match(line)
		if m:
			indent = len(m.group(1))
			if indent % 4 != 0:
				raise 'Inconsistent indentation at "%s"' % (line,)
			level = indent/4
			name = m.group(2)
			isDir = (m.group(3) == '/')
			hint = m.group(4)
			#print dirStack, name
			#print 'level = %d, name = %s, isDir = %d' % (level, name, isDir)
			if prevLevel != level:
				if not (prevWasDir or isDir) or not level <= prevLevel + 1:
					raise 'Inconsistent indentation change at %s' % (line,)
				dirStack = dirStack[:level]
			if isDir:
				if prevLevel == level and prevWasDir:
					dirStack = dirStack[:level]
				dirStack.append(name)
				destDirList.append(os.path.join(*dirStack))
			else:
				destFileList.append((os.path.join(*(dirStack + [name])), hint))
			prevLevel = level
			prevWasDir = isDir
		else:
			sys.stderr.write('IGNORE: %s\n' % (line.strip(),))
			pass
	return (destFileList, destDirList)


#------------------------------------------------------------------------------
# MAIN PROGRAM
#------------------------------------------------------------------------------
# Delete the dest directory and recreate
destDir = '../../newtree_src'
sourceDir = '..'
try:
	shutil.rmtree(destDir)
except:
	pass
os.mkdir(destDir)


#------------------------------------------------------------------------------
# Create the directory structure.
(destFileList, destDirList) = parseDestSpec('new_spec.txt')
for dir in destDirList:
	newDir = os.path.join(destDir, dir)
	os.mkdir(newDir)

# Copy over the files
guesser = FileSourceGuesser(findFiles(sourceDir, excludeDirs = ['.git', 'newtree']))
publicHeaderList = []
for (destFileName, hintName) in destFileList:
	sourceFile = guesser.find(destFileName, hintName)
	if sourceFile is None:
		raise 'FILE NOT FOUND: %s\n' % (destFileName,)
	destFile = os.path.join(destDir, destFileName)
	shutil.copy(sourceFile, destFile)
	# Save the mapping from source to dest for all library public headers.
	if destFile.endswith(".h") and destFile.find("include") != -1:
		publicHeaderList.append((sourceFile, destFile))


#------------------------------------------------------------------------------
# Modify all headers in the project to correctly #include public headers by
# the full path.

# First construct a dictionary mapping from "X.h" the appropriate
# "aqsis/.../X.h"
headerNameMap = dict()
for (sourceFile, destFile) in publicHeaderList:
	orig_h_name = os.path.basename(sourceFile)
	new_h_name = destFile[destFile.find('include/')+len('include/'):]
	if headerNameMap.has_key(orig_h_name):
		raise 'Argh, ambiguous header name %s' % (orig_h_name,)
	headerNameMap[orig_h_name] = new_h_name

# Add a special case for aqsis_config.h which comes from a generated file,
# aqsis_config.h.in.cmake, and version.h which comes from aqsis_version.h
headerNameMap['aqsis_config.h'] = 'aqsis/config.h'
headerNameMap['version.h'] = 'aqsis/version.h'
headerNameMap['ri.inl'] = 'aqsis/ri/ri.inl'

# Construct a regex to match offending #include lines
hPattern = re.compile(r'#\s*include\s+[<"](%s)[">]' % ('|'.join(headerNameMap.keys()),))
#badPattern = re.compile('^\s*#\s*include\s+<(%s)>' % ('|'.join(headerNameMap.keys()),))

# Now modify any lines in the file.
allSource = findFiles(destDir, re.compile(r'\.(h|cpp|c|yy|ll|fl)$').search)
for line in fileinput.input(allSource, inplace=True):
	m = hPattern.search(line)
	if m:
		origName = m.group(1)
		sys.stdout.write(re.sub('[<"]%s[">]' % (origName,),
					'<%s>' % (headerNameMap[origName],), line, 1))
	else:
		sys.stdout.write(line)


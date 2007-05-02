#!/usr/bin/python
#
# Scientific python script to test image filtering for mipmap creation in Aqsis
# Copyright (C) 2007, Christopher J. Foster
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

from __future__ import division

import matplotlib.pylab as pylab

import numpy
from numpy import pi, r_, size, zeros, ones, ceil, floor, array, linspace, meshgrid

import scipy
from scipy import cos
from scipy.signal import boxcar, convolve2d as conv2
from scipy.misc import imsave, imread

# The purpose of this script is to investigate mipmap generation and indexing
# in more-or-less the same way which it will be used internally by Aqsis.

#----------------------------------------------------------------------
#----------------------------------------------------------------------
# Filter Kernels, including any windowing.
#----------------------------------------------------------------------
def sincKer(width, even, scale = 2):
	'''
	Return a sinc filter kernel with specified width and scale
	
	width - width of the filter (used by the window)
	even - true if the generated filter should have an even number of points.
	scale - zeros will be centered around 0 with spacing "scale"
	'''
	if even:
		numPts = max(2*int((width+1)/2), 2)
	else:
		numPts = max(2*int(width/2) + 1, 3)
	x = r_[0:numPts]-(numPts-1)*0.5
	k = scipy.sinc(x/scale)
	k *= cos(linspace(-pi/4,pi/4,len(k)))
	ker = numpy.outer(k,k)
	ker /= sum(sum(ker))
	return ker


#----------------------------------------------------------------------
# Image manipuation
#----------------------------------------------------------------------
def seperableConv2(A, k, *args):
	'''
	perform a 2D convolution, assuming that the convolution kernel k is
	square-seperable.  In this case we may just do two 1D convolutions.
	'''
	center = array(k.shape)//2
	k0 = k[center[0]:center[0]+1,:]
	k1 = k[:,center[1]:center[1]+1]
	return conv2(conv2(A,k0, *args),k1, *args)


#----------------------------------------------------------------------
def imConv(im, ker, conv2Func=conv2):
	'''
	Convolve an image with the given filter kernel
	'''
	res = zeros(im.shape)
	normalisation = conv2Func(ones(im.shape[0:2],'d'), ker, 'full')
	# compute how much of the full convolution to trim from top left and bottom right
	trimTl = numpy.int_(ceil((array(ker.shape)-1)/2))
	trimBr = numpy.int_(floor((array(ker.shape)-1)/2))
	for ii in range(0,size(im,2)):
		fullconv = conv2Func(im[:,:,ii], ker, 'full') / normalisation
		res[:,:,ii] = fullconv[trimTl[0]:-trimBr[0], trimTl[1]:-trimBr[1]]
	return res


#----------------------------------------------------------------------
def imDownsamp(im, ker, skip):
	'''
	Downsample an image by
	(1) Applying the filter kernel ker at each pixel
	(2) Decimate by taking elements 0::skip from the result
	(3) Clamp the resulting intensities between 0 and 1 
	'''
	res = imConv(im,ker)
	#res = imConv(im,ker,seperableConv2)
	res = res[(skip-1)//2::skip, (skip-1)//2::skip, :]
	res = imClamp(res)
	return res


#----------------------------------------------------------------------
def imClamp(im):
	'''
	Clamp an image between zero and one.
	'''
	return numpy.minimum(numpy.maximum(im,0),1)


#----------------------------------------------------------------------
#----------------------------------------------------------------------
# Functions for creating test images
#----------------------------------------------------------------------
def getGridImg(N, gridSize):
	'''
	Get a test image containig a one pixel wide grid 
	
	N - side length
	gridSize - size of the one-pixel wide black grid in the image.
	'''

	im = zeros((N, N, 3))

	x = linspace(0,1,N)
	[xx,yy] = meshgrid(x,x)
	im[:,:,0] = xx
	im[:,:,1] = 0.5
	im[:,:,2] = yy

	im[0:-gridSize-1:gridSize,:,:] = 0
	im[:,0:-gridSize-1:gridSize,:] = 0

	im[:,-1,:] = 0
	im[-1,:,:] = 0

	return im


#----------------------------------------------------------------------
#----------------------------------------------------------------------
# Utils for creating/manipulating mipmaps
#----------------------------------------------------------------------

#----------------------------------------------------------------------
class MipLevel:
	'''
	Class to hold a level of mipmap.
	'''
	def __init__(self, data, sOffset, tOffset):
		self.data = data
		self.sOffset = sOffset
		self.tOffset = tOffset

	def sample(self, s, t):
		'''
		Sample the mipmap level at the coordinates specified by i and j (may be
		arrays)
		'''
		


#----------------------------------------------------------------------
# Musings on methods of creating and accessing mipmaps
#
# Consider a mipmap. Let:
# t = texture coordinates; 0 <= t <= 1.
# l = mipmap level
# 
# Consider the following variables:
# i_l(t) = raster coordinates for level l as a function of texture coordinate t.
# o_l = offset for level l in level 0 units
# w_l = width of level l in level 0 units
# s_l = size of level l in pixels.
# 
# Example mipmap, created with my current scheme:
#     0   1   2   3   4  <-- i_0
# l   |   |   |   |   |   o_l  w_l  s_l
# 0  [x   x   x   x   x]  0    4    5
# 1  [x       x       x]  0    2    3
# 2  [x               x]  0    1    2
# 3  [        x        ]  2    0    1
# 
# Another example (powers of two)
#     0   1   2   3   4   5   6   7 <-- i_0
# l   |   |   |   |   |   |   |   |   o_l   w_l  s_l
# 0  [x   x   x   x   x   x   x   x]  0     7    8
# 1  [  x       x       x       x  ]  0.5   6    4
# 2  [      x               x      ]  1.5   4    2
# 3  [              x              ]  3.5   0    1
# 
# Formula for texture coordinates -> raster space
# 
# w_l = (w_0 - 2*o_l)
# 
# Raster units for level 0 are:
# i_0(t) = t * w_0
# Raster units for level l are:
# i_l(t) = (i_0 - o_0)/w_l
# 
# Or, what we really need, which is raster units for level l in terms of
# the texture coordinate t:
# i_l(t) = (t * w_0 - o_0)/w_l
# 
# 
# Lesson: odd texture sizes make it easy to index; even sizes are hard.
# 
# 
# Abandoning the constant-coefficients approach (makes it very slow to generate
# mipmaps), we would have:
# 
#     0   1   2   3   4   5   6   7 <-- i_0
# l   |   |   |   |   |   |   |   |   o_l   w_l  s_l
# 0  [x   x   x   x   x   x   x   x]  0     7    8
# 1  [x        x         x        x]  0     7    4
# 2  [x                           x]  0     7    2
# 3  [              x              ]  0     7    1
# 
# The the raster units for level l in terms of the texture coordinates t
# are then very simple:
# 
# i_l(t) = t*(s_l - 1)

#----------------------------------------------------------------------
# New test implementation of mipmapping
class MipMap:
	'''
	Class to encapsulate a (power-of-2) mipmapped texture in much the same way
	mipmapping will be done internally in Aqsis.
	'''
	def __init__(self, img, filterWidth, kerFunc=sincKer):
		self.filterWidth = filterWidth
		self.kerFunc = kerFunc
		self.genMipMap(img)

	def genMipMap(self, img):
		self.levels = []

		scale = 2
		self.levels.append(img)
		while size(img,0) > 1:
			ker = self.kerFunc(self.filterWidth, (size(img,0) % 2) == 0, scale)
			img = imgDownsamp(img, ker, scale)
			self.levels.append(img)
			print 'generated level:  side length = %d,  number of points in filter = %d' \
				% (size(img,0), len(ker))

	def getLevelSizes(imgWidth):
		'''
		Get the desired widths of a mipmap as a list

		The widths result from the process as follows (x's represent sample points)
		Even number of points:
			[x x x x]
			->
			[ x   x ]
		Odd number of points:
			[x x x x x]
			->
			[x   x   x]
		'''
		widths = [imgWidth]
		while imgWidth > 1:
			imgWidth = (imgWidth+1)/2
			widths.append(imgWidth)
		return widths

	def sampleTrilinera(self):
		'''
		Implement trilinear sampling
		'''
		pass

#----------------------------------------------------------------------
def flattenImageList(mipmap, dim):
	'''
	"Flatten" mipmap levels into a single image.
	
	mipmap - list of scaled images
	dim - dimension number to lay the images out along
	'''

	len = 0
	for mipLevel in mipmap:
		len += size(mipLevel,dim)
	mipSize = list(mipmap[0].shape)
	mipSize[dim] = len

	mipmapImg = ones(mipSize)*0.7
	pos = 0
	for mipLevel in mipmap:
		if dim == 0:
			mipmapImg[pos:pos+size(mipLevel,0), 0:size(mipLevel,1), :] = mipLevel
		else:
			mipmapImg[0:size(mipLevel,0), pos:pos+size(mipLevel,1), :] = mipLevel
		pos += size(mipLevel,0)

	return mipmapImg


#----------------------------------------------------------------------
# High level driver functions.
#----------------------------------------------------------------------

#----------------------------------------------------------------------
# Main program
#----------------------------------------------------------------------

if __name__ == '__main__':
	# Choose image
	#im = imread('testbox.png')[:,:,:3]/256.0
	#mipmap = genMipmapIter(im, 7, True, boxcar, sincKer)
	#pylab.imshow(mipAll)
	#imsave('mipmap.png', mipAll)
	pass

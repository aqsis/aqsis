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
from scipy import cos, ogrid, ndimage
from scipy.signal import boxcar, convolve2d as conv2
# Use ndimage.convolve!
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
	k *= cos(linspace(-pi/2,pi/2,len(k)))
	ker = numpy.outer(k,k)
	ker /= sum(ker.ravel())
	return ker


#----------------------------------------------------------------------
# Image manipuation
#----------------------------------------------------------------------
def applyToChannels(img, fxn, *args, **kwargs):
	'''
	Apply the given function to each of the channels in img in turn.
	Additional positional and keyword arguments are passed to the called fxn.
	'''
	numChannels = img.shape[-1]
	outList = []
	for i in range(0,numChannels):
		outList.append(fxn(img[:,:,i], *args, **kwargs))
	return array(outList).transpose([1,2,0])


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
# Musings on methods of creating and accessing mipmaps
#
# Consider a mipmap. Let:
# t = texture coordinates; 0 <= t <= 1.
# l = mipmap level
# 
# Consider the following variables:
# i_l(t) = raster coordinates for level l as a function of texture coordinate t.
# o_l = offset for level l in level 0 units
# s_l = span of level l in level 0 units
# d_l = interval between pixels of level l in level 0 raster units.
# w_l = width of level l in pixels.
# 
# Example mipmap, created with my current scheme:
#     0   1   2   3   4  <-- i_0
# l   |   |   |   |   |   o_l  s_l  w_l  d_l
# 0  [x   x   x   x   x]  0    4    5    1
# 1  [x       x       x]  0    2    3    2
# 2  [x               x]  0    1    2    4
# 3  [        x        ]  2    0    1    ?
# 
# Another example (powers of two)
#     0   1   2   3   4   5   6   7 <-- i_0
# l   |   |   |   |   |   |   |   |   o_l   s_l  w_l
# 0  [x   x   x   x   x   x   x   x]  0     7    8
# 1  [  x       x       x       x  ]  0.5   6    4
# 2  [      x               x      ]  1.5   4    2
# 3  [              x              ]  3.5   0    1
# 
# Formulas for texture coordinates -> raster space
# 
# s_l = (s_0 - 2*o_l)
# 
# Raster units for level 0 are:
# i_0(t) = t * s_0
# Raster units for level l are:
# i_l(t) = (i_0(t) - o_0)/d_l
# 
# Or, what we really need, which is raster units for level l in terms of
# the texture coordinate t:
# i_l(t) = t * s_0/d_l - o_0/d_l
# 
# 
# Lesson: odd texture sizes make it easy to index; even sizes are hard.
# 
# 
# Abandoning the constant-coefficients approach (makes it very slow to generate
# mipmaps), we would have:
# 
#     0   1   2   3   4   5   6   7 <-- i_0
# l   |   |   |   |   |   |   |   |   o_l   s_l  w_l
# 0  [x   x   x   x   x   x   x   x]  0     7    8
# 1  [x        x         x        x]  0     7    4
# 2  [x                           x]  0     7    2
# 3  [              x              ]  0     7    1
# 
# The the raster units for level l in terms of the texture coordinates t
# are then very simple:
# 
# i_l(t) = t*(w_l - 1)


#----------------------------------------------------------------------
class ImageDownsampler:
	'''
	Class which holds a filter kernel (function & width) and is able to
	correctly use it to downsample odd or even-sized images.
	'''
	def __init__(self, kerWidth, kerFunc=sincKer):
		self.kerFunc = kerFunc
		self.kerWidth = kerWidth
	def downsample(self, img):
		'''
		Downsample an image by
		(1) Applying the filter kernel at each pixel
		(2) Decimate by taking elements 0::2 from the result
		(3) Clamp the resulting intensities between 0 and 1 
		'''
		ker = self.kerFunc(self.kerWidth, (size(img,0) % 2) == 0)
		res = applyToChannels(img, ndimage.correlate, ker, mode='constant', cval=0)
		res = res[(1-(size(img,0) % 2))::2, (1-(size(img,1) % 2))::2, :]
		res = res.clip(min=0, max=1)
		return res

#----------------------------------------------------------------------
class MipLevel:
	'''
	Class to hold a level of mipmap, which knows how to bilinear interpolation
	on itself.
	'''
	def __init__(self, image, offset, mult):
		'''
		image - pixel data; a NxMx3 array
		offset, mult - length two vectors which specify an affine
			transformation from texture coordinates to this mipmap's raster
			coordinates:
			i = mult[0] * s + offset[0]
			j = mult[1] * t + offset[1]
		'''
		self.image = image
		self.offset = offset.copy()
		self.mult = mult

	def sample(self, s, t):
		'''
		Sample the mipmap level at the coordinates specified by s and t.  For
		performance, s & t should be 1D arrays.
		'''
		i = self.mult[0]*s + self.offset[0]
		j = self.mult[1]*t + self.offset[1]
		return applyToChannels(self.image, ndimage.map_coordinates, \
				array([i,j]), order=1, mode='constant', cval=0)

	def display(self):
		pylab.imshow(self.image, interpolation='nearest')


#----------------------------------------------------------------------
class MipMap:
	'''
	Class to encapsulate a (power-of-2) mipmapped texture in much the same way
	mipmapping will be done internally in Aqsis.
	'''
	def __init__(self, img, filterWidth, kerFunc=sincKer):
		self.downsampler = ImageDownsampler(filterWidth, kerFunc)
		self.levels = []
		self.genMipMap(img)

	def genMipMap(self, img):
		scale = 2
		level0Span = array(img.shape[0:2])-1
		offset = array([0,0],'d')
		self.levels = [MipLevel(img, offset, level0Span)]
		# delta is distance between pixels; level 0 units
		delta = 1
		# offsets are between 1st pixel of 0th level to 1st pixel of current level
		#
		# eg, in 1D
		#     [x   x   x   x..]
		#          <--->          <- level 0 delta
		#     [  x       x ...]
		#      <->                <- level 1 offset
		while (array(img.shape[0:2]) > 1).all():
			# calculate start offsets for this level.
			offset += (1-(array(img.shape[0:2]) % 2))*delta/2
			delta *= 2
			# downsample
			img = self.downsampler.downsample(img)
			# create a new MipLevel to hold the image.
			self.levels.append(MipLevel(img, -offset/delta, level0Span/delta))
			print 'generated level size', img.shape[0:2]

	def sampleTrilinear(self):
		'''
		Trilinear sampling
		'''
		pass

	def displayLevel(self, level):
		self.levels[level].display()


#----------------------------------------------------------------------
# Main program
#----------------------------------------------------------------------

if __name__ == '__main__':
	# Choose image
	im = imread('lena_std.png')[:,:,:3]/256.0
	mipmap = MipMap(im, 8)

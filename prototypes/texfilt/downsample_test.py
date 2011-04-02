#!/usr/bin/python
#
# Scientific python script to test image filtering for mipmap creation in Aqsis
# Copyright (C) 2007, Christopher J. Foster
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice,
#   this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
# * Neither the name of the software's owners nor the names of its
#   contributors may be used to endorse or promote products derived from this
#   software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# (This is the New BSD license)

# This script investigates some of the factors important when downsampling an
# image:
#
# * Filter types
# * Filter windows
# * Correctly filtering odd and even-sized images

from __future__ import division

import matplotlib.pylab as pylab

import numpy
from numpy import r_, size, zeros, ones, ceil, floor, array, linspace, meshgrid

import scipy
from scipy.signal import boxcar, convolve2d as conv2
from scipy.misc import imsave, imread

#----------------------------------------------------------------------
#----------------------------------------------------------------------
# Windowing functions
#----------------------------------------------------------------------
def lanczos(N, tau=1):
	'''
	return N-point Lanczos window with width tau
	'''
	return scipy.sinc(linspace(-1,1,N)/tau)


#----------------------------------------------------------------------
def makeTrimmedWindow(winFunc):
	'''
	Return a window function constructed from the given window function, but
	without the two edge points (see natWin).
	'''
	return lambda N, *args: winFunc(N+2, *args)[1:-1]


#----------------------------------------------------------------------
def applyWindow(ker, windowFunc):
	'''
	Window a filter kernel with the given windowing function.
	'''
	window = windowFunc(len(ker))
	window = numpy.outer(window, window)
	return ker * window


#----------------------------------------------------------------------
#----------------------------------------------------------------------
# Filter Kernels
#----------------------------------------------------------------------
def sincKer(width, scale):
	'''
	Return a sinc filter kernel with specified width and scale
	
	width - total number of points in the filter (not radius!)
	scale - zeros will be centered around 0 with spacing "scale"
	'''
	x = r_[0:width+1]-width*0.5
	k = numpy.outer(scipy.sinc(x/scale), scipy.sinc(x/scale))
	return k


#----------------------------------------------------------------------
def boxKer(width, scale):
	'''
	Trivial box filter kernel
	'''
	return ones(width+1)


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
	res = clamp(res)
	return res


#----------------------------------------------------------------------
def trilinear():
	'''
	Trilinear interpolation (not implemented...)
	'''
	pass


#----------------------------------------------------------------------
def clamp(im):
	'''
	Clamp an image between zero and one.
	'''
	return numpy.minimum(numpy.maximum(im,0),1)


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
# Functions for creating/manipulating mipmaps
#----------------------------------------------------------------------
def genMipmapIter(im, width, selectWidth, windowFunc, kerFunc):
	'''
	Generate a mipmap by downsampling each image in turn to get the next level.
	
	im - input image
	width - desired "width" of filter kernel.  the kernel will have width+1 points.
	selectWidth - whether the function should try to adjust the filter width for
	even/odd sized images.  To avoid loosing information, even
	filters should go with even sized images and vice versa
	windowFunc  - windowing function to use on the filter.
	
	'''

	mipmap = []

	scale = 2

	ker = kerFunc(width, scale)
	ker = applyWindow(ker, windowFunc)

	while size(im,0) > 1:
		mipmap.append(im)

		if selectWidth:
			if (size(im,0) + width + 1) % 2 == 0:
				ker = kerFunc(width,scale)
			else:
				ker = kerFunc(width+1,scale)
			ker = applyWindow(ker, windowFunc)
		
		print 'side length = %d,  number of points in filter = %d' \
			% (size(im,0), len(ker))
		im = imDownsamp(im, ker, scale)

	return mipmap



#----------------------------------------------------------------------
def genMipmapIterDownsamp4(im, width, selectWidth, windowFunc, kerFunc):
	'''
	Same as genMipmapIter, except tries to downsample by a factor of 4 rather
	than two when possible.
	
	(Beware: ugly hack)
	'''
	pass


#----------------------------------------------------------------------
def flattenImageList(mipmap, dim):
	'''
	"Flatten" mipmap levels into a single image.
	
	mipmap - cell array of scaled images
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
def compareOddEven(im):
	'''
	Compare the difference between sampling an image with an odd or even-sized
	filter.
	'''

	kerFunc = sincKer
	windowFunc = makeTrimmedWindow(lanczos)

	# Use an odd-width filter
	mipmap = genMipmapIter(im, 7, False, windowFunc, kerFunc)
	imsave('mipmap_odd_width.png', flattenImageList(mipmap,1))

	# Use an even-width filter
	mipmap = genMipmapIter(im, 8, False, windowFunc, kerFunc)
	imsave('mipmap_even_width.png', flattenImageList(mipmap,1))


#----------------------------------------------------------------------
def investigateWindowTypes(im, kerFunc):
	'''
	grab the level-three mipmaps arising from using a bunch of windows with
	a fixed filter kernel.
	'''

	# generate a version using the boxcar kernel [1,1] for reference.
	mipmap = genMipmapIter(im, 1, True, boxcar, boxker)
	miplevel3 = [mipmap[3]]

	winfuncs = [lanczos, hamming, hanning, blackman, boxcar]
	# generate mipmaps using the other window functions
	for func in winfuncs:
		mipmap = genMipmapIter(im, 7, True, func, kerfunc)
		miplevel3.append(mipmap[3])

	imsave('mipmap_lvl3_all_wins.png', flattenImageList(miplevel3,1))


#----------------------------------------------------------------------
def plotWindowFuncs(N):
	'''
	Plot various window functions.
	'''
	# Try kaiser window - just need to choose a width...
	winFuncs = [lanczos, hamming, hanning, blackman, boxcar]
	winFuncNames = ['lanczos', 'hamming', 'hanning', 'blackman', 'boxcar']
	clf()
	hold(True)
	for func in winFuncs:
		plot(func(N))

	legend(winFuncNames)
	axis([1,N,0,1.05])


#----------------------------------------------------------------------
# Main program
#----------------------------------------------------------------------

if __name__ == '__main__':
	# Choose image
	im = imread('lena_std.png')[:,:,:3]/256.0
	#im = getGridImg(246, 20)

	# Compare odd and even filter sizes on the same image
	compareOddEven(im)

	# Compare convolution strategies for square seperable filters.

	# Get a mipmap
	#mipmap = genMipmapIter(im, 7, True, boxcar, sincKer)
	# show/save mipmap
	#mipAll = flattenImageList(mipmap, 1)
	#pylab.imshow(mipAll, interpolation='nearest')
	#pylab.imshow(mipAll)
	#imsave('mipmap.png', mipAll)

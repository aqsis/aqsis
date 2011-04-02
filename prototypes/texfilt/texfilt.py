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

'''
This script investigates texture access.  There are two main parts:
	1) Mipmap generation & access
	2) Texture filtering using anisotropic filtering.  This includes
		* stochastic filtering with a weight function over a quadrilateral
		* elliptical gaussian filtering (EWA)

To run the script, you'll need scientific python; specifically, the packages
	* scipy
	* matplotlib
On a unix machine, ipython is highly recommended for a nice interactive
interface which meshes very well with matplotlib.
'''

from __future__ import division

import matplotlib.pylab as pylab
from pylab import imshow

import numpy
from numpy import pi, r_, size, zeros, ones, eye, ceil, floor, array, \
		linspace, meshgrid, random, arange, dot, diag, asarray

import scipy
from scipy import cos, sin, exp, log, log2, sqrt, ogrid, mgrid, ndimage
import scipy.linalg as linalg
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
	scale - zeros will be centred around 0 with spacing "scale"
	'''
	if even:
		numPts = max(2*int((width+1)/2), 2)
	else:
		numPts = max(2*int(width/2) + 1, 3)
	x = r_[0:numPts]-(numPts-1)*0.5
	k = scipy.sinc(x/scale)
	k *= cos(linspace(-pi/4,pi/4,len(k)))
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
	outArr = array(outList)
	return outArr.transpose(range(1,len(outArr.shape)) + [0])


#----------------------------------------------------------------------
#----------------------------------------------------------------------
# Functions for creating test images
#----------------------------------------------------------------------
def getGridImg(N, gridSize, gridWidth=1):
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

	for i in range(0,gridWidth):
		im[i:-gridSize//2:gridSize,:,:] = 0
		im[:,i:-gridSize//2:gridSize,:] = 0

	im[:,-1,:] = 0
	im[-1,:,:] = 0

	return im


def getCentreTest(N=512):
	'''
	Get a test image consisting of a 2x2 checkered plane.

	N - side length
	'''
	im = zeros((N, N, 3))

	im[:N/2,N/2:,:] = 1
	im[N/2:,:N/2,:] = 1

	return im


def getEdgeTest(N=512, borderwidth=20):
	'''
	Get a white test image with a black bordering strip

	N - side length
	'''
	im = ones((N, N, 3))

	im[borderwidth:2*borderwidth,:,:] = 0
	im[-2*borderwidth:-borderwidth,:,:] = 0
	im[:,borderwidth:2*borderwidth,:] = 0
	im[:,-2*borderwidth:-borderwidth,:] = 0

	return im

#----------------------------------------------------------------------
#----------------------------------------------------------------------
# Utils for creating/manipulating mipmaps
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
		(2) Decimate by taking every second element from the result
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
		# store the image with some ghost points to work around a bug in
		# scipy.ndimage, where incorrect interpolation happens for negative
		# indices.
		self.image = zeros((image.shape[0]+2,image.shape[1]+2,image.shape[2]))
		self.image[1:-1,1:-1,:] = image
		self.offset = array(offset,'d')
		self.mult = array(mult,'d')
		# Variances for the reconstruction & prefilters when using EWA.  A
		# variance of 1/(2*pi) gives a filter with centeral weight 1, but in
		# practise this is slightly too small (resulting in a little bit of
		# aliasing).  Therefore it's adjusted up slightly.
		self.varianceXY = 1.3/(2*pi)
		self.varianceST = 1.3/(2*pi)

	def rasterCoords(self,s,t):
		'''
		Return the raster coordinates for this mipmap from the texture
		coordinates (s,t)
		'''
		i = scipy.atleast_1d(self.mult[0]*s + self.offset[0] + 1)
		j = scipy.atleast_1d(self.mult[1]*t + self.offset[1] + 1)
		return (i,j)

	def sampleBilinear(self, s, t):
		'''
		Sample the mipmap level at the coordinates specified by s and t.  For
		performance, s & t can be 1D arrays.
		'''
		i,j = self.rasterCoords(s,t)
		return applyToChannels(self.image, ndimage.map_coordinates, \
				array([i,j]), order=1, mode='constant', cval=0)

	def getRandPointsInQuad(self, s, t, numPoints):
		'''
		Get a random set of points inside the quadrilatiral given by the
		texture-space vertices
		[s[1], t[1]], ... , [s[4], t[4]].
		
		The sample should be uniform if the quadrialteral can be made from a
		square with a _linear_ transformation.  Otherwise it will be biased.
		The form of biasing is probably exactly what we want if we're taking
		pixel samples...
		'''
		lerp1 = numpy.random.rand(numPoints)
		lerp1m = 1-lerp1
		lerp2 = numpy.random.rand(numPoints)
		st = array([s,t])
		op = numpy.outer
		randPts = lerp2*(op(st[:,0],lerp1) + op(st[:,1],lerp1m)) \
				+ (1-lerp2)*(op(st[:,3],lerp1) + op(st[:,2],lerp1m))
		#weights = exp(-8*((lerp1-0.5)**2 + (lerp2-0.5)**2))
		weights = ones(numPoints)
		return (randPts[0,:], randPts[1,:], weights)

	def filterQuadAF(self, s, t, numSamples):
		'''
		Filter the texture by sampling inside the given quadriateral specified
		by the texture-space vertices
		[s[1], t[1]], ... , [s[4], t[4]],
		and taking an average.  Sample points are chosen stochastically over
		the quadrilateral.  This is essentially Monte Carlo integration, with a
		reconstruction filter equal to bilinear interpolation.
		'''
		sRnd, tRnd, w = self.getRandPointsInQuad(s, t, numSamples)
		samples = self.sampleBilinear(sRnd, tRnd)
		return ( samples*w.reshape(w.shape+(1,)) ).sum(0) / w.sum()

	def estimateJacobianInverse(self, s, t):
		'''
		Estimate the inverse Jacobian from a small (s,t) box using finite
		differences.  The assumption is that the (s,t) box has come from a 1x1
		box in the final coordinate system.
		'''
		return 0.5 * array([[s[1]-s[0] + s[2]-s[3], s[3]-s[0] + s[2]-s[1]],
							[t[1]-t[0] + t[2]-t[3], t[3]-t[0] + t[2]-t[1]]])

	def filterEWA(self, sBox, tBox, deltaX=None, deltaY=None, J=None, returnWeights=False):
		'''
		Filter a portion of the texture using an elliptical gaussian filter, as
		described in Heckbert's 1989 thesis, "Fundamentals of Texture Mapping
		and Image Warping".  This kind of filter is also called an EWA filter for
		"Elliptical Weighted Average".
		
		s, t - texture coordinates of a box to be filtered over
		deltaX, deltaY - pixel side length in the final discrete image.
		J - Jacobian of the coordinate transformation.
		returnWeights - true if you just want the filter weights returned for
						debugging rather than the filtered image.
		'''
		# convert to raster coords
		sBox,tBox = self.rasterCoords(sBox,tBox)
		# centre of box
		s0,t0 = sBox.mean(), tBox.mean()
		if not returnWeights and (s0 < 0 or s0 > self.image.shape[0] \
								or t0 < 0 or t0 > self.image.shape[1]):
			return 0
		if J is None:
			# Estimate the Jacobian from the texture box.
			Ji = self.estimateJacobianInverse(sBox, tBox)
		else:
			# Else we have an analytical Jacobian specified.
			# Adjust J for raster->texture coords
			J = dot(dot(diag((1/deltaX, 1/deltaY)), J), diag((1/self.image.shape[0], 1/self.image.shape[1])))
			# get the inverse of J
			Ji = linalg.inv(J)
		# width parameter is log(n) where the filter falls to 1/n of it's
		# centeral value at the edge.
		width = 4
		# V = covariance matrix
		V = self.varianceXY*dot(Ji, Ji.transpose()) + self.varianceST*eye(2)
		# Q = (+ve definite) quadratic form matrix.
		Q = 0.5*linalg.inv(V)
		det = Q[0,0]*Q[1,1] - Q[0,1]*Q[1,0]
		# compute bounding radii
		sRad = sqrt(Q[1,1]*width/det)
		tRad = sqrt(Q[0,0]*width/det)
		# integer filter width
		sWidth = int(floor(s0+sRad)) - int(ceil(s0-sRad)) + 1
		tWidth = int(floor(t0+tRad)) - int(ceil(t0-tRad)) + 1
		# compute edge of filter region in level0 image:
		sStart = ceil(s0-sRad)
		tStart = ceil(t0-tRad)
		# compute filter weights.  I'm being sloppy here, and not bothering to
		# truncate outside the relevant ellipse
		s,t = ogrid[0:sWidth, 0:tWidth]
		s = s - (s0-sStart)
		t = t - (t0-tStart)
		if not returnWeights and (sStart < 0 or sStart + sWidth-1 >= self.image.shape[0] \
				or tStart < 0 or tStart + tWidth-1 >= self.image.shape[1]):
			return 0.5
		weights = exp(-(Q[0,0]*s*s + (Q[0,1]+Q[1,0])*s*t + Q[1,1]*t*t))
		weights /= weights.sum()
		if returnWeights:
			return weights
		imSec = self.image[sStart:sStart+sWidth,tStart:tStart+tWidth]
		return applyToChannels(imSec, lambda x: (x*weights).sum())

	def display(self, s=None, t=None):
		if s is None or t is None:
			imshow(self.image[1:-1,1:-1,:], interpolation='nearest')
		else:
			imshow(self.sampleBilinear(s,t), interpolation='nearest')


#----------------------------------------------------------------------
class TextureMap:
	'''
	Class to encapsulate a (power-of-2) mipmapped texture in much the same way
	mipmapping will be done internally in Aqsis.
	'''
	def __init__(self, img, filterWidth, kerFunc=sincKer, \
			levelCalcMethod='minQuadWidth', numSamples=100):
		'''
		img - texture image
		filterWidth - filter width for mipmap downsampling
		kerFunc - kernel function for mipmap downsampling
		levelCalcMethod - method used for calculating required mipmap level
			from a quadrialteral.  filtering.  May be one of: 'minSideLen',
			'minQuadWidth', 'trilinear', 'minDiag', 'level0'.
		numSamples - number of samples to used when performing stochastic
			filtering
		'''
		self.downsampler = ImageDownsampler(filterWidth, kerFunc)
		self.levels = []
		self.levelCalcMethod = levelCalcMethod
		self.genMipMap(img)
		self.numSamples = numSamples

	def genMipMap(self, img):
		'''
		Generate a mipmap, by downsampling by successive powers of two, using
		the held kernel function and width.

		In this function we make the requirement that the filter coefficients
		for filtering between levels are constant across the image for a
		particular mipmap level.  This restriction means that even-sized images
		have to be filtered with even- sized filter kernels, and vice versa.
		Even sizes also require that an offset be taken into account, since the
		edges of the downsampled levels don't lie on the edges of the original
		image.

		Examples showing mipmap sample points (represented as x's) follow for
		the 1D case.  Several variables are also shown as a function of the
		mipmap level, l on the rhs:

		i_l(t) = raster coordinates for level l as a function of texture coordinate t.
		o_l = offset for level l in level 0 units
		s_l = span of level l in level 0 units
		d_l = interval between pixels of level l in level 0 raster units.
		w_l = width of level l in pixels.


		(mostly) odd numbers of points:
		    0   1   2   3   4                   <-- i_0
		l   |   |   |   |   |                    o_l  s_l  w_l  d_l
		0  [x   x   x   x   x]                   0    4    5    1
		1  [x       x       x]                   0    2    3    2
		2  [x               x]                   0    1    2    4
		3  [        x        ]                   2    0    1    ?
		
		Another example (powers of two)
		    0   1   2   3   4   5   6   7       <-- i_0
		l   |   |   |   |   |   |   |   |        o_l   s_l  w_l
		0  [x   x   x   x   x   x   x   x]       0     7    8
		1  [  x       x       x       x  ]       0.5   6    4
		2  [      x               x      ]       1.5   4    2
		3  [              x              ]       3.5   0    1

		Notice that the higher level mipmaps for the power of two-sized image
		do *not* have points which lie on the edge of the image.  This means
		that we've got to incorporate an offset when we sample the level.

		Formulas for texture coordinates -> raster space
		
		s_l = (s_0 - 2*o_l)
		
		Raster units for level 0 are:
		i_0(t) = t * s_0
		Raster units for level l are:
		i_l(t) = (i_0(t) - o_0)/d_l
		
		Or, what we really need, which is raster units for level l in terms of
		the texture coordinate t:
		i_l(t) = t * s_0/d_l - o_0/d_l
		
		((
			Note that we could abandon the constant-coefficients approach
			(which would make it very slow to generate mipmaps), we would have:
			
				0   1   2   3   4   5   6   7 <-- i_0
			l   |   |   |   |   |   |   |   |   o_l   s_l  w_l
			0  [x   x   x   x   x   x   x   x]  0     7    8
			1  [x        x         x        x]  0     7    4
			2  [x                           x]  0     7    2
			3  [              x              ]  0     7    1
			
			The the raster units for level l in terms of the texture coordinates t
			are then very simple:
			
			i_l(t) = t*(w_l - 1)
		))
		'''
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
			# The below is a theoretically broken, but very usable version, -
			# mipmap offsets are set to zero.
			#self.levels.append(MipLevel(img, 0*offset, array(img.shape[0:2])-1))
			print 'generated level size', img.shape[0:2]

	def filterTrilinear(self, sBox, tBox, level=None):
		'''
		Trilinear filtering over the mipmaps

		s, t - texture coordinates of points to sample at
		ds,dt - size of a "box" on the screen.
		level - mipmap level to use.  If equal to None, calculate from ds & dt
		'''
		if level is None:
			level = min(len(self.levels)-1, self.calculateLevel(sBox, tBox))
		level = max(level,0)
		level1 = int(floor(level))
		interp = level - level1
		level2 = min(len(self.levels)-1, int(level1 + 1))
		s0 = sBox.mean()
		t0 = tBox.mean()
		samp1 = self.levels[level1].sampleBilinear(s0,t0)
		samp2 = self.levels[level2].sampleBilinear(s0,t0)
		return (1-interp)*samp1 + interp*samp2

	def calculateLevel(self, s, t):
		'''
		Calculate the appropriate mipmap level for texture filtering over a
		quadrilateral given by the texture-space vertices
		[s[1], t[1]], ... , [s[4], t[4]].

		There are many ways to do this; the instance variable levelCalcMethod
		selects the desired one.  The most correct way is to choose the
		minSideLen method, as long as the quadrilateral is vaguely
		rectangular-shaped.  This only works if you're happy to use lots of
		samples however, otherwise you get aliasing.
		'''
		s = s.copy()*self.levels[0].image.shape[0]
		t = t.copy()*self.levels[0].image.shape[1]
		if self.levelCalcMethod == 'minSideLen':
			# Get mipmap level with minimum feature size equal to the shortest
			# quadrilateral side
			s1 = pylab.concatenate((s, s[0:1]))
			t1 = pylab.concatenate((t, t[0:1]))
			minSideLen2 = (numpy.diff(s1)**2 + numpy.diff(t1)**2).min()
			level = log2(minSideLen2)/2
		elif self.levelCalcMethod == 'minQuadWidth':
			# Get mipmap level with minimum feature size equal to the width of
			# the quadrilateral.  This one is kinda tricky.
			# v1,v2 = vectors along edges
			v1 = array([0.5*(s[1]-s[0] + s[2]-s[3]), 0.5*(t[1]-t[0] + t[2]-t[3]),])
			v2 = array([0.5*(s[3]-s[0] + s[2]-s[1]), 0.5*(t[3]-t[0] + t[2]-t[1]),])
			v1Sq = dot(v1,v1)
			v2Sq = dot(v2,v2)
			level = 0.5*log2(min(v1Sq,v2Sq) * (1 - dot(v1,v2)**2/(v1Sq*v2Sq)))
		elif self.levelCalcMethod == 'minDiag':
			# Get mipmap level with minimum feature size equal to the minimum
			# distance between the centre of the quad and the vertices.  Sort
			# of a "quad radius"
			#
			# This is more-or-less the algorithm used in Pixie...
			minDiag2 = ((s - s.mean())**2 + (t - t.mean())**2).min()
			level = log2(minDiag2)/2
		#elif self.levelCalcMethod == 'sqrtArea':
			# Get mipmap level with minimum feature size estimated as the
			# square root of the area of the box.
		elif self.levelCalcMethod == 'trilinear':
			# Get mipmap level which will result in no aliasing when plain
			# trilinear filtering is used (no integration)
			maxDiag2 = ((s - s.mean())**2 + (t - t.mean())**2).max()
			level = log2(maxDiag2)/2
		elif self.levelCalcMethod == 'level0':
			# Else just use level 0.  Correct texture filtering will take care
			# of any aliasing...
			level = 0
		else:
			raise "Invalid mipmap level calculation type: %s" % self.levelCalcMethod
		return max(level,0)

	def filterQuadAF(self, s, t):
		'''
		Filter the texture by sampling inside the given quadriateral specified
		by the texture-space vertices
		[s[1], t[1]], ... , [s[4], t[4]],
		and taking an average.  Sample points are chosen stochastically over
		the quadrilateral at the mipmap level returned by calculateLevel.
		This is essentially Monte Carlo integration, with a reconstruction
		filter equal to bilinear interpolation.
		'''
		level = int(self.calculateLevel(s,t))
		return self.levels[level].filterQuadAF(s,t, self.numSamples)

	def filterEWA(self, sBox, tBox, deltaX=None, deltaY=None, J=None, returnWeights=False):
		'''
		Filter a portion of the texture using an elliptical gaussian filter, as
		described in Heckbert's 1989 thesis, "Fundamentals of Texture Mapping
		and Image Warping".  This kind of filter is also called an EWA filter for
		"Elliptical Weighted Average".

		s, t - Texture box over which to filter
		deltaX, deltaY - pixel side length in the final discrete image.
		J - Jacobian of the coordinate transformation.
		'''
		level = int(self.calculateLevel(sBox,tBox))
		col = self.levels[level].filterEWA(sBox, tBox, deltaX, deltaY, J, returnWeights)
		return col #* 0.9**level

	def displayLevel(self, level, s=None, t=None):
		'''
		Display a mipmap level
		'''
		self.levels[level].display(s,t)


#----------------------------------------------------------------------
#----------------------------------------------------------------------
# 2D texture transformation classes ("warps")
#
# These are useful as example texture warps to test anisotropic filtering
# techniques.
#
# Each warp class defines three functions
# fwd - the forward transform
# inv - the inverse transform
# jacobian - the jacobian of the forward transform
#----------------------------------------------------------------------
class PerspectiveWarp:
	'''
	Simplified perspective transformation, between "texture coordinates", (s,t)
	on an infinite plane with parametric form P(s,t) = [s,-1,t],
	and "image coordinates", (x,y) in the x,y plane.

	We use the LH coordinate system, with x=right, y=up, z=into the screen.
	'''
	def __init__(self, d=1):
		'''
		d - distance from view window to viewer
		'''
		self.d = d

	def fwd(self, s, t):
		'''
		Forward perspective transformation

		Params:
			s,t - texture coordinates along x & z directions

		Returns:
			(x,y) - coordinates on the view window.
		'''
		x = 1/(1+t/self.d) * s
		y = -1/(1+t/self.d)
		return (x,y)

	def jacobian(self, s, t):
		'''
		return the Jacobian matrix of the forward transformation
		'''
		D = 1/self.d*(1+t/self.d)**(-2)
		return D*array([[self.d+t, -s], [0, 1]])

	def inv(self, x, y):
		'''
		Inverse perspective transformation

		Params:
			x,y - coordinates in the view window

		Returns:
			(s,t) - texture coordinates along x & z directions
		'''
		s = -x/y
		t = -self.d*(1+1/y)
		return (s,t)

#----------------------------------------------------------------------
class AffineWarp:
	'''
	A class representing an affine transformation
	'''
	def __init__(self, A=eye(2), b=array([0,0]), theta=None, scale=None):
		'''
		Affine transformation is x' = A*x + b
		A - linear part of the affine trans.
		b - translation
		theta - if this is set to a number, make A into a rotation with angle theta.
		scale - if this is set to a number, scale A by this factor.
		'''
		self.A = asarray(A)
		self.b = asarray(b)
		if theta is not None:
			self.theta = theta
			theta *= pi/180
			self.A = array([[cos(theta), sin(theta)], [-sin(theta), cos(theta)]])
		if scale is not None:
			self.A *= scale
		self.Ai = linalg.inv(self.A)

	def fwd(self, s, t):
		return (self.A[0,0]*s + self.A[0,1]*t + self.b[0], \
				self.A[1,0]*s + self.A[1,1]*t + self.b[1])

	def inv(self, x, y):
		return (self.Ai[0,0]*(x-self.b[0]) + self.Ai[0,1]*(y - self.b[1]), \
				self.Ai[1,0]*(x-self.b[0]) + self.Ai[1,1]*(y - self.b[1]))

	def jacobian(self, s, t):
		return self.A

	def __mul__(self, rhs):
		'''
		The * operator composes affine transformations
		'''
		return AffineWarp(A=dot(self.A,rhs.A), b=dot(self.A,rhs.b)+self.b)

#----------------------------------------------------------------------
class CompositionWarp:
	'''
	A class for holding the composition of several transformations
	'''
	def __init__(self, *args):
		'''
		Takes a list of transformations to be composed.
		The transformation list is evaluated from right to left in the same
		manner as it is usually written mathematically.
		'''
		self.transList = list(args)
		self.transList.reverse()
	
	def fwd(self, s,t):
		x,y = s,t
		for trans in self.transList:
			x,y = trans.fwd(x,y)
		return (x,y)
	
	def inv(self, x,y):
		s,t = x,y
		for trans in reversed(self.transList):
			s,t = trans.inv(s,t)
		return (s,t)
	
	def jacobian(self, s,t):
		J = eye(2)
		for trans in self.transList:
			J = dot(trans.jacobian(s,t),J)
			s,t = trans.fwd(s,t)
		return J

#----------------------------------------------------------------------
def plotPerspectiveWarp(u, v, direc='f'):
	'''
	Plot u,v on an axis, and the things they transform into under an (possibly
	inverse) perspective transformation on another axis.

	u,v - either image coords if direc='b' or texture coords if direc='f'
	direc - transformation direction; 'f' = forward, 'b'=backward
	'''
	perspec = PerspectiveWarp()
	if direc == 'f':
		s,t = u,v
		x,y = perspec.fwd(s,t)
	else:
		x,y = u,v
		s,t = perspec.inv(x,y)

	# plot texture coords
	pylab.subplot(211)
	pylab.plot(s,t)
	pylab.axis('equal')

	# plot image coords
	pylab.subplot(212)
	pylab.plot(x,y)
	pylab.axis('equal')
	pylab.axis([-1,1,-1,0])

#----------------------------------------------------------------------
# High level driver functions.
#----------------------------------------------------------------------
def genTrilinearSeq(mipmap, s, t, ds, dt):
	'''
	Generate and save a sequence of images by trilinear interpolation at
	various levels, but with the same texture coordinates (s,t)
	'''
	for i in range(0,31):
		level = i/31.0*8
		image = mipmap.sampleTrilinear(s,t,ds,dt,level)
		imsave('downsamp%0.2d.png' % i, image)

#----------------------------------------------------------------------
def plotPixelBoxes():
	'''
	Plot the outlines of some "pixels" in the image plane, as they transform to
	the texture coordinates.  This allows us to see what kinds of filter shapes
	we *should* have to do proper texture filtering...  Naive trilinear
	mipmapping only gets square filters correct...
	'''
	x = array([1,1,-1,-1,1])
	y = array([-1,1,1,-1,-1])
	plotPerspectiveWarp(x,y+1,'f')
	plotPerspectiveWarp(x,y+7,'f')
	plotPerspectiveWarp(x,y+4,'f')
	plotPerspectiveWarp(x+3,y+4,'f')
	plotPerspectiveWarp(x*0.05,y*0.05-0.1,'b')
	plotPerspectiveWarp(x*0.01,y*0.01-0.1,'b')
	plotPerspectiveWarp(x*0.01-0.5,y*0.01-0.1,'b')
	plotPerspectiveWarp(x*0.01+0.5,y*0.01-0.1,'b')
	plotPerspectiveWarp(x*0.01+0.5,y*0.01-0.05,'b')

#----------------------------------------------------------------------
def textureWarp(x, y, tex, trans=PerspectiveWarp(), method='EWA'):
	'''
	Warp the given texture.

	x,y - image space coordinates for output.
	'''
	im = zeros((x.shape[1]-1, x.shape[0]-1, 3))
	deltaX = x[1,0]-x[0,0]
	deltaY = y[0,1]-y[0,0]
	print "Warping the texture..."
	for i in range(0,x.shape[0]-1):
		for j in range(0,x.shape[1]-1):
			xBox = array((x[i,j], x[i+1,j], x[i+1,j+1], x[i,j+1]))
			yBox = array((y[i,j], y[i+1,j], y[i+1,j+1], y[i,j+1]))
			sBox, tBox = trans.inv(xBox, yBox)
			if method == 'EWA':
				# use Elliptical Weighted Average filter, with Jacobian
				# estimation via finite differences
				col = tex.filterEWA(sBox, tBox)
			elif method == 'EWA_analytical_J':
				# Use EWA with an analytical Jacobian.  This can be used as a
				# check against the numerical estimate.  Last time I looked, it
				# was spot-on!
				J = trans.jacobian(sBox.mean(),tBox.mean())
				col = tex.filterEWA(sBox, tBox, deltaX, deltaY, J)
			elif method == 'MC_integration':
				# Use Monte Carlo integration over a box.
				col = tex.filterQuadAF(sBox, tBox)
			elif method == 'trilinear':
				# Use plain old trilinear filtering
				col = tex.filterTrilinear(sBox, tBox)
			else:
				raise "Invalid method specified."
			im[-1-j,i,:] = col
		if i % 20 == 0:
			print "done col %d, " % i
	return im

#----------------------------------------------------------------------
def showMipmapOffsetDifferences():
	'''
	Show the differences in warped images generated when the mipmap offsets are
	turned off (very small).
	'''
	im = getEdgeTest(1000, 100)
	texture = TextureMap(im, 8)

	N = 1000
	x,y = mgrid[-0.5:0.5:N*1j,-0.22:-0.005:N*2//5*1j]
	rot = AffineWarp(b=(0.5,0.5)) * AffineWarp(theta=45) * AffineWarp(b=(-0.5,-0.5))
	trans = CompositionWarp(PerspectiveWarp(0.4),AffineWarp(A=diag((10,30)), b = array((-5,4)))*rot)
	imWarp = textureWarp(x,y, texture, trans)
	imsave('tmp_offset_bad.png',imWarp)

#----------------------------------------------------------------------
def showLevelCalcDifferences():
	'''
	Compare several of the best mipmap level calculation methods for EWA filters.
	'''
	im = getGridImg(1000, 30, 2)
	texture = TextureMap(im, 8, levelCalcMethod='minQuadWidth')

	N = 500
	x,y = mgrid[-0.5:0.5:N*1j,-0.22:-0.005:N*2//5*1j]
	trans = CompositionWarp(PerspectiveWarp(),AffineWarp(A=diag((10,30)), b = array((-5,4))))
	imWarp = textureWarp(x,y, texture, trans)
	imshow(imWarp)
	imsave('level_minQuadWidth.png', imWarp)

	texture.levelCalcMethod='minSideLen'
	imWarp = textureWarp(x,y, texture, trans)
	imshow(imWarp)
	imsave('level_minSideLen.png', imWarp)

	texture.levelCalcMethod='level0'
	imWarp = textureWarp(x,y, texture, trans)
	imshow(imWarp)
	imsave('level_level0.png', imWarp)

#----------------------------------------------------------------------
def showFilteringMethodDifferences():
	'''
	Compare several different filtering methods
	'''
	im = getGridImg(1000, 30, 2)
	texture = TextureMap(im, 8, levelCalcMethod='minQuadWidth')

	N = 700
	x,y = mgrid[-0.5:0.5:N*1j,-0.22:-0.005:N*2//5*1j]
	trans = CompositionWarp(PerspectiveWarp(),AffineWarp(A=diag((10,30)), b = array((-5,4))))
	imWarp = textureWarp(x,y, texture, trans, method='EWA')
	imshow(imWarp)
	imsave('method_EWA.png', imWarp)
	imWarp = textureWarp(x,y, texture, trans, method='EWA_analytical_J')
	imshow(imWarp)
	imsave('method_EWA_analytical_J.png', imWarp)
	imWarp = textureWarp(x,y, texture, trans, method='MC_integration')
	imshow(imWarp)
	imsave('method_MC_integration.png', imWarp)
	texture.levelCalcMethod='trilinear' # need a special level calculation for trilin.
	imWarp = textureWarp(x,y, texture, trans, method='trilinear')
	imshow(imWarp)
	imsave('method_trilinear.png', imWarp)

#----------------------------------------------------------------------
# Main program
#----------------------------------------------------------------------

if __name__ == '__main__':
	# a usage example
	if True:
		# Choose image & create mipmap
		#im = getCentreTest(1000)
		im = getGridImg(1000, 30, 2)
		#im = getEdgeTest(1000, 100)
		#im = imread('lena_std.png')[:,:,:3]/256.0
		texture = TextureMap(im, 8, levelCalcMethod='minQuadWidth')
		# Warp image to new coordinate system.
		N = 500
		x,y = mgrid[-0.5:0.5:N*1j,-0.22:-0.005:N*2//5*1j]
		trans = CompositionWarp(PerspectiveWarp(),AffineWarp(A=diag((10,30)), b = array((-5,4))))
		imWarp = textureWarp(x,y, texture, trans)
		pylab.hold(False)
		imshow(imWarp)
		imsave('imWarp.png',imWarp)
		pylab.draw()
	else:
		# Area for hacking out solutions.
		#showLevelCalcDifferences()
		#showFilteringMethodDifferences()
		im = getGridImg(1000, 30, 2)
		texture = TextureMap(im, 8, levelCalcMethod='minQuadWidth')
		N = 500
		x,y = mgrid[-0.5:0.5:N*1j,-0.22:-0.005:N*2//5*1j]
		trans = CompositionWarp(PerspectiveWarp(),AffineWarp(A=diag((10,30)), b = array((-5,4))))
		i = N//2
		j = N//4
		xBox = array((x[i,j], x[i+1,j], x[i+1,j+1], x[i,j+1]))
		yBox = array((y[i,j], y[i+1,j], y[i+1,j+1], y[i,j+1]))
		sBox, tBox = trans.inv(xBox, yBox)
		weights = texture.filterEWA(sBox, tBox, returnWeights=True)

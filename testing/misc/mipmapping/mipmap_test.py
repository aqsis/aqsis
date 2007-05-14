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
		linspace, meshgrid, random, arange, dot, diag

import scipy
from scipy import cos, exp, log, log2, sqrt, ogrid, mgrid, ndimage
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
		self.offset = offset.copy()
		self.mult = mult

	def sample(self, s, t):
		'''
		Sample the mipmap level at the coordinates specified by s and t.  For
		performance, s & t should be 1D arrays.
		'''
		i = self.mult[0]*s + self.offset[0] + 1
		j = self.mult[1]*t + self.offset[1] + 1
		return applyToChannels(self.image, ndimage.map_coordinates, \
				array([i,j]), order=1, mode='constant', cval=0)

	def display(self, s=None, t=None):
		if s is None or t is None:
			imshow(self.image[1:-1,1:-1,:], interpolation='nearest')
		else:
			imshow(self.sample(s,t), interpolation='nearest')


#----------------------------------------------------------------------
class TextureMap:
	'''
	Class to encapsulate a (power-of-2) mipmapped texture in much the same way
	mipmapping will be done internally in Aqsis.
	'''
	def __init__(self, img, filterWidth, kerFunc=sincKer, levelCalcMethod='minSideLen'):
		'''
		img - texture image
		filterWidth - filter width for mipmap downsampling
		kerFunc - kernel function for mipmap downsampling
		levelCalcMethod - method used for calculating required mipmap
			filtering.  May be one of: 'minSideLen', 'minDiag', 'lvlZero'.
		level from a quadrialteral.
		'''
		self.downsampler = ImageDownsampler(filterWidth, kerFunc)
		self.levels = []
		self.levelCalcMethod = levelCalcMethod
		self.genMipMap(img)

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
			print 'generated level size', img.shape[0:2]

	def sampleTrilinear(self, s, t, level=None):
		'''
		Trilinear sampling
		'''
		if level is None:
			level = calculateLevel(s[1,0]-s[0,0], t[0,1]-t[0,0])
		level = max(level,0)
		print "trilinear sample at level = %f" % level
		level1 = int(floor(level))
		interp = level - level1
		level2 = int(level1 + 1)
		samp1 = self.levels[level1].sample(s,t)
		samp2 = self.levels[level2].sample(s,t)
		return (1-interp)*samp1 + interp*samp2

	def calculateLevel(self, ds, dt):
		'''
		Calculate the appripriate mipmap level for texture filtering over a
		square region of side lengths ds & dt in texture space.
		'''
		diag = sqrt( ((ds*self.levels[0].image.shape[0])**2 + 
				(dt*self.levels[0].image.shape[1])**2)/2 )
		level = log2(diag)
		return max(level,0)

	def calculateLevelQuad(self, s, t):
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
		elif self.levelCalcMethod == 'minDiag':
			# Get mipmap level with minimum feature size equal to the minimum
			# distance between the centre of the quad and the vertices.
			#
			# This is more-or-less the algorithm used in Pixie...
			minDiag2 = ((s - s.mean())**2 + (t - t.mean())**2).min()
			level = log2(minDiag2)/2
		#elif self.levelCalcMethod == 'sqrtArea':
			# Get mipmap level with minimum feature size estimated as the
			# square root of the area of the box.
		else:
			# Else just use level 0.  Correct texture filtering will take care
			# of any aliasing...
			level = 0
		return max(level,0)

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
		op = numpy.outer
		lerp1 = numpy.random.rand(numPoints)
		lerp1m = 1-lerp1
		lerp2 = numpy.random.rand(numPoints)
		st = array([s,t])
		randPts = lerp2*(op(st[:,0],lerp1) + op(st[:,1],lerp1m)) \
				+ (1-lerp2)*(op(st[:,3],lerp1) + op(st[:,2],lerp1m))
		#weights = exp(-8*((lerp1-0.5)**2 + (lerp2-0.5)**2))
		weights = ones(numPoints)
		return (randPts[0,:], randPts[1,:], weights)

	def filterQuadAF(self, s, t, numPoints=100):
		'''
		Filter the texture by sampling inside the given quadriateral specified
		by the texture-space vertices
		[s[1], t[1]], ... , [s[4], t[4]],
		and taking an average.  Sample points are chosen stochastically over
		the quadrilateral at the mipmap level returned by calculateLevelQuad.
		This is essentially Monte Carlo integration, with a reconstruction
		filter equal to bilinear interpolation.
		'''
		sRnd, tRnd, w = self.getRandPointsInQuad(s,t,numPoints)
		level = int(self.calculateLevelQuad(s,t))
		samples = self.levels[level].sample(sRnd, tRnd)
		return ( samples*w.reshape(w.shape+(1,)) ).sum(0) / w.sum()

	def filterEllipticGaussianAF(self, s0, t0, deltaX, deltaY, J, returnWeights=False):
		'''
		Filter a portion of the texture using an elliptical gaussian filter, as
		described in Heckbert's 1989 thesis, "Fundamentals of Texture Mapping
		and Image Warping".  This kind of filter is also called an EWA filter for
		"Elliptical Weighted Average".

		s0, t0 - texture coordinates of the centre of the filter
		deltaX, deltaY - pixel side length in the final discrete image.
		J - Jacobian of the coordinate transformation.
		returnWeights - true if you just want the filter weights returned for
						debugging rather than the filtered image.
		'''
		if not returnWeights and (s0 < 0 or s0 > 1 or t0 < 0 or t0 > 1):
			return 0
		# convert to pixel coords
		s0 *= self.levels[0].image.shape[0]
		t0 *= self.levels[0].image.shape[1]
		# get the inverse of J and adjust for pixel coords
		Ji = dot(diag(self.levels[0].image.shape[0:2]), linalg.inv(J))
		# width parameter is log(n) where the filter falls to 1/n of it's
		# centeral value at the edge.
		width = 4
		# Variances for the reconstruction & prefilters.
		magicalFudgeFactor = 200  # TODO: WTH do I need this???
		varianceXY = 1/(2*pi) / magicalFudgeFactor
		varianceST = 1/(2*pi) / magicalFudgeFactor
		# V = covariance matrix
		V = varianceXY*dot(Ji, dot(diag([deltaX**2,deltaY**2]), Ji.transpose())) + varianceST*eye(2)
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
		#crap *= 1
		if not returnWeights and (sStart < 0 or sStart + sWidth-1 >= self.levels[0].image.shape[0] \
				or tStart < 0 or tStart + tWidth-1 >= self.levels[0].image.shape[1]):
			#s = 
			return 0.5
		weights = exp(-(Q[0,0]*s*s + (Q[0,1]+Q[1,0])*s*t + Q[1,1]*t*t))
		weights /= weights.sum()
		if returnWeights:
			return weights
		# TODO: Use a mipmap here rather than mipmap level[0]
		imSec = self.levels[0].image[sStart:sStart+sWidth,tStart:tStart+tWidth]
		return applyToChannels(imSec, lambda x: (x*weights).sum())

	def displayLevel(self, level, s=None, t=None):
		'''
		Display a mipmap level
		'''
		self.levels[level].display(s,t)


#----------------------------------------------------------------------
# Perspective transformations
#
# Simplified perspective transformation, between "texture coordinates", (s,t)
# on an infinite plane with parametric form P(s,t) = [s,-1,t],
# and "image coordinates", (x,y) in the x,y plane.  This is useful as an
# example nonaffine texture warp to test anisotropic filtering techniques.
#
# We use the LH coordinate system, with x=right, y=up, z=into the screen.
#----------------------------------------------------------------------
class PerspectiveTrans:
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
def plotPerspectiveTrans(u, v, direc='f'):
	'''
	Plot u,v on an axis, and the things they transform into under an (possibly
	inverse) perspective transformation on another axis.

	u,v - either image coords if direc='b' or texture coords if direc='f'
	direc - transformation direction; 'f' = forward, 'b'=backward
	'''
	perspec = PerspectiveTrans()
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
def genTrilinearSeq(mipmap, s, t):
	'''
	Generate and save a sequence of images by trilinear interpolation at
	various levels, but with the same texture coordinates (s,t)
	'''
	for i in range(0,31):
		level = i/31.0*8
		image = mipmap.sampleTrilinear(s,t,level)
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
	plotPerspectiveTrans(x,y+1,'f')
	plotPerspectiveTrans(x,y+7,'f')
	plotPerspectiveTrans(x,y+4,'f')
	plotPerspectiveTrans(x+3,y+4,'f')
	plotPerspectiveTrans(x*0.05,y*0.05-0.1,'b')
	plotPerspectiveTrans(x*0.01,y*0.01-0.1,'b')
	plotPerspectiveTrans(x*0.01-0.5,y*0.01-0.1,'b')
	plotPerspectiveTrans(x*0.01+0.5,y*0.01-0.1,'b')
	plotPerspectiveTrans(x*0.01+0.5,y*0.01-0.05,'b')

#----------------------------------------------------------------------
def textureMapPlane(x, y, tex, numSamples=100):
	'''
	Map the given texture onto a plane, according to the perspective
	transformations given above.

	x,y - image space coordinates for output.
	'''
	perspec = PerspectiveTrans()
	im = zeros((x.shape[1]-1, x.shape[0]-1, 3))
	deltaX = x[1,0]-x[0,0]
	deltaY = y[0,1]-y[0,0]
	print "Warping the texture..."
	for i in range(0,x.shape[0]-1):
		for j in range(0,x.shape[1]-1):
#			# Quadrilateral filter region
#			# compute (s,t) box to filter with
#			xBox = array((x[i,j], x[i+1,j], x[i+1,j+1], x[i,j+1]))
#			yBox = array((y[i,j], y[i+1,j], y[i+1,j+1], y[i,j+1]))
#			sBox, tBox = perspec.inv(xBox, yBox)
#			# remap the texture coords so that more of the plane is covered by
#			# texture.
#			sBox = sBox/4+0.5
#			tBox = tBox/4-0.3
#			im[-1-j,i,:] = tex.filterQuadAF(sBox, tBox, numSamples)

			# Use an EWA filter
			x0 = (x[i:i+2,j:j+2]).mean()
			y0 = (y[i:i+2,j:j+2]).mean()
			s0,t0 = perspec.inv(x0, y0)
			J = perspec.jacobian(s0,t0)
			# Remap the texture coords so that more of the plane is covered by
			# texture.  Need to also adjust the Jacobian if we do this...
			s0 = s0/4+0.5
			t0 = t0/4-0.3
			J /= 4
			im[-1-j,i,:] = tex.filterEllipticGaussianAF(s0, t0, deltaX, deltaY, J)
		if i % 20 == 0:
			print "done col %d, " % i
	return im

#----------------------------------------------------------------------
# Main program
#----------------------------------------------------------------------

if __name__ == '__main__':
	# Choose image
	#im = imread('lena_std.png')[:,:,:3]/256.0
	N = 512
	#im = getCentreTest(N)
	im = getGridImg(1000, 30, 2)
	texture = TextureMap(im, 8, )
	s,t = mgrid[0:1:N*1j,0:1:N*1j]
	x,y = mgrid[-0.5:0.5:N*1j,-0.5:-0.1:N*2//5*1j]
	stride = 1
	imWarp = textureMapPlane(x[::stride,::stride],y[::stride,::stride], texture)
	imshow(imWarp)

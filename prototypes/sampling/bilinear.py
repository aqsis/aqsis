# Simple testbed for reverse bilinear lookup: via Newton's method.  This tends
# to converge to within the required ~1e-4 accuracy within two iterations if we
# start at the centre of the patch.

from pylab import *

def bilerp(A,B,C,D, u,v):
	return (1-v)*((1-u)*A + u*B) + v*((1-u)*C + u*D)

def plotPatch(A,B,C,D):
	plotLine(A,B,D,C,A)

def plotLine(*vecs2d):
	x = [v[0] for v in vecs2d]
	y = [v[1] for v in vecs2d]
	plot(x, y, '.-');

def plotPoint(P):
	plot([P[0]], [P[1]], '.-')

def makePatch(ax, ay, bx, by, cx, cy, dx, dy):
	return (array([ax, ay], dtype='f'), array([bx, by], dtype='f'),
			array([cx, cy], dtype='f'), array([dx, dy], dtype='f'))

# Irregular.
#A,B,C,D = makePatch(0.1,0.1,  1.1,0.1,  -0.1,1.5,  1,1)

# Rectangular
A,B,C,D = makePatch(0,0,  1,0,  0,2,  1,2)

# Degenerate corner point
#A,B,C,D = makePatch(0.1,0.1,  0.1,0.1,  -0.1,1.5,  1,1)

# Very stretched on one side
A,B,C,D = makePatch(0,0,  1,0,  -10,2,  10,2)

# Add a large offset.
#offset = array([1000, 1000], dtype='f')
#A += offset
#B += offset
#C += offset
#D += offset

E = C - A
F = B - A
G = A - B - C + D

uvIn = array([0.8, 0.8])
P = bilerp(A,B,C,D, uvIn[0], uvIn[1])

def errorFxn(uv):
	return A - P + E*uv[1] + F*uv[0] + G*uv[1]*uv[0]

# Perform an iteration of Newton's method.
def solveIter(uv):
	return uv + solve(array([F + G*uv[1], E+ G*uv[0]]).T, -errorFxn(uv))

uv = array([0.5, 0.5])
uvAll = [uv]
print 'uvIn = ', uvIn
for i in range(0,3):
	uv = solveIter(uv)
	uvAll.append(uv)
	print uv

clf()
plotPatch(A,B,C,D)
plotLine(P)
plotLine(*[bilerp(A,B,C,D,uv[0],uv[1]) for uv in uvAll])
axis('equal')
draw()

#!/usr/bin/python

from __future__ import division
import numpy as np
import matplotlib.pyplot as pl
import numpy.linalg as la


# This script tests some data structures for holding surfaces during the
# splitting stage of an reyes pipeline.  In the presence of arbitrary bucket
# orders, it's tricky to do this efficiently.
#
# One obvious possibility is to store the surfaces in some kind of spatial
# tree structure, for example a BSP tree.  The problem here is that we have no
# immediate method for finding the *top* surface for a given bucket.  However,
# we can arrange things so that the number of references to any given piece of
# geometry is at most 4, which hopefully makes up for the difficulty of
# finding the topmost surface.
#
# Another possibility is to use a flat array (eg, with one element per
# bucket), but in this case the number of expired references might be pretty
# large...


#-------------------------------------------------------------------------------
# Bounding box
class Bound:
    def __init__(self, xmin, xmax, ymin, ymax):
        self.xmin = xmin
        self.xmax = xmax
        self.ymin = ymin
        self.ymax = ymax

    def plot(self, **kwargs):
        bndx, bndy = self.boundary()
        pl.plot(bndx, bndy, '-', **kwargs)

    def xrange(self):
        return self.xmax - self.xmin
    def yrange(self):
        return self.ymax - self.ymin
    def center(self):
        return ((self.xmin + self.xmax)/2, (self.ymin + self.ymax)/2)

    def boundary(self):
        return (np.array((self.xmin, self.xmax, self.xmax, self.xmin, self.xmin)),
                np.array((self.ymin, self.ymin, self.ymax, self.ymax, self.ymin)))

    def __repr__(self):
        return '[%f %f] x [%f %f]' % (self.xmin, self.xmax,
                                      self.ymin, self.ymax)


#-------------------------------------------------------------------------------
# The vertices for the corners of the patch, P, are stored as a 2x2 array.
# (Vertices are 2-vectors, so P has shape 2x2x2.)
#
# Vertex numbering:
#
# P[0,0] ---- P[1,0]
#  |           |
#  |           |
#  |           |
#  |           |
# P[0,1] ---- P[1,1]
#
class Patch:
    def __init__(self, P):
        self.P = P

    def split(self):
        L1 = la.norm(self.P[0,0] - self.P[1,0])
        L2 = la.norm(self.P[0,0] - self.P[0,1])
        if L1 >= L2:
            # split vertical edge
            midPoints = np.sum(self.P, axis=0)/2
            P1 = self.P.copy()
            P1[1,:] = midPoints
            P2 = self.P.copy()
            P2[0,:] = midPoints
            return (Patch(P1), Patch(P2))
        else:
            # split horizontal edge
            midPoints = np.sum(self.P, axis=1)/2
            P1 = self.P.copy()
            P1[:,1] = midPoints
            P2 = self.P.copy()
            P2[:,0] = midPoints
            return (Patch(P1), Patch(P2))

    def plot(self, **kwargs):
        'Plot a line around the patch'
        x = self.P[:,:,0].flatten()
        y = self.P[:,:,1].flatten()
        x[2],x[3] = x[3],x[2]
        y[2],y[3] = y[3],y[2]
        pl.plot(np.concatenate((x, [x[0]])),
                np.concatenate((y, [y[0]])), '.-', **kwargs)

    def bound(self):
        'Return a bound for the patch'
        x = self.P[:,:,0].flatten()
        y = self.P[:,:,1].flatten()
        return Bound(np.amin(x), np.amax(x), np.amin(y), np.amax(y))


def recursiveSplit(buckets, gHolder, level, plot=False):
    'Split level times, and store into buckets'
    buckets.insert(gHolder)
    if level <= 0:
        return
    g1,g2 = gHolder.obj.split()
    recursiveSplit(buckets, Holder(g1), level-1, plot=plot)
    recursiveSplit(buckets, Holder(g2), level-1, plot=plot)
    if plot:
        g1.plot(color='r', linewidth=0.5)
        g2.plot(color='r', linewidth=0.5)
    gHolder.expired = True


#-------------------------------------------------------------------------------
# Holder for geometry.  The "expired" flag allows us to treat the holder as a
# weak reference.  (Actually, we keep the object around for convenience, but
# in a real renderer we would delete obj rather than setting self.expired to
# True.)
class Holder:
    def __init__(self, obj):
        self.obj = obj
        self.bound = obj.bound()
        self.expired = False

def findExpired(surfaces):
    'Find the expired surfaces in a list'
    return [h for h in surfaces if h.expired]

def findValid(surfaces):
    'Find the expired surfaces in a list'
    return [h for h in surfaces if not h.expired]

#-------------------------------------------------------------------------------
# Node of a 2D BSP tree
class BspNode:
    def __init__(self, bound, children):
        self.bound = bound
        self.children = children
        self.surfaces = []

    def toList(self):
        'Convert this branch of the tree to a list for pretty printing'
        return [[c.toList() for c in self.children], self.bound, self.surfaces]

    def plot(self, depth, plotEmptyNodes=True):
        'Plot this branch of the tree'
        for c in self.children:
            c.plot(depth-1, plotEmptyNodes=plotEmptyNodes)
        color = 'b'
        linewidth = max(depth**2/20, 0.25)
        validRefs = findValid(self.surfaces)
        if validRefs:
            color = 'g'
            if plotEmptyNodes:
                # Make the bucket stand out
                linewidth = 6
            center = self.bound.center()
            pl.text(center[0], center[1], str(len(validRefs)),
                    fontsize=12, horizontalalignment='center',
                    verticalalignment='center')
        if plotEmptyNodes or validRefs:
            self.bound.plot(color=color, linewidth=linewidth)

    def insert(self, h, depth):
        'Insert a geometry holder into the branch at the given depth'
        hb = h.bound
        sb = self.bound
        if hb.xmin > sb.xmax or hb.xmax <= sb.xmin or \
           hb.ymin > sb.ymax or hb.ymax <= sb.ymin:
            return
        if depth == 0:
            self.surfaces.append(h)
        else:
            for c in self.children:
                c.insert(h, depth-1)

    def findNode(self, address):
        '''
        Find a node with the given address

        address is a tuple of elements {0,1}, with address[i] giving the half
        of the tree which should be descended into at depth i.
        '''
        if not address:
            return self
        return self.children[address[0]].findNode(address[1:])


def makeBspTree(bound, nsplit, splitX):
    '''
    Create a BSP tree branch of the given depth.
    
    bound - bound for the branch
    nsplit - depth of branch
    splitX - if true, split in X direction first, then in Y.
    '''
    if nsplit == 0:
        return BspNode(bound, [])
    if(splitX):
        xmid = (bound.xmin + bound.xmax)/2
        b1 = Bound(bound.xmin, xmid, bound.ymin, bound.ymax)
        b2 = Bound(xmid, bound.xmax, bound.ymin, bound.ymax)
        return BspNode(bound, [makeBspTree(b1, nsplit-1, False),
                               makeBspTree(b2, nsplit-1, False)])
    else:
        ymid = (bound.ymin + bound.ymax)/2
        b1 = Bound(bound.xmin, bound.xmax, bound.ymin, ymid)
        b2 = Bound(bound.xmin, bound.xmax, ymid, bound.ymax)
        return BspNode(bound, [makeBspTree(b1, nsplit-1, True),
                               makeBspTree(b2, nsplit-1, True)])


#-------------------------------------------------------------------------------
# A wrapper around a BSP tree root node
#
# Provides convenient services on top of those provided by the recursive node
# functions.
class BspTree:
    def __init__(self, bound, depth):
        assert(depth % 2 == 0)
        self.depth = depth
        self.root = makeBspTree(bound, depth, True)

    def plot(self, **kwargs):
        'Plot the tree'
        self.root.plot(self.depth+1, **kwargs)

    def insert(self, h):
        '''
        Insert geometry holder h into the tree at the appropriate level.

        Here we choose the tree level such that the geometry bound is smaller
        than the size of a tree node at that level.  This ensures that the
        geomerty is inserted into at most nodes.
        '''
        # Size of h's bound relative to the root
        Lx = h.bound.xrange() / self.root.bound.xrange()
        Ly = h.bound.yrange() / self.root.bound.yrange()
        # Find desired depth
        #
        # Since we split in the x-direction first, the xdepth is one less than
        # the ydepth for Lx == Ly.
        xdepth = 2*int(-np.log2(Lx))-1
        ydepth = 2*int(-np.log2(Ly))
        depth = max(0, min(xdepth, ydepth, self.depth-1))
        self.root.insert(h, depth+1)

    def findNode(self, address):
        'Find node at the given address.  See BspNode for details.'
        return self.root.findNode(address)


#-------------------------------------------------------------------------------
# Store surfaces in a flat array, rather than a tree structure.
class FlatBucketArray:
    def __init__(self, bound, depth):
        assert(depth % 2 == 0)
        self.N = 2**(depth//2)
        # Set up initially empty buckets
        self.buckets = np.empty((self.N,self.N), dtype='object')
        for j in range(self.N):
            for i in range(self.N):
                self.buckets[i,j] = []
        self.bound = bound

    def insert(self, h):
        # Fractional coordinates relative to self.bound
        xmin = (h.bound.xmin - self.bound.xmin)/self.bound.xrange()
        xmax = (h.bound.xmax - self.bound.xmin)/self.bound.xrange()
        ymin = (h.bound.ymin - self.bound.ymin)/self.bound.yrange()
        ymax = (h.bound.ymax - self.bound.ymin)/self.bound.yrange()
        # Convert fractional coords to bucket indices, and clamp to array
        # extent.
        xmin = max(0, min(self.N-1, int(self.N * xmin)))
        xmax = max(0, min(self.N-1, int(np.ceil(self.N * xmax))))
        ymin = max(0, min(self.N-1, int(self.N * ymin)))
        ymax = max(0, min(self.N-1, int(np.ceil(self.N * ymax))))
        # Insert into array
        for j in range(ymin,ymax):
            for i in range(xmin,xmax):
                self.buckets[j,i].append(h)

    def plot(self):
        dx = self.bound.xrange()/self.N
        dy = self.bound.yrange()/self.N
        bndx = dx * np.array([0,1,1,0,0])
        bndy = dy * np.array([0,0,1,1,0])
        for j in range(self.N):
            for i in range(self.N):
                if findValid(self.buckets[i,j]):
                    pl.plot(self.bound.xmin + dx*i + bndx,
                            self.bound.ymin + dy*j + bndy, 'g')


#-------------------------------------------------------------------------------
# Make a patch to test with
sx = 0.8
sy = 0.8
corners = np.array([[[-sx, 0], [0,sy]],
                    [[0, -sy], [sx,0]]], dtype='d')
corners += [0.501,0.501]
patch = Patch(corners)

bucketsBound = Bound(-2,2,-2,2)

# bucket array version
pl.figure(1)
pl.clf()
bucketsArray = FlatBucketArray(bucketsBound, 10)
recursiveSplit(bucketsArray, Holder(patch), 8, plot=True)
bucketsArray.plot()
pl.axis('equal')
bucketsBound.plot(color='b')

# Count refs in flat array
expiredRefs = 0
validRefs = 0
for row in bucketsArray.buckets:
    for bucket in row:
        currExpiredRefs = len(findExpired(bucket))
        expiredRefs += currExpiredRefs
        validRefs += len(bucket) - currExpiredRefs

print 'Flat array: valid refs = %d,  expired refs = %d,  valid = %.1f%%' \
        % (validRefs, expiredRefs, 100*validRefs/(validRefs+expiredRefs))

# Tree version
pl.figure(2)
pl.clf()
tree = BspTree(bucketsBound, 10)
recursiveSplit(tree, Holder(patch), 8, plot=True)

tree.plot(plotEmptyNodes=False)
pl.axis('equal')
pl.show()
bucketsBound.plot(color='b')

# Count refs in tree
def countRefs(node):
    totExpiredRefs = len(findExpired(node.surfaces))
    totValidRefs = len(node.surfaces) - totExpiredRefs
    for c in node.children:
        v,e = countRefs(c)
        totValidRefs += v
        totExpiredRefs += e
    return (totValidRefs, totExpiredRefs)
validRefs, expiredRefs = countRefs(tree.root)
print 'Tree: valid refs = %d,  expired refs = %d,  valid = %.1f%%' \
        % (validRefs, expiredRefs, 100*validRefs/(validRefs+expiredRefs)) 


# vi :set et:

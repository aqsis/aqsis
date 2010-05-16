from pylab import *
from IPython.Debugger import Tracer; keyboard = Tracer()

#------------------------------------------------------------------------------
# Experimental code for generating samples positions which are well-distributed
# in time and space, and appropriate for use with interleaved sampling.
#
# Demo of some of the experimental code:
'''
tileWidth = 21
t = jitteredSamp(tileWidth**2)
ts = makeTileSet(t, r=2)
h = SpatialHash()
showFiltered(t[applyTileSet(30, ts, h)], lambda x,y:x, gaussKer(8), 4)
show()
'''


def scatter3D(*args, **kwargs):
    '''Convenience function to plot 3D scatter diagrams'''
    from mpl_toolkits.mplot3d import Axes3D
    ax = Axes3D(gcf())
    ax.scatter(*args, **kwargs)

def centred_figimage(image, *args, **kwargs):
    '''
    Show an image in the centre of a figure at natural (unscaled) resolution.
    '''
    bound = gcf().get_window_extent()
    xo = (bound.width - image.shape[0])//2
    yo = (bound.height - image.shape[1])//2
    figimage(image, *args, xo=xo, yo=yo, **kwargs)

def figimages(images, *args, **kwargs):
    '''
    Show two unscaled images evenly laid out horizontally in the figure.
    '''
    bound = gcf().get_window_extent()
    wf = bound.width
    wh = bound.height
    w = images[0].shape[1]
    h = images[0].shape[0]
    n = len(images)
    hgap = (wf - w*n)/(n+1)
    yo = (wh - h)//2
    figimage(images[0], *args, xo=hgap, yo=yo, **kwargs)
    figimage(images[1], *args, xo=2*hgap+w, yo=yo, **kwargs)


def eucDist(a, b):
    '''
    Get the Euclidian distance between a and b

    a and b may be arrays, in which case the sum is taken along the last
    dimension.
    '''
    return sqrt(sum((a-b)**2, axis=-1))



def primes(N):
    '''Find all primes less than or equal to N'''
    isprime = repeat(True, N+1)
    isprime[0] = False
    isprime[1] = False
    for i in range(0,N+1):
        if isprime[i]:
            isprime[2*i::i] = False
    return find(isprime)


def radicalInverse(n, b):
    '''
    Compute the radical inverse of n in base b.  This is handy for
    constructing low-discrepency sequences of sample positions.
    '''
    r = 0.0
    i = 1
    while n != 0:
        d = n % b
        n = n // b
        r += float(d)/b**i
        i += 1
    return r


def radicalInverseSeq(n, b):
    '''Radical inverses of the first n integers starting at 0'''
    return array([radicalInverse(i,b) for i in range(0,n)])


def rinvTuvSamples(n):
    '''time and lens position samples using radical inverse sequence'''
    t = radicalInverseSeq(n, 2)
    r = sqrt(radicalInverseSeq(n, 3))
    theta = 2*pi*radicalInverseSeq(n, 5)
    return array([t, r*cos(theta), r*sin(theta)]).T


def jitteredSamp(n):
    '''Create a simple jittered sampling pattern'''
    return 1.0/n * (r_[0:n] + rand(n))


def emptyTileIndex(width):
    '''Create a width x width tile filled with invalid indices.'''
    return -1 + zeros((width,width), np.int)


def sampleDist(tuv1, tuv2):
    '''
    Get the distance between (t,u,v) tuples when optimizing.

    Optimizing involves choosing a (t,u,v) triple which is well separated
    from the other triples in the spatial neighbourhood.  This function
    determines what "well-separated" means.

    tuv1 and tuv2 should be arrays which are broadcastable to the same shape,
    with (t,u,v) triples in the last dimension.
    '''
    dt = tuv1[...,0] - tuv2[...,0]
    du = tuv1[...,1] - tuv2[...,1]
    dv = tuv1[...,2] - tuv2[...,2]
    return sqrt(4*dt*dt + du*du + dv*dv)
    # Some other possibilities...
    #tDist = fabs(tuv1[...,0] - tuv2[...,0])
    #uvDist = eucDist(tuv1[...,1:],tuv2[...,1:])
    #return 2*tDist + uvDist
    #return eucDist(tuv1, tuv2)
    #return uvDist
    #return fmin(tDist, uvDist)
    #return sqrt(sum((tuv1-tuv2)**2, axis=-1))


def fillTilePeriodic(tuv, ind, r, falloff, activeRegion=None):
    '''
    Fill up a tile with well-distributed time samples.

    *tuv* is the set of time and lens offsets to be indexed by the array ind.
    *ind* is the array which will be filled with time indices.  Elements of
    ind which are equal to -1 will be filled with a valid index before
    returning.  *r* is the distribution quality radius (we aim for all samples
    in rectangles of size 2*r+1 square to be well-distributed).  *falloff*
    also controls the distribution quality, determining how quickly the
    "repulsion" between sample time/lens offset falls off spatially.
    *activeRegion* is a slice tuple indicating the region of ind which is to
    be touched with the filling algorithm.
    '''
    width = ind.shape[0]
    assert(ind.shape[1] == width and len(ind.shape) == 2)
    if activeRegion is None:
        nActive = width**2
        active = True
    else:
        active = zeros(ind.shape, dtype='bool')
        active[activeRegion] = True
        nActive = sum(active)
    # sample indices which remain to be placed
    remainingInds = setdiff1d(r_[0:nActive], unique1d(ind[(ind != -1) & active]))
    # Array of positions in the tile which need to be filled
    toFill = zip(*((ind == -1) & active).nonzero())
    shuffle(toFill)
    assert(len(toFill) == remainingInds.size)
    # Indices into times for positions which have already been placed
    # neighbour indices
    nbhy, nbhx = indices((2*r+1, 2*r+1)) - r
    weights = exp(-falloff*(nbhx**2 + nbhy**2))
    for py, px in toFill:
        # Find list neighbour samples which already have valid sample indices
        # assigned
        x = (nbhx + (px + width)) % width
        y = (nbhy + (py + width)) % width
        nbhInds = ind[y, x]
        w = weights[nbhInds >= 0].ravel()
        nbhInds = nbhInds[nbhInds >= 0].ravel()
        unusedTuv = tuv[remainingInds]
        if nbhInds.size > 0:
            # Choose time sample which minimizes energy
            E = sum(w[:,newaxis]/sampleDist(tuv[nbhInds][:,newaxis],
                                            unusedTuv[newaxis,:]), axis=0)
            i = remainingInds[argmin(E)]
        else:
            # Choose time sample at random if none are within the radius
            i = remainingInds[randint(remainingInds.size)]
        ind[py,px] = i
        # delete i from remainingInds
        remainingInds = delete(remainingInds, find(remainingInds == i))


def makeTileSet(tuv, r=2, ncol=2, falloff=0.7):
    '''
    Create a complete corner tile set with two colours.

    We use the tiling scheme given in the paper "An Alternative for Wang
    Tiles: Colored Edges versus Colored Corners" by A. Lagae and P. Dutre,
    with some modifications for our specific circumstances.
    '''
    width = int(sqrt(tuv.shape[0]))
    assert(width**2 == tuv.shape[0])
    assert(ncol <= 3)

    eWidth = r + r%2
    cWidth = 2*r + eWidth

    cornerInit = emptyTileIndex(width)
    fillTilePeriodic(tuv, cornerInit, r, falloff)

    # Chop sections out of the tile to represent the corners
    corners = [
        cornerInit[0:cWidth,0:cWidth],
        cornerInit[width//2:width//2+cWidth, width//2:width//2+cWidth]
    ]
    if ncol == 3:
        corners.append(cornerInit[0:cWidth, width//2:width//2+cWidth])

    for c in corners:
        c[:r,:r]   = -1
        c[-r:,:r]  = -1
        c[:r,-r:]  = -1
        c[-r:,-r:] = -1

    # Create all edges
    horizEdges = zeros((ncol,ncol), dtype=object)
    vertEdges = zeros((ncol,ncol), dtype=object)
    for edgeInd in range(ncol**2):
        c1 = edgeInd % ncol
        c2 = (edgeInd//ncol) % ncol
        # Insert corner samples into top left and top right of tile
        #
        # +--------------+
        # |C            C|  C = corner
        # |CCeeeeeeeeeeCC|  e = edge to be extracted
        # |CCeeeeeeeeeeCC|
        # |C            C|
        # |              |
        # .              .
        ind = emptyTileIndex(width)
        ind[:cWidth,:cWidth//2] = corners[c1][:,cWidth//2:]
        ind[:cWidth,-cWidth//2:] = corners[c2][:,:cWidth//2]
        # Optimize the tile & extract edge
        fillTilePeriodic(tuv, ind, r, falloff)
        horizEdges[c1,c2] = ind[r:r+eWidth,cWidth//2:-cWidth//2]

        # Same thing for vertical edge
        ind = emptyTileIndex(width)
        ind[:cWidth//2,:cWidth] = corners[c1][cWidth//2:,:]
        ind[-cWidth//2:,:cWidth] = corners[c2][:cWidth//2,:]
        fillTilePeriodic(tuv, ind, r, falloff)
        vertEdges[c1,c2] = ind[cWidth//2:-cWidth//2,r:r+eWidth]

    # We have generated a consistent set of two colour corners and all possible
    # associated edges.  Next we fill in the tiles themselves using these
    # corners & edges as the boundaries

    tileSet = zeros((ncol,ncol,ncol,ncol), dtype=object)
    for tileInd in range(ncol**4):
        c1 = tileInd % ncol
        c2 = (tileInd//ncol) % ncol
        c3 = (tileInd//(ncol**2)) % ncol
        c4 = (tileInd//(ncol**3)) % ncol
        # Make tile with corners:
        # c1--c2
        # |   |
        # c3--c4
        ind = emptyTileIndex(width + eWidth)
        # Fill in corners
        cw = eWidth + r
        ind[:cw,:cw] = corners[c1][-cw:,-cw:]
        ind[:cw,-cw:] = corners[c2][-cw:,:cw]
        ind[-cw:,:cw] = corners[c3][:cw,-cw:]
        ind[-cw:,-cw:] = corners[c4][:cw,:cw]
        # Fill in edges
        # top, bottom
        ind[:eWidth,cw:-cw]  = horizEdges[c1,c2]
        ind[-eWidth:,cw:-cw] = horizEdges[c3,c4]
        # left, right
        ind[cw:-cw,:eWidth]  = vertEdges[c1,c3]
        ind[cw:-cw,-eWidth:] = vertEdges[c2,c4]
        #
        # Now have something like:
        #
        # CCeeeeeeCC
        # C--------C
        # e--------e
        # e--------e
        # C--------C
        # CCeeeeeeCC
        #

        activeRegion = (slice(eWidth//2,-eWidth//2), )*2
        # Remove duplicate indices by choosing next closest avaliable time.
        # TODO: This is a bit quick & dirty; there's probably a better method
        # of doing it...
        indActive = ind[activeRegion].copy()
        unusedIndSet = set(range(width**2)).difference(indActive[indActive != -1])
        ind1d = indActive.ravel()
        indSorted = argsort(ind1d)
        for n in range(1, ind1d.size):
            i = ind1d[indSorted[n-1]]
            if i != -1 and ind1d[indSorted[n]] == i:
                unusedInds = array(list(unusedIndSet))
                iNew = unusedInds[argmin(sampleDist(tuv[i], tuv[unusedInds]))]
                ind1d[indSorted[n-1]] = iNew
                unusedIndSet.remove(iNew)
        ind[activeRegion] = indActive

        # generate a bunch of candidate tile interiors & take the best one.
        ncandidates = 1
        candidates = []
        E = []
        for candidateNum in range(ncandidates):
            indCurr = ind.copy()
            fillTilePeriodic(tuv, indCurr, r, falloff, \
                             activeRegion=activeRegion)

            E.append(sum(sampleQuality(tuv[indCurr], r)))
            candidates.append(indCurr[activeRegion])

        #print 'tile %d done' % (tileInd,)
        tileSet[c1,c2,c3,c4] = candidates[argmin(E)]

    return tileSet


class SpatialHash:
    '''
    A function object that computes a hash of an integer lattice point.

    The result is reduced modulo ncolors so that it lies in the set
    [0, ncolors).

    The method used is successive permutations via a permutation table, (see,
    for example, Ken Perlin's paper "Improved Noise", 2002).
    '''
    def __init__(self, ncolors, tableSize=None):
        if tableSize is None:
            self.N = 256
            # A particular permutation for definiteness
            self.P = array([
               41,  52, 248, 166, 162,  92,  95,  11,   3,  94, 219, 198, 170,
              165,  80, 231,  75,  19,  20, 240, 169, 105,  15,  51,  45,  84,
               85, 157, 232,  23,  97,  14, 174,   0,  82,  42, 145,  69, 111,
              178, 158, 138,  17,  70, 238, 137, 179,  64, 172,  61,  53, 177,
               91,  73, 234,  18, 148, 223,  68, 187, 123,   7,   8,  12,   2,
               27, 133,  66, 194, 160, 189, 142, 253,  81,  87, 212, 193, 102,
              210, 154,  28,  21,  24, 101, 192, 249, 168,  71, 233, 237,  98,
               79, 119, 152, 183, 116, 211, 217,  29, 184, 220,  54,  31, 163,
              134, 120,  78, 245, 195, 204,  56, 140, 180,  99, 112,  57, 146,
              228, 201, 106, 197, 115, 250,  47, 122,  40, 203, 132,  33,  89,
              225, 196,  83,  58, 182,  34, 218, 205,  50,  10, 206,  88,  49,
                1, 109, 254,  67, 144, 150, 230, 235, 213, 181, 242, 208, 246,
               86, 236, 190,  13, 188,  59, 209,  48,  74, 214, 175, 215,  77,
              114,  35, 252, 227, 118,  72, 167, 153, 129, 121, 164, 244, 243,
              226, 125,  44,  62, 239, 136, 151, 173, 171,   5, 110,  96, 216,
              222, 113, 229, 161, 155, 221, 124, 103,   6, 255, 108, 126, 147,
               32,  93, 135, 200, 100,  43, 251, 141, 130, 117,  39,  60, 185,
              149,  38,  26, 143,  55,  36, 127, 128,   4,  16, 207, 139, 159,
               22, 191,  25, 156, 131,  46,  30, 224, 176,  65,  90, 202, 186,
              104, 107,   9, 247,  63, 199,  37, 241,  76])
        else:
            # Generate a new table
            self.N = tableSize
            self.P = r_[0:self.N]
            shuffle(self.P)
        self.ncolors = ncolors
    def __call__(self,x,y):
        return self.P[(self.P[x%self.N] + y)%self.N] % self.ncolors


class SpatialPsudoHash:
    '''A spatial "hash" which really uses psuedo random values
    (not a hash at all!)'''
    def __init__(self, ncols=2):
        self.ncolors = ncols
    def __call__(self, x, y):
        return int_(rand(*array(x).shape)*self.ncolors)


def applyTileSet(width, tileSet, cornerHash):
    '''
    Use a set of tiles to tile the plane.

    width is the width and height of the tiling in number of tiles.
    cornerHash represents a spatial hash function with which to compute colors
    for the tile corners; this determines the particular tile placement used.
    '''
    y,x = indices((width+1, width+1))
    h = cornerHash(x,y)
    tWidth = tileSet[0,0,0,0].shape[0]
    samps = zeros((tWidth*width, tWidth*width), dtype=np.int)
    for ty in range(0,width):
        for tx in range(0,width):
            c1 = h[tx, ty]
            c2 = h[tx+1, ty]
            c3 = h[tx, ty+1]
            c4 = h[tx+1, ty+1]
            tile = tileSet[c1,c2,c3,c4]
#            tile = tile.copy()
#            shuffle(tile.ravel())
            # Compute the appropriate tile using a spatial hash.
            samps[ty*tWidth:(ty+1)*tWidth, tx*tWidth:(tx+1)*tWidth] = tile
    return samps


def saveTileSet(fileName, tuv, tileSet):
    out = open(fileName, 'w')
    #out.write('AQSF\n')
    out.write('%d %d\n' % tileSet[0,0,0,0].shape)
    tuv.tofile(out, sep=' ')
    out.write('\n')
    for tile in tileSet.flat:
        tile.tofile(out, sep=' ')
        out.write('\n')
    out.close()


def hashTilingInds(width, hash):
    '''
    Compute tile indices for tiling a (width x width) square using the
    supplied hash function for the tile corner colors.
    '''
    ty,tx = indices((width,width))
    ncol = hash.ncolors
    return ((hash(tx,ty)*ncol + hash(tx+1,ty))*ncol + hash(tx,ty+1))*ncol + \
            hash(tx+1,ty+1)


def findGoodHashPerm(tableSize, ncol):
    '''
    Find a hash function permutation table which is "nice" in the sense that
    it results in few adjacent repititions of the same tile.

    WARNING: Actually, this function doesn't really do a good job; it results
    in permutations which have fewer adjacencies overall, but some regions
    which locally have a lot of repitition.
    '''
    while True:
        h = SpatialHash(ncol, tableSize=tableSize)
        ind = hashTilingInds(tableSize, h)
        goodness0 = sum(diff(ind, axis=0) == 0) / float(ind.size)
        goodness1 = sum(diff(ind, axis=1) == 0) / float(ind.size)
        print abs(goodness0-goodness1), goodness0, goodness1
        if abs(goodness0-goodness1) < 0.02 and goodness0 < 0.057 \
                                           and goodness1 < 0.057:
            break
    print h.P
    return h


#------------------------------------------------------------------------------
# Stuff for visualizing tile quality

def sampleQuality(tuv, r, falloff=0.7):
    '''
    Show the quality of the provided sample times/lens offsets as a function
    of space.

    Poor quality samples will show up as large values in the per-sample
    "energy" which is computed by this function.  Note that periodic boundary
    conditions are used.
    '''
    E = 0
    for j in range(-r,r):
        for i in range(-r,r):
            if i == 0 and j == 0:
                continue
            w = exp(-falloff*(i**2 + j**2))
            E += w/sampleDist(tuv, roll(roll(tuv, i, 0), j, 1))
    return E


def showTileQuality(tuv, tileSet, r):
    '''Show the quality of all tiles in a 2-colour set'''
    for c1 in range(0,2):
        for c2 in range(0,2):
            for c3 in range(0,2):
                for c4 in range(0,2):
                    subplot(4, 4, ((c1*2+c2)*2+c3)*2+c4 + 1)
                    imshow(sampleQuality(tuv[tileSet[c1][c2][c3][c4]], r), 
                           interpolation='nearest', vmax=100, cmap=cm.gray)


#------------------------------------------------------------------------------
# Stuff for simulating image formation via sampling & filtering

def applyFilter(A, ker, step):
    '''
    Apply the filter kernel ker to the array A.

    step is the number of pixels in A between successive filtering operations.
    For 4x4 samples per pixel, we'd want to set step=4.
    '''
    offset = (ker.shape[0]-step)//2
    borderPix = (offset-1)//step + 1
    borderSamps = borderPix*step - offset
    width = A.shape[0]//step - 2*borderPix
    height = A.shape[1]//step - 2*borderPix
    result = zeros([width,height])
    for j in range(0,ker.shape[1]):
        for i in range(0,ker.shape[0]):
            iA = i# + borderSamps
            jA = j# + borderSamps
            result += ker[i,j]*A[iA:iA+width*step:step, \
                                 jA:jA+height*step:step]
    return result


def gaussKer(N, tightness=1):
    '''Create a gaussian filter kernel'''
    x,y = ogrid[0:N,0:N]
    x = 2*(0.5+x)/N - 1
    y = 2*(0.5+y)/N - 1
    k = exp(-tightness*8*(x**2 + y**2))
    return k/sum(k)

def boxKer(N):
    '''Create a box filter kernel'''
    k = ones((N,N))
    return k/sum(k)


def showFilteredDof(tuv, kernel, step, diskRad=0.25, dofRad=0.15):
    '''
    Simulation of the DoF sampling process for a constant depth disk.
    '''
    pos = indices(tuv.shape[0:2]).swapaxes(0,2)/float(tuv.shape[0])
    diskPos = array([0.5,0.5])
    #diskPos = multiply.outer(tuv[...,0], array([1,1])) # combined DoF & MB
    # Compute sampled result
    imRaw = sqrt(sum((pos - diskPos + dofRad*tuv[...,1:])**2, axis=-1)) < diskRad
    imFilt = applyFilter(imRaw, kernel, step)
    # Compute exact result
    imExactRaw = zeros(imRaw.shape)
    # Three regions in exact image:
    # 1) filter entirely not in disk (yeilds black)
    # 2) filter entirely in disk (yeilds white)
    dist = eucDist(pos, diskPos)
    imExactRaw[dist <= (diskRad - dofRad)] = 1
    # 2) partial overlap between filter & disk
    partial = (dist > (diskRad - dofRad)) & (dist < (diskRad + dofRad))
    d = dist[partial]
    a = d/2 * (1 + (dofRad/d)**2 - (diskRad/d)**2)
    b = d/2 * (1 + (diskRad/d)**2 - (dofRad/d)**2)
    h = sqrt(dofRad**2 - a**2)
    imExactRaw[partial] = (dofRad**2*arctan2(h,a) + \
                           diskRad**2*arctan2(h,b) - (a+b)*h) / (pi*dofRad**2)
    imExact = applyFilter(imExactRaw, kernel, step)
    stddev = std(imFilt.ravel() - imExact.ravel())
    print
    print "std deviation =", stddev
    # Plot results.
    #figimage(imExact, vmin=0, vmax=1, cmap=cm.gray)
    #centred_figimage(abs(imExact-imFilt), vmin=0, vmax=1, cmap=cm.gray)
    figimages([imFilt, abs(imExact-imFilt)], vmin=0, vmax=1, cmap=cm.gray)


def showFilteredMb(tuv, kernel, step, cutTime=lambda x,y:x):
    '''
    Cheap simulation of the sampling process, followed by filtering & display.

    cutTime is a function which maps (x,y) position in the image into a cutoff
    time.  Sample times less than the cutoff are considered to have "hit an
    object" and are coloured white.  Sample times which are greater than the
    cutoff are coloured black.  The results are then filtered with the
    provided kernel and step, by passing to applyFilter().
    '''
    y,x = mgrid[0:tuv.shape[0], 0:tuv.shape[1]]
    x = x/float(tuv.shape[1])
    y = y/float(tuv.shape[0])
    cutTime = cutTime(x,y)
    # Compute filtered & exact images, difference & std deviation
    imFilt = applyFilter(tuv[...,0] < cutTime, kernel, step)
    imExact = applyFilter(cutTime, kernel, step)
    stddev = std(imFilt.ravel() - imExact.ravel())
    subplot(1, 2, 1)
    imshow(imFilt, interpolation='nearest', vmin=0, vmax=1, cmap=cm.gray)
    title('Filtered image\nstd dev = %f' % stddev)
    subplot(1, 2, 2)
    imshow(abs(imFilt-imExact), interpolation='nearest',
           vmin=0, vmax=1, cmap=cm.gray)
    title('difference [abs(filtered - exact)]')
    print
    print "std deviation =", stddev


#------------------------------------------------------------------------------

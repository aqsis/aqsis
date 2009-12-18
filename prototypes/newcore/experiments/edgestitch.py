from pylab import *
import time

def stitch(e1, e2):
    if len(e1) < len(e2):
        e1, e2 = e2, e1
    clf()
    y = [-0.1,0.1]
    plot(e1, 0*e1 + y[0], 'r*-')
    plot(e2, 0*e2 + y[1], 'bx-')
    plot([e1[0], e2[0]], y, 'k:')
    j = 0
    for i in range(1,len(e1)):
        if j != len(e2)-1 and abs(e1[i] - e2[j]) > abs(e1[i] - e2[j+1]):
            j += 1
        plot([e1[i], e2[j]], y, 'k:')
    axis('equal')
    xlim([-0.1,1.1])

for i in range(3,20):
    stitch(linspace(0,1,5), linspace(0,1,i))
    draw()
    time.sleep(0.5)

#!/usr/bin/python

import time
import subprocess
import sys
import math
import optparse

# Little program to time a process for a number of repetitions, and compute
# some statistics.

parser = optparse.OptionParser(usage=
'''%prog [options] -- command [arg1 ...]

Time a process for a number of repetitions, and print some statistics about the
resulting set of times.  Some idea of the error is provided by estimating the
error in the calculated mean time.  Use the -r option to provide a reference
time for comparison and the script will determine whether there's any change.
A crude measure of the statistical confidence in the change is provided.''')
parser.add_option('-n', default=5, type='int', dest='nRuns',
                  help='Number of timing runs for the command')
parser.add_option('-r', '--reference', default=None, type='float',
                  dest='refTime',
                  help='Reference timing for comparison')
opts, command = parser.parse_args(sys.argv[1:])

if len(command) < 1:
    parser.print_help()
    sys.exit(1)

nRuns = opts.nRuns


# Run command nRuns times & collect times for analysis
tAll = []
devnull = open('/dev/null', 'w')
# devnull = open('nul:', 'w')  # apparently you would use this on windows (?)
for i in range(0,nRuns):
    # Run the command, ignoring stdout & stderr.
    t1 = time.time()
    subprocess.Popen(command, stdout=devnull, stderr=devnull).wait()
    t = time.time() - t1
    sys.stdout.write('%.3f ' % (t,))
    sys.stdout.flush()
    tAll.append(t)


# Compute & print stats
tSum = 0.0
for t in tAll:
    tSum += t
mean = tSum/nRuns

# We've got a tiny sample, so use less biased estimator for stdandard
# deviation.
variance = 0.0
for t in tAll:
    variance += (t - mean)**2
variance /= nRuns-1
stddev = math.sqrt(variance)
errorinmean = stddev/math.sqrt(nRuns)

print u'''
Stats:
    mean:    %f \xb1 %.2f%%
    stddev:  %f''' \
% (mean, 100*errorinmean/mean, stddev)

# Compare to some given reference time, if supplied.
refTime = opts.refTime
if refTime is not None:
    # Determine how statistically significant the difference between refTime
    # and the measured time is based on the estimate of the error in the mean.
    relMeanDifference = abs(mean - refTime) / errorinmean
    confidenceStr = 'no change?'
    if relMeanDifference > 1.5:
        confidenceStr = 'possible change'
    if relMeanDifference > 3:
        confidenceStr = 'likely changed'
    if relMeanDifference > 10:
        confidenceStr = 'changed'
    print '    speedup: %f%%   (confidence %0.2f, %s)' \
            % (100*(refTime - mean)/mean, relMeanDifference, confidenceStr)


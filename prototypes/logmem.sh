#!/bin/bash

# Simple script to log the amount of system memory used by a given command.
# This measures the resident set size in kB using ps.  (Hopefully the rss is
# the right thing to measure!)

if [[ -z $1 ]] ; then
	echo "Usage: logmem.sh [-i interval_in_s] command arg1 arg2 ..."
	exit 0
fi

logInterval=1
if [[ "$1" == "-i" ]] ; then
	shift
	logInterval=$1
	shift
fi

cmdName=$(basename $1)

# Start command in background
$@ &

procPid=$(ps -o comm,pid | grep "^$cmdName" | awk '{print $2}');
logFile=logmem_$procPid.txt

echo "logging memory usage to $logFile"

# Get the resident size every log interval.
while ps -o comm | grep -q "^$cmdName"; do
	ps -o comm,rss | grep "^$cmdName" | tee -a $logFile 1>&2
	sleep $logInterval
done

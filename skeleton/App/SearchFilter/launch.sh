#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`

cd $progdir
echo $0 $* > debug.log

HOME=$progdir LD_LIBRARY_PATH="$progdir:$LD_LIBRARY_PATH" ./filter_roms 2>&1 >> debug.log

# Restart if return_code is 42
while [ $? -eq 42 ]; do
    HOME=$progdir LD_LIBRARY_PATH="$progdir:$LD_LIBRARY_PATH" ./filter_roms 2>&1 >> debug.log
done

#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`

cd $progdir
echo "$(date):" $0 $* >> debug.log

if [ "$1" == "" ]
then
    LD_LIBRARY_PATH="$progdir/lib:$LD_LIBRARY_PATH" ./filter 2>&1 >> debug.log
elif [ "$1" == "clear" ]
then
    LD_LIBRARY_PATH="$progdir/lib:$LD_LIBRARY_PATH" ./search clear 2>&1 >> debug.log
elif [ "$1" == "search" ] || [ "$(basename "$1")" == "[Enter search term...].txt" ]
then
    LD_LIBRARY_PATH="$progdir/lib:$LD_LIBRARY_PATH" ./search 2>&1 >> debug.log
else
    echo "$(date):" "eval" "$1" >> debug.log
    cd /mnt/SDCARD/RetroArch
    eval $1
fi
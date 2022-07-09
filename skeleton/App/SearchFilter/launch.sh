#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`

cd $progdir
echo "$(date):" $0 $* >> debug.log

if [ "$1" == "" ]
then
    LD_LIBRARY_PATH="$progdir/lib:$LD_LIBRARY_PATH" ./filter 2>&1 >> debug.log
elif [ "$(basename "$1")" == "Enter keyword....txt" ]
then
    LD_LIBRARY_PATH="$progdir/lib:$LD_LIBRARY_PATH" ./search 2>&1 >> debug.log
else
    "${@:2}"
fi

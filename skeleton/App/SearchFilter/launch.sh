#!/bin/sh
echo $0 $*
progdir=`cd -- "$(dirname "$0")" >/dev/null 2>&1; pwd -P`

cd $progdir
echo "$(date):" $0 $* >> debug.log

if [ "$1" == "" ]
then
    echo "$(date):" "[filter]" >> debug.log
    LD_LIBRARY_PATH="$progdir/lib:$LD_LIBRARY_PATH" ./filter 2>&1 >> debug.log
elif [ "$1" == "clear" ]
then
    echo "$(date):" "[clear]" >> debug.log
    LD_LIBRARY_PATH="$progdir/lib:$LD_LIBRARY_PATH" ./search clear 2>&1 >> debug.log
elif [ "$1" == "search" ] || [ "$(basename "$1")" == "Enter search term....txt" ]
then
    echo "$(date):" "[search]" >> debug.log
    LD_LIBRARY_PATH="$progdir/lib:$LD_LIBRARY_PATH" ./search 2>&1 >> debug.log
else
    launch=`echo "$1" | awk '{split($0,a,":"); print a[1]}'`
    romfile=`echo "$1" | awk '{split($0,a,":"); print a[2]}'`
    echo "$(date):" "[launch]:" "$launch" >> debug.log
    echo "$(date):" "[rom]:" "$romfile" >> debug.log
    cd /mnt/SDCARD/RetroArch
    chmod a+x "$launch"
    "$launch" "$romfile" 2>&1 >> $progdir/debug.log
fi

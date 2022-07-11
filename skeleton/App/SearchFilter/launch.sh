#!/bin/sh
echo $0 $*
progdir=`cd -- "$(dirname "$0")" >/dev/null 2>&1; pwd -P`
filename=`basename "$1"`

cd $progdir
echo "$(date):" $0 $* >> debug.log
echo "$filename" >> debug.log

if [ "$1" == "" ]
then
    LD_LIBRARY_PATH="$progdir/lib:$LD_LIBRARY_PATH" ./filter 2>&1 >> debug.log

elif [ "$1" == "clear" ]
then
    LD_LIBRARY_PATH="$progdir/lib:$LD_LIBRARY_PATH" ./search clear 2>&1 >> debug.log

elif [ "$1" == "nocache" ]
then
    LD_LIBRARY_PATH="$progdir/lib:$LD_LIBRARY_PATH" ./tools nocache 2>&1 >> debug.log

elif [ "$1" == "search" ] || [ "$filename" == "Enter search term....txt" ]
then
    LD_LIBRARY_PATH="$progdir/lib:$LD_LIBRARY_PATH" ./search 2>&1 >> debug.log

elif [ "$1" == "boxart" ] || [ "$filename" == "Fix favorites boxart.txt" ]
then
    LD_LIBRARY_PATH="$progdir/lib:$LD_LIBRARY_PATH" ./tools boxart 2>&1 >> debug.log

elif [ "$1" == "favsort" ] || [ "$filename" == "Sort favorites.txt" ]
then
    LD_LIBRARY_PATH="$progdir/lib:$LD_LIBRARY_PATH" ./tools favsort 2>&1 >> debug.log

elif [ "$1" == "favtools" ] || [ "$filename" == "Add tools to favorites.txt" ]
then
    LD_LIBRARY_PATH="$progdir/lib:$LD_LIBRARY_PATH" ./tools favtools 2>&1 >> debug.log

else
    launch=`echo "$1" | awk '{split($0,a,":"); print a[1]}'`
    romfile=`echo "$1" | awk '{split($0,a,":"); print a[2]}'`
    cd /mnt/SDCARD/RetroArch
    chmod a+x "$launch"
    "$launch" "$romfile" 2>&1 >> $progdir/debug.log
fi

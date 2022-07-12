#!/bin/sh
echo $0 $*
progdir=`cd -- "$(dirname "$0")" >/dev/null 2>&1; pwd -P`
dname=`dirname "$1"`
filename=`basename "$1"`
datadir="/mnt/SDCARD/Emu/SEARCH/../../App/SearchFilter/data"

cd $progdir
echo "$(date):" $0 $* >> debug.log
echo "$filename" >> debug.log

if [ "$dname" = "$datadir" ] || [ "$dname" = "$datadir/~Tools" ]
then
    if [ "$filename" = "Enter search term....txt" ]
    then
        mode="search"
    elif [ "$filename" = "Fix favorites boxart.txt" ]
    then
        mode="boxart"
    elif [ "$filename" = "Sort favorites.txt" ]
    then
        mode="favsort"
    elif [ "$filename" = "Add tools to favorites.txt" ]
    then
        mode="favtools"
    elif [ "$filename" = "Clean recent list.txt" ]
    then
        mode="recents"
    else
        mode="noop"
    fi
elif [ "$1" = "" ]
then
    mode="filter"
else
    mode="$1"
fi

echo "launch mode:" $mode

if [ "$mode" = "noop" ]
then
    noop=1

elif [ "$mode" = "filter" ]
then
    LD_LIBRARY_PATH="$progdir/lib:$LD_LIBRARY_PATH" ./filter 2>&1 >> debug.log

elif [ "$mode" = "clear" ]
then
    LD_LIBRARY_PATH="$progdir/lib:$LD_LIBRARY_PATH" ./search clear 2>&1 >> debug.log

elif [ "$mode" = "search" ]
then
    LD_LIBRARY_PATH="$progdir/lib:$LD_LIBRARY_PATH" ./search 2>&1 >> debug.log

elif [ "$mode" = "boxart" ] || [ "$mode" = "favsort" ] || [ "$mode" = "favtools" ] || [ "$mode" = "recents" ]
then
    # Mode must be a tool function
    LD_LIBRARY_PATH="$progdir/lib:$LD_LIBRARY_PATH" ./tools "$mode" 2>&1 >> debug.log

else
    launch=`echo "$1" | awk '{split($0,a,":"); print a[1]}'`
    romfile=`echo "$1" | awk '{split($0,a,":"); print a[2]}'`
    cd /mnt/SDCARD/RetroArch
    chmod a+x "$launch"
    "$launch" "$romfile" 2>&1 >> $progdir/debug.log

fi

cd $progdir
LD_LIBRARY_PATH="$progdir/lib:$LD_LIBRARY_PATH" ./tools recents --silent 2>&1 >> debug.log

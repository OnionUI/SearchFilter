#!/bin/sh
echo $0 $*
progdir=`cd -- "$(dirname "$0")" >/dev/null 2>&1; pwd -P`
ext=`echo "$(basename "$1")" | awk -F. '{print tolower($NF)}'`
lib="$progdir/lib:$LD_LIBRARY_PATH"

cd $progdir
echo "$(date):" $0 $* >> debug.log

if [ "$ext" = "miyoocmd" ]; then
    filename=`basename "$1" .miyoocmd`

    if [ "$filename" = "Enter search term..." ]; then
        mode="search"
    elif [ "$filename" = "1. Fix favorites boxart" ]; then
        mode="boxart"
    elif [ "$filename" = "2. Sort favorites (A-Z)" ]; then
        mode="favsort"
    elif [ "$filename" = "3. Sort favorites (by system)" ]; then
        mode="favsort2"
    elif [ "$filename" = "4. Add tools to favorites" ]; then
        mode="favtools"
    elif [ "$filename" = "5. Clean recent list" ]; then
        mode="recents"
    elif [ "$filename" = "6. Install filter" ]; then
        mode="install_filter"
    elif [ "$filename" = "7. Uninstall filter" ]; then
        mode="uninstall_filter"
    elif [ "$filename" = "~Filter" ]; then
        mode="filter"
    elif [ "$filename" = "~Clear filter" ]; then
        mode="clear_filter"
    elif [ "$filename" = "~Refresh roms" ]; then
        mode="refresh"
    else
        mode="noop"
    fi
elif [ "$1" = "" ]; then
    mode="search"
else
    mode="$1"
fi

echo "launch mode:" $mode

if [ "$mode" = "noop" ]; then
    echo noop >> $progdir/debug.log

elif [ "$mode" = "filter" ] || [ "$mode" = "clear_filter" ] || [ "$mode" = "install_filter" ] || [ "$mode" = "uninstall_filter" ] || [ "$mode" = "refresh" ]; then
    LD_LIBRARY_PATH="$lib" ./filter "$mode" "$2" 2>&1 >> debug.log
elif [ "$mode" = "clear" ]; then
    LD_LIBRARY_PATH="$lib" ./search clear 2>&1 >> debug.log

elif [ "$mode" = "search" ]; then
    LD_LIBRARY_PATH="$lib" ./search 2>&1 >> debug.log

elif [ "$mode" = "boxart" ] || [ "$mode" = "favsort" ] || [ "$mode" = "favsort2" ] || [ "$mode" = "favtools" ]; then
    # Mode must be a tool function
    LD_LIBRARY_PATH="$lib" ./tools "$mode" --display 2>&1 >> debug.log

elif [ "$mode" = "recents" ]; then
    # Mode must be a tool function
    LD_LIBRARY_PATH="$lib" ./tools recents --clean_all --display 2>&1 >> debug.log

else
    launch=`echo "$1" | awk '{split($0,a,":"); print a[1]}'`
    romfile=`echo "$1" | awk '{split($0,a,":"); print a[2]}'`
    cd /mnt/SDCARD/RetroArch
    chmod a+x "$launch"
    "$launch" "$romfile" 2>&1 >> $progdir/debug.log

fi

cd $progdir
LD_LIBRARY_PATH="$lib" ./tools recents 2>&1 >> debug.log

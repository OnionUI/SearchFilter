#!/bin/sh
progdir=`dirname "$0"`
emupath=`echo "$0" | awk '{split($0,a,"/../../"); print a[1]}'`
ext=`echo "$(basename "$1")" | awk -F. '{print tolower($NF)}'`

if [ "$ext" = "miyoocmd" ]
then
    $progdir/launch.sh "$1" "$emupath"
else
    launch="$emupath/launch.sh"
    chmod a+x "$launch"
    "$launch" "$1"
fi

if [ -f "$progdir/.disableRecentlist" ]
then
    rm -f /mnt/SDCARD/Roms/recentlist.json
fi

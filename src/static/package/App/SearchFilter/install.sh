#!/bin/sh
echo $0 $*
progdir=`cd -- "$(dirname "$0")" >/dev/null 2>&1; pwd -P`

cd $progdir

if [ -f .disableRecentlist ]
then
    recentlist=0
else
    recentlist=1
fi

if [ -f PACKAGE.zip ]
then
    mv ./PACKAGE.zip /tmp/SEARCHFILTER_PACKAGE.zip

    cd /mnt/SDCARD
    rm -rf $progdir/*
    unzip -o /tmp/SEARCHFILTER_PACKAGE.zip

    cd $progdir

    if [ recentlist -eq 0 ]
    then
        mv .disableRecentlist_ .disableRecentlist
    fi

    rm -f /tmp/SEARCHFILTER_PACKAGE.zip
else
    rm -f config.json
    rm -f install.sh
    rm -f icon.png
fi

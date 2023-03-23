#!/bin/sh
cd /mnt/SDCARD/.tmp_update
./bin/filter install_filter

rm -f /appconfigs/romwinidx.json 2> /dev/null

#!/bin/bash
#
# Téléverse un firmware sur un ensemble de radiateurs
# Si le script a un seul argument, il s'agit d'un unique radiateur
# Si le script a deux arguments, il s'agit d'une série de radiateurs
#

espota=~/Library/Arduino15/packages/esp32/hardware/esp32/2.0.0/tools/espota.py
firmware=FirmwareRadiateur.ino.mhetesp32minikit.bin

if [ ! -f "$firmware" ]
then
    echo "export the firmware first"
    exit 1
fi

if [ $# -eq 2 ]
then
    pass=$1
    start=$2
    stop=$2
else
    if [ $# -eq 3 ]
    then
        pass=$1
        start=$2
        stop=$3
    else
        echo "usage: ./upload.sh <password> <num> or ./upload.sh <password> <start num> <stop num>"
    fi
fi

if [ $start -gt $stop ]
then
    echo "usage: ./upload.sh <password> <start num> <stop num> with <start num> ≤ <stop num>"
fi

# Vérifie que espota.py est présent
if [ ! -f "$espota" ]
then
    echo "espota.py not found"
    exit 1
fi

# Change son mode
chmod +x "$espota"

# 
for i in $(seq $start $stop)
do
   $espota --ip=heater$i.local --auth=$pass --file=$firmware -r
done

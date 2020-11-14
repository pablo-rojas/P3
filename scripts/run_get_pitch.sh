#!/bin/bash

# Put here the program (maybe with path)

pot=-44
rmax=0.35
r1=0.68
zcr=0.0

GETF0="get_pitch --power "$pot" --rmaxnorm "$rmax" --r1norm "$r1" --zcr "$zcr

for fwav in pitch_db/train/*.wav; do
    ff0=${fwav/.wav/.f0}
    echo "$GETF0 $fwav $ff0 ----"
    $GETF0 $fwav $ff0 > /dev/null || (echo "Error in $GETF0 $fwav $ff0"; exit 1)
done

exit 0

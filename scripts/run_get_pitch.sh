#!/bin/bash

# Put here the program (maybe with path)

pot=-44
rmax=0.35
r1=0.68
zcr=0.5



for r1 in $(seq 0.67 0.01 0.75); do
    
    for pot in $(seq 44.0 0.1 45.0); do
        
        for rmax in $(seq 0.34 0.01 0.45); do

            for zcr in $(seq 0.4 0.01 0.7); do

                GETF0="get_pitch --power "$pot" --rmaxnorm "$rmax" --r1norm "$r1" --zcr "$zcr

                for fwav in pitch_db/train/*.wav; do
                    ff0=${fwav/.wav/.f0}
                    echo "$GETF0 $fwav $ff0 ----"
                    $GETF0 $fwav $ff0 > /dev/null || (echo "Error in $GETF0 $fwav $ff0"; exit 1)
                done

                echo $pot$'\t'$rmax$'\t'$r1$'\t'$zcr  >> total5.txt
                ~/PAV/bin/pitch_evaluate pitch_db/train/*.f0ref | head -n 508 > evaluation5.txt
                tail -n 1 evaluation5.txt | awk '{ print $3 }' >> total5.txt

            done
        done
    done
done




exit 0

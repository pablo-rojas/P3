#!/bin/bash

# Put here the program (maybe with path)

pot=-44
rmax=0.35
r1=0.68
zcr=0.5



for r1 in $(seq 0.67 0.01 0.75); do
    
    for pot in $(seq 43.0 0.1 44.0); do
        
        for rmax in $(seq 0.32 0.01 0.45); do

            for zcr in $(seq 0.4 0.01 0.7); do

                GETF0="get_pitch --power "$pot" --rmaxnorm "$rmax" --r1norm "$r1" --zcr "$zcr

                for fwav in pitch_db/train/*.wav; do
                    ff0=${fwav/.wav/.f0}
                    echo "$GETF0 $fwav $ff0 ----"
                    $GETF0 $fwav $ff0 > /dev/null || (echo "Error in $GETF0 $fwav $ff0"; exit 1)
                done

                echo $pot$'\t'$rmax$'\t'$r1$'\t'$zcr  >> total.txt
                ~/PAV/bin/pitch_evaluate pitch_db/train/*.f0ref | head -n 508 > evaluation.txt
                tail -n 1 evaluation.txt | awk '{ print $3 }' >> total.txt

            done
        done
    done
done




exit 0

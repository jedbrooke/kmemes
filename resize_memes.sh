#!/bin/bash
MEMES=/home/jedbrooke/source/kmemes/imgur_memes
OUTPUT_DIR=/home/jedbrooke/source/kmemes/memes_resized
for f in $(ls $MEMES); do
    if [ ! -f $OUTPUT_DIR/$f ]; then
        echo "converting $f"
        convert $MEMES/$f -resize 512x512! $OUTPUT_DIR/$f
        if [ $? -ne 0 ]; then
            echo "error with $f"
            rm $MEMES/$f
        fi
    fi
done
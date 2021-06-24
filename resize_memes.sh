#!/bin/bash
MEMES=/home/jedbrooke/Documents/source/kmemes/memes
OUTPUT_DIR=/home/jedbrooke/Documents/source/kmemes/memes_resized
for f in $(ls $MEMES); do
    if [ ! -f $OUTPUT_DIR/$f ]; then
        echo "converting $f"
        convert $MEMES/$f -resize 512x512! $OUTPUT_DIR/$f
    fi
done
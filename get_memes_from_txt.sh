OUTPUT_DIR=/home/jedbrooke/source/kmemes/imgur_memes
for i in $(cat $1); do
    wget -P $OUTPUT_DIR $i --quiet
    echo "fetching $i"
    sleep 1
done 
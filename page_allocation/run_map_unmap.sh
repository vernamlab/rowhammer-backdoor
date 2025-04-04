#!/bin/bash
## Create a large zero file
make
truncate -s 10M nullbytes
DIR="./multiple_run"
echo  $DIR
for VARIABLE in {1..1}
do
    echo $VARIABLE

    sync; echo 3 > /proc/sys/vm/drop_caches
    #sudo ./relocate 100
    sudo taskset -c 15 ./map_unmap
    mv buffer_addr.txt "$DIR/${VARIABLE}_buffer_addr.txt"
    mv file_addr.txt "$DIR/${VARIABLE}_file_addr.txt"
    mv unmapped_addr.txt "$DIR/${VARIABLE}_unmapped_addr.txt"

done
sudo chown `whoami` multiple_run/*
cd $DIR
python3 ../find_ngram.py

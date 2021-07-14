#!/bin/bash

file=$1
sector=$2
length=$3

xxd -u -a -g 1 -c 16 -s $[$sector*512] -l $length $file

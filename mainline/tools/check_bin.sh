#!/bin/bash

file=$1
addr=$2
length=$3

xxd -u -a -g 1 -c 16 -s $addr -l $length $file

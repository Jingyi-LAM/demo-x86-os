#!/bin/bash

dd if=output/20MB.img of=dump.log bs=512 skip=36864 count=80

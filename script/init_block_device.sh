#!/bin/bash

available_loop=$(losetup -f)
echo "$available_loop" > ./configure
sudo losetup -v `losetup -f` ./raw_block
sudo chmod a+rw $available_loop
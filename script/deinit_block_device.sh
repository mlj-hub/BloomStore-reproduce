#!/bin/bash

loop_dev=`cat configure`
sudo losetup -d $loop_dev
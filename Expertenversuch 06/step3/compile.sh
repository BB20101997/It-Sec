#!/bin/bash
clear && make -C /lib/modules/$(uname -r)/build M=$PWD modules



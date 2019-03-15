#!/bin/bash
index=0
index=9

while true; do

  #index=$((index+1))
  echo "%${index}$.1p %${index}n"
  echo "%${index}$.153p %${index}\$n" | ./fmtstr "%p %p"

  read -n 1 -p "" test
done

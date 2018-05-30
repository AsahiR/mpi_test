#!/usr/bin/env bash
# sort log/proc28* by proc and output for plot.py
output=varying_core
find log -type f -name '*_13_13' | 
grep -v v |
xargs cat |
gawk 'NR%2==0{print}' |
sort -t,  -k1,1 -n >$output

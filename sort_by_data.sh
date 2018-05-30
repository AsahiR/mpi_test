#!/usr/bin/env bash
# sort log/proc28* by data_size and output for plot.py
output=varying_data
find log -type f -name 'proc28*' | 
grep -v v |
xargs cat |
gawk 'NR%2==0{print}' |
sort -t,  -k3,3 -n >$output

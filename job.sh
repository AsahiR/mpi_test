#!/bin/sh
#$ -cwd
#$ -l f_node=1
#$ -l h_rt=0:10:00

. /etc/profile.d/modules.sh
module load cuda
module load openmpi

nt=40
base=2
index=13
proc=28

while getopts ":n:p:i:" OPT
do
  case $OPT in
    i) index=$OPTARG;;
    n) procpernode=$OPTARG;;
    p) proc=$OPTARG;;
    :) echo "option argument need.-p=proc.-n=procpernode.-i=index"
  esac
done

if [ ! $procpernode ]; then
  procpernode=$proc
fi

x_size=$(($base ** $index))
y_size=$(($base ** $index))
mpirun -n $proc -npernode $procpernode ./diffusion -x $x_size -y $y_size

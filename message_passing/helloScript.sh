#!/bin/bash
#PBS -l nodes=4:ppn=2:physical
#PBS -l walltime=00:10:00
#PBS -o /home/Students/cdelaney3/cloud_parallel/Assignment#5_Delaney/job_out
#PBS -j oe
#PBS -N Assignment#5

date
export PROGRAM="/home/Students/cdelaney3/cloud_parallel/Assignment#5_Delaney"
mpirun -machinefile $PBS_NODEFILE $PROGRAM
date
exit 0

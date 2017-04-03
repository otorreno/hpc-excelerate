#!/bin/bash
# number of tasks:
#SBATCH -n 5
# memory:
#SBATCH --mem=2gb
# time:
#SBATCH --time=00:10:00
# Set output and error files (deactivated here)
##SBATCH --error=job.%J.err
##SBATCH --output=job.%J.out
# partition:
#SBATCH --partition=local

module load openmpi

time mpirun -np 5 ~/hpc-excelerate/day1/bin/mpi_mapreduce_dynamic_distribution query.fasta-map
echo "Done." 

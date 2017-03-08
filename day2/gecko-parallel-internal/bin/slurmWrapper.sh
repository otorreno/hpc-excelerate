#!/bin/bash

# number of cores in the same node
##SBATCH -c 1
# amount of memory
##SBATCH --mem=20gb
# walltime
##SBATCH --time=10:00:00

echo "time $@"
time $@

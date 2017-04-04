# GECKO (original version)
Software aimed at pairwise sequence comparison generating high quality results (equivalent to MUMmer) with controlled memory consumption and comparable or faster execution times particularly with long sequences.

http://bitlab-es.com/gecko/

## Compilation instructions
First you need to enter in the **gecko/src** folder of the **day2** folder. Then the following line will compile all the GECKO modules:
`make all`

## Running your first comparisons
The **data** folder present at the same level as the **src** folder contains two folders:
* **exp1** which contains two genomes to be compared 
* **exp2** which contains 4 genomes to be compared following an all-versus-all study

### Pairwise genome comparison
The fist exercise consists on comparing the 2 genomes inside the **exp1** folder. To do that you should enter such folder and execute the following line:
`../../bin/workflow.sh G1.fna G2.fna 100 65 32`
The parameters mean the following:
* **G1.fna** - query sequence
* **G2.fna** - reference sequence
* **100** - minimal HSP length
* **65** - minimal HSP similarity (i.e. actual score/max score * 100.0)
* **32** - K-mer size for the hits/seeds step. Lower values increase sensitivity and running time

This execution will generate two folders:
* **intermediateFiles** - which will contain the input sequence dictionaries and the hits/seeds of the comparison
* **results** - which will contain the resulting CSV, frags, frags.INF and frags.MAT files.

### Multiple genome comparison<a name="multiple"></a>
This second exercise compares **6** - *n* genomes following an all-versus-all study. This study leads to **15** - *c=n·(n-1)/2* genome comparisons given the symmetric property of genome comparisons. To execute this exercise you need to use the following line (being at the **exp2** folder):
`../../bin/allVsAll.sh . 100 65 16 fasta`
* **.** - folder where the genomes are stored (current directory in this case)
* **100** - minimal HSP length
* **65** - minimal HSP similarity (i.e. actual score/max score * 100.0)
* **16** - K-mer size for the hits/seeds step. Lower values increase sensitivity and running time
* **fasta** - Extension of the genome files to be compared

## Visualizing the results
The GECKO-MGV web visualization tool can be used to explore the results. It is available at: https://pistacho.ac.uma.es/

For our simple exercise you should download the CSV from the cluster to your local computer. After that you have to load the file in the web application using the "Load from local" button (i.e. the floppy disk one).

A complete user manual of the web application is available at (in case you want to explore further functionality): https://chirimoyo.ac.uma.es/gecko/documents/GeckoMGV-ECCB-SupplementaryMaterial.pdf

# GECKO (in parallel)
## Internal parallelisation level
The **gecko-parallel-internal** folder contains the code, scripts and data to be used to illustrate the internal parallelisation level.

### Compilation instructions
First you need to enter in the **gecko-parallel-internal/src** folder of the **day2** folder. Then the following lines will compile all the GECKO modules:
`module load openmpi`
`make all`

### Running your first comparison in parallel
The **test** folder present at the same level as the **src** folder contains five folders:
* **run2** which contains the genomes, and the workload files for the dictionary and hits programs. The workload in this case is the following:
`A,C`
`G,T` 
* **run4** is equivalent to the previous folder but the workloads vary to match 4 cores and 4 tasks per core
* **run8** is equivalent to the previous folder but the workloads vary to match 8 cores and 2 tasks per core
* **run16** is equivalent to the previous folder but the workloads vary to match 16 cores and 4 tasks per core

Before executing a comparison in parallel you need to modify the **BASE_DIR** variable of the **workflow.sh** script which is in the **bin** folder. This variable should point to the full path of the bin folder.

After this modification, you can navigate to one of the **test/runX** folders in order to execute the the parallel version of GECKO with a given number of cores (2, 4, 8 and 16 in these examples). You need to use the following submission lines:
`sbatch --ntasks 5 --mem=60gb --time=00:15:00 -p local ../../bin/workflow.sh S1 S2 100 65 32 3 2`

`sbatch --ntasks 9 --mem=60gb --time=00:15:00 -p local ../../bin/workflow.sh S1 S2 100 65 32 5 4`

`sbatch --ntasks 17 --mem=60gb --time=00:15:00 -p local ../../bin/workflow.sh S1 S2 100 65 32 9 2`

`sbatch --ntasks 65 --mem=60gb --time=00:15:00 -p local ../../bin/workflow.sh S1 S2 100 65 32 17 4`

As you may have noticed, the execution line is similar to the one of the sequential version but with 2 additional parameters at the end. For the first execution line:
* **3** which is the number of cores
* **2** which is the number of simultaneous tasks per core

What precedes the **workflow.sh** script in the submission line are specific parameters of the distributed resources manager:
* **ntasks**: number of cores (not neccesarily in the same node)
* **mem**: amount of memory required
* **time**: walltime required to run the application

The execution generates a number of intermediate files and also the **.frags and .csv** resulting files. In addition the file **time.txt** will contain the execution time ([h]:mm:ss format) of the parallelised modules.

## External parallelisation level
The **gecko-parallel-external** folder contains the code, scripts and data to be used to illustrate the external parallelisation level.

### Compilation instructions
First you need to enter in the **gecko-parallel-external/src** folder of the **day2** folder. Then the following lines will compile all the GECKO modules:
`module load openmpi`
`make all`

### Running the all versus all comparison in parallel
In this exercise we will compare the same 6 genomes as in the [multiple genome comparison exercise](#multiple) of the original and sequential version of GECKO. For this exercise you need to enter the **data** folder. Once in the mentioned folder, you need to perform the following steps:
* **Generate all the reverse complementary sequences** with the following steps:
  * Calculate the **once** reverses:
    * `../bin/generateReversesWorkload.sh fasta > workloadReverses.sh`
    * `sbatch --ntasks 4 --mem=60gb --time=01:00:00 --partition=local ../bin/slurmWrapperMPI.sh mpirun -np 4 /home/griduser050/hpc-excelerate/day2/gecko-parallel-external/bin/scheduler workloadReverses.sh 2`
  * Move them to the **reverses** folder:
    * `mkdir reverses`
    * `mv *-revercomp.fasta reverses/`
* **Generate all the dictionaries for the forward and reverse complementary sequences** with the following steps:
  * Calculate the **once** dictionaries:
    * `../bin/generateDictionariesWorkload.sh fasta > workloadDictionaries.sh`
    * `sbatch --ntasks 7 --mem=60gb --time=01:00:00 --partition=local ../bin/slurmWrapperMPI.sh mpirun -np 7 /home/griduser050/hpc-excelerate/day2/gecko-parallel-external/bin/scheduler workloadDictionaries.sh 2`
  * Move them to the **dictionaries** folder:
    * `mkdir dictionaries`
    * `mv *.d2h* dictionaries/`
    * `rm -rf *.words.sort`
* **Generate all the fragment files for all the comparisons** with the following steps:
  * Calculate the fragments:
    * `../bin/allVsAll.sh 100 65 16 fasta > workloadFrags.sh`
    * `sbatch --ntasks 7 --mem=60gb --time=01:00:00 --partition=local ../bin/slurmWrapperMPI.sh mpirun -np 7 /home/griduser050/hpc-excelerate/day2/gecko-parallel-external/bin/scheduler workloadFrags.sh 2`
  * Results will be stored in the **frags** folder

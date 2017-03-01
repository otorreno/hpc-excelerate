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

###Pairwise genome comparison
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

###Multiple genome comparison 
This second exercise compares **6** - *n* genomes following an all-versus-all study. This study leads to **15** - *c=n·(n-1)/2* genome comparisons given the symmetric property of genome comparisons. To execute this exercise you need to use the following line (being at the **exp2** folder):
`../../bin/allVsAll.sh . 100 65 32 fasta`
* **.** - folder where the genomes are stored (current directory in this case)
* **100** - minimal HSP length
* **65** - minimal HSP similarity (i.e. actual score/max score * 100.0)
* **32** - K-mer size for the hits/seeds step. Lower values increase sensitivity and running time
* **fasta** - Extension of the genome files to be compared

## Visualizing the results
The GECKO-MGV web visualization tool can be used to explore the results. It is available at: https://pistacho.ac.uma.es/

For our simple exercise you should download the CSV from the cluster to your local computer. After that you have to load the file in the web application using the "Load from local" button (i.e. the floppy disk one).

A complete user manual of the web application is available at (in case you want to explore further functionality): https://chirimoyo.ac.uma.es/gecko/documents/GeckoMGV-ECCB-SupplementaryMaterial.pdf

# GECKO (in parallel)
TBC


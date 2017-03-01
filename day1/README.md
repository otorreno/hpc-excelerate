# Compilation instructions
## Hello world
`mpicc -O3 mpi_hello_world.c -o ../bin/mpi_hello_world`

## Bandwith benchmark
`mpicc -O3 mpi_bandwith.c -o ../bin/mpi_bandwith`

## MapReduce static distribution
`mpicc -O3 mpi_mapreduce_static_distribution.c -o ../bin/mpi_mapreduce_static_distribution`

## MapReduce dynamic distribution (database search example)
`gcc -O3 map_sequences.c map_functions.c -o ../bin/map_sequences`
`mpicc -O3 map_mapreduce_dynamic_distribution.c -o ../bin/map_mapreduce_dynamic_distribution`

# Execution instructions
## Hello world
The following line executes the **mpi-hello-world** example with 4 cores. Each MPI process will provide a greeting, printing the host name and its rank:
`mpirun -np 4 bin/mpi_hello_world`

## Bandwith benchmark
The following line executes the **mpi-bandwith** program using 4 nodes, which exchange messages of different sizes:
`mpirun -np 4 bin/mpi_bandwith`

## MapReduce static distribution
The following line executes the **mpi-mapreduce-static-distribution** program, which reads a binary file containing a vector of N integers and then splits such file into C partes, being C the number of available cores:
`mpirun -np 5 mpi_mapreduce_static_distribution my_file my_file.output`

## MapReduce dynamic distribution (database search example)
In this example we will execute a blast search in parallel. The first step is to create the blast database with the following command:
`makeblastdb -in db.fasta -dbtype nucl`

Next, we need to split the input data **query.fasta** using the **map_sequences** program:
`./bin/map_sequences query.fasta 4 blastn -query query.fasta -db db.fasta -out query.blast`

This splits the **query.fasta** file into 4 parts, and generates 2 workload files **query.fasta-map and query.fasta-red**. The first file contains the main workload, whereas the second file contains the reduce function (in this case a simple cat of the separate BLAST reports). To execute the main workload in parallel we use the following line:
`mpirun -np 5 mapreduce_dynamic_distribution query.fasta-map`

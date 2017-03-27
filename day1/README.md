# Compilation instructions
## Shared memory
### OpenMP Hello world
`gcc -O3 -fopenmp omp_hello.c -o ../bin/omp_hello`

### OpenMP Loop-work sharing
`gcc -O3 -fopenmp omp_workshare1.c -o ../bin/omp_workshare1`

### OpenMP Sections-work sharing
`gcc -O3 -fopenmp omp_workshare2.c -o ../bin/omp_workshare2`

### OpenMP Reduction
`gcc -O3 -fopenmp omp_reduction.c -o ../bin/omp_reduction`

## Distributed memory
### MPI Hello world
`mpicc -O3 mpi_hello_world.c -o ../bin/mpi_hello_world`

### MPI Bandwith benchmark
`mpicc -O3 mpi_bandwith.c -o ../bin/mpi_bandwith`

### MPI MapReduce static distribution
`mpicc -O3 mpi_mapreduce_static_distribution.c -o ../bin/mpi_mapreduce_static_distribution`

### MPI MapReduce dynamic distribution (database search example)
`gcc -O3 map_sequences.c map_functions.c -o ../bin/map_sequences`
`mpicc -O3 map_mapreduce_dynamic_distribution.c -o ../bin/map_mapreduce_dynamic_distribution`

# Execution instructions
## Shared memory
### OpenMP Hello world
The following line executes the **openmp-hello-world** example. Each OpenMP thread will provide a greeting, printing its rank. The master process will print the total number of OpenMP threads:
`../bin/omp_hello`

### OpenMP Loop-work sharing
The following line executes the **openmp-loop-work-sharing** example. Each OpenMP thread will dynamically process a chunk of a given size of the for loop. They will print their rank, which position they calculated and the actual computed value:
`../bin/omp_workshare1`

### OpenMP Sections-work sharing
The following line executes the **openmp-sections-work-sharing** example. Each of the OpenMP sections will be executed in parallel. The thread(s) executing the sections will print their rank, which position they calculated and the actual computed value:
`../bin/omp_workshare2`

### OpenMP Reduction
The following line executes the **openmp-reduction** example. The sum of a given vector is calculated in a parallel for OpenMP construct. OpenMP is instructed to reduce the partial results using the '+' operator:
`../bin/omp_reduction`

## Distributed memory

### MPI Hello world
The following line executes the **mpi-hello-world** example with 4 cores. Each MPI process will provide a greeting, printing the host name and its rank:
`mpirun -np 4 bin/mpi_hello_world`

### MPI Bandwith benchmark
The following line executes the **mpi-bandwith** program using 4 nodes, which exchange messages of different sizes:
`mpirun -np 4 bin/mpi_bandwith`

### MPI MapReduce static distribution
The following line executes the **mpi-mapreduce-static-distribution** program, which reads a binary file containing a vector of N integers and then splits such file into C partes, being C the number of available cores:
`mpirun -np 5 mpi_mapreduce_static_distribution my_file my_file.output`

### MPI MapReduce dynamic distribution (database search example)
In this example we will execute a blast search in parallel. The first step is to create the blast database with the following command:
`makeblastdb -in db.fasta -dbtype nucl`

Next, we need to split the input data **query.fasta** using the **map_sequences** program:
`./bin/map_sequences query.fasta 4 blastn -query query.fasta -db db.fasta -out query.blast`

This splits the **query.fasta** file into 4 parts, and generates 2 workload files **query.fasta-map and query.fasta-red**. The first file contains the main workload, whereas the second file contains the reduce function (in this case a simple cat of the separate BLAST reports). To execute the main workload in parallel we use the following line:
`mpirun -np 5 mapreduce_dynamic_distribution query.fasta-map`

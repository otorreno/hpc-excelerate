# Compilation instructions
## Hello world
`mpicc -O3 mpi_hello_world.c -o mpi_hello_world`

## Bandwith benchmark
`mpicc -O3 mpi_bandwith.c -o mpi_bandwith`

## MapReduce static distribution
`mpicc -O3 mpi_mapreduce_static_distribution.c -o mpi_mapreduce_static_distribution`

# Execution instructions
## Hello world
The following line executes the **mpi-hello-world** example with 4 cores. Each MPI process will provide a greeting, printing the host name and its rank:
`mpirun -np 4 mpi_hello_world`

## Bandwith benchmark
The following line executes the **mpi-bandwith** program using 4 nodes, which exchange messages of different sizes:
`mpirun -np 4 mpi_bandwith`

## MapReduce static distribution
The following line executes the **mpi-mapreduce-static-distribution** program, which reads a binary file containing a vector of N integers and then splits such file into C partes, being C the number of available cores:
`mpicc -O3 mpi_mapreduce_static_distribution.c -o mpi_mapreduce_static_distribution`

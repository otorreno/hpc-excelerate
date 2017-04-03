#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>

#define WORKTAG    1
#define ENDTAG     2

void master(char *input_file, char *output_file);
void worker(char *input_file, char *output_file);

void function(int* array, int size){
    int i;
    
    for(i=0; i < size; i++){
        array[i] = 2 * array[i];
    }
}

// Main --------------------------------
int main(int ac, char **argv) {
    //Testing input parameters
    if(ac!=3){
        fprintf(stderr, "USE: ./mpi_mapreduce_static_distribution input_file output_file\n");
        return -1;
    }
    
    //Initializing MPI environment
    int myID;
    MPI_Init(&ac, &argv);
    MPI_Comm_rank( MPI_COMM_WORLD, &myID);
    
    if (myID == 0) master(argv[1], argv[2]);
    else worker(argv[1], argv[2]);
    
    MPI_Finalize();
    return 0;
}

// Master --------------------------
void master(char * input_file, char *output_file) {
	unsigned long coordinates[2];
	MPI_Status status;
    int ntasks, i, N;
    long file_length;
	FILE * fIN;
	
	//Verifying number of available processors.
    MPI_Comm_size(MPI_COMM_WORLD, &ntasks);
    fprintf(stdout, "MASTER: pool of MPI process available: %d\n", ntasks);

	//Opening input file
	if ((fIN=fopen(input_file,"rb"))==NULL) {
		fprintf(stderr, "MASTER: Invalid input file\n");
		for (i = 1; i < ntasks; i++) {
			MPI_Send(0, 0, MPI_LONG, i, ENDTAG, MPI_COMM_WORLD);
		}
		return;
	}
	
	//Calculating the length of the input file
	fseek(fIN, 0, SEEK_END);
	file_length = ftell(fIN)/sizeof(int);
	fseek(fIN, 0, SEEK_SET);
	
    //Debug purpose
    unsigned long amounts[ntasks];
    for(i=0;i<ntasks;i++){
        amounts[i] = 0;
    }
	
	/*************************************/
	/* Step 1: SPLITTING/BALANCING       */
	/*************************************/
	
	//Check if the module is 0
	if (file_length % (ntasks - 1) != 0) {
	    fprintf(stderr, "MASTER: The file length should be multiple of ntasks\n");
	    for (i = 1; i < ntasks; i++) {
			MPI_Send(0, 0, MPI_LONG, i, ENDTAG, MPI_COMM_WORLD);
		}
		return;
	}
	
	//Calculate the amount of numbers for each worker
	unsigned long amount = file_length / (ntasks - 1);
	
	//Calculate and distribute the coordinates to each worker
	for (i = 1; i < ntasks; i++) {
		//Starting coordinate
		coordinates[0] = (i-1) * amount;
		//End coordinate
		coordinates[1] = (i * amount) - 1;
		//Amount of numbers assigned to the worker 'i'
		amounts[i] += coordinates[1] - coordinates[0] + 1;
		
		MPI_Send(coordinates, 2, MPI_LONG, i, WORKTAG, MPI_COMM_WORLD);
	}

	//After executing the assigned work, the workers will end
    unsigned long total = 0;
	for (i = 1; i < ntasks; i++) {
		MPI_Send(0, 0, MPI_LONG, i, ENDTAG, MPI_COMM_WORLD);
        fprintf(stdout, "MASTER: Process %d has been assigned %lu numbers\n",i, amounts[i]);
        total += amounts[i];
	}
    
    fprintf(stdout, "MASTER: Total amount of numbers distributed: %lu\n",total);
	
	//Waiting for every worker to confirm finalisation
	for (i = 1; i < ntasks; i++) {
		MPI_Recv(&N, 1, MPI_INT, MPI_ANY_SOURCE,MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	}
	
	/*************************************/
	/* Step 3: MERGING                   */
	/*************************************/
	FILE *f2, *fF;
	char tmp[100];
	long tN=0;
	int *aux = (int *)malloc(amount*sizeof(int));
	
	//Opening Final output file
	if ((fF=fopen(output_file, "wb"))==NULL){
		fprintf(stderr, "MASTER: ERROR opening output file\n");
		return;
	}
	
	//Looping in every chunk file
	for (i=1; i < ntasks; i++) {
		sprintf(tmp, "%s-%04d", output_file, i);
		fprintf(stdout, "MASTER: Opening %s file for reading\n",tmp);
		
		if ((f2=fopen(tmp,"rb"))==NULL) {
			fprintf(stderr, "MASTER: Invalid CHUNK input file\n");
			return;
		}

		fread(aux, sizeof(int), amount, f2);
		tN += amount;
		fwrite(aux, sizeof(int), amount, fF);

		fclose(f2);
		unlink(tmp);
	}

	free(aux);

	fprintf(stdout,"ReduceChunk from 0000 to %04d total numbers=%ld\n", ntasks-1, tN);
	fclose(fF);
}

// Worker ----------------------------------
void worker(char *input_file, char *output_file){
	char filename[100];
	FILE *fIn, *fOut;
	unsigned long coordinates[2];
	MPI_Status status;
	int myID;
	int r;

	MPI_Comm_rank( MPI_COMM_WORLD, &myID);
	
	//for filename purpose
	sprintf(filename,"%s-%04d", output_file, myID);
	//Opening output file --------------------------------
	if ((fOut=fopen(filename,"wb"))==NULL) {
		fprintf(stderr, "SLAVE number (%d): Impossible to open output file\n",myID);
		return;
	}

	//Open the input file -------------------------------------------------
	if ((fIn=fopen(input_file,"rb"))==NULL) {
		fprintf(stderr, "SLAVE number (%d): Impossible to open input file\n",myID);
		return;
	}
	
	while(1) {
		//Waiting for orders
		MPI_Recv(coordinates, 2, MPI_LONG, MPI_ANY_SOURCE,MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		
		int size = (coordinates[1]-coordinates[0]+1);
		int *array = (int *) malloc(size*sizeof(int));
		
		if (status.MPI_TAG == ENDTAG){
			fclose(fIn);
			fclose(fOut);
			//Warn the Master that this process has finished.
			MPI_Send(&myID, 1, MPI_INT, 0, WORKTAG, MPI_COMM_WORLD);
            printf("SLAVE n°%d: Received ENDTAG... terminating\n",myID);
			return;
		}

        fprintf(stdout, "SLAVE number (%d): received work, starting processing, from %lu to %lu\n",myID,coordinates[0],coordinates[1]);
		
		/*************************************/
		/* Step 2: PROCESSING                */
		/*************************************/
		//move file descriptor to the starting position
		fseek(fIn, coordinates[0]*sizeof(int), SEEK_SET);
		
		//Read the assigned chunk
		if(fread(array, sizeof(int), size, fIn) != size){
		    if(ferror(fIn)){
				fprintf(stderr, "Error reading from the input file\n");
				exit(-1);
			}
		}
		
		//Process the chunk
		function(array, size);
		
		//Write the partial result
		if(fwrite(array, sizeof(int), size, fOut) != size){
		    if(ferror(fIn)){
				fprintf(stderr, "Error writing to the output file\n");
				exit(-1);
			}
		}
		
		//Closing the files
		fclose(fIn);
		fclose(fOut);
				
		//Deallocating memory
		free(array);
	}
	return;
}

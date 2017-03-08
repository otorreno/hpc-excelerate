#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <math.h>
#include <ctype.h>
#include <mpi.h>
#include <inttypes.h>
#include <omp.h>

#include "structs.h"
#include "commonFunctions.h"
#include "dictionaryFunctions.h"

#define MAXLINE 	1024
#define MAXPREFIXES 	10

int main(int argc, char **argv) {
	FILE *f;
	char c, *file, prefixes[MAXLINE], *pTable[MAXPREFIXES], *workToDo[MAXLINE];
	int wsize, i, p, id, nTasks, nPrefixes, tpn;

	MPI_Status status;
	
	int64_t fileSize;

	if (argc != 5) {
		terror("USE: dictionary seq.fasta wsize workload.txt tasks_per_node");
	}

	memset(prefixes,0,MAXLINE);
	memset(pTable,0,MAXPREFIXES);
	memset(workToDo,0,MAXLINE);

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	MPI_Comm_rank(MPI_COMM_WORLD, &id);

	wsize = atoi(argv[2]);
	tpn = atoi(argv[4]);

	if ((f = fopen(argv[1], "rt")) == NULL)
		terror("opening sequence file");

	//The master process reads the workload, distribute the tasks and calculates the sequence length
	if(id == 0){
		readWorkloadFile(argv[3], workToDo, &nTasks);

		//fprintf(stdout, "After reading the workload\n");

		for(i = 1; i < p; i++){
			//Prepare prefixes and send
			MPI_Send(workToDo[i], strlen(workToDo[i]), MPI_CHAR, i, 1, MPI_COMM_WORLD); 
		}

		memcpy(prefixes, workToDo[0], strlen(workToDo[0]));

		fseek(f, 0, SEEK_END);
		fileSize = ftell(f);
		rewind(f);

		// first line (skip ID fasta Line)
		c = fgetc(f);
		i=0;
		while (c != '\n'){
			c = fgetc(f);
			i++;
		}
		fileSize-=i;
	}

	//The slaves wait for their tasks (i.e. prefixes)	
	if(id != 0) MPI_Recv(prefixes, 1024, MPI_CHAR, 0, 1, MPI_COMM_WORLD, &status);

	//The fileSize is broadcasted to all the MPI processes
	MPI_Bcast(&fileSize, 1, MPI_INT64_T, 0, MPI_COMM_WORLD);

	//All the processes reserve memory for the sequence
	file = (char *)malloc((fileSize+1)*sizeof(char));
	//Only the master reads from the file
	if(id == 0){
		fread(file, sizeof(char), fileSize, f);
		fclose(f);
	}
	//And then broadcasts the file content to the slaves
	MPI_Bcast(file, fileSize, MPI_CHAR, 0, MPI_COMM_WORLD);

	//Calculating the work to be done based on the received prefixes
	nPrefixes = prefixTable(prefixes, pTable);

	//Start tpn simultaneous processes
	#pragma omp parallel num_threads(tpn)
	{
		int tid = omp_get_thread_num();
		char *prefix = NULL, *seq = NULL;
		int prefixSize = 0, n = 0, Tot = 0, NoACGT = 0, NoC = 0, BAD = wsize, index, aux = 0;
		char fname[MAXLINE];
		unsigned long j = 0, s = 0;
		char c;
		location loc;
		node *start = NULL;
		node *currentTreeRow;
		list_node *currentLocationRow;
		unsigned long i = 0, nWords = 0;
		FILE *fout;

		if ((seq = (char*) malloc(wsize + 1)) == NULL)
			terror("memory for Seq");
		seq[wsize] = '\0';

		//Node Memory pool
		unsigned long actualNodes = 0;
		treeMemory_node *nodePool = (treeMemory_node *) malloc(
				sizeof(treeMemory_node));
		currentTreeRow = nodePool->data = (node *) malloc(
				NODEPOOLSIZE * sizeof(node));
		nodePool->next = NULL;
		memset(nodePool->data, 0, NODEPOOLSIZE * sizeof(node));

		//Location Memory pool
		unsigned long actualLocations = 0;
		locationMemory_node *locationPool = (locationMemory_node *) malloc(
				sizeof(locationMemory_node));
		currentLocationRow = locationPool->data = (list_node *) malloc(
				LOCATIONPOOLSIZE * sizeof(list_node));
		locationPool->next = NULL;
		memset(locationPool->data, 0, LOCATIONPOOLSIZE * sizeof(list_node));

		//pick the prefix to be processed
		prefix = pTable[tid];
		prefixSize = strlen(prefix);
		
		//Opening the output file
		sprintf(fname, "%s.%s.dict", argv[1], prefix);
		if ((fout = fopen(fname, "wb")) == NULL)
			terror("opening words file");

		//Starting the computation
		c = toupper(file[j]);
		while (j<(unsigned long)fileSize) {
			if (!isupper(c)) {
				if (c == '>') {
					c = toupper(file[++j]);
					while (c != '\n' && j<(unsigned long)fileSize) {
						c = toupper(file[++j]);
					}

					s++;
					n++;
					Tot = 0;
					BAD = wsize;
				}
				NoC++;
				c = toupper(file[++j]);
				continue;
			}
			index = MIN(Tot, wsize - 1);
			seq[index] = c;
			switch (c) {
				case 'A':
					BAD--;
					break;
				case 'C':
					BAD--;
					break;
				case 'G':
					BAD--;
					break;
				case 'T':
					BAD--;
					break;
				default:
					NoACGT++;
					Tot = -1;
					BAD = wsize;
					break;
			}
			//If the word is correct add it to the tree
			if (Tot > (wsize - 2) && !BAD) {
				loc.pos = n - wsize + 1;
				loc.seq = s;

				//Store the word only if the word starts with the assigned prefix
				if (strncmp(seq, prefix, prefixSize) == 0) {
					//Corresponds to the actual interval
					aux = insert(seq, wsize, loc, &start, currentTreeRow,
							&actualNodes, currentLocationRow, &actualLocations);

					nWords += aux;

					//Check if the node pool is full
					if (actualNodes > NODEPOOLSIZE) {
						treeMemory_node *aux = nodePool;
						while (aux->next != NULL) {
							aux = aux->next;
						}
						aux->next = (treeMemory_node *) malloc(
								sizeof(treeMemory_node));
						aux->next->data = (node *) malloc(
								NODEPOOLSIZE * sizeof(node));
						aux->next->next = NULL;
						currentTreeRow = aux->next->data;
						actualNodes = 0;
					}

					//Check if the location pool is full
					if (actualLocations > LOCATIONPOOLSIZE) {
						locationMemory_node *aux = locationPool;
						while (aux->next != NULL) {
							aux = aux->next;
						}
						aux->next = (locationMemory_node *) malloc(
								sizeof(locationMemory_node));
						aux->next->data = (list_node *) malloc(
								NODEPOOLSIZE * sizeof(list_node));
						aux->next->next = NULL;
						currentLocationRow = aux->next->data;
						actualLocations = 0;
					}
				}

				//Move all the characters one position to the left
				for (i = 1; i < (unsigned long)wsize; i++) {
					seq[i - 1] = seq[i];
				}
				BAD = 1;
			}
			Tot++;
			n++;
			c = toupper(file[++j]);
		}

		//Write the tree to the file
		writeTree(start, fout);

		//Destroy the tree and rewind the input file to start from zero
		//Free up memory from the pool
		if (nodePool->next != NULL) {
			//Free all but the first row of the pool matrix
			treeMemory_node *aux = nodePool->next;
			treeMemory_node *ant = nodePool->next;
			nodePool->next = NULL;
			do {
				aux = aux->next;
				free(ant->data);
				free(ant);
				ant = aux;
			} while (aux != NULL);
			currentTreeRow = nodePool->data;
		}

		if (locationPool->next != NULL) {
			//Free all but the first row of the pool matrix
			locationMemory_node *aux = locationPool->next;
			locationMemory_node *ant = locationPool->next;
			locationPool->next = NULL;
			do {
				aux = aux->next;
				free(ant->data);
				free(ant);
				ant = aux;
			} while (aux != NULL);
			currentLocationRow = locationPool->data;
		}

		memset(nodePool->data, 0, NODEPOOLSIZE * sizeof(node));
		memset(locationPool->data, 0, LOCATIONPOOLSIZE * sizeof(list_node));

		start = NULL;
		BAD = wsize;
		Tot = n = s = actualNodes = actualLocations = nWords = j = 0;
		free(seq);

		fclose(fout);
	}

	//Freeing memory
	free(file);
	if(id==0) for(i=0;i<nTasks;i++)	
		free(workToDo[i]);
	for(i=0;i<nPrefixes;i++)
		free(pTable[i]);

	MPI_Finalize();

	return 0;
}

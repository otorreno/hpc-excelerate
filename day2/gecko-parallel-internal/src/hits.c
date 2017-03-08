/* hits : determine HITS between two sequences based on their 
 dictionaries from disk
 Syntax: hits prefixNameX prefixNameY Outfile

 prefixNameX & Y :refers to *.d2hW : index of words-Pos-Ocurrences
 and *.d2hP : words positions
 Outfile is in the form of [Diag][posX][posY]

 -define an I-O buffer to reduce disk activity
 - define a function to compare words instead of chars
 - use static buffer for positions
 - New parameter: FreqThre (frequency threshold) to avoid high
 frequency words (word.freq> FreqThr are skipped)
 - new parameter (same meaning as in w2hd): PrefixSize

 For long repetitions some of them will be skipped (the step is
 computed as NREP / MaxREP

 oscart@uma.es
 ---------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <mpi.h>
#include <omp.h>
#include "structs.h"
#include "commonFunctions.h"
#include "dictionaryFunctions.h"
#include "comparisonFunctions.h"

#define MAXBUF 10000000
#define MaxREP 10000
#define MAXLINE 1024
#define MAXPREFIXES 10

int hits(char *prefixNameX, char *prefixNameY, char *outputFile, int freqThr, int wSize);

int main(int ac, char** av) {
	int freqThr, wSize, p, id, nTasks, nPrefixes, i, tpn;
	char *prefixNameX, *prefixNameY, *workloadFile, *outFile, *workToDo[MAXLINE], prefixes[MAXLINE], *pTable[MAXPREFIXES];
	MPI_Status status;

	if (ac != 8)
		terror("USE: hits prefixNameX prefixNameY workloadFile Outfile FreqThre PrefixSize tpn");
	prefixNameX = av[1];
	prefixNameY = av[2];
	workloadFile = av[3];
	outFile = av[4];
	freqThr = (uint64_t) atoi(av[5]);
	wSize = atoi(av[6]);
	tpn = atoi(av[7]);

	memset(prefixes,0,MAXLINE);
	memset(pTable,0,MAXPREFIXES);
	memset(workToDo,0,MAXLINE);

	MPI_Init(&ac, &av);
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	MPI_Comm_rank(MPI_COMM_WORLD, &id);

	//The master process reads the workload, distribute the tasks and calculates the sequence length
	if(id == 0){
		readWorkloadFile(workloadFile, workToDo, &nTasks);

		//fprintf(stdout, "After reading the workload\n");

		for(i = 1; i < p; i++){
			//Prepare prefixes and send
			MPI_Send(workToDo[i], strlen(workToDo[i]), MPI_CHAR, i, 1, MPI_COMM_WORLD); 
		}

		memcpy(prefixes, workToDo[0], strlen(workToDo[0]));
	}

	//The slaves wait for their tasks (i.e. prefixes)	
	if(id != 0) MPI_Recv(prefixes, 1024, MPI_CHAR, 0, 1, MPI_COMM_WORLD, &status);

	//Calculating the work to be done based on the received prefixes
	nPrefixes = prefixTable(prefixes, pTable);

		//Start tpn simultaneous processes
	#pragma omp parallel num_threads(tpn)
	{
		int tid = omp_get_thread_num();
		char tmp1[MAXLINE], tmp2[MAXLINE], tmp3[MAXLINE];
		sprintf(tmp1, "%s.%s", prefixNameX, pTable[tid]);
		sprintf(tmp2, "%s.%s", prefixNameY, pTable[tid]);
		sprintf(tmp3, "%s.%s", outFile, pTable[tid]);
		hits(tmp1, tmp2, tmp3, freqThr, wSize);
	}

	MPI_Finalize();

	return 0;
}

int hits(char *prefixNameX, char *prefixNameY, char *outputFile, int freqThr, int wSize){
	char fname[1024];
	int hitsInBuf = 0;
	unsigned long i, j;
	int comp;
	int firstMatch = 0, endMatch = 0;
	uint64_t nHits = 0, wordMatches = 0;
	int stepX, stepY;
	long offsetWordBefore = 0, offsetWordAfter = 0;
	FILE *fX, *fY, *fOut;

	location *posX = NULL, *posY = NULL;
	hashentry heX, heY;
	hit *hBuf;

	// I-O buffer
	if ((hBuf = (hit*) calloc(sizeof(hit), MAXBUF)) == NULL)
		terror("HITS: memory for I-O buffer");
	// word positions buffers
	if ((posX = (location*) calloc(MAXBUF, sizeof(location))) == NULL)
		terror("memory for posX buffer.. using MAXBUF=10MB");
	if ((posY = (location*) calloc(MAXBUF, sizeof(location))) == NULL)
		terror("memory for posY buffer.. using MAXBUF=10MB");

	// Sequence X file
	sprintf(fname, "%s.dict", prefixNameX);
	if ((fX = fopen(fname, "rb")) == NULL) {
		terror("opening seqX.dict file");
	}

	// Sequence Y file
	sprintf(fname, "%s.dict", prefixNameY);
	if ((fY = fopen(fname, "rb")) == NULL) {
		terror("opening seqY.dict file");
	}

	// OUT file
	if ((fOut = fopen(outputFile, "wb")) == NULL)
		terror("opening OUT file");

	// kick-off
	if (readHashEntry(&heX, fX, freqThr) == -1)
		terror("no hash (1)");
	if (readHashEntry(&heY, fY, freqThr) == -1)
		terror("no hash (2)");

	while (!feof(fX) && !feof(fY)) {
		comp = wordcmp(&heX.w.b[0], &heY.w.b[0], wSize);
		if (comp < 0) {
			loadWordOcurrences(heX, &posX, &fX);
			readHashEntry(&heX, fX, freqThr);
			//Save position of first missmatch after matches and rewind
			if(firstMatch){
				offsetWordAfter = ftell(fY) - sizeof(hashentry);
				fseek(fY, offsetWordBefore, SEEK_SET);
				readHashEntry(&heY, fY, freqThr);
				firstMatch = 0;
				endMatch = 1;
			}
			continue;
		}
		if (comp > 0) {
			//No more matches, go to the next word
			if(endMatch){
				fseek(fY, offsetWordAfter, SEEK_SET);
				endMatch = 0;
			} else {
				loadWordOcurrences(heY, &posY, &fY);
			}
			readHashEntry(&heY, fY, freqThr);
			continue;
		}

		wordMatches++;

		// Load word position for seqX
		if(!firstMatch)loadWordOcurrences(heX, &posX, &fX);

		// Saving the offset of the first match
		if(wSize < 32 && !firstMatch){
			offsetWordBefore = ftell(fY) - sizeof(hashentry);
			firstMatch = 1;
		}

		// Load word position for seqY
		loadWordOcurrences(heY, &posY, &fY);

		// Hits-----------------------
		if (heX.num > MaxREP)
			stepX = heX.num / MaxREP;
		else
			stepX = 1;
		if (heY.num > MaxREP)
			stepY = heY.num / MaxREP;
		else
			stepY = 1;

		for (i = 0; i < heX.num; i += stepX)
			for (j = 0; j < heY.num; j += stepY) {
				hBuf[hitsInBuf].diag = posX[i].pos - posY[j].pos;
				hBuf[hitsInBuf].posX = posX[i].pos;
				hBuf[hitsInBuf].seqX = posX[i].seq;
				hBuf[hitsInBuf].posY = posY[j].pos;
				hBuf[hitsInBuf].seqY = posY[j].seq;

				hitsInBuf++;
				if (hitsInBuf == MAXBUF - 1) {
					fwrite(hBuf, sizeof(hit), hitsInBuf, fOut);
					hitsInBuf = 0;
				}
			}

		nHits += ((heX.num / stepX) * (heY.num / stepY));

		if(!firstMatch)readHashEntry(&heX, fX, freqThr);
		readHashEntry(&heY, fY, freqThr);

	}

	//Closing dictionary files
	fclose(fX);
	fclose(fY);

	//Checking if there is something still at the buffer
	if (hitsInBuf != 0) {
		fwrite(hBuf, sizeof(hit), hitsInBuf, fOut);
	}

	free(hBuf);
	free(posX);
	free(posY);

	fclose(fOut);

	fprintf(stdout, "HITS: matches=%" PRIu64 " Tot HITS=%" PRIu64 "\n", wordMatches, nHits);

	return 0;
}
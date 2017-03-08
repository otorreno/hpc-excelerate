/* filterHits
 Syntax: filterHits fileIn fileOut wordLength

 fileIn  Ordered Hits file (Diag/posX/posY)
 fileOut Ordered Out file  (Diag/posX/posY)
 wordSize    word size with which the hits were calculated

 oscart@uma.es
 -------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "structs.h"
#include "commonFunctions.h"
#include "comparisonFunctions.h"

/**
 * function to check if the hits belong to different sequences
 * in multi-FASTA files
 */
int differentSequences(hit h1, hit h2){
	return h1.seqX != h2.seqX || h1.seqY != h2.seqY;
}

int main(int ac, char** av) {
	int wSize = 32;
	int64_t diagonal;
	uint64_t lastPosition;
	uint64_t originalNumberOfHits = 0, finalNumberOfHits = 0, hitsRead = 0;
	FILE* fIn, *fOut;

	hit hits[2];

	if (ac != 4)
		terror("USE:filterHits fileIn fileOut wordSize");

	if ((fIn = fopen(av[1], "rb")) == NULL)
		terror("opening HITS input file");

	if ((fOut = fopen(av[2], "wb")) == NULL)
		terror("opening HITS OUT file");
	wSize =  atoi(av[3]);
	lastPosition = 0;

	hitsRead = fread(&hits[0], sizeof(hit), 1, fIn);
	originalNumberOfHits += hitsRead;
	if(hitsRead>0)
		finalNumberOfHits += fwrite(&hits[0], sizeof(hit), 1, fOut);
	diagonal = hits[0].diag;
	while (hitsRead > 0) {
		hitsRead = fread(&hits[1], sizeof(hit), 1, fIn);
		originalNumberOfHits += hitsRead;

		if(differentSequences(hits[0], hits[1])){
			lastPosition = hits[1].posX + (2 * wSize - 1);
			diagonal = hits[1].diag;
			finalNumberOfHits += fwrite(&hits[1], sizeof(hit), 1, fOut);
			memcpy(&hits[0], &hits[1], sizeof(hit));
			continue;
		}

		if (hitsRead == 0 && originalNumberOfHits > 1) {
			if(diagonal != hits[0].diag || hits[0].posX > (lastPosition)){
				finalNumberOfHits += fwrite(&hits[0], sizeof(hit), 1, fOut);
			}
			continue;
		}

		if (diagonal != hits[1].diag || hits[1].posX > lastPosition) {
			lastPosition = hits[1].posX + (2 * wSize - 1);
			diagonal = hits[1].diag;
			finalNumberOfHits += fwrite(&hits[1], sizeof(hit), 1, fOut);
			memcpy(&hits[0], &hits[1], sizeof(hit));
		}
	}

	fprintf(stdout,
			"\nfilterHits\noriginal number of Hits=%" PRIu64 "  Final number of hits=%" PRIu64 "\n",
			originalNumberOfHits, finalNumberOfHits);
	fclose(fIn);
	fclose(fOut);

	return 0;
}

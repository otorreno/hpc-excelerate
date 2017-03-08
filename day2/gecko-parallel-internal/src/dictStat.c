#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "structs.h"
#include "commonFunctions.h"
#include "dictionaryFunctions.h"
#include "comparisonFunctions.h"

#define MAXBUF 10000000
#define MaxREP 10000

int main(int ac, char** av) {

	char w[33];
	unsigned long i;

	uint64_t freqThr;

	FILE *fX;

	location *posX = NULL;
	hashentry heX;

	if (ac != 3)
		terror("USE: dictStat prefixNameX FreqThre");
	freqThr = (uint64_t) atoi(av[2]);

	// word positions buffers
	if ((posX = (location*) calloc(MAXBUF, sizeof(location))) == NULL)
		terror("memory for posX buffer.. using MAXBUF=10MB");

	// Sequence X file
	if ((fX = fopen(av[1], "rb")) == NULL) {
		terror("opening seqX.dict file");
	}

	if (readHashEntry(&heX, fX, freqThr) == -1)
		terror("no hash (1)");

	while (!feof(fX)) {
		showWord(&heX.w, w);
		fprintf(stdout, "%.32s", w);
		fprintf(stdout,"  : num=%-7" PRIu64 ":", heX.num);
		loadWordOcurrences(heX, &posX, &fX);
		for(i = 0; i < heX.num; i++) {
			fprintf(stdout, "(%" PRIu64 ",%" PRIu64 ") ", posX[i].pos, posX[i].seq);
		}
		fprintf(stdout, "\n");
		readHashEntry(&heX, fX, freqThr);
	}

	//Closing dictionary file
	fclose(fX);

	return 0;
}


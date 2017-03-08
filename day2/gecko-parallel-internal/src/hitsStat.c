#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "structs.h"
#include "commonFunctions.h"

int main(int ac, char**av)
{
	FILE *f1;
	hit H;
	unsigned long nHits;

	if (ac!=2)
		terror("Use: hitsStat InputFile");

	if ((f1=fopen(av[1],"rb"))==NULL) 
		terror("Input file open error");

	fseek(f1,0, SEEK_END);
	nHits = ftell(f1)/sizeof(hit);
	fprintf(stdout,"File size=%ld  nHits=%ld\n", nHits * sizeof(hit), nHits);
	fprintf(stdout,"---------------[RET]\n");fgetc(stdin);
	fseek(f1,0, SEEK_SET);
	
	fread(&H,sizeof(hit),1,f1);
	while(!feof(f1)){
		fprintf(stdout,"d=%-7" PRId64 " pX=%-7" PRIu64 " pY=%-7" PRIu64 " seqX=%-7" PRIu64 " seqY=%-7" PRIu64 "\n",H.diag,H.posX,H.posY,H.seqX,H.seqY);
		fread(&H,sizeof(hit),1,f1);
	}

	fclose(f1);
	return 0;

}




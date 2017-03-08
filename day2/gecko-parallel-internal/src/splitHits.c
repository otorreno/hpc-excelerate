/*
   SplitHit for MPI environment.

   Please, compile specifying the balancing mode:
	- PBAL: Divide inputfile into chunks of equal amount of hits.
	- DIAG: Divide inputfile into chunks following the diagonals.
	- EDIAG: Divide inputfile into one instruction for each diagonal.

   oscart@uma.es
   -------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include "structs.h"
#include "commonFunctions.h"
#include "comparisonFunctions.h"

void perfectBalancing(int nInstr, instruction newInstr, FILE *fIN, FILE *fOUT) {
	hit readHit0, readHit1;
	unsigned long fUnit;
    unsigned long i, NH, total = 0;

	//Amount of hits to balance
	fseek(fIN, 0, SEEK_END);
	NH =  ftell(fIN) / sizeof(hit);
	fseek(fIN, 0, SEEK_SET);

	//Getting the divisors
	newInstr.end = 0;
	fUnit = NH/nInstr;
	if ((NH % nInstr) != 0) {
		fUnit++;
	}

	//Splitting into chunk file.
    for (i = 0; i < (unsigned long)nInstr; i++) {
		//Getting the coordinates
		newInstr.start = newInstr.end;
		newInstr.end = (i+1)*fUnit-1;

		fseek(fIN, newInstr.end*sizeof(hit),SEEK_SET);
		
		//Until we finished the hits of a Diagonal...
		fread(&readHit0,sizeof(hit),1, fIN);
		fread(&readHit1,sizeof(hit),1, fIN);
		while(readHit0.diag == readHit1.diag && !feof(fIN)) {
			fread(&readHit1,sizeof(hit),1, fIN);
			newInstr.end++;
		}
		//Last instruction
		if( i == (unsigned long)(nInstr - 1) ) {
			newInstr.end = NH-1;
		}

		//Writing new instruction into output file
		total += newInstr.end - newInstr.start + 1;
		fwrite(&newInstr,sizeof(instruction),1,fOUT);
		newInstr.end++;
    }
}

void diagonalBalancing(int nInstr, instruction newInstr, FILE *fIN, FILE *fOUT) {
	double maxD = 0, minD = 0;
	unsigned long i;
	hit readHit;

	// Finding the max value to make the files
	while(!feof(fIN)) {
		fread(&readHit,sizeof(hit),1, fIN);
		if( readHit.diag > maxD ) maxD = readHit.diag;
		if( readHit.diag < minD ) minD = readHit.diag;
	}

	rewind(fIN);
	// calculation to get the number to divide found value by
	unsigned long semidiv = ((maxD - minD)/ (nInstr-1))/2;

	// Going through the file and writting to other
	newInstr.end = 0;
	fread(&readHit,sizeof(hit),1, fIN);
	for (i = 0; i < (unsigned long) nInstr; i++) {
		newInstr.start = newInstr.end;
		while(readHit.diag <= minD+(((2*i)+1)*semidiv) && !feof(fIN)) {
			fread(&readHit,sizeof(hit),1, fIN);
			newInstr.end++;
		}

		fwrite(&newInstr,sizeof(instruction),1,fOUT);
		newInstr.end++;
	}
}

void eachDiagBalancing(instruction newInstr, FILE *fIN, FILE *fOUT) {
	hit readHit0, readHit1;

	newInstr.end = 0;
	while(!feof(fIN)) {
		newInstr.start = newInstr.end;
		fseek(fIN, newInstr.end * sizeof(hit), SEEK_SET);
		//Until we finish the hits of a Diagonal...
		fread(&readHit0, sizeof(hit), 1, fIN);
		do {
			fread(&readHit1, sizeof(hit), 1, fIN);
			newInstr.end++;
		}while(readHit0.diag == readHit1.diag && !feof(fIN)) ;
		
		//Writing the coordinates to the output file
		newInstr.end --;
		fwrite(&newInstr,sizeof(instruction),1,fOUT);

		newInstr.end ++;
	}
}

int main(int ac, char **argv) {
    instruction newInstr;
    FILE *fIN, *fOUT;
    int nInstr;

	//Testing input parameters
	if(ac!=4)
		terror("USE: ./splitHits HITFileIN INSTRFileOUT nINSTR");
	//Initializing variables
	if ((fIN =fopen(argv[1],"rb"))==NULL){
		terror("Opening input file");
	}

	if ((fOUT=fopen(argv[2],"wb"))==NULL){
		terror("Opening output file");
	}

    newInstr.start = newInstr.end = 0;

	nInstr = atoi(argv[3]);
	if(nInstr <= 1) {
		terror("Invalid number of instruction asked");
	}

#ifdef PBAL
	perfectBalancing(nInstr, newInstr, fIN, fOUT);
#else
	#ifdef DIAG
		diagonalBalancing(nInstr, newInstr, fIN, fOUT);
	#else
		#ifdef EDIAG
			eachDiagBalancing(newInstr, fIN, fOUT);
		#else
			printf("Please, compile specifying the balancing mode:\n");
			printf("----------------------------------------------\n");
			printf(" - PBAL: Divide inputfile into chunks of equal amount of hits.\n");
			printf(" - DIAG: Divide inputfile into chunks following the diagonals.\n");
			printf(" - EDIAG: Divide inputfile into one instruction for each diagonal.\n");
			exit(-1);
		#endif
	#endif
#endif

    fclose(fIN);
    fclose(fOUT);

	return 0;

}

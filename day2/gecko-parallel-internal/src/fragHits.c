/*
 Parallel fragHits
 Before executing this program you should execute splitHits
 to calculate the hits file coordinates to be processed by
 each worker

 Search for all By-Identitity ungapped fragments with length > MinLen

 This program is an enhancement of AllFrags (that searches the full space)
 In this case, start-Fragments (HITS) has been detected in previous steps and
 this hits are used as seeds to extend the fragment. A backward step is also
 included to extend the fragment in the second opposite direction from the hit
 
 Syntax:

 AllFragsHits SeqX.file SeqY.file HitsFile Out.file Lmin SimThr

 Some new parameters have been included:
 - HitsFile (binary) long Diag/posX/posY
 - Lmin     Minimum length in the case of fixed length and percentage otherwise
 - SimThr   Similarity (identity) Threshold

 - SeqX & SeqY files are in fasta format
 - Out file (binary) will save a fragment with the format specified in structs.h.
   ----------------------------------------------
*/

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include "structs.h"
#include "commonFunctions.h"
#include "comparisonFunctions.h"

#define WORKTAG    1
#define ENDTAG     2
#define POINT      4

void master(char ** av);
void worker(char ** av);
int FragFromHit(struct FragFile *myF, hit *H, struct Sequence *sX,
		uint64_t n0, struct Sequence *sY,
		uint64_t n1, uint64_t nSeqs1, uint64_t LenOrPer, uint64_t SimTh, int WL,
		int fixedL, char strand);

int main(int ac, char **argv) {
    //Testing input parameters
    if(ac!=11)
        terror("USE: ./FragHits SeqX SeqY HitsFileIN FragFileOut Lmin Simthr WL fixedLen(1/0) strand(f/r) InstrFileIN");
    
    //Initializing MPI environment
    int myID;
    MPI_Init(&ac, &argv);
    MPI_Comm_rank( MPI_COMM_WORLD, &myID);

    //Starting the master and slaves
    if (myID == 0) master(argv);
    else worker(argv);

    MPI_Finalize();
    return 0;
}

void master(char ** av) {
    unsigned long Coord[2];
    MPI_Status status;
    int ntasks, i, N;
    instruction nI;
    FILE * fIN;
    
    //Verifying number of available processors.
    MPI_Comm_size(MPI_COMM_WORLD, &ntasks);

    //Opening coordinates
    if ((fIN=fopen(av[10],"rt"))==NULL) {
        printf("MASTER: Invalid input file\n");
        for (i = 1; i < ntasks; i++) {
            MPI_Send(0, 0, MPI_LONG, i, ENDTAG, MPI_COMM_WORLD);
        }
        return;
    }

    //Distributing every coordinates
    fread(&nI,sizeof(instruction),1,fIN);
	while(!feof(fIN)) {
		for (i = 1; i < ntasks && !feof(fIN); i++) {
			Coord[0] = nI.start;
			Coord[1] = nI.end;
			MPI_Send(Coord, 2, MPI_LONG, i, WORKTAG, MPI_COMM_WORLD);
			fread(&nI,sizeof(instruction),1,fIN);
		}
	}
	fclose(fIN);

	//After executing every assigned instructions, the workers will end...
	for (i = 1; i < ntasks; i++) {
		MPI_Send(0, 0, MPI_LONG, i, ENDTAG, MPI_COMM_WORLD);
	}

	//Waiting for every worker.
	for (i = 1; i < ntasks; i++) {
		MPI_Recv(&N, 1, MPI_INT, MPI_ANY_SOURCE,MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	}
	
	//Merge
	FILE *f2, *fF;
	uint64_t n1, n2;
	char tmp[1024];
	struct FragFile F;
	long tF=0;
	//Opening Final output file
	if ((fF=fopen(av[4],"wb"))==NULL){
		fprintf(stderr, "MASTER: ERROR Opening Output file\n");
		return;
	}
	//Looping in every chunk file
	for (i=1; i < ntasks; i++) {
		sprintf(tmp,"%s-%04d",av[4],i);
		if ((f2=fopen(tmp,"rb"))==NULL) {
			fprintf(stderr, "MASTER: Invalid CHUNK input file\n");
			return;
		}

		readSequenceLength(&n1, f2);
		readSequenceLength(&n2, f2);
		if (i==1) {
			writeSequenceLength(&n1, fF);
			writeSequenceLength(&n2, fF);
		}

		readFragment(&F, f2);
		while (!feof(f2)){
			tF++;
			writeFragment(&F, fF);
			readFragment(&F, f2);

		}
		fclose(f2);
		//unlink(tmp);
	}
	fclose(fF);
}

void worker(char ** av){
	struct Sequence *sX, *sY;
	uint64_t n0, n1, ns0, ns1;
	int Lmin, SimTh, WL, fixedL;
	int newFrag;
	uint64_t nFrags = 0, nHitsUsed = 0, nHits = 0;
	int64_t lastOffset, lastDiagonal;
	struct FragFile myF;
	char filename[1024];
	char strand;
	FILE *f, *fH, *fOut;
	char *seqX, *seqY;
	unsigned long N[2];
	MPI_Status status;
	int myID;
	hit H;

	MPI_Comm_rank( MPI_COMM_WORLD, &myID);
	
	//Initializing the sequences
	seqX = av[1];
	if ((f=fopen(av[1],"rt"))==NULL) {
		fprintf(stderr, "SLAVE n°%d: Invalid SeqX file\n",myID);
		return;
	}
	sX=LeeSeqDB(f, &n0, &ns0, 0);
	fclose(f);
	
	seqY = av[2];
	if ((f=fopen(av[2],"rt"))==NULL) {
		fprintf(stderr, "SLAVE n°%d: Invalid SeqY file\n",myID);
		return;
	}
	sY=LeeSeqDB(f, &n1, &ns1, 0);
	fclose(f);

	//Initialize variables
	Lmin = atoi(av[5]);
	SimTh = atoi(av[6]);
	WL = atoi(av[7]);
	fixedL = atoi(av[8]);
	strand = av[9][0];

	//for filename purpose
	sprintf(filename,"%s-%04d",av[4],myID);
	//Opening output file
	if ((fOut=fopen(filename,"wb"))==NULL) {
		fprintf(stderr, "SLAVE n°%d: Impossible to open output file\n",myID);
		return;
	}

	//Write sequence lengths
	writeSequenceLength(&n0, fOut);
	writeSequenceLength(&n1, fOut);

	//Open hits file -------------------------------------------------
	if ((fH=fopen(av[3],"rb"))==NULL) {
		fprintf(stderr, "SLAVE n°%d: Impossible to open Hits file\n",myID);
		return;
	}
	
	while(1) {
		//Waiting for orders
		MPI_Recv(N, 2, MPI_LONG, MPI_ANY_SOURCE,MPI_ANY_TAG, MPI_COMM_WORLD, &status);

		//Check if the worker should exit
		if (status.MPI_TAG == ENDTAG){
			fclose(fH);
			fclose(fOut);
			//Warn the Master that this process has finished.
			MPI_Send(&myID, 1, MPI_INT, 0, WORKTAG, MPI_COMM_WORLD);
			return;
		}

		//read Hits and calculate the fragments
		//position in the assigned coordinate
		fseek(fH,N[0]*sizeof(hit), SEEK_SET);
		fread(&H,sizeof(hit),1,fH);
		lastDiagonal  = H.diag;
		lastOffset = H.posX-1;
		//Iterate until the final coordinate
		while(N[0] <= N[1] && !feof(fH)) {
			nHits++;
			N[0]++;
			if (lastDiagonal!= H.diag) {
				lastDiagonal = H.diag;
				lastOffset=H.posX-1;
			}

			if (lastOffset > ((int64_t) H.posX)) {
				//The hit is covered by a previous frag in the same diagonal
				if(fread(&H, sizeof(hit), 1, fH)!=1){
					if(ferror(fH))terror("Error reading from hits file");
				}
				continue;
			}

			nHitsUsed++;
			newFrag = FragFromHit(&myF, &H, sX, n0, sY, n1, ns1, Lmin, SimTh,
						WL, fixedL, strand);
			if (newFrag) {
				writeFragment(&myF, fOut);
				lastOffset = H.posX + myF.length;
				nFrags++;
			}
			if(fread(&H, sizeof(hit), 1, fH)!=1){
				if(ferror(fH))terror("Error reading from hits file");
			}
		}
	}
}

/**
 * Compute a fragments from one seed point
 * Similarirty thershold and length > mimL
 */
int FragFromHit(struct FragFile *myF, hit *H, struct Sequence *sX,
		uint64_t n0, struct Sequence *sY,
		uint64_t n1, uint64_t nSeqs1, uint64_t Lm, uint64_t SimTh, int WL,
		int fixedL, char strand) {
	int64_t ldiag, ldiag2;
	int64_t xfil, ycol;
	/* for version with backward search */
	int64_t xfil2, ycol2;
	int fragmentLength = 0;
	/* for version Maximum global---*/
	int64_t xfilmax, ycolmax;
	/* for version with backward search */
	int64_t xfilmax2, ycolmax2;
	int nIdentities = 0, maxIdentities = 0;
	char valueX, valueY;
	int fscore = 0, fscoreMax = 0; // full score

	uint64_t minLength =
			(fixedL) ?
					Lm :
					(uint64_t) (min(getSeqLength(sX),
							getSeqLength(sY)) * (Lm / 100.0));

	// Initialize values
	ldiag = min(n0 - H->posX, n1 - H->posY);
	//var to know how much we have until we reach the origin of coordinates
	ldiag2 = min(H->posX, H->posY);
	xfil = H->posX + WL;
	xfil2 = H->posX - 1;
	ycol = H->posY + WL;
	ycol2 = H->posY - 1;
	fragmentLength += WL;
	xfilmax = xfil;
	xfilmax2 = xfil2;
	ycolmax = ycol;
	ycolmax2 = ycol2;
	nIdentities = maxIdentities = WL;
	fscore = POINT * WL;
	fscoreMax = fscore;

	// now, looking for end_frag---
	while (fragmentLength < ldiag) {
		valueX = getValue(sX, xfil);
		valueY = getValue(sY, ycol);
		if (valueX == '*' || valueY == '*') {
			//separator between sequences
			break;
		}

		if (valueX == 'N' || valueY == 'N') {
			fscore -= 1;
		} else {
			if (valueX == valueY) {
				// match
				fscore += POINT;
				nIdentities++;
				if (fscoreMax < fscore) {
					fscoreMax = fscore;
					xfilmax = xfil;
					ycolmax = ycol;
					maxIdentities = nIdentities;
				}
			} else {
				fscore -= POINT;
			}
		}

		xfil++;
		ycol++;
		fragmentLength++;
		if (fscore < 0)
			break;
	}

	/**
	 * Backward search --- Oscar (Sept.2013)
	 **/
	fragmentLength = 0;
	fscore = fscoreMax;
	xfilmax2 = H->posX;
	ycolmax2 = H->posY;
	nIdentities = maxIdentities;
	if (xfil2 >= 0 && ycol2 >= 0)
		while (fragmentLength < ldiag2) {
			valueX = getValue(sX, xfil2);
			valueY = getValue(sY, ycol2);
			if (valueX == '*' || valueY == '*') {
				//separator between sequences
				break;
			}

			if (valueX == 'N' || valueY == 'N') {
				fscore -= 1;
			} else {
				if (valueX == valueY) {
					// matches----
					fscore += POINT;
					nIdentities++;
					if (fscoreMax < fscore) {
						fscoreMax = fscore;
						xfilmax2 = xfil2;
						ycolmax2 = ycol2;
						maxIdentities = nIdentities;
					}
				} else {
					fscore -= POINT;
				}
			}

			xfil2--;
			ycol2--;
			fragmentLength++;
			if (fscore < 0)
				break;
		}

	// Set the values of the FragFile
	myF->diag = H->diag;
	myF->xStart = (uint64_t) xfilmax2 - H->seqX;
	myF->yStart = (uint64_t) ycolmax2 - H->seqY;
	myF->xEnd = (uint64_t) xfilmax - H->seqX;
	myF->yEnd = (uint64_t) ycolmax - H->seqY;;
	myF->length = myF->xEnd - myF->xStart + 1;
	myF->score = fscoreMax;
	myF->ident = maxIdentities;
	myF->similarity = myF->score * 100.0
			/ scoreMax(&sX->datos[myF->xStart], &sY->datos[myF->yStart],
					myF->length, POINT);
	myF->seqX = H->seqX;
	myF->seqY = (strand=='f')? H->seqY : nSeqs1 - H->seqY - 1;
	myF->block = 0;
	myF->strand = strand;

	if (myF->length > minLength && myF->similarity > SimTh)
		return 1;
	else
		return 0;
}
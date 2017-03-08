/*

generateCSV.c

This program takes a fragment file with CSB marked and extract CSB composition

Use: generateCSV file.frags file.csv\n

oscart@uma.es
--------------------------------------------------
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <math.h>

#include "structs.h"
#include "comparisonFunctions.h"

int main(int ac,char** av){
	struct FragFile* f;
	int nf; // Number of fragments
	uint64_t xtotal,ytotal;
	FILE *fOut;
	
	if(ac!=3){
		fprintf(stderr, "Use: getInfo file.frags file.csv\n");
		exit(-1);
	}
	
	if ((fOut = fopen(av[2], "wt")) == NULL){
		fprintf(stderr, "Error opening the output CSV file\n");
		exit(-1);
	}

	// Read fragments
	nf=0;
	f=readFragments(av[1], &nf, &xtotal, &ytotal);
	
	/******************************************/
	int i;
	
	// Print header. Frags file info
	fprintf(fOut, "All by-Identity Ungapped Fragments (Hits based approach)\n");
	fprintf(fOut, "[Abr.98/Apr.2010/Dec.2011 -- <ortrelles@uma.es>\n");
	fprintf(fOut, "SeqX filename        : <unknown>\n");
	fprintf(fOut, "SeqY filename        : <unknown>\n");
	fprintf(fOut, "SeqX name            : <unknown>\n");
	fprintf(fOut, "SeqY name            : <unknown>\n");
	fprintf(fOut, "SeqX length          : %" PRIu64 "\n", xtotal);
	fprintf(fOut, "SeqY length          : %" PRIu64 "\n", ytotal);
	fprintf(fOut, "Min.fragment.length  : <unknown>\n");
	fprintf(fOut, "Min.Identity         : <unknown>\n");
	fprintf(fOut, "Tot Hits (seeds)     : <unknown>\n");
	fprintf(fOut, "Tot Hits (seeds) used: <unknown>\n");
	fprintf(fOut, "Total fragments      : %d\n", nf);
	fprintf(fOut, "========================================================\n");
	fprintf(fOut, "Total CSB: 0\n");
	fprintf(fOut, "========================================================\n");
	fprintf(fOut, "Type,xStart,yStart,xEnd,yEnd,strand(f/r),block,length,score,ident,similarity,%%ident,SeqX,SeqY\n");
	
	double similarity,likeness;
				
	for(i=0;i<nf;i++){
		similarity=(((double)f[i].score)/((double)f[i].length*4.0));
		likeness=(((double)f[i].ident)/((double)f[i].length));
		
		if(f[i].strand=='r'){
			f[i].yStart = ytotal - f[i].yStart - 1;
			f[i].yEnd = ytotal - f[i].yEnd - 1;
		}

		fprintf(fOut, "Frag,%d,%d,%d,%d,%c,%d,%d,%d,%d,%.2f,%.2f,%d,%d\n",(int)f[i].xStart,(int)f[i].yStart,(int)f[i].xEnd,(int)f[i].yEnd,f[i].strand,(int)f[i].block,(int)f[i].length,(int)f[i].score,(int)f[i].ident,similarity,likeness,(int)f[i].seqX,(int)f[i].seqY);
				
	}

	fclose(fOut);
	
	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLID 200
#define MAXLS 1000000

struct Sequence{
   char ident[MAXLID];
   char *data;
};

struct Chunks{
   int chunkX;
   int chunkY;
   int x;
   int y;
};

struct SortChunks{
   struct Chunks elem;
   struct SortChunks* next;
};

/**
 * Function to read a set of sequences from the given fasta file 'f'.
 * The sum of the length of all the sequences contained in the fasta file is returned in the totalLength variable.
 * The number of sequences in the fasta file is returned in the nSequences variable
 * The functions returns an array with all the sequences contained in the fasta file
 **/
struct Sequence* readSequences(FILE *f, unsigned long *totalLength, unsigned long *nSequences);

/**
 * Function to split a given fasta file 'file' into nChunks of equal size
 * It returns the number of chunks.
 */
unsigned long splitSequencesBlast(char *file, int nChunks);

/**
 * Function to split a given fasta file 'file' into nChunks of different size
 * It returns the number of chunks.
 */
unsigned long splitSequencesBlastGSS(char *file, int nChunks, unsigned long minSize, int gss);

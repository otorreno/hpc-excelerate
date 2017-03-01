#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "map_functions.h"

unsigned long* createChunkListGSS(unsigned long n, unsigned long N, unsigned long minsize, unsigned long *len, int gss);

struct Sequence* readSequences(FILE *f, unsigned long *totalLength, unsigned long *nSequences){
   char c;
   unsigned long i;
   struct Sequence *sequences, *aux;
   char data[MAXLS];

   //Initialize
   *totalLength = 0;
   *nSequences = 0;

   //Memory
   if((sequences=aux=(struct Sequence*)malloc(sizeof(struct Sequence)))==NULL){
      fprintf(stderr, "Not enough memory for a sequence structure\n");
      exit(-1);
   }
   
   while((c=fgetc(f))!= '>' && !feof(f)); //Looking for the start of the first sequence

   while(!feof(f)){ 
      //Sequence ID...
      i=0;

      //Reallocating the array for one more sequence
      (*nSequences)++;
      if((sequences = (struct Sequence*)realloc(sequences, (*nSequences) * sizeof(struct Sequence)))==NULL){
         fprintf(stderr, "Re-alloc error in te readSequence function\n");
         exit(-1);
      }

      aux = sequences + *nSequences - 1;

      aux->ident[i++] = '>';
      c=fgetc(f);
      while(i<MAXLID && c!='\n')
      {
         if(feof(f))return 0;

         aux->ident[i++]=c;
         c=fgetc(f);
      } 
   
      aux->ident[i]=0;

      //The sequence...
      c=fgetc(f);
   
      i = 0;
      while(!feof(f) && c!='>')
      {
         c=toupper(c);
         if (i >= MAXLS) {
            fprintf(stderr, "Sequence length longer than the maximum allowed, which is: %d\n", MAXLS);
            exit(-1);
         }
         if(isupper(c)) data[i++]=c;
         c=fgetc(f);
      }

      if(i < MAXLS) data[i]=0;
      (*totalLength) += i;
      aux->data = (char*)malloc((i+1)*sizeof(char));
      memcpy(aux->data, data, i+1);
   }

   return sequences;
}

unsigned long splitSequencesBlast(char *file, int nChunks){
  FILE   *fIn, *fOut;
  char tmp[1000];
  struct Sequence *sequences;
  unsigned long totalLength;
  unsigned long nSequences;
  unsigned long i, j, chunkSize;
  
  if ((fIn = fopen(file,"rt"))==NULL) {
    fprintf(stderr, "Error opening input file: %s\n", file);
  }

  //Reading the sequences
  sequences = readSequences(fIn, &totalLength, &nSequences);
  fclose(fIn);

  //Calculating the chunk size
  chunkSize = nSequences/nChunks;
  if (chunkSize * nChunks < nSequences) chunkSize++;

  //Creating nChunks files
  for (i=0; i<nChunks; i++) {
     sprintf(tmp, "%s-%04lu", file, i);
     if ((fOut = fopen(tmp,"wt"))==NULL) {
        fprintf(stderr, "opening sequence Y file. Seq: %s\n", tmp);
     }

     j = 0;
     while(j < chunkSize && sequences != NULL){
         fprintf(fOut, "%s\n", sequences->ident);
         fprintf(fOut,"%s\n",sequences->data);

         sequences++;
         j++;
     }

     fclose(fOut);
  }
  return nChunks;
}



unsigned long splitSequencesBlastGSS(char *file, int nChunks, unsigned long minSize, int gss){
  FILE   *fIn, *fOut;
  char tmp[1000];
  struct Sequence *sequences;
  unsigned long totalLength;
  unsigned long nSequences;
  unsigned long i, j, length, *chunkList;
  
  if ((fIn = fopen(file,"rt"))==NULL) {
    fprintf(stderr, "Error opening input file: %s\n", file);
    exit(-1);
  }

  //Reading the sequences
  sequences = readSequences(fIn, &totalLength, &nSequences);
  fclose(fIn);
  chunkList = createChunkListGSS(nSequences, nChunks, minSize, &length, gss);

  i = 0;
  while(i < length){
     sprintf(tmp, "%s-%04lu", file, i);
     if ((fOut = fopen(tmp,"wt"))==NULL) {
        fprintf(stderr, "opening sequence Y file. Seq: %s\n", tmp);
        exit(-1);
     }

     j = 0;
     while(j < chunkList[i] && sequences != NULL){
         fprintf(fOut, "%s\n", sequences->ident);
         fprintf(fOut,"%s\n",sequences->data);

         sequences++;
         j++;
     }
     i++;
     fclose(fOut);
  }

  free(chunkList);

  return length;
}

unsigned long* createChunkListGSS(unsigned long n, unsigned long N, unsigned long minsize, unsigned long *len, int gss){
   unsigned long length, i, chunk, totalSize;
   unsigned long *chunkList, *chunkMlist;

   length = 0;
   i=0;
   totalSize = n;
   chunkList = NULL;

   //gss = 1 -> mgss else gss
   if(gss)
      n = n / 2;

   while(n > 0){

      length++;

      if ((chunkList = (unsigned long*)realloc(chunkList, length * sizeof(unsigned long))) == NULL) {
         fprintf(stderr, "reallocating memory in createChunkListGSS function\n");
         exit(-1);
      }

      chunk = n / N;

      if (chunk < minsize){
         if (n >= minsize){
            chunk = minsize;
         }else{
            chunk = n;
         }
      }

      n = n - chunk;
      i = length - 1;

      chunkList[i] = chunk;
   }

   //if the last chunk is less size than minimun size, this chunk is added
   // to the first element of the list (the bigger one)
   i = length - 1;
   if (chunkList[i] < minsize){
      chunkList[0] += chunkList[length - 1];
      length--;
   }

   //mgss
   if(gss){
      chunk=0;
      chunkMlist = (unsigned long*)malloc(length*2*sizeof(unsigned long));
      for(i = (length-1); i >= 0; i--){
         chunkMlist[length - i - 1] = chunkList[i];
         chunkMlist[length + i] = chunkList[i];
         chunk += chunkList[i]*2;
      }
      
      if(chunk < totalSize){
         chunkMlist[length] += totalSize - chunk;
      }

      length *= 2;
      *len = length;
      return chunkMlist;
   }

   //normal gss
   *len = length;
   return chunkList;
}

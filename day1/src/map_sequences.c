#include <stdio.h>
#include <stdlib.h>
#include "map_functions.h"

char* getInput(int, char**, int*);
char* getOutput(int, char**, int*, char**, char**);

int main(int ac, char**av){
   int nChunks, iin, iout;
   FILE *fOut;
   char tmp[1000];
   char *parameters, *aux, *input, *prefix, *suffix;
   unsigned long i, length;

   if (ac<5){ 
      fprintf(stderr, "USE: ./mapSequences inputFile.fasta nChunks +commandLine (-query input -out output )\n");
      exit(-1);
   }
   
   nChunks=atoi(av[2]);

   input  = getInput(ac, av, &iin);
   getOutput(ac, av, &iout, &prefix, &suffix);
   
   length = splitSequencesBlast(input, nChunks);
   //length = splitSequencesBlastGSS(input, nChunks, 1, 0);

   sprintf(tmp,"%s-map",av[1]);
   if ((fOut=fopen(tmp,"wt"))==NULL) {
      fprintf(stderr, "opening final MAP script file\n");
      exit(-1);
   }
   
   //parameters command line (join all strings)
   parameters=(char*)malloc(sizeof(char)*(strlen(av[3])+1));
   sprintf(parameters, "%s", av[3]);
   for(i=4; i<ac;i++){
      if(i!=iin && i!=iin-1 && i!=iout && i!= iout-1){
         aux=(char*)malloc(sizeof(char)*(strlen(av[i])+1));
         sprintf(aux, " %s", av[i]);
         parameters = (char*)realloc(parameters, sizeof(char)*(strlen(aux)+strlen(parameters)+1));
         strcat(parameters, aux);
      }
   }

   for (i=0;i<length;i++) {
      fprintf(fOut, "%s %s %s-%04lu %s %s-%04lu.%s\n", parameters, av[iin-1], av[iin], i, av[iout-1], prefix, i, suffix);
   }

   fclose(fOut);

   //Reduce
   sprintf(tmp, "%s-red", av[1]);
   if ((fOut=fopen(tmp,"wt"))==NULL) {
      fprintf(stderr, "opening final REDUCE script file\n");
      exit(-1);
   }

   fprintf(fOut, "cat ");

   for (i=0;i<length;i++) {
      fprintf(fOut, "%s-%04lu.%s ", prefix, i, suffix);
   }

   fprintf(fOut, "> %s.%s", prefix, suffix);

   fclose(fOut);

   return 0;
}

char* getInput(int ac, char** arg, int* index){
   int fin=0;
   int i=0;
   char* res = NULL;
   while(!fin && i<ac){
      if(strcmp(arg[i], "-query")==0){
         fin=1;
         i++;
         res = arg[i];
      }else{
         i++;  
      } 
   }
 
   *index = i;
   return res;
}

char* getOutput(int ac, char** arg, int* index, char** prefix, char** suffix){
   int end=0;
   int i=0;
   char* res = NULL;

   end=0;
   i=0;
   while(!end && i<ac){
      if(strcmp(arg[i], "-out")==0){
         end=1;
         i++;
         res = arg[i];
      }else{
         i++;  
      } 
   }

   (*prefix) = strtok(res, ".");
   (*suffix) = strtok(NULL, ".");
   *index = i;
   return res;
}

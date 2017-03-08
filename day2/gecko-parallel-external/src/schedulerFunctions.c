#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>

#include "commonFunctions.h"
#include "schedulerFunctions.h"

char** readWorkloadFile(char *nomfich, int *n) {

   FILE *fwork;
   char *cadena, tmp[MAXLIN];
   int i; 
   char **table;

   *n = 0;

   fwork = fopen(nomfich, "rt");
   if (fwork == NULL) terror ("opening Workload file");

   fgets(tmp, MAXLIN, fwork);
   while(!feof(fwork)) {
        *n = *n + 1;
        fgets(tmp, MAXLIN, fwork);
   }

   rewind(fwork);

   if ((table = (char**)malloc(*n*sizeof(char*))) == NULL)
      terror("Memory, ReadWorkloadfile");

   for(i=0; i<*n; i++){
      cadena = table[i] = (char *)malloc(MAXLIN*sizeof(char));
      fgets(cadena, MAXLIN, fwork);
      if(cadena[strlen(cadena)-1]=='\n') cadena[strlen(cadena)-1] = 0x00;
   }

   fclose(fwork);

   return table;
}

char* prepareMessage(int* countCommand, int totalCommands, char** table, int* length){
   char *messageAux;
   messageAux = (char *)malloc(MAXLIN*sizeof(char));
   int nmsg;

   nmsg = 0;
   memset(messageAux, 0, MAXLIN);
   if((*countCommand) < totalCommands){      
      strncat(messageAux, table[(*countCommand)], strlen(table[(*countCommand)]));
      (*countCommand)++;
   } else {
      strncat(messageAux, "", 0);
   }

   *length = strlen(messageAux) + 1;

   return messageAux;
}

int getCommandLine(char *completeLine, char **arguments) {
  char *tmp = (char *)strdup(completeLine);
  char *token;
  int narg=0;

  token=strtok(tmp," "); 
  while (token != NULL) {
    arguments[narg]=(char *)strdup(token);
    narg++;
    token=strtok(NULL," ");
  } 

  free(tmp);

  arguments[narg]=NULL;

  return narg;
}

int getMessage(char *completeLine, char **messages) {
  char *token;
  int narg=0;
  
  if((token=strtok(completeLine,";")) != NULL) { 
    messages[narg]=(char *)strdup(token);
    narg++; 

    while ((token=strtok(NULL,";")) != NULL) {
  	  messages[narg]=(char *)strdup(token);
  	  narg++;
    } 
  }

  messages[narg]=NULL;

  return narg;
}

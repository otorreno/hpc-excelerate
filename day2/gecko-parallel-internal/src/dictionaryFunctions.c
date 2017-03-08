#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>

#include "structs.h"
#include "commonFunctions.h"

int letterToIndex(char c) {
	// coding (a=0,c=1,g=2,t=3, others=-1 )
	switch (c) {
	case 'A':
		return 0;
	case 'C':
		return 1;
	case 'G':
		return 2;
	case 'T':
		return 3;
	default:
		return -1;
	}
}

int seq2word(char* buf, int wsize, word* w) {
	int i;
	int b = 6;

	memset(w, 0, sizeof(word));

	for (i = 0; i < wsize; i++) {
		w->b[i / 4] |= letterToIndex(buf[i]) << b;
		b -= 2;
		if (b < 0)
			b = 6;
	}
	return 0;
}

void skipIDLine(FILE *fIn) {
	char c;
	// first line (skip ID fasta Line)
	c = fgetc(fIn);
	while (c != '\n')
		c = fgetc(fIn);
}

int loadSequence(char *fileName, char *seq, uint64_t *Tot) {
	FILE *fIn;
	char c;

	//Opening input and output files
	if ((fIn = fopen(fileName, "rt")) == NULL)
		terror("opening sequence file");

	//Skip the identification of the sequence
	skipIDLine(fIn);

	// Load Sequence into memory
	c = toupper(fgetc(fIn));
	while (!feof(fIn)) {
		//Check if is a letter
		if (c < 65 || c > 90) {
			/*
			 * If not is a start of a sequence,
			 * then read a new char and continue
			 */
			if (c != '>') {
				c = toupper(fgetc(fIn));
				continue;
			}
		}

		//Get the index of the letter
		seq[*Tot] = letterToIndex(c);

		//Check if is a multi-sequence file
		if (c == '>') {
			skipIDLine(fIn);
		}

		(*Tot)++;
		c = toupper(fgetc(fIn));
	}

	fclose(fIn);
	return 0;
}

int wordcmp(unsigned char *w1, unsigned char*w2, int n) {

	int i = 0, limit;

	if(n%4 != 0){
		w1[n/4] = w1[n/4] >> (2*(3-((n-1)%4)));
		w1[n/4] = w1[n/4] << (2*(3-((n-1)%4)));
		w2[n/4] = w2[n/4] >> (2*(3-((n-1)%4)));
		w2[n/4] = w2[n/4] << (2*(3-((n-1)%4)));
		limit=(n/4)+1;
	} else {
		limit = n/4;
	}

	for (i=0;i<limit;i++) {
		if (w1[i]<w2[i]) return -1;
		if (w1[i]>w2[i]) return +1;
	}
	return 0;
}

void showWord(word* w, char *ws) {
	char Alf[] = { 'A', 'C', 'G', 'T' };
	int i;
	int wsize = 8;
	unsigned char c;
	for (i = 0; i < wsize; i++) {
		c = w->b[i];
		c = c >> 6;
		ws[4*i] = Alf[(int) c];
		c = w->b[i];
		c = c << 2;
		c = c >> 6;
		ws[4*i+1] = Alf[(int) c];
		c = w->b[i];
		c = c << 4;
		c = c >> 6;
		ws[4*i+2] = Alf[(int) c];
		c = w->b[i];
		c = c << 6;
		c = c >> 6;
		ws[4*i+3] = Alf[(int) c];
	}
}

void destroy_tree(node **leaf) {
	if (*leaf != NULL) {
		destroy_tree(&(*leaf)->left);
		destroy_tree(&(*leaf)->right);
		list_node *aux;
		while ((*leaf)->positions != NULL) {
			aux = (*leaf)->positions;
			(*leaf)->positions = (*leaf)->positions->next;
			aux = NULL;
		}
		(*leaf)->last = NULL;
		(*leaf) = NULL;
	}
}

int insert(char *key, int wsize, location loc, node **leaf, node *nodePool,
		unsigned long *actualNodes, list_node *locationPool, unsigned long *actualLocations) {
	if (*leaf == NULL) {
		*leaf = &nodePool[(*actualNodes)++];
		memcpy((*leaf)->key_value, key, wsize);
		//initialize the children to null
		(*leaf)->left = NULL;
		(*leaf)->right = NULL;
		(*leaf)->ocurrences = 1;
		//Insert the position in the list
		(*leaf)->positions = (*leaf)->last =
				&locationPool[(*actualLocations)++];
		(*leaf)->positions->next = (*leaf)->last->next = NULL;
		memcpy(&(*leaf)->last->loc, &loc, sizeof(location));

		//Returning 1 to add it to the var controlling the number of words in the tree
		return 1;
	}
	int cmp = strncmp(key, (*leaf)->key_value, wsize);
	if (cmp < 0) {
		return insert(key, wsize, loc, &(*leaf)->left, nodePool, actualNodes,
				locationPool, actualLocations);
	} else if (cmp > 0) {
		return insert(key, wsize, loc, &(*leaf)->right, nodePool, actualNodes,
				locationPool, actualLocations);
	} else {
		//The node is already in the tree, just add the location
		(*leaf)->ocurrences++;
		(*leaf)->last->next = &locationPool[(*actualLocations)++];
		(*leaf)->last = (*leaf)->last->next;
		(*leaf)->last->next = NULL;
		memcpy(&(*leaf)->last->loc, &loc, sizeof(location));

		//Returning 0 because the word is already in the tree
		return 0;
	}

	return -1;
}

void printRoot(node *leaf, FILE* fOut) {
	hashentry he;

	seq2word(leaf->key_value, 33, &he.w);
	he.num = leaf->ocurrences;
	fwrite(&he, sizeof(hashentry), 1, fOut);
	list_node *aux = leaf->positions;
	while (aux != NULL) {
		fwrite(&aux->loc, sizeof(location), 1, fOut);
		aux = aux->next;
	}

}

void writeTree(node *leaf, FILE* fOut) {
	if (leaf != NULL) {
		writeTree(leaf->left, fOut);
		printRoot(leaf, fOut);
		writeTree(leaf->right, fOut);
	}
}

unsigned long time() {
	struct timeval tv;

	gettimeofday(&tv, NULL);

	return (tv.tv_usec / 1000) + (tv.tv_sec * 1000);
}

void kmerIndex2Word(int index, int K, char *word){
	unsigned long resto;
	unsigned long cociente = index;
	char alph[]={'A','C','G','T'};
	int i=0;
	
	for(i=0;i<K;i++)
		word[i]='A';

	i=0;
	while(cociente>=4){ // 4 is the alphabet size
		resto=cociente%4;
		cociente=cociente/4;
		word[K-i-1]=alph[resto];
		i++;
	}
	word[K-i-1]=alph[cociente];
}

int prefixTable(char *prefixes, char **table) {
  char *token;
  int nprefixes=0;

  //fprintf(stdout, "completeLine: %s\n", completeLine);

  if((token=strtok(prefixes,",")) != NULL) { // busca una palabra separada por espacios
      table[nprefixes]=(char *)strdup(token);// copia la palabra
      nprefixes++; // incrementa el contador de prefijos
      while ((token=strtok(NULL,",")) != NULL) {// busca mas palabras en misma linea
	  table[nprefixes]=(char *)strdup(token);
	  nprefixes++;
      } 
  }

  table[nprefixes]=NULL;

  return nprefixes;
}

void readWorkloadFile(char *nomfich, char **workToDo, int *n) {

	FILE *fwork;
	char cadena[1024];

	//n is the number of command line
	*n = 0;

	fwork = fopen(nomfich, "rt");
	if (fwork == NULL) terror ("opening Workload file");


	// counting the number of processess in workload file
	fgets(cadena, 1024, fwork);
	while(!feof(fwork)) {
		if(cadena[strlen(cadena)-1]=='\n') cadena[strlen(cadena)-1] = 0x00;
		workToDo[*n] = (char *) strdup(cadena);
		*n = *n + 1;
		fgets(cadena, 1024, fwork);
	}

	fclose(fwork);
}

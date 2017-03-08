#ifndef DICTIONARY_FUNCTIONS_H
#define DICTIONARY_FUNCTIONS_H

#define NODEPOOLSIZE 2000000
#define LOCATIONPOOLSIZE 2000000

#define MIN(a,b) (((a)<(b))?(a):(b))

//Internal structures to work with a memory pool
typedef struct mn {
	node *data;
	struct mn *next;
} treeMemory_node;

typedef struct ln {
	list_node *data;
	struct ln *next;
} locationMemory_node;

/**
 * Compress the word stored in 'buf' using 2 bits per letter
 * The result will be at word 'w'
 */
int seq2word(char* buf, int wsize, word* w);

/**
 * Function to skip the identification line of a fasta sequence
 */
void skipIDLine(FILE *fIn);

/**
 * Function to convert the alphabet letter
 * to an index.
 * coding (a=0,c=1,g=2,t=3,'>'=4 others=9)
 */
int letterToIndex(char c);

/**
 * Function to load the sequence.
 * The code was present inside words.c program
 */
int loadSequence(char *fileName, char *seq, uint64_t *Tot);

/**
 * Function to print in stdout the given compressed word
 */
void showWord(word* w, char *ws);

/**
 * Function to compare two k-mers
 * The function returns 0 if equal, 1 if greater
 * than and -1 otherwise
 */
int wordcmp(unsigned char *w1, unsigned char*w2, int n);

/**
 * Function to destroy the binary tree
 */
void destroy_tree(node **leaf);

/**
 * Insert the given word 'key' in the binary tree
 */
int insert(char *key, int wsize, location loc, node **leaf, node *nodePool, unsigned long *actualNodes, list_node *locationPool, unsigned long *actualLocations);

/**
 * Write the binary tree in order (left,
 * root, right)
 */
void writeTree(node *leaf, FILE* fOut);

/**
 * Function to return the time in milliseconds since epoch
 */
unsigned long time();

/**
 * Function to convert a k-mer hash index to the actual k-mer word
 */
void kmerIndex2Word(int index, int K, char *word);

/**
 * Function used by each MPI dictionary process to know the prefixes it should compute
 */
int prefixTable(char *prefixes, char **table);

/**
 * Function used by the master process to read the complete dictionary workload
 * which will be stored in the workToDo variable and later send to the slaves
 */
void readWorkloadFile(char *nomfich, char **workToDo, int *n);

#endif /* DICTIONARY_FUNCTIONS_H */

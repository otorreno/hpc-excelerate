#ifndef STRUCTS_H
#define STRUCTS_H

#include <inttypes.h>
//Structs required for the dotplot workflow
#define MAXLID 200
#define MAXLS 1000000000
#define READBUF 50000000 //50MB

//Struct for words program
typedef struct {
	//Each letter is stored using 2 bits
	//We have 4 letters per byte and a
	//maximum of 32 in 'b'
	unsigned char b[8];
} word;

//Struct for w2hd program
typedef struct {
	//Word compressed in binary format
	word w;
	//Number of ocurrences inside the
	//sequence. This is used to know the
	//number of locations stored in the
	//positions file
	uint64_t num;
} hashentry;

//Struct for w2hd program
typedef struct {
	//Ocurrence position in the sequence
	uint64_t pos;
	//For multiple sequence files this var
	//reflects in what sequence occurs the
	//word
	uint64_t seq;
} location;

//Struct for hits, sortHits and filterHits programs
typedef struct {
	//Diagonal where the hit is located
	//This value is calculated as:
	//posX - posY
	int64_t diag;
	//Ocurrence position in sequence X
	uint64_t posX;
	//Ocurrence position in sequence Y
	uint64_t posY;
	//For multiple sequence files this var
	//reflects in what sequence of X file
	//occurs the word
	uint64_t seqX;
	//For multiple sequence files this var
	//reflects in what sequence of Y file
	//occurs the word
	uint64_t seqY;
} hit;

//Struct for FragHits, af2png and leeFrag programs
struct FragFile {
	//Diagonal where the frag is located
	//This value is calculated as:
	//posX - posY
	int64_t diag;
	//Start position in sequence X
	uint64_t xStart;
	//Start position in Sequence Y
	uint64_t yStart;
	//End position in Sequence X
	uint64_t xEnd;
	//End position in Sequence Y
	uint64_t yEnd;
	//Fragment Length
	//For ungaped aligment is:
	//xEnd-xStart+1
	uint64_t length;
	//Number of identities in the
	//fragment
	uint64_t ident;
	//Score of the fragment. This
	//depends on the score matrix
	//used
	uint64_t score;
	//Percentage of similarity. This
	//is calculated as score/scoreMax
	//Where score max is the maximum
	//score possible
	float similarity;
	//sequence number in the 'X' file
	uint64_t seqX;
	//sequence number in the 'Y' file
	uint64_t seqY;
	//synteny block id
	int64_t block;
	//'f' for the forward strain and 'r' for the reverse
	char strand;
};

//Struct for leeSeqDB function
struct Sequence {
	char ident[MAXLID + 1];
	char *datos;
};

//Structs for the binaryTree program to calculate the dictionary
typedef struct l {
	//Word ocurrence
	location loc;
	//Pointer to the next node in the list
	struct l *next;
} list_node;

typedef struct n {
	//Word without compression to speed up the process
	char key_value[33];
	//Pointer to the left node in the tree
	struct n *left;
	//Pointer to the right node in the tree
	struct n *right;
	//Number of ocurrences of the given word
	int64_t ocurrences;
	//Pointer to the word list of ocurrences
	list_node *positions;
	//Pointer to the last node in the list
	//to speed up the insertion
	list_node *last;
} node;

// Coordinates
typedef struct {
	//Starting coordinate
	long start;
	//End coordinate
	long end;
} instruction;

#endif

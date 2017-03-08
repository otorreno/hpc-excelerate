/* reduce 2 fragments files (forward and reverse) produced in parallel

 Syntax: reduceFrags PrefixFRAGSfiles nChunks fileOUT

 oscart@uma.es
 --------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "structs.h"
#include "commonFunctions.h"
#include "comparisonFunctions.h"

int main(int ac, char** av) {
	uint64_t n1, n2;
	FILE *fIn, *fOut;

	struct FragFile frag;

	if (ac != 4)
		terror("Syntax: combineFrags forwardStrandFrags reverseStrandFrags fileOUT");

	if ((fIn = fopen(av[1], "rb")) == NULL)
		terror("Input forwardStrandFrags open error");

	if ((fOut = fopen(av[3], "wb")) == NULL)
		terror("OUTput file open error");

	// sequences lengths
	readSequenceLength(&n1, fIn);
	readSequenceLength(&n2, fIn);

	writeSequenceLength(&n1, fOut);
	writeSequenceLength(&n2, fOut);

	//First file...
	readFragment(&frag, fIn);

	while (!feof(fIn)) {
		writeFragment(&frag, fOut);
		readFragment(&frag, fIn);
	}

	fclose(fIn);

	//Second file...
	if ((fIn = fopen(av[2], "rb")) == NULL)
		terror("Input reverseStrandFrags open error");

	readSequenceLength(&n1, fIn);
	readSequenceLength(&n2, fIn);

	readFragment(&frag, fIn);

	while (!feof(fIn)) {
		writeFragment(&frag, fOut);
		readFragment(&frag, fIn);
	}

	fclose(fIn);
	fclose(fOut);

	return 0;
}
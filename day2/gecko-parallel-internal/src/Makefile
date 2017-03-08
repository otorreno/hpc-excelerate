CC=gcc
MPICC=mpicc
CFLAGS=-O3 -D_FILE_OFFSET_BITS=64 -Wall -Wextra
BIN=../bin

all: reverseComplement dictionary dictStat hits hitsStat sortHits filterHits splitHits fragHits combineFrags fragStat generateCSV

reverseComplement: reverseComplement.c
	$(CC) $(CFLAGS) reverseComplement.c commonFunctions.c -o $(BIN)/reverseComplement

dictionary: dictionary.c
	$(MPICC) $(CFLAGS) -fopenmp dictionary.c commonFunctions.c dictionaryFunctions.c -lm -o $(BIN)/dictionary

dictStat: dictStat.c
	$(CC) $(CFLAGS) dictStat.c commonFunctions.c dictionaryFunctions.c comparisonFunctions.c -o $(BIN)/dictStat

hits: hits.c
	$(MPICC) $(CFLAGS) -fopenmp hits.c comparisonFunctions.c dictionaryFunctions.c commonFunctions.c -o $(BIN)/hits

hitsStat: hitsStat.c
	$(CC) $(CFLAGS) hitsStat.c commonFunctions.c -o $(BIN)/hitsStat

sortHits: sortHits.c
	$(MPICC) $(CFLAGS) sortHits.c -o $(BIN)/sortHits

filterHits: filterHits.c
	$(CC) $(CFLAGS) filterHits.c comparisonFunctions.c commonFunctions.c -o $(BIN)/filterHits

splitHits: splitHits.c
	$(CC) $(CFLAGS) -DPBAL splitHits.c commonFunctions.c -o $(BIN)/splitHits

fragHits: fragHits.c
	$(MPICC) $(CFLAGS) fragHits.c comparisonFunctions.c commonFunctions.c -o $(BIN)/fragHits

combineFrags: combineFrags.c
	$(CC) $(CFLAGS) combineFrags.c comparisonFunctions.c commonFunctions.c -o $(BIN)/combineFrags

fragStat: fragStat.c
	$(CC) $(CFLAGS) fragStat.c comparisonFunctions.c commonFunctions.c -o $(BIN)/fragStat

generateCSV: generateCSV.c
	$(CC) $(CFLAGS) generateCSV.c comparisonFunctions.c commonFunctions.c -o $(BIN)/generateCSV

clean:
	rm -rf $(BIN)/reverseComplement $(BIN)/dictionary $(BIN)/dictStat $(BIN)/hits $(BIN)/hitsStat $(BIN)/sortHits $(BIN)/filterHits $(BIN)/splitHits $(BIN)/fragHits $(BIN)/combineFrags $(BIN)/fragStat $(BIN)/generateCSV

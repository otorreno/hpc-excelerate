CC=gcc
MPICC=mpicc
CFLAGS=-O3 -D_FILE_OFFSET_BITS=64 -Wall
BIN=../bin

all: reverseComplement words sortWords w2hd wordsStat hits sortHits filterHits FragHits combineFrags hitsStat hdStat fragStat getInfoCSB scheduler

hits: hits.c
	$(CC) $(CFLAGS) hits.c comparisonFunctions.c dictionaryFunctions.c commonFunctions.c -o $(BIN)/hits

sortHits: sortHits.c
	$(CC) $(CFLAGS) -DBaseType=hit sortHits.c comparisonFunctions.c commonFunctions.c quicksort.c -lpthread -o $(BIN)/sortHits

filterHits: filterHits.c
	$(CC) $(CFLAGS) filterHits.c comparisonFunctions.c commonFunctions.c -o $(BIN)/filterHits

FragHits: FragHits.c
	$(CC) $(CFLAGS) FragHits.c comparisonFunctions.c commonFunctions.c -o $(BIN)/FragHits
	
combineFrags: combineFrags.c
	$(CC) $(CFLAGS) combineFrags.c comparisonFunctions.c commonFunctions.c -o $(BIN)/combineFrags
	
hitsStat: hitsStat.c
	$(CC) $(CFLAGS) hitsStat.c commonFunctions.c -o $(BIN)/hitsStat
	
hdStat: hdStat.c
	$(CC) $(CFLAGS) hdStat.c commonFunctions.c dictionaryFunctions.c -o $(BIN)/hdStat
	
fragStat: fragStat.c
	$(CC) $(CFLAGS) fragStat.c comparisonFunctions.c commonFunctions.c -o $(BIN)/fragStat
	
reverseComplement: reverseComplement.c
	$(CC) $(CFLAGS) reverseComplement.c commonFunctions.c -o $(BIN)/reverseComplement

words: words.c
	$(CC) $(CFLAGS) words.c dictionaryFunctions.c commonFunctions.c -o $(BIN)/words

sortWords: sortWords.c
	$(CC) $(CFLAGS) -DBaseType=wentry sortWords.c dictionaryFunctions.c commonFunctions.c quicksort.c -lpthread -o $(BIN)/sortWords

w2hd: w2hd.c
	$(CC) $(CFLAGS) w2hd.c dictionaryFunctions.c commonFunctions.c -o $(BIN)/w2hd

wordsStat: wordsStat.c
	$(CC) $(CFLAGS) wordsStat.c dictionaryFunctions.c commonFunctions.c -o $(BIN)/wordsStat

getInfoCSB:
	$(CC) $(CFLAGS) getInfoCSB.c fragmentv2.c commonFunctions.c comparisonFunctions.c -lm -o $(BIN)/getInfo

scheduler: scheduler.c
	$(MPICC) $(CFLAGS) scheduler.c schedulerFunctions.c commonFunctions.c -o $(BIN)/scheduler

clean:
	rm -rf $(BIN)/getInfo
	rm -rf $(BIN)/reverseComplement $(BIN)/words $(BIN)/sortWords $(BIN)/w2hd $(BIN)/wordsStat
	rm -rf $(BIN)/hits $(BIN)/sortHits $(BIN)/filterHits $(BIN)/FragHits $(BIN)/combineFrags $(BIN)/hitsStat $(BIN)/fragStat $(BIN)/hdStat $(BIN)/scheduler

/**
 * Base code obtained from the University of Cambridge website:
 * http://www.hpc.cam.ac.uk/using-clusters/compiling-and-development/parallel-programming-mpi-example
 */

#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define WORKTAG     1
#define DIETAG     2
#define MAXLINE 1024
#define MAXARGS 200

char *nextTask(FILE *workload_file);
int getCommandLine(char *completeLine, char **arguments);
void master(char *workload_file);
void slave();

int main(int argc, char **argv) {
	int         myrank;
	char *workload_file;
	
	if (argc < 2){
	    fprintf(stderr, "ERROR. Use: ./mpi_master_slave workload_file\n");
	    exit(-1);
	}
	
	workload_file = argv[1];
	
	MPI_Init(&argc, &argv);   /* initialize MPI */
	MPI_Comm_rank(
	MPI_COMM_WORLD,   /* always use this */
	&myrank);      /* process rank, 0 thru N-1 */
	
	if (myrank == 0) {
		master(workload_file);
	} else {
		slave();
	}
	
	MPI_Finalize();       /* cleanup MPI */

	return 0;
}

void master(char *workload_file) {
	int	ntasks, rank, result, execution_status, submitted;
	char *work;	
	MPI_Status     status;
	FILE *wF;
	
	MPI_Comm_size(
	MPI_COMM_WORLD,   /* always use this */
	&ntasks);          /* #processes in application */
	
	if ((wF = fopen(workload_file, "rt")) == NULL){
	    fprintf(stderr, "Error opening workload file: %s\n", workload_file);
	}
	
	work = nextTask(wF);
	rank = 1;
	submitted = 0;
	while (work != NULL) {
	    //Seed the slaves.
	    MPI_Send(work,         /* message buffer */
	    strlen(work)+1,              /* one data item */
	    MPI_CHAR,        /* data item is an integer */
	    rank,           /* destination process rank */
	    WORKTAG,        /* user chosen message tag */
	    MPI_COMM_WORLD);/* always use this */
	    
	    free(work);
		work = nextTask(wF); /* get_next_work_request */
		rank++;
		submitted++;
		
		if (submitted >= (ntasks-1)){
		    MPI_Recv(&execution_status,       /* message buffer */
		    1,              /* one data item */
		    MPI_INT,     /* of type double real */
		    MPI_ANY_SOURCE, /* receive from any sender */
		    MPI_ANY_TAG,    /* any type of message */
		    MPI_COMM_WORLD, /* always use this */
		    &status);       /* received message info */
		    
		    rank = status.MPI_SOURCE;
		}
	}
	
	//Wait until all the work is finished
	while (rank > 1) {
	    MPI_Recv(&execution_status,       /* message buffer */
	    1,              /* one data item */
	    MPI_INT,     /* of type double real */
	    MPI_ANY_SOURCE, /* receive from any sender */
	    MPI_ANY_TAG,    /* any type of message */
	    MPI_COMM_WORLD, /* always use this */
	    &status);       /* received message info */
	    
	    rank--;
	}
	
	//Tell all the slaves to exit.
	for (rank = 1; rank < ntasks; ++rank) {
		MPI_Send(0, 0, MPI_INT, rank, DIETAG, MPI_COMM_WORLD);
	}
	
	fclose(wF);
}

void slave() {
	int                 count, ppid, child_status, narg, myrank;
	char work[MAXLINE], *args[MAXARGS];
	MPI_Status          status;
	
	memset(work, 0, MAXLINE * sizeof(char));
	
    MPI_Comm_rank(
	MPI_COMM_WORLD,   /* always use this */
	&myrank);      /* process rank, 0 thru N-1 */
	for (;;) {
		MPI_Recv(&work, MAXLINE, MPI_CHAR, 0, MPI_ANY_TAG,
		MPI_COMM_WORLD, &status);
		
		//Check the tag of the received message.
		if (status.MPI_TAG == DIETAG) {
			return;
		}
		
		ppid = fork();
        if(ppid == 0){
            narg = getCommandLine(work, args);
            execvp(args[0], args);
	    fprintf(stdout, "%s\n", args[0]);
            fprintf(stderr, "Impossible exec");
            exit(-1);
        }else{
            ppid = wait(&child_status);
        }
        memset(work, 0, MAXLINE * sizeof(char));
		MPI_Send(&child_status, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	}
}

char *nextTask(FILE *workload_file) {
    char *res = (char *)malloc(MAXLINE * sizeof(char));
    fgets(res, MAXLINE, workload_file);
    if (!feof(workload_file)) {
        if(res[strlen(res)-1] == '\n')
            res[strlen(res)-1]='\0';
        return res;
    }
    
    return NULL;
}

int getCommandLine(char *completeLine, char **arguments) {
  char *token;
  int narg=0;

  if((token=strtok(completeLine," ")) != NULL) { // busca una palabra separada por espacios

      arguments[narg]=(char *)strdup(token);// copia la palabra
      narg++; // incrementa el contador de palabras

      while ((token=strtok(NULL," ")) != NULL) {// busca mas palabras en misma linea
	  arguments[narg]=(char *)strdup(token);
	  narg++;
      } 
  }

  arguments[narg] = NULL;

  return narg;
}

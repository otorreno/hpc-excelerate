/* scheduler.c 
      - scheduler script tpw

      - where: script is the workload file where each line is a command for executing in a process

               tpw is 'task per worker' determinate the number of tasks it will execute in a worker.

      This program distributes the tasks define in script to the available workers.

*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>
#include <mpi.h>  // MPI lib.
#include <fcntl.h>
#include <sys/wait.h>

#include "commonFunctions.h"
#include "schedulerFunctions.h"

#define WORKTAG 1 // task continue
#define ENDTAG 2  // end task

//Read a file and send each row of the file to a worker
void master(char* workloadFile);

//execute a command line from the master. Create a process and execute a secuencial program
void worker(int tpw);

int main(int ac, char **argv) {

   int myID;

   if(ac != 3){
     terror("Use: ./scheduler SCRIPT_FILENAME TASK_PER_NODE");
   }

   MPI_Init(&ac, &argv);
   MPI_Comm_rank(MPI_COMM_WORLD, &myID);

   if (myID == 0){
      master(argv[1]); //Id 0 MASTER
   }
   else{
      worker(atoi(argv[2]));
   }
   
   MPI_Finalize(); //Shut down mpi

   return 0;
}

void master(char* workloadFile) {

   char **table;
   char *message;
   int ntasks, i, estado, length;
   int totalCommands, countCommand; //number of command line
   MPI_Status status;

   countCommand = 0;

   //Read workload
   table = readWorkloadFile(workloadFile, &totalCommands);

   // Number of workers
   MPI_Comm_size(MPI_COMM_WORLD, &ntasks);
 
   //Distribute the workload stored in table
   for(i = 1; i < ntasks; i++){
      //prepare the msg with the format: 'size;msg1;msg2;...'
      message = prepareMessage(&countCommand, totalCommands, table, &length);

      if (length > 0){
         //send the message
         MPI_Send(message,
                  length,
                  MPI_CHAR,
                  i,
                  WORKTAG,
                  MPI_COMM_WORLD); 
       }
       free(message);     
   }

   while(countCommand < totalCommands){
      //MASTER waiting SLAVE to finish to send them more work
      MPI_Recv(&estado,
               1,
               MPI_INT,
               MPI_ANY_SOURCE,
               MPI_ANY_TAG,
               MPI_COMM_WORLD,
               &status);

      //prepare a new message
      message = prepareMessage(&countCommand, totalCommands, table, &length); 

      //send it
      MPI_Send(message,
               length,
               MPI_CHAR,
               status.MPI_SOURCE,
               WORKTAG,
               MPI_COMM_WORLD);
      free(message); 
   }

   for(i=1; i<ntasks ;i++){
      //MASTER waiting SLAVE to finish to send them ENDTAG
      MPI_Recv(&estado,
               1,
               MPI_INT,
               MPI_ANY_SOURCE,
               MPI_ANY_TAG,
               MPI_COMM_WORLD,
               &status);
   }

   //MASTER sending ENDTAG to all workers
   for (i = 1; i < ntasks; i++){
      MPI_Send("0",
               1,
               MPI_CHAR,
               i,
               ENDTAG,
               MPI_COMM_WORLD);
   }

   //MASTER confirming SLAVES finalization
   for (i = 1; i < ntasks; i++){
       MPI_Recv(&estado, 
                1, 
                MPI_INT, 
                MPI_ANY_SOURCE,
                MPI_ANY_TAG,
                MPI_COMM_WORLD, 
                &status);
   }
}

void worker(int tpw){
   int myID, ppid, estado, narg, i, nproc;
   char **av;
   char message[MAXLIN];
   MPI_Status status;

   nproc = 0;

   MPI_Comm_rank(MPI_COMM_WORLD, &myID);

   //Receive thw workload from the Master
   MPI_Recv(&message,
            MAXLIN,
            MPI_CHAR,
            0,
            MPI_ANY_TAG, 
            MPI_COMM_WORLD,
            &status);

   //Initialzing the arguments array
   av = (char **)malloc(MAXARGS * sizeof(char *));
   for(i=0;i<MAXARGS;i++){
      av[i]=NULL;
   }

   while(status.MPI_TAG == WORKTAG){
      int j;
     
      //Splitting the message into its arguments
      narg = getCommandLine(message, av);

      ppid = fork();
      if(ppid == -1){ 
        fprintf(stdout, "(%d) Error in fork\n", myID);
        perror("Fork error");
      }
      if(ppid == 0){
          if(!narg){
              terror("without arguments!");
          }
          execvp(av[0], av);
          perror("Impossible exec");
      }else{
          nproc++;
          //Free arguments structure
          for(j=0;j<narg;j++){
            free(av[j]);
            av[j]=NULL;
          }
      }

      //Wait for a child if we reach the maximum.
      if(nproc == tpw){
         ppid = wait(&estado);
         nproc--;
      }

      //SLAVE sending finalization message to master
      MPI_Send(&ppid,
               1,
               MPI_INT,
               0,
               ENDTAG,
               MPI_COMM_WORLD);

      //SLAVE waiting a new message
      MPI_Recv(&message,
               MAXLIN,
               MPI_CHAR,
               0,
               MPI_ANY_TAG,
               MPI_COMM_WORLD,
               &status);
  }

  //Wait for the childs.
  while(nproc > 0){
     ppid = wait(&estado);
     nproc--;
  }

   //SLAVE sending exit message
   MPI_Send(&estado,
            1,            // data size
            MPI_INT,      // data type
            0,            // id 0=master
            ENDTAG,       // tag
            MPI_COMM_WORLD);// constant to communicate mpi
}


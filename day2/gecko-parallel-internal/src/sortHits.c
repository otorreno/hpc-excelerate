// sortHits.c - Parallel sorting algorithm of the hits structure based on quicksort
// compile: mpicc -O3 -Wall -Wextra qsort.c -o qsort
// run:     mpirun -np num_cores qsort in_file out_file

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <mpi.h>
#include <inttypes.h>
#include "structs.h"

#define SWAP(a,b,t) t=a; a=b; b=t;
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

int GT(hit a1, hit a2) {
  if(a1.diag > a2.diag)
    return 1;
  else if (a1.diag < a2.diag)
    return 0;
  if (a1.posX > a2.posX)
    return 1;
  return 0;
}

// (quick) sort slice of array v; slice starts at s and is of length n
void quicksort(hit * v, int64_t s, int64_t n)
{
  hit x, t;
  int64_t p, i;
  
  // base case
  if (n <= 1)
    return;
  // pick pivot and swap with first element
  x = v[s + n/2];
  SWAP(v[s], v[s + n/2], t);
  // partition slice starting at s+1
  p = s;
  for (i = s+1; i < s+n; i++){
    if (!GT(v[i], x)) {
      p++;
      SWAP(v[i], v[p], t);
    }
  }
  // swap pivot into place
  SWAP(v[s], v[p], t);
  // recurse into partition
  quicksort(v, s, p-s);
  quicksort(v, p+1, s+n-p-1);
}


// merge two sorted arrays v1, v2 of lengths n1, n2, respectively
hit * merge(hit * v1, int64_t n1, hit * v2, int64_t n2)
{
  hit * result = (hit *)malloc((n1 + n2) * sizeof(hit));
  if(result == NULL){
    fprintf(stdout, "** MERGE Error. Not enough memory\n");
    exit(-1);
  }
    
  int64_t i = 0;
  int64_t j = 0;
  int64_t k = 0;

  while(i<n1 && j<n2) {
    if(!GT(v1[i],v2[j])) {
      memcpy(&result[k], &v1[i], sizeof(hit));
      i++; k++;
    } else {
      memcpy(&result[k], &v2[j], sizeof(hit));
      j++; k++;
    }
  }
  if(i==n1){
    memcpy(&result[k], &v2[j], (n2-j+1)*sizeof(hit));
  } else {
    memcpy(&result[k], &v1[i], (n1-i+1)*sizeof(hit));
  }

  return result;
}


int main(int argc, char ** argv)
{
  int64_t n;
  hit * data = NULL;
  int64_t c, s;
  hit * chunk;
  int64_t o;
  hit * other;
  int p, id, step;
  MPI_Status status;
  FILE * file = NULL;
  int64_t i;
  int64_t r;

  int64_t tmp, aux;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &p);

  //Define the hit datatype
  const int nitems=5;
  int          blocklengths[5] = {1,1,1,1,1};
  MPI_Datatype types[5] = {MPI_INT64_T, MPI_UINT64_T, MPI_UINT64_T, MPI_UINT64_T, MPI_UINT64_T};
  MPI_Datatype mpi_hit_type;
  MPI_Aint     offsets[5];

  offsets[0] = offsetof(hit, diag);
  offsets[1] = offsetof(hit, posX);
  offsets[2] = offsetof(hit, posY);
  offsets[3] = offsetof(hit, seqX);
  offsets[4] = offsetof(hit, seqY);

  MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_hit_type);
  MPI_Type_commit(&mpi_hit_type);

  MPI_Comm_rank(MPI_COMM_WORLD, &id);

  //Only the master process
  if (id == 0) {
    // read size of data
    file = fopen(argv[1], "rb");
    fseek(file, 0, SEEK_END);
    n = ftell(file)/sizeof(hit);
    fseek(file, 0, SEEK_SET);
    // compute chunk size
    c = n/(int64_t)p; if (n%((int64_t)p)) c++;
    // read data from file
    data = (hit *)malloc(((int64_t)p)*c * sizeof(hit));
    if(data == NULL){
      fprintf(stdout, "** MASTER Error. Not enough memory\n");
      exit(-1);
    }
    if((r = fread(data, sizeof(hit), n, file))!= n){
      fprintf(stdout, "MASTER - Error reading hits file. Read: %" PRId64 "\n", r);
      exit(-1);
    }

    fclose(file);
    // pad data with 0 -- doesn't matter
    for (i = n; i < ((int64_t)p)*c; i++)
      memset(&data[i], 0, sizeof(hit));
  }

  // All the slaves waiting the master
  MPI_Barrier(MPI_COMM_WORLD);

  // broadcast the file size.
  MPI_Bcast(&n, 1, MPI_INT64_T, 0, MPI_COMM_WORLD);

  // compute chunk size
  c = n/(int64_t)p; if (n%(int64_t)p) c++;

  // scatter data
  chunk = (hit *)malloc(c * sizeof(hit));
  if(chunk == NULL){
    fprintf(stdout, "** Scatter Error. Not enough memory\n");
    exit(-1);
  }
  
  //The master distributes data chunks
  MPI_Scatter(data, c, mpi_hit_type, chunk, c, mpi_hit_type, 0, MPI_COMM_WORLD);

  //Only the master frees the memory the rest did not allocate it
  if (id == 0) {
    free(data);
    data = NULL;
  }

  // compute size of own chunk and sort it
  if (n >= c * (((int64_t)id)+1))
    s = c;
  else
    s = n - c * ((int64_t)id);

  fprintf(stdout, "Process: %d. Sorting my chunk of size: %" PRId64 "\n", id, s);
  quicksort(chunk, 0, s);
  fprintf(stdout, "Process: %d. Chunk sorted\n", id);

  // up to log_2 p merge steps
  for (step = 1; step < p; step = 2*step) {
    if ((id % (2*step))==0) {
    // id is multiple of 2*step: merge in chunk from id+step (if it exists)
        if ((id+step) < p) {
          // compute size of chunk to be received
          if (n >= c * (int64_t)(id+2*step))
            o = c * (int64_t)step;
          else
            o = n - c * (int64_t)(id+step);

          //receive other chunk
          other = (hit *)malloc(o * sizeof(hit));
          if(other == NULL){
            fprintf(stdout, "** Recv chunk Error. Not enough memory\n");
            exit(-1);
          }
          //if data to be received is greater than the maximum wait for multiple chunks
          if(o > INT_MAX){
            tmp = o;
            aux = 0;
            int toBeReceived = MIN(INT_MAX,o);
            while(o>0){
                toBeReceived = MIN(INT_MAX,o);
                MPI_Recv(other+aux, toBeReceived, mpi_hit_type, id+step, 0, MPI_COMM_WORLD, &status);
    		o -= toBeReceived;
          	aux += toBeReceived;
            }
            o = tmp;
          } else {
            //data fits in one Recv call
            MPI_Recv(other, o, mpi_hit_type, id+step, 0, MPI_COMM_WORLD, &status);
          }
          //merge and free memory
          data = merge(chunk, s, other, o);

          //deallocate old chunks
          free(chunk);
          free(other);

          //update current chunk
          chunk = data;
          //update current chunk size
          s = s + o;
        }
    } else {
      //id is no multiple of 2*step: send chunk to id-step and exit loop
      //if chunk size cannot be send in one MPI_Send call, split it in several Sends 
      if(s > INT_MAX){
	tmp = s;
        aux = 0;
        int toBeSent = 0;
        while(s>0){
          toBeSent = MIN(INT_MAX,s);
          MPI_Send(chunk+aux, toBeSent, mpi_hit_type, id-step, 0, MPI_COMM_WORLD);
	  s -= toBeSent;
          aux+=toBeSent;
        }
        s = tmp;
      } else {
        //data fits in one send
        MPI_Send(chunk, s, mpi_hit_type, id-step, 0, MPI_COMM_WORLD);
      }
      //finish execution
      break;
    }
  }

  // write sorted data to out file
  if (id == 0) {
    file = fopen(argv[2], "wb");
    if(fwrite(chunk, sizeof(hit), s, file)!=(unsigned long)s){
      fprintf(stdout, "MASTER - Error writting hits file.\n");
      exit(-1);
    }
    fclose(file);
  }

  MPI_Finalize();
  return 0;
}

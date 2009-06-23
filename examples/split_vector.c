#include <stdio.h>
#include "athread.h"

#define GANG 10 
#define VEC_SIZE (GANG*10)

int global_result=0;

void *split_func(void *in, int split_factor, size_t total_size, int rank) {
  int *spt_data;
  int i;
  int *vector = (int *) in;
  int index;

  spt_data = malloc(total_size/split_factor * sizeof(int));
  for (i=0; i < split_factor; i++) {
    index = (split_factor * rank) + i;
    spt_data[i] = vector[index];
  }

  return (void *) spt_data;
}

void *merge_func(void *ret_th, int split_factor, int rank, void *output) {
  int *result = malloc(sizeof(int));

  *result = *(int *) ret_th + *(int *) output;
  return (void *) *result;
}

void *my_thread(void *data) {
  int *vector = (int *) data;
  int i;
  int *result = malloc(sizeof(int));

  *result = 0;
  for (i=0; i<GANG; i++) {
    *result += vector[i];
  }

  return (void *) result;
}

int main(int argc, char *argv[]) 
{
  int *vetor; 
  void *result;
  int i;
  athread_t sth;
  athread_attr_t attr;

  vetor = malloc(sizeof(int) * VEC_SIZE);
  result = malloc(sizeof(int));

  for (i=0; i<VEC_SIZE; i++) {
    vetor[i] = i+1;
  }

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &athread_remote_rank);  
  MPI_Comm_size(MPI_COMM_WORLD, &athread_remote_size);
	
	aRemoteInit(&argc, &argv);
	aInit(&argc, &argv);
  
	athread_attr_init(&attr);
  athread_attr_set_inputsize(&attr, VEC_SIZE * sizeof(int));
  athread_attr_set_returnsize(&attr, sizeof(int));
  athread_attr_set_splitfactor(&attr, GANG);
  athread_attr_set_splitfunction(&attr, split_func);
  athread_attr_set_mergefunction(&attr, merge_func);
  athread_create(&sth, &attr, my_thread, (void *) vetor);

  athread_join(sth, result);
  printf("Resultado ==> %d\n", *(int*) result);


  aTerminate();
  return 0;
}

#include <stdio.h>
#include "constants.h"
#include "threadControl.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>


int main(int argc, char* argv[]) {
  if (argc != 3) {
     printf("Wrong number of arguments\n");
     return -1;
  }

  int num_threads = atoi(argv[1]);

  if (num_threads > MAX_BANK_OFFICES) {
    printf("Too many offices\n");
    return -1;
  }

  char * password = argv[2];
  if (strlen(password) > MAX_PASSWORD_LEN || strlen(password) < MIN_PASSWORD_LEN) {
    printf("Wrong password size\n");
    return -2;
  }
  else {
    password +=1;
    password -=1;
  }



//CRIACAO DE THREADS, PRECISAMOS DE UMA FUNCAO: func FOI O QUE PUS PROVISORIAMENTE

  pthread_t t_ids[num_threads];
      int i;
      for(i = 0; i < num_threads; i++) {
          if(pthread_create(&(t_ids[i]), NULL, threadInit, NULL) != 0) {
              fprintf(stderr, "Error creating thread %d\n", i);
          }
      }



//FIFO PARA COMUNICAÇÃO, AINDA ERRADO POR CAUSA DO NOME

  if(mkfifo(SERVER_FIFO_PATH, 0660) != 0) {
        fprintf(stderr, "Error creating fifo\n");
        return -3;
    }

//CHAMAR FUNCAO DE CRIAÇÃO DE CONTA DO ADMIN


  return 0;
}

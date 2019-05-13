#include <stdio.h>
#include "constants.h"
#include "threadControl.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "queue.c"
#include "types.h"
#include "accountarray.h"



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

//  unlink(SERVER_FIFO_PATH);
//CRIACAO DE THREADS, PRECISAMOS DE UMA FUNCAO: func FOI O QUE PUS PROVISORIAMENTE

  pthread_t t_ids[num_threads];
      int i;
      for(i = 0; i < num_threads; i++) {
          if(pthread_create(&(t_ids[i]), NULL, threadInit, NULL) != 0) {
              fprintf(stderr, "Error creating thread %d\n", i);
          }
          else printf("Created thread!");
      }

//FIFO PARA COMUNICAÇÃO, AINDA ERRADO POR CAUSA DO NOME
  int fd;
  if(mkfifo(SERVER_FIFO_PATH, 0660) != 0) {
        fprintf(stderr, "Error creating fifo\n");
        return -3;
    }
  else printf ("Cool!");

  fd = open(SERVER_FIFO_PATH, O_RDONLY | O_NONBLOCK);



//  struct Queue * queue = createQueue(100);



//CHAMAR FUNCAO DE CRIAÇÃO DE CONTA DO ADMIN
  unlink(SERVER_FIFO_PATH);
  return 0;
}
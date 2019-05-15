#include <stdio.h>
#include "constants.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "queue.c"
#include "types.h"
#include "accountarray.h"

static pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static struct Queue * queue;

void * threadInit(void * args);



int main(int argc, char* argv[]) {

    time_t t;

    srand((unsigned) time(&t));


    char*  a= malloc(HASH_LEN);

    
    if (argc != 3) {
         printf("Wrong number of arguments\n");
         return -1;
     }

     int num_threads = atoi(argv[1]);

     char * password = argv[2];
     if (strlen(password) > MAX_PASSWORD_LEN || strlen(password) < MIN_PASSWORD_LEN) {
         printf("Wrong password size\n");
         return -2;
     }

     queue = createQueue(100);

    //CHAMAR FUNCAO DE CRIAÇÃO DE CONTA DO ADMIN


    //CRIACAO DE THREADS, PRECISAMOS DE UMA FUNCAO: func FOI O QUE PUS PROVISORIAMENTE

     pthread_t t_ids[num_threads];
     int i;
     for(i = 0; i < num_threads; i++) {
         if(pthread_create(&(t_ids[i]), NULL, threadInit, NULL ) != 0) {
             fprintf(stderr, "Error creating thread %d\n", i);
         }
         else printf("Created thread!");
     }

     printf("Ended thread creation!\n");


    //FIFO QUE RECEBE OS REQUESTS
     if(mkfifo(SERVER_FIFO_PATH, 0660) != 0) {
         fprintf(stderr, "Error creating fifo\n");
         return -3;
     }
     else printf ("Cool!\n");

    //ABRE E LÊ REQUESTS DO FIFO. SE NAO HOUVER ESPERA QUE HAJA
     int fd = open(SERVER_FIFO_PATH, O_RDONLY | O_NONBLOCK);
     if(fd == -1) {
         printf("Could not open fifo\n");
         return -4;
     }
     tlv_request_t tlv_req;
     while(true) { // mudar este true para condição de encerramento do server

         if (read(fd,&tlv_req,sizeof(tlv_request_t)) <= 0)
             continue;

         enqueue(queue,tlv_req);
         printf("len %d\n",tlv_req.type);
     }
     close(fd);

     unlink(SERVER_FIFO_PATH);
     printf("UNLINKED, ABOUT TO RETURN\n");
    return 0;
}

void * threadInit(void * args) {

    /* pthread_mutex_lock(&queue_mutex);
     printf("TRYING\n");
     //RETIRAR DA QUEUE;

     // codigo para abrir o fifo de resposta
     /*char response_fifo[USER_FIFO_PATH_LEN];

         strcpy(response_fifo, USER_FIFO_PATH_PREFIX);
         strcat(response_fifo,arr);

         fd = open(response_fifo, O_RDONLY );

         printf("Opened the response fifo!");
     */


    //VER QUAL A AÇÃO A CHAMAR
    // pthread_mutex_unlock(&queue_mutex);

}

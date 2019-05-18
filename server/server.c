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
#include "sope.h"


static pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static struct Queue * queue;
static bool workingThreads[MAX_BANK_OFFICES] = {0};
static int threadNumber;
static bool serverUp = true;

int slog_fd;

void * threadInit(void * args);
bool existingActiveThread(int threadNo);
struct tlv_reply requestParser(struct tlv_request request, int threadNo);
struct tlv_request requestFromQueue();
struct tlv_reply closeServer(struct tlv_request request, int threadNo);
bool checkPassword(struct tlv_request req);

int main(int argc, char* argv[]) {

    time_t t;
    srand((unsigned) time(&t));

    slog_fd =STDOUT_FILENO;// open(SERVER_LOGFILE, O_RDWR |O_CREAT |O_TRUNC|O_APPEND, 0666);

    if (argc != 3) {
        printf("Wrong number of arguments\n");
        return -1;
    }

    int num_threads = atoi(argv[1]);
    threadNumber = num_threads;
    struct account_mut * acc = malloc(MAX_BANK_ACCOUNTS * sizeof(struct account_mut));

    setAccountsArray(acc);

    char * password = argv[2];
    if (strlen(password) > MAX_PASSWORD_LEN || strlen(password) < MIN_PASSWORD_LEN) {
        printf("Wrong password size\n");
        return -2;
    }

    queue = createQueue(100);

    pthread_t t_ids[num_threads];
    int i;
    int *in = malloc(sizeof(int));
    for(i = 0; i < num_threads; i++) {

        *in = i;
        if(pthread_create(&(t_ids[i]), NULL, threadInit, in) != 0) {
            fprintf(stderr, "Error creating thread %d\n", i);
        }
        else logBankOfficeOpen(slog_fd, i, t_ids[i]);
    }

//CHAMAR FUNCAO DE CRIAÇÃO DE CONTA DO ADMIN
    creatAdmin(argv[2]);

//FIFO QUE RECEBE OS REQUESTS
    if(mkfifo(SERVER_FIFO_PATH, 0660) != 0) {
        fprintf(stderr, "Error creating fifo\n");
        return -3;
    }

//ABRE E LÊ REQUESTS DO FIFO. SE NAO HOUVER ESPERA QUE HAJA
    int fd = open(SERVER_FIFO_PATH, O_RDONLY | O_NONBLOCK);
    if(fd == -1) {
        printf("Could not open fifo\n");
        return -4;
    }
    tlv_request_t tlv_req;
    while(serverUp) {

        if (read(fd,&tlv_req,sizeof(tlv_request_t)) <= 0)
            continue;

        enqueue(queue,tlv_req);
        logRequest(slog_fd,tlv_req.value.header.pid,&tlv_req);
    }
    printf("yoyo\n");

    close(fd);

    printf("yoyo\n");

    for(i =0; i < num_threads; i++) {
      printf("thread: %d\n", i);
        /**in = i;
        if(pthread_join(t_ids[i],NULL) != 0) {
            fprintf(stderr, "Error joining thread %d\n", i);
        }
        else logBankOfficeClose(slog_fd, i, t_ids[i]);*/
        logBankOfficeClose(slog_fd, i, t_ids[i]);
        pthread_cancel(t_ids[i]);
    }

    free(in);
    free(acc);
    printf("UNLINKED, ABOUT TO RETURN\n");
    return 0;
}

void requestHandler( tlv_request_t request, int threadNo) {

    //ELABORA A RESPOSTA DA QUEUE
    struct tlv_reply answer;
    answer = requestParser(request, threadNo);

    //CRIA O FIFO DE RESPOSTA
    char response_fifo[USER_FIFO_PATH_LEN];
    sprintf(response_fifo, "%s%d", USER_FIFO_PATH_PREFIX,request.value.header.pid);

    //ABRE FIFO, ESCREVE E FECHA

    int fd = open(response_fifo, O_WRONLY);
    int attempts = 0;

    while(fd < 0 && attempts < 10) {
        fd = open(response_fifo, O_WRONLY);
        attempts++;
    }

    write(fd,&answer, sizeof(answer));
    close(fd);
    logReply(slog_fd,threadNo, &answer);
    workingThreads[threadNo] = false;
}

void * threadInit(void * args) {
    int threadNo = * (int *) args;
    while(serverUp) { //mudar para condição de fecho do server

        pthread_mutex_lock(&queue_mutex);

        while (isEmpty(queue)) {
            sleep(0.5);
        }

        workingThreads[threadNo] = true;

        //VAI BUSCAR A QUEUE
        struct tlv_request request;
        request = requestFromQueue();

        //TIRA O LOCK DA QUEUE
        pthread_mutex_unlock(&queue_mutex);

        requestHandler(request, threadNo);
    }
}

struct tlv_reply requestParser(struct tlv_request request, int threadNo) {

    switch(request.type) {
    case OP_CREATE_ACCOUNT:
        return addAccount(request, threadNo);
        break;

    case OP_BALANCE:
        return balanceCheck(request, threadNo);
        break;

    case OP_TRANSFER:
        return transferMoney(request, threadNo);
        break;

    case OP_SHUTDOWN:
        return closeServer(request, threadNo);
        break;

    default:
        break;
    }
}

struct tlv_request requestFromQueue() {
    tlv_request_t request;
    request = dequeue(queue);
    logRequest(slog_fd, request.value.header.pid,&request);
    return request;  // espero que esteja correto
}


struct tlv_reply closeServer(struct tlv_request request, int threadNo) {
    struct rep_header resp;
    resp.account_id = request.value.header.account_id;

    struct rep_value resp_val;
    resp_val.header = resp;

    unlink(SERVER_FIFO_PATH);

    struct rep_shutdown shut;
    int work = 0;
    for (unsigned int i = 0; i < threadNumber; i++) {
        if (workingThreads) {
            work++;
        }
    }
    shut.active_offices = work;

    struct tlv_reply reply;
    reply.value = resp_val;
    reply.type = OP_SHUTDOWN;

    if (request.value.header.account_id == 0) {
        printf("\n%d\n", serverUp);
        serverUp = false;
        printf("\n%d\n", serverUp);
        while (!existingActiveThread(threadNo)) {
        }
        resp.ret_code = RC_OK;
    }
    else {
        resp.ret_code = RC_OP_NALLOW;
    }

    reply.length = sizeof(resp_val);
    return reply;

}

bool existingActiveThread(int threadNo) {
    for (unsigned int i = 0; i < threadNumber; i++) {
      if (i != threadNo) {
        if (workingThreads[i]) {
            return false;
        }
      }
  }
  return true;
}

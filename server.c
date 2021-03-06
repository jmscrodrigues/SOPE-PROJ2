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
struct tlv_reply requestParser(struct tlv_request request, int threadNo);
struct tlv_request requestFromQueue();
struct tlv_reply closeServer(struct tlv_request request);
bool checkPassword(struct tlv_request req);

int main(int argc, char* argv[]) {

    time_t t;
    srand((unsigned) time(&t));

    slog_fd =open(SERVER_LOGFILE, O_RDWR |O_CREAT |O_TRUNC|O_APPEND, 0666);

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

    close(fd);

    for(i =0; i < num_threads; i++) {
        logBankOfficeClose(slog_fd, i, t_ids[i]);
        pthread_cancel(t_ids[i]);
    }

    free(in);
    free(acc);
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
        logSyncMech(slog_fd,threadNo,SYNC_OP_MUTEX_LOCK,SYNC_ROLE_CONSUMER,9999);

        while (isEmpty(queue)) {
            sleep(0.5);
        }

        workingThreads[threadNo] = true;

        //VAI BUSCAR A QUEUE
        struct tlv_request request;
        request = requestFromQueue();

        //TIRA O LOCK DA QUEUE
        pthread_mutex_unlock(&queue_mutex);
        logSyncMech(slog_fd,threadNo,SYNC_OP_MUTEX_UNLOCK,SYNC_ROLE_CONSUMER,9999);

        requestHandler(request, threadNo);
    }

    return NULL;
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
        return closeServer(request);
        break;

    case __OP_MAX_NUMBER:
        break;
    }
    struct tlv_reply nullReply;
    nullReply.length = 0;
    return nullReply;
}

struct tlv_request requestFromQueue() {
    tlv_request_t request;
    request = dequeue(queue);
    logRequest(slog_fd, request.value.header.pid,&request);
    return request;
}


struct tlv_reply closeServer(struct tlv_request request) {
    struct rep_header resp;
    resp.account_id = request.value.header.account_id;

    struct rep_value resp_val;
    resp_val.header = resp;

    struct rep_shutdown shut;
    int work = 0;
    for (int i = 0; i < threadNumber; i++) {
        if (workingThreads[i]) {
            work++;
        }
    }
    shut.active_offices = work;

    struct tlv_reply reply;
    reply.value = resp_val;
    reply.type = OP_SHUTDOWN;
    reply.value.shutdown = shut;

    if (request.value.header.account_id == 0) {
        serverUp = false;
        sleep(1);
        unlink(SERVER_FIFO_PATH);
        while (isEmpty(queue) != 0) {
        }
        resp.ret_code = RC_OK;
    }
    else {
        resp.ret_code = RC_OP_NALLOW;
    }

    reply.length = sizeof(resp_val);
    return reply;

}

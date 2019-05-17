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

void * threadInit(void * args);
bool existingActiveThread();
struct tlv_reply requestParser(struct tlv_request request);
struct tlv_request requestFromQueue();
struct tlv_reply closeServer(struct tlv_request request);



int main(int argc, char* argv[]) {

    time_t t;

    srand((unsigned) time(&t));

    char*  a= malloc(HASH_LEN);

    if (argc != 3) {
        printf("Wrong number of arguments\n");
        return -1;
    }

    int num_threads = atoi(argv[1]);
    threadNumber = num_threads;

    char * password = argv[2];
    if (strlen(password) > MAX_PASSWORD_LEN || strlen(password) < MIN_PASSWORD_LEN) {
        printf("Wrong password size\n");
        return -2;
    }

    queue = createQueue(100);



    pthread_t t_ids[num_threads];
    int i;
    for(i = 0; i < num_threads; i++) {
        int *in = malloc(sizeof(int));
        *in = i;
        if(pthread_create(&(t_ids[i]), NULL, threadInit, in) != 0) {
            fprintf(stderr, "Error creating thread %d\n", i);
        }
        else logBankOfficeOpen(STDOUT_FILENO, i, t_ids[i]);
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
    while(true) { // mudar este true para condição de encerramento do server

        if (read(fd,&tlv_req,sizeof(tlv_request_t)) <= 0)
            continue;

        enqueue(queue,tlv_req);
        logRequest(STDOUT_FILENO,tlv_req.value.header.pid,&tlv_req);
    }
    close(fd);

    //  unlink(SERVER_FIFO_PATH);
    printf("UNLINKED, ABOUT TO RETURN\n");
    return 0;
}

void requestHandler( tlv_request_t request, int threadNo) {

    //ELABORA A RESPOSTA DA QUEUE
    struct tlv_reply answer;
    answer = requestParser(request);
    // codigo para abrir o fifo de resposta

    //CRIA O FIFO DE RESPOSTA
    char response_fifo[USER_FIFO_PATH_LEN];
    char * pid = malloc(6);   // ex. 34567
    sprintf(pid, "%d", request.value.header.pid);

    strcpy(response_fifo, USER_FIFO_PATH_PREFIX);
    strcat(response_fifo,pid);

    //ABRE FIFO, ESCREVE E FECHA
    int fd = open(response_fifo, O_WRONLY );

    if(fd > 0)
        printf("DEBUG: %s\n",response_fifo);
    write(fd,&answer, sizeof(answer));
    close(fd);
    logReply(STDOUT_FILENO,threadNo, &answer);
    workingThreads[threadNo] = false;
}

void * threadInit(void * args) {
    int threadNo = * (int *) args;
    while(true) { //mudar para condição de fecho do server

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

struct tlv_reply requestParser(struct tlv_request request) {
    switch(request.type) {
    case OP_CREATE_ACCOUNT:
        return addAccount(request);
        break;

    case OP_BALANCE:
        return balanceCheck(request);
        break;

    case OP_TRANSFER:
        return transferMoney(request);
        break;

    case OP_SHUTDOWN:
        return closeServer(request);
        break;

    default:
        break;
    }
}

struct tlv_request requestFromQueue() {
    tlv_request_t request;
    request = dequeue(queue);
    logRequest(STDOUT_FILENO, request.value.header.pid,&request);
    return request;  // espero que esteja correto
}


struct tlv_reply closeServer(struct tlv_request request) {
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

    uint32_t size;

    if (request.value.header.account_id != 0) {
        while (!existingActiveThread()) {
        }
        resp.ret_code = RC_OK;
    }
    else {
        resp.ret_code = RC_OP_NALLOW;
    }

    reply.length = sizeof(resp_val);
    return reply;

}

bool existingActiveThread() {
    for (unsigned int i = 0; i < threadNumber; i++) {
        if (workingThreads) {
            return false;
        }
    }
    return true;
}

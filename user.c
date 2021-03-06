#include <stdio.h>
#include "constants.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "types.h"
#include "sope.h"

void argumentParser(char argv5[],  char ret[3][MAX_PASSWORD_LEN]);

int main(int argc, char* argv[]) {

    if (argc != 6) {
        printf("Usage: %s <ID> <password> <delay_ms> <operation> <args>\n", argv[0]);
        return -1;
    }
    int argv4 = atoi(argv[4]);

    if(argv4 > 3 || argv4 <0) {
        printf("<operation> must be between 1 and 4\n");
        return -2;
    }

    if((argv4 == 1 || argv4 == 3 ) && strcmp(argv[5],"") != 0) {
        printf("Operation 2 takes no arguments. Use \"\" \n");
        return -3;
    }

    char argv5 [3][MAX_PASSWORD_LEN];
    argumentParser(argv[5], argv5);

    //-- req_header-----
    req_header_t req_header;
    req_header.pid = getpid();

    req_header.account_id = atoi(argv[1]);
    strcpy(req_header.password, argv[2]);
    req_header.op_delay_ms = atoi(argv[3]);
    //----------------
    //--req_value-------
    req_value_t req_value;
    req_value.header = req_header;
    //--Opt-transfer--creation
    if( argv4 == 0) {
        //req_create_account_t
        req_create_account_t req_create;
        req_create.account_id = atoi(argv5[0]);
        req_create.balance = atoi(argv5[1]);
        strcpy(req_create.password, argv5[2]);
        //--req_value-creation-------
        req_value.create = req_create;
        //------------------

    } else if(argv4== 2) {
        //req_transfer_t
        req_transfer_t req_transfer;
        req_transfer.account_id = atoi(argv5[0]);
        req_transfer.amount = atoi(argv5[1]);
        //--req_value-transfer------
        req_value.transfer = req_transfer;
        //------------------
    }

    //--tlv_request-----
    tlv_request_t tlv_req;
    tlv_req.type = argv4;
    tlv_req.length = sizeof(req_value);
    tlv_req.value = req_value;
    //------------------

    int ulog = open(USER_LOGFILE, O_RDWR |O_CREAT |O_APPEND,0666);
    int fd = open(SERVER_FIFO_PATH, O_WRONLY );

    write(fd,&tlv_req, sizeof(tlv_req)); //mandar mensagem tlv
    close(fd);
    logRequest(ulog,getpid(),&tlv_req);

    char response_fifo[USER_FIFO_PATH_LEN];
    sprintf(response_fifo, "%s%d", USER_FIFO_PATH_PREFIX, getpid());

    if(mkfifo(response_fifo, 0660) != 0) {
        fprintf(stderr, "Error creating fifo\n");
        return -5;
    }

    fd = open(response_fifo, O_RDONLY | O_NONBLOCK);

    tlv_reply_t tlv_reply;

    int time = FIFO_TIMEOUT_SECS;
    while(time) {

        if (read(fd, &tlv_reply,sizeof(tlv_reply)) > 0) { //lê mensagens tlv
            logReply(ulog,getpid(), &tlv_reply);
            close(fd);
            unlink(response_fifo);
            return 0;
        }
        time--;
        sleep(1);
    }

    tlv_reply.type = tlv_req.type;
    tlv_reply.value.header.account_id = tlv_req.value.header.account_id;
    tlv_reply.value.header.ret_code = RC_SRV_TIMEOUT;
    tlv_reply.value.balance.balance = 0;
    tlv_reply.value.shutdown.active_offices = 0;
    tlv_reply.value.transfer.balance = 0;
    tlv_reply.length = sizeof(tlv_reply.value);

    logReply(STDOUT_FILENO,getpid(),&tlv_reply);

    close(fd);
    close(ulog);
    unlink(response_fifo);

    return 0;
}


void argumentParser(char argv5[], char ret[3][MAX_PASSWORD_LEN]) {

    char delim[] = " ";

    char *ptr = strtok(argv5, delim);

    int i = 0;
    while (ptr != NULL && i < 3)
    {
        strcpy(ret[i],ptr);
        ptr = strtok(NULL, delim);
        i++;
    }
}
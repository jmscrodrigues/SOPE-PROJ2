#include <stdio.h>
#include "constants.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <types.h>

void argumentParser(char argv5[],  char ret[3][MAX_PASSWORD_LEN]);

int main(int argc, char* argv[]) {
    if (argc != 6) {
        printf("Usage: %s <ID> <password> <delay_ms> <operation> <args>\n", argv[0]);
        return -1;
    }

    if((strcmp(argv[4],"1") == 0 || strcmp(argv[4],"3") == 0 ) && strcmp(argv[5],"") != 0) {
        printf("Operation 2 takes no arguments. Use \"\" \n");
        return -2;
    }

    char argv5 [3][MAX_PASSWORD_LEN];
    argumentParser(argv[5], argv5);

    //-- req_header-----
    req_header_t req_header;
    req_header.pid = getpid();
    req_header.account_id = atoi(argv[1]);
    strcpy(req_header.password, argv[2]); //eventualmente fazer o Hash
    req_header.op_delay_ms = atoi(argv[3]);
    //----------------
    //--req_value-------
    req_value_t req_value;
    req_value.header = req_header;
    //--Opt-transfer--creation
    if(*argv[4]== '1') {
        //req_create_account_t
        req_create_account_t req_create;
        req_create.account_id = atoi(argv5[0]);
        req_create.balance = atoi(argv5[1]);
        strcpy(req_create.password, argv[2]);
        //--req_value-creation-------
        req_value.create = req_create;
        //------------------

    } else   if(*argv[4]== '3') {
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
    tlv_req.type = atoi(argv[4]);
    tlv_req.length = sizeof(req_value);
    //------------------

    int fd = open(SERVER_FIFO_PATH, O_WRONLY );

    write(fd,&tlv_req, sizeof(tlv_req)); //mandar mensagem tlv
    close(fd);

    char pid[6];
    sprintf(pid,"%d",getpid());
    
    char response_fifo[USER_FIFO_PATH_LEN];
    strcpy(response_fifo, USER_FIFO_PATH_PREFIX);
    strcat(response_fifo,pid);

    fd = open(response_fifo, O_RDONLY );
    tlv_reply_t tlv_reply;
    read(fd, &tlv_reply,sizeof(tlv_reply));//lê mensagens tlv

    printf("%s",pid);

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
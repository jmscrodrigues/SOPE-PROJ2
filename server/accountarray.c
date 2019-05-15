#include "accountarray.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


static struct account_mut accounts[MAX_BANK_ACCOUNTS+1];
static bool usedIds[MAX_BANK_ACCOUNTS+1] = {0};

struct tlv_reply addAccount(struct tlv_request request) {
    struct rep_header resp;
    resp.account_id = request.value.create.account_id;

    struct rep_value resp_val;
    resp_val.header = resp;

    struct tlv_reply reply;
    reply.value = resp_val;
    reply.type = OP_CREATE_ACCOUNT;

    uint32_t size;

    printf("1\n");

    if (request.value.header.account_id == 0) {
        if (usedIds[request.value.create.account_id] == 0) {
            if (request.value.create.balance >= MIN_BALANCE) {
                if (request.value.create.balance <= MAX_BALANCE) {
                    if (strlen(request.value.create.password) > MIN_PASSWORD_LEN) {
                        if (strlen(request.value.create.password) < MAX_PASSWORD_LEN) {

                            struct bank_account acc;
                            acc.account_id = request.value.create.account_id;
                            acc.balance = request.value.create.balance;
                            //Salt bae
                            char salt[SALT_LEN];
                            generateSalt(salt);
                            strcpy(acc.salt,salt);
                            //Hash bae
                            char hash[HASH_LEN];
                            generateHash(request.value.create.password, salt, hash,request.value.create.account_id);
                            strcpy(acc.hash,hash);
                            accounts[request.value.create.account_id].bank = acc;
                            pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
                            accounts[request.value.create.account_id].mutex =  mutex;
                            usedIds[request.value.create.account_id] = true;
                            resp.ret_code = RC_OK;
                            /*size = sizeof(resp_val);
                            reply.length = size;
                            return reply;*/
                        }
                        else {
                            resp.ret_code = RC_OTHER;
                        }
                    }
                    else {
                        resp.ret_code = RC_OTHER;
                    }
                }
                else {
                    resp.ret_code = RC_TOO_HIGH;
                }
            }
            else {
                resp.ret_code = RC_NO_FUNDS;
            }
        }
        else {
            resp.ret_code = RC_ID_IN_USE;
        }
    }
    else {
        resp.ret_code = RC_OP_NALLOW;
    }

    size = sizeof(resp_val);
    reply.length = size;
    return reply;
}

struct tlv_reply transferMoney(struct tlv_request request) {
    struct rep_header resp;
    resp.account_id = request.value.header.account_id;

    struct rep_value resp_val;
    resp_val.header = resp;

    if (usedIds[request.value.transfer.account_id] != 0 && usedIds[request.value.header.account_id] != 0) {
        pthread_mutex_lock(&(accounts[request.value.transfer.account_id].mutex));
        pthread_mutex_lock(&(accounts[request.value.header.account_id].mutex));
    }
    //TESTAR MUTEX E TENTAR ACEDER


    struct rep_transfer transf;
    transf.balance = accounts[request.value.transfer.account_id].bank.balance + request.value.transfer.amount;
    resp_val.transfer = transf;

    struct tlv_reply reply;
    reply.value = resp_val;
    reply.type = OP_TRANSFER;

    uint32_t size;

    if (request.value.header.account_id != 0) {
        if (request.value.header.account_id != request.value.transfer.account_id) {

            if (usedIds[request.value.transfer.account_id] != 0) {
                if (accounts[request.value.header.account_id].bank.balance - request.value.transfer.amount > MIN_BALANCE) {
                    if (accounts[request.value.transfer.account_id].bank.balance + request.value.transfer.amount > MAX_BALANCE) {
                        accounts[request.value.transfer.account_id].bank.balance += request.value.transfer.amount;
                        accounts[request.value.header.account_id].bank.balance -= request.value.transfer.amount;
                        resp.ret_code = RC_OK;
                        /*  size = sizeof(resp_val);
                          reply.length = size;
                          return reply;*/
                    }
                    else {
                        resp.ret_code = RC_TOO_HIGH;
                    }
                }
                else {
                    resp.ret_code = RC_NO_FUNDS;
                }
            }
            else {
                resp.ret_code = RC_ID_NOT_FOUND;
            }
        }
        else {
            resp.ret_code = RC_SAME_ID;
        }
    }
    else {
        resp.ret_code = RC_OP_NALLOW;
    }

    //RESETAR MUTEX
    pthread_mutex_unlock(&(accounts[request.value.transfer.account_id].mutex));
    pthread_mutex_unlock(&(accounts[request.value.header.account_id].mutex));

    reply.length = sizeof(resp_val);
    return reply;

}

struct tlv_reply balanceCheck(struct tlv_request request) {

    struct rep_header resp;
    resp.account_id = request.value.header.account_id;

    struct rep_value resp_val;
    resp_val.header = resp;
    struct rep_balance bal;
    // LOCK NO MUTEX
    if (usedIds[request.value.header.account_id]  != 0) {
        pthread_mutex_lock(&(accounts[request.value.header.account_id].mutex));
    }

    bal.balance = accounts[request.value.header.account_id].bank.balance;
    resp_val.balance = bal;

    struct tlv_reply reply;
    reply.value = resp_val;
    reply.type = OP_BALANCE;

    uint32_t size;

    if (request.value.header.account_id != 0) {
        if (usedIds[request.value.header.account_id]  != 0) {
            resp.ret_code = RC_OK;
        }
        else {
            resp.ret_code = RC_ID_NOT_FOUND;
        }
    }
    else {
        resp.ret_code = RC_OP_NALLOW;
    }

    //RESETAR MUTEX
    pthread_mutex_unlock(&(accounts[request.value.header.account_id].mutex));

    size+=sizeof(resp_val);
    reply.length = size;
    return reply;
}
/*
struct tlv_reply closeServer(struct tlv_request request) {

  //ESTE AQUI NAO SEI FAZER POR CAUSA DOS THREADS
}*/

void getExternalCommand(char* outPutStr, char* commands[]) {

    int des_p[2];

    if(pipe(des_p) == -1) {
        perror("Pipe failed");
        exit(1);
    }
    int n = fork();
    if(n == 0)            //first fork
    {
        close(STDOUT_FILENO);  //closing stdout
        dup(des_p[1]);         //replacing stdout with pipe write
        close(des_p[0]);       //closing pipe read
        close(des_p[1]);

        execvp(commands[0], commands);
        perror("execv failed");
        exit(1);
    }

    if(n > 0) {
        close(STDIN_FILENO);   //closing stdin
        dup(des_p[0]);         //replacing stdin with pipe read
        close(des_p[1]);       //closing pipe write
        close(des_p[0]);
        int n = read(STDIN_FILENO,outPutStr,HASH_LEN);
    }
}

void generateHash(char pass[MAX_PASSWORD_LEN], char salt[SALT_LEN], char hash[HASH_LEN], int32_t userID) {


    char toHash[MAX_PASSWORD_LEN + SALT_LEN];
    char passName[25];
    sprintf(passName,"sope_secure_pass%d",userID);
  
    strcpy(toHash,pass);
    strcat(toHash,salt);

    FILE* tmp_passFile = fopen(passName, "w+");

    if(tmp_passFile == NULL) {
        perror("Unable to create file for hashing");
        exit(10);
    }

    fputs(toHash,tmp_passFile);

    fclose(tmp_passFile);

    char* commands[] = {"sha256sum",passName,0};

    char t_hash[HASH_LEN];
    getExternalCommand(hash,commands);

    remove("sope_secure_pass");
}

void generateSalt(char salt[SALT_LEN+1]) {
    const char *hex_digits = "0123456789abcdef";
    char t_salt[SALT_LEN+1]= {0};

    for( int i = 0 ; i < SALT_LEN; i++ ) {
        salt[i] = hex_digits[ ( rand() % 16 ) ];
    }
}
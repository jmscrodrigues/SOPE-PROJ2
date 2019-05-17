#include "accountarray.h"

struct account_mut * accounts;
static bool usedIds[MAX_BANK_ACCOUNTS+1] = {0};

void setAccountsArray(struct account_mut * acc) {
    accounts = acc;
}

void creatAdmin(char * pass) {
    bank_account_t admin;
    admin.account_id = 0;
    char salt[SALT_LEN];
    generateSalt(salt);
    strcpy(admin.salt,salt);
    //Hash bae
    char hash[HASH_LEN];
    generateHash(pass, salt, hash, 0);
    strcpy(admin.hash,hash);
    admin.balance = 0;

    accounts[0].bank = admin;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    accounts[0].mutex =  mutex;
    usedIds[0] = true;
}

struct tlv_reply addAccount(struct tlv_request request) {
    struct rep_header resp;
    resp.account_id = request.value.create.account_id;

    struct rep_value resp_val;

    struct tlv_reply reply;

    printf("\nPrinting the boolean: %d\n", checkPassword(request));

    if (request.value.header.account_id == 0) {
        if (usedIds[request.value.create.account_id] == 0) {

            if (request.value.create.balance >= MIN_BALANCE) {
                if (request.value.create.balance <= MAX_BALANCE) {
                    if (strlen(request.value.create.password) > MIN_PASSWORD_LEN) {
                        if (strlen(request.value.create.password) < MAX_PASSWORD_LEN) {
                            struct account_mut acc;
                            acc.bank.account_id = request.value.create.account_id;
                          //  printf("ID A CRIAR: %d\n", acc.bank.account_id);
                            acc.bank.balance = request.value.create.balance;
                          //  printf("\nthe real balance: %d\n", acc.bank.balance);
                            //Salt bae
                            char salt[SALT_LEN];
                            generateSalt(salt);
                            strcpy(acc.bank.salt,salt);
                            //Hash bae
                            char hash[HASH_LEN];
                            generateHash(request.value.create.password, salt, hash,request.value.create.account_id);
                            strcpy(acc.bank.hash,hash);
                            pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
                            acc.mutex =  mutex;
                            usedIds[request.value.create.account_id] = true;
                            accounts[request.value.create.account_id] = acc;
                        //    printf("OU VAI OU RACHA CRL: %d\n",accounts[request.value.create.account_id].bank.balance);
                        //    printf("ID? %d", accounts[request.value.create.account_id].bank.account_id);
                            resp.ret_code = RC_OK;

                        //    printf("\nid conta: %d\n", accounts[request.value.create.account_id].bank.account_id);
                        //    printf("\nbooleano: %d\n", usedIds[request.value.create.account_id]);
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
            printf("\n in use \n");
        }
    }
    else {
        resp.ret_code = RC_OP_NALLOW;
    }

    reply.type = OP_CREATE_ACCOUNT;
    resp_val.header = resp;
    reply.value = resp_val;
    reply.length = sizeof(resp_val);

  //  printf("BALANCE: %d", accounts[request.value.create.account_id].bank.balance);
    return reply;
}

struct tlv_reply transferMoney(struct tlv_request request) {
    struct rep_header resp;
    resp.account_id = request.value.header.account_id;

    struct rep_value resp_val;

    if (usedIds[request.value.transfer.account_id] != 0 && usedIds[request.value.header.account_id] != 0) {
        pthread_mutex_lock(&(accounts[request.value.transfer.account_id].mutex));
        pthread_mutex_lock(&(accounts[request.value.header.account_id].mutex));
    }
    //TESTAR MUTEX E TENTAR ACEDER

    printf("\nPrinting the boolean: %d\n", checkPassword(request));

    struct rep_transfer transf;

    struct tlv_reply reply;

    uint32_t size;

    if (request.value.header.account_id != 0) {
        if (request.value.header.account_id != request.value.transfer.account_id) {
          //  printf("1 \n");
            if (usedIds[request.value.transfer.account_id] != 0) {
              //  printf("2 \n");
                if (((accounts[request.value.header.account_id].bank.balance) - (request.value.transfer.amount)) > MIN_BALANCE) {
                  //  printf("3 \n");
                    if (((accounts[request.value.transfer.account_id].bank.balance) + (request.value.transfer.amount)) < MAX_BALANCE) {
                    //    printf("tudo suss \n");
                        accounts[request.value.transfer.account_id].bank.balance += request.value.transfer.amount;
                        accounts[request.value.header.account_id].bank.balance -= request.value.transfer.amount;
                        resp.ret_code = RC_OK;

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

    //printf("dei unlock\n");

  //  printf("%d\n",accounts[request.value.transfer.account_id].bank.balance);


    reply.type = OP_TRANSFER;
    transf.balance = accounts[request.value.transfer.account_id].bank.balance;
    resp_val.transfer = transf;
    resp_val.header = resp;
    reply.value = resp_val;
    reply.length = sizeof(resp_val);
    return reply;

}

struct tlv_reply balanceCheck(struct tlv_request request) {

    struct rep_header resp;
    resp.account_id = request.value.header.account_id;

    struct rep_value resp_val;

    struct rep_balance bal;

    printf("\nPrinting the boolean: %d\n", checkPassword(request));

  //  printf("\nid conta: %d\n", accounts[request.value.header.account_id].bank.account_id);
  //  printf("\nbooleano: %d\n", usedIds[request.value.header.account_id]);

    // LOCK NO MUTEX
    if (usedIds[request.value.header.account_id]  != 0) {
        pthread_mutex_lock(&(accounts[request.value.header.account_id].mutex));
      //  printf("Bloqueou mutex!\n");
    }


    struct tlv_reply reply;


    if (request.value.header.account_id != 0) {
    //    printf("DIFERENTE DO ADMIN NO BALANCE\n");
        if (usedIds[request.value.header.account_id]  != 0) {
      //      printf("CONTA EXISTE, A CHECKAR BALANCE\n");
            resp.ret_code = RC_OK;
      //      printf("OK\n");
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
  //  printf("Resetou mutex \n");

    bal.balance = accounts[request.value.header.account_id].bank.balance;
  //  printf("O balance do menino: %d", bal.balance);
    reply.type = OP_BALANCE;
  //  printf("HALFWAY DONE\n");
    resp_val.balance = bal;
    resp_val.header = resp;
    reply.value = resp_val;
    reply.length = sizeof(resp_val);
  //  printf("A enviar balance: %d", reply.value.balance.balance);
    return reply;
}

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

    remove(passName);
}

void generateSalt(char salt[SALT_LEN+1]) {
    const char *hex_digits = "0123456789abcdef";
    char t_salt[SALT_LEN+1]= {0};

    for( int i = 0 ; i < SALT_LEN; i++ ) {
        salt[i] = hex_digits[ ( rand() % 16 ) ];
    }
}

bool checkPassword(struct tlv_request req) {
  char newHash[HASH_LEN +1];
  char oldHash[HASH_LEN + 1];
  char salt[SALT_LEN +1];

  strcpy(salt, accounts[req.value.header.account_id].bank.salt);//salt = accounts[req.value.header.account_id].bank.salt;
  strcpy(oldHash, accounts[req.value.header.account_id].bank.hash);
  generateHash(req.value.header.password, salt,newHash, rand() % 100 + 1);

  printf("HASH NOVO: %s\n", newHash);
  printf("HASH ANTIGO: %s\n", oldHash);


  if (strncmp(newHash,oldHash,HASH_LEN) == 0) {
    return true;
  }
  else return false;
}

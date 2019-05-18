#include <pthread.h>
#include "types.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


struct tlv_reply addAccount(struct tlv_request request);
struct tlv_reply transferMoney(struct tlv_request request);
struct tlv_reply balanceCheck(struct tlv_request request);

void generateHash(char pass[MAX_PASSWORD_LEN+11], char salt[SALT_LEN+1], char hash[HASH_LEN+1], int32_t userID);
void getExternalCommand(char* outPutStr, char* commands[]);
void generateSalt(char salt[SALT_LEN]);
void creatAdmin(char * pass);
void setAccountsArray(struct account_mut * acc);
bool checkPassword(struct tlv_request req);

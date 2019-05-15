#include <pthread.h>
#include "types.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

struct tlv_reply addAccount(struct tlv_request request);
struct tlv_reply transferMoney(struct tlv_request request);
struct tlv_reply balanceCheck(struct tlv_request request);
//struct tlv_reply closeServer(struct tlv_request request);

void generateHash(char pass[MAX_PASSWORD_LEN], char salt[SALT_LEN], char hash[HASH_LEN], int32_t userID);
void getExternalCommand(char* outPutStr, char* commands[]);
void generateSalt(char salt[SALT_LEN]);

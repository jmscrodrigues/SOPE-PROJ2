#include "accountarray.h"

static struct bank_account accounts[4097];
static bool usedIds[4097] = {0};

enum ret_code addAccount(struct req_create_account acc) {
  if (usedIds[acc.account_id] == 0) {
    if (acc.balance >= MIN_BALANCE) {
     if (acc.balance <= MAX_BALANCE) {
       if (strlen(acc.password) > MIN_PASSWORD_LEN) {
         if (strlen(acc.password) < MAX_PASSWORD_LEN) {
           accounts[acc.account_id] = acc;
           usedIds[header.account_id] = true;
           return RC_OK;
         }
         else return OTHER;
       }
       else return OTHER;
      }
      else return RC_TOO_HIGH;
    }
    else return RC_NO_FUNDS;
  }
  else return RC_ID_IN_USE;
}



enum ret_code transferMoney(struct bank_account acc, struct bank_account destiny, int amount) {
  return RC_OK;
}

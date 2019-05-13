#include "accountarray.h"

static struct bank_account accounts[4097];
static bool usedIds[4097] = {0};

struct tlv_reply addAccount(struct tlv_request request) {
  struct rep_header resp;
  resp.account_id = request.value.create.account_id;

  struct rep_value resp_val;
  resp_val.header = resp;

  struct tlv_reply reply;
  reply.value = resp_val;
  reply.type = OP_CREATE_ACCOUNT;

  uint32_t size;

  if (request.value.header.account_id == 0) {
    if (usedIds[request.value.create.account_id] == 0) {
      if (request.value.create.balance >= MIN_BALANCE) {
        if (request.value.create.balance <= MAX_BALANCE) {
          if (strlen(request.value.create.password) > MIN_PASSWORD_LEN) {
            if (strlen(request.value.create.password) < MAX_PASSWORD_LEN) {
              struct bank_account acc;
              acc.account_id = request.value.create.account_id;
              acc.balance = request.value.create.balance;
              strcpy(acc.salt,"saltTest123");
              strcpy(acc.hash,"teste");
              accounts[request.value.create.account_id] = acc;
              usedIds[request.value.create.account_id] = true;
              resp.ret_code = RC_OK;
              size+=sizeof(resp_val);
              reply.length = size;
              return reply;
            }
            else {resp.ret_code = RC_OTHER;}
          }
          else {resp.ret_code = RC_OTHER;}
        }
        else {resp.ret_code = RC_TOO_HIGH;}
      }
      else {resp.ret_code = RC_NO_FUNDS;}
    }
    else {resp.ret_code = RC_ID_IN_USE;}
    }
    else {resp.ret_code = RC_OP_NALLOW;}

    size+=sizeof(resp_val);
    reply.length = size;
    return reply;
  }



struct tlv_reply transferMoney(struct tlv_request request) {

}

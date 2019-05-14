#include "accountarray.h"

static struct account_mut accounts[4097];
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
              accounts[request.value.create.account_id].bank = acc;
              pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
              accounts[request.value.create.account_id].mutex =  mutex;
              usedIds[request.value.create.account_id] = true;
              resp.ret_code = RC_OK;
              /*size = sizeof(resp_val);
              reply.length = size;
              return reply;*/
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

    size = sizeof(resp_val);
    reply.length = size;
    return reply;
  }



struct tlv_reply transferMoney(struct tlv_request request) {
  struct rep_header resp;
  resp.account_id = request.value.header.account_id;

  struct rep_value resp_val;
  resp_val.header = resp;

  struct rep_transfer transf;
  transf.balance = accounts[request.value.transfer.account_id].bank.balance += request.value.transfer.amount;
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
            resp.ret_code = RC_OK;
          /*  size = sizeof(resp_val);
            reply.length = size;
            return reply;*/
          }
          else {resp.ret_code = RC_TOO_HIGH;}
        }
        else {resp.ret_code = RC_NO_FUNDS;}
      }
      else {resp.ret_code = RC_ID_NOT_FOUND;}
    }
    else {resp.ret_code = RC_SAME_ID;}
  }
  else {resp.ret_code = RC_OP_NALLOW;}

  size = sizeof(resp_val);
  reply.length = size;
  return reply;

}

struct tlv_reply balanceCheck(struct tlv_request request) {

  struct rep_header resp;
  resp.account_id = request.value.header.account_id;

  struct rep_value resp_val;
  resp_val.header = resp;

  struct rep_balance bal;
  bal.balance = accounts[request.value.transfer.account_id].bank.balance;
  resp_val.balance = bal;

  struct tlv_reply reply;
  reply.value = resp_val;
  reply.type = OP_BALANCE;

  uint32_t size;

  if (request.value.header.account_id != 0) {
    resp.ret_code = RC_OK;
  }
  else {resp.ret_code = RC_OP_NALLOW;}

  size+=sizeof(resp_val);
  reply.length = size;
  return reply;
}
/*
struct tlv_reply closeServer(struct tlv_request request) {

  //ESTE AQUI NAO SEI FAZER POR CAUSA DOS THREADS
}*/

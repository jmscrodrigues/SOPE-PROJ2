#include <pthread.h>
#include "types.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

enum ret_code addAccount(struct bank_account acc);
enum ret_code transferMoney(struct bank_account acc, struct bank_account destiny, int amount);

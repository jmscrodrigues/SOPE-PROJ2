#include <pthread.h>
#include "types.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

struct tlv_reply addAccount(struct tlv_request request);
struct tlv_reply transferMoney(struct tlv_request request);

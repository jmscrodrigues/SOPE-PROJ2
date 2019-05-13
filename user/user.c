#include <stdio.h>
#include "constants.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
  if (argc != 6) {
     printf("Usage: %s <ID> <password> <delay_ms> <operation> <args>\n", argv[0]);
     return -1;
  }

  int fd1 = open(SERVER_FIFO_PATH, O_WRONLY );


    return 0;
}
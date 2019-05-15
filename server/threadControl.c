#include "threadControl.h"


void * threadInit(void * args) {

  pthread_mutex_lock( *(* pthread_mutex_t) &args);

}

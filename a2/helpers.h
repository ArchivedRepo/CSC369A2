#include <pthread.h>

void lock(pthread_mutex_t *lock); 
void condWait(pthread_cond_t *cond, pthread_mutex_t *lock); 
void condDestroy(pthread_cond_t *cond); 
void mutexDestroy(pthread_mutex_t *lock); 
void condBroadcast(pthread_cond_t *cond); 

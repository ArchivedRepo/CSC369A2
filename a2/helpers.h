#include <pthread.h>

void lock(pthread_mutex_t *lock); 
void cond_wait(pthread_cond_t *cond, pthread_mutex_t *lock); 
void cond_destroy(pthread_cond_t *cond); 
void mutex_destroy(pthread_mutex_t *lock); 
void cond_broadcast(pthread_cond_t *cond); 

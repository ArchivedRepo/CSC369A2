#include <pthread.h>

/**
* @brief Locks a mutex and does error checking.
*
* @param lock pointer to the mutex to lock.
*/
void lock(pthread_mutex_t *lock); 

/**
* @brief Wait on a mutex and a conditional variable, and does error checking.
*
* @param lock pointer to the mutex to wait.
* @param cond conditional variable to wait.
*/
void condWait(pthread_cond_t *cond, pthread_mutex_t *lock);

/**
* @brief Destroy a condition variable and does error checking.
*
* @param cond pointer to the condition variable to destroy.
*/
void condDestroy(pthread_cond_t *cond); 

/**
* @brief Destroy a mutex and does error checking.
*
* @param lock pointer to the mutex to destroy.
*/
void mutexDestroy(pthread_mutex_t *lock); 

/**
* @brief Broadcast on a conditional variable and does error checking.
*
* @param cond conditional variable to broadcast.
*/
void condBroadcast(pthread_cond_t *cond); 

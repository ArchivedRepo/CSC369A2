#include "helpers.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void lock(pthread_mutex_t *lock) {
	if (pthread_mutex_lock(lock) != 0) {
		perror("pthread_mutex_lock");
		exit(1);
	}
}

void condWait(pthread_cond_t *cond, pthread_mutex_t *lock) {
	if (pthread_cond_wait(cond, lock) != 0) {
		perror("pthread_cond_wait");
		exit(1);
	}
}

void condDestroy(pthread_cond_t *cond) {
	if (pthread_cond_destroy(cond) != 0) {
		perror("Destroy Conditional Variable");
		exit(1);
	}
}

void mutexDestroy(pthread_mutex_t *lock) {
	if (pthread_mutex_destroy(lock) != 0) {
		perror("pthread_mutex_destroy");
		exit(1);
	}
}

void condBroadcast(pthread_cond_t *cond) {
	if (pthread_cond_broadcast(cond) != 0) {
		perror("pthread_cond_broadcase");
		exit(1);
	}
}
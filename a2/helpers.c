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

void cond_wait(pthread_cond_t *cond, pthread_mutex_t *lock) {
	if (pthread_cond_wait(cond, lock) != 0) {
		perror("pthread_cond_wait");
		exit(1);
	}
}

void cond_destroy(pthread_cond_t *cond) {
	if (pthread_cond_destroy(cond) != 0) {
		perror("Destroy Conditional Variable");
		exit(1);
	}
}

void mutex_destroy(pthread_mutex_t *lock) {
	if (pthread_mutex_destroy(lock) != 0) {
		perror("pthread_mutex_destroy");
		exit(1);
	}
}

void cond_broadcast(pthread_cond_t *cond) {
	if (pthread_cond_broadcast(cond) != 0) {
		perror("pthread_cond_broadcase");
		exit(1);
	}
}
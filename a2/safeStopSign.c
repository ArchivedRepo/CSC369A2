/**
* CSC369 Assignment 2
*
* This is the source/implementation file for your safe stop sign
* submission code.
*/
#include "safeStopSign.h"
#include "common.h"

/*
 * Return whether the path is okay to go. Return 1 on okay, 0 otherwise.
 */ 
static int can_pass(int *path, int length, int *occupy) {
	for (int i =0; i< length; i++) {
		if (occupy[path[i]] == 1 ) {
			return 0;
		} 
	}
	return 1;
}

static void lock(pthread_mutex_t *lock) {
	if (pthread_mutex_lock(lock) != 0) {
		perror("pthread_mutex_lock");
		exit(1);
	}
}

static void cond_wait(pthread_cond_t *cond, pthread_mutex_t *lock) {
	if (pthread_cond_wait(cond, lock) != 0) {
		perror("pthread_cond_wait");
		exit(1);
	}
}

static void cond_destroy(pthread_cond_t *cond) {
	if (pthread_cond_destroy(cond) != 0) {
		perror("Destroy Conditional Variable");
		exit(1);
	}
}

static void mutex_destroy(pthread_mutex_t *lock) {
	if (pthread_mutex_destroy(lock) != 0) {
		perror("pthread_mutex_destroy");
		exit(1);
	}
}

static void cond_broadcast(pthread_cond_t *cond) {
	if (pthread_cond_broadcast(cond) != 0) {
		perror("pthread_cond_broadcase");
		exit(1);
	}
}

void initSafeStopSign(SafeStopSign* sign, int count) {
	initStopSign(&sign->base, count);

	// TODO: Add any initialization logic you need.
	for (int i = 0; i < DIRECTION_COUNT; i++) {
		initMutex(&sign->enter_lock[i]);
		initMutex(&sign->lane_lock[i]);
		sign->enter_count[i] = 0;
		sign->exit_count[i] = 0;
		initConditionVariable(&sign->exit_cond[i]);
	}
	initConditionVariable(&sign->wait_cond);
	initMutex(&sign->occupy_lock);
	for (int i = 0; i < QUADRANT_COUNT; i++) {
		sign->occupy[i] = 0;
	}
}

void destroySafeStopSign(SafeStopSign* sign) {
	destroyStopSign(&sign->base);

	// TODO: Add any logic you need to clean up data structures.
	for (int i =0; i < DIRECTION_COUNT; i++) {
		mutex_destroy(&sign->enter_lock[i]);
		mutex_destroy(&sign->lane_lock[i]);
		cond_destroy(&sign->exit_cond[i]);
	}
	cond_destroy(&sign->wait_cond);
	mutex_destroy(&sign->occupy_lock);
}

void runStopSignCar(Car* car, SafeStopSign* sign) {

	// TODO: Add your synchronization logic to this function.
	int lane_index = getLaneIndex(car);
	lock(&sign->lane_lock[lane_index]);
	EntryLane* lane = getLane(car, &sign->base);
	enterLane(car, lane);
	long index = sign->enter_count[lane_index];
	sign->enter_count[lane_index]++;
	unlock(&sign->lane_lock[lane_index]);

	int path[3];
	int length = getStopSignRequiredQuadrants(car, path);

	lock(&sign->enter_lock[lane_index]);
	lock(&sign->occupy_lock);
	while (can_pass(path, length, sign->occupy) == 0) {
		cond_wait(&sign->wait_cond, &sign->occupy_lock);
	}
	for (int i = 0; i<length; i++) {
		sign->occupy[path[i]] = 1;
	}
	unlock(&sign->occupy_lock);

	goThroughStopSign(car, &sign->base);

	lock(&sign->occupy_lock);
	for (int i = 0; i<length; i++) {
		sign->occupy[path[i]] = 0;
	}
	cond_broadcast(&sign->wait_cond);
	unlock(&sign->occupy_lock);
	unlock(&sign->enter_lock[lane_index]);

	lock(&sign->lane_lock[lane_index]);
	while (index != sign->exit_count[lane_index]) {
		cond_wait(&sign->exit_cond[lane_index], &sign->lane_lock[lane_index]);
	}
	exitIntersection(car, lane);
	sign->exit_count[lane_index]++;
	cond_broadcast(&sign->exit_cond[lane_index]);
	unlock(&sign->lane_lock[lane_index]);
}

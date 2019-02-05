/**
* CSC369 Assignment 2
*
* This is the source/implementation file for your safe stop sign
* submission code.
*/
#include "safeStopSign.h"
#include "common.h"
#include "helpers.h"

/*
 * Return whether the path is okay to go. Return 1 on okay, 0 otherwise.
 */ 
static int canPass(int *path, int length, int *occupy) {
	for (int i =0; i< length; i++) {
		if (occupy[path[i]] == 1 ) {
			return 0;
		} 
	}
	return 1;
}

void initSafeStopSign(SafeStopSign* sign, int count) {
	initStopSign(&sign->base, count);

	// TODO: Add any initialization logic you need.
	for (int i = 0; i < DIRECTION_COUNT; i++) {
		initMutex(&sign->enterLock[i]);
		initMutex(&sign->laneLock[i]);
		sign->enterCount[i] = 0;
		sign->exitCount[i] = 0;
		initConditionVariable(&sign->exitCond[i]);
	}
	initConditionVariable(&sign->waitCond);
	initMutex(&sign->occupyLock);
	for (int i = 0; i < QUADRANT_COUNT; i++) {
		sign->occupy[i] = 0;
	}
}

void destroySafeStopSign(SafeStopSign* sign) {
	destroyStopSign(&sign->base);

	// TODO: Add any logic you need to clean up data structures.
	for (int i =0; i < DIRECTION_COUNT; i++) {
		mutexDestroy(&sign->enterLock[i]);
		mutexDestroy(&sign->laneLock[i]);
		condDestroy(&sign->exitCond[i]);
	}
	condDestroy(&sign->waitCond);
	mutexDestroy(&sign->occupyLock);
}

void runStopSignCar(Car* car, SafeStopSign* sign) {

	// TODO: Add your synchronization logic to this function.
	int lane_index = getLaneIndex(car);
	lock(&sign->laneLock[lane_index]);
	EntryLane* lane = getLane(car, &sign->base);
	enterLane(car, lane);
	long index = sign->enterCount[lane_index];
	sign->enterCount[lane_index]++;
	unlock(&sign->laneLock[lane_index]);

	int path[3];
	int length = getStopSignRequiredQuadrants(car, path);

	lock(&sign->enterLock[lane_index]);
	lock(&sign->occupyLock);
	while (canPass(path, length, sign->occupy) == 0) {
		condWait(&sign->waitCond, &sign->occupyLock);
	}
	for (int i = 0; i<length; i++) {
		sign->occupy[path[i]] = 1;
	}
	unlock(&sign->occupyLock);
	goThroughStopSign(car, &sign->base);
	lock(&sign->occupyLock);
	for (int i = 0; i<length; i++) {
		sign->occupy[path[i]] = 0;
	}
	condBroadcast(&sign->waitCond);
	unlock(&sign->occupyLock);
	unlock(&sign->enterLock[lane_index]);

	lock(&sign->laneLock[lane_index]);
	while (index != sign->exitCount[lane_index]) {
		condWait(&sign->exitCond[lane_index], &sign->laneLock[lane_index]);
	}
	exitIntersection(car, lane);
	sign->exitCount[lane_index]++;
	condBroadcast(&sign->exitCond[lane_index]);
	unlock(&sign->laneLock[lane_index]);
}

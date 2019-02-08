/**
* CSC369 Assignment 2
*
* This is the source/implementation file for your safe stop sign
* submission code.
*/
#include "safeStopSign.h"
#include "helpers.h"


int check_road_clear(int *quadrants, int quadrantCount, SafeStopSign* sign){
	
	for(int i=0; i<quadrantCount; i++){
		if(sign->quadCount[quadrants[i]] != 0){
			return 1;
		}
	}
	return 0;
}

void initSafeStopSign(SafeStopSign* sign, int count) {
	initStopSign(&sign->base, count);

	// TODO: Add any initialization logic you need.
	for(int i=0; i<DIRECTION_COUNT; i++){
		initMutex(&sign->laneLock[i]);
		initConditionVariable(&sign->laneCond[i]);
		sign->enterCount[i] = 0;
		sign->exitCount[i] = 0;
	}

	initMutex(&sign->quadLock);
	initConditionVariable(&sign->quadCond);
	for (int i = 0; i < QUADRANT_COUNT; i++) {
		sign->quadCount[i] = 0;
	}
	
}

void destroySafeStopSign(SafeStopSign* sign) {
	destroyStopSign(&sign->base);

	// TODO: Add any logic you need to clean up data structures.
	for(int i=0; i<DIRECTION_COUNT; i++){
		mutexDestroy(&sign->laneLock[i]);
		condDestroy(&sign->laneCond[i]);
	}
	mutexDestroy(&sign->quadLock);
	condDestroy(&sign->quadCond);
}

void runStopSignCar(Car* car, SafeStopSign* sign) {

	// TODO: Add your synchronization logic to this function.

	EntryLane* lane = getLane(car, &sign->base);
	int laneIndex = getLaneIndex(car);
	int quadrants[QUADRANT_COUNT];
	int quadrantCount = getStopSignRequiredQuadrants(car, quadrants);

	lock(&sign->laneLock[laneIndex]);
	int count = sign->enterCount[laneIndex];
	enterLane(car, lane);
	sign->enterCount[laneIndex]++;
	unlock(&sign->laneLock[laneIndex]);

	lock(&sign->quadLock);
	while(check_road_clear(quadrants, quadrantCount, sign)){
		condWait(&sign->quadCond, &sign->quadLock);
	}
	for(int i=0; i<quadrantCount; i++){
		sign->quadCount[quadrants[i]]++;
	}
	unlock(&sign->quadLock);

	goThroughStopSign(car, &sign->base);

	lock(&sign->quadLock);
	for(int i=0; i<quadrantCount; i++){
		sign->quadCount[quadrants[i]]--;
	}
	condBroadcast(&sign->quadCond);
	unlock(&sign->quadLock);

	lock(&sign->laneLock[laneIndex]);
	while(sign->exitCount[laneIndex]!= count){
		condWait(&sign->laneCond[laneIndex], &sign->laneLock[laneIndex]);
	}
	exitIntersection(car, lane);
	sign->exitCount[laneIndex]++;
	condBroadcast(&sign->laneCond[laneIndex]);
	unlock(&sign->laneLock[laneIndex]);
}

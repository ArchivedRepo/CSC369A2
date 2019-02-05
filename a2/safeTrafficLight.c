/**
* CSC369 Assignment 2
*
* This is the source/implementation file for your safe traffic light 
* submission code.
*/
#include "safeTrafficLight.h"
#include "helpers.h"

/**
 * Return whether the car can go in the given light state.
 * Return 1 on can go, 0 otherwise
 */  
static int canGo(Car* car, LightState state) {
	CarPosition pos = car->position;
	if (pos == NORTH || pos == SOUTH) {
		if (state == NORTH_SOUTH) {
			return 1;
		}
	} else {
		if (state == EAST_WEST) {
			return 1;
		}
	}
	return 0;
}

static CarPosition get_oppo_pos(Car* car) {
	if (car->position == NORTH) {
		return SOUTH;
	} else  if (car->position == SOUTH) {
		return NORTH;
	} else if (car->position == WEST) {
		return EAST;
	} else if (car->position == EAST) {
		return WEST;
	}
	// Should not reach here
	assert(FALSE);
}

void beforeSleep(void* lock_ptr) {
	unlock((pthread_mutex_t*)lock_ptr);
}

void afterSleep(void* lock_ptr) {
	lock((pthread_mutex_t*) lock_ptr);
}

void initSafeTrafficLight(SafeTrafficLight* light, int horizontal, int vertical) {
	initTrafficLight(&light->base, horizontal, vertical);

	// TODO: Add any initialization logic you need.
	for (int i =0;i< TRAFFIC_LIGHT_LANE_COUNT;i++) {
		initMutex(&light->laneLock[i]);
		initConditionVariable(&light->exitCond[i]);
		light->enterCount[i] = 0;
		light->exitCount[i] = 0;
	}
	initMutex(&light->lightLock);
	initConditionVariable(&light->lightCond);
	initConditionVariable(&light->leftCond);
}

void destroySafeTrafficLight(SafeTrafficLight* light) {
	destroyTrafficLight(&light->base);

	// TODO: Add any logic you need to free data structures
	for (int i = 0; i< TRAFFIC_LIGHT_LANE_COUNT;i++) {
		initMutex(&light->laneLock[i]);
		initConditionVariable(&light->exitCond[i]);
	}
	mutexDestroy(&light->lightLock);
	condDestroy(&light->lightCond);
	condDestroy(&light->leftCond);
}

void runTrafficLightCar(Car* car, SafeTrafficLight* light) {

	// TODO: Add your synchronization logic to this function.
	int lane_index = getLaneIndexLight(car);
	lock(&light->laneLock[lane_index]);
	EntryLane* lane = getLaneLight(car, &light->base);
	enterLane(car, lane);
	int index = light->enterCount[lane_index];
	light->enterCount[lane_index]++;
	unlock(&light->laneLock[lane_index]);

	// Enter and act are separate calls because a car turning left can first
	// enter the intersection before it needs to check for oncoming traffic.
	lock(&light->lightLock);
	while (canGo(car, getLightState(&light->base)) == 0) {
		condWait(&light->lightCond, &light->lightLock);
	}
	enterTrafficLight(car, &light->base);
	if (car->action == STRAIGHT || car->action == RIGHT_TURN) {
		actTrafficLight(car, &light->base, beforeSleep, afterSleep, 
		(void*)&light->lightLock);
		// actTrafficLight(car, &light->base, NULL, NULL, NULL);
		condBroadcast(&light->leftCond);
	} else {
		while (getStraightCount(&light->base, get_oppo_pos(car)) != 0) {
			condWait(&light->leftCond, &light->lightLock);
		}
		actTrafficLight(car, &light->base, beforeSleep, afterSleep
		, (void*)&light->lightLock);
		// actTrafficLight(car, &light->base, NULL, NULL, NULL);
	}
	condBroadcast(&light->lightCond);
	unlock(&light->lightLock);

	lock(&light->laneLock[lane_index]);
	while (light->exitCount[lane_index] != index) {
		condWait(&light->exitCond[lane_index], &light->laneLock[lane_index]);
	}
	exitIntersection(car, lane);
	light->exitCount[lane_index]++;
	condBroadcast(&light->exitCond[lane_index]);
	unlock(&light->laneLock[lane_index]);

}

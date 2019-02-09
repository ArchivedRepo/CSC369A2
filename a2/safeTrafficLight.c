/**
* CSC369 Assignment 2
*
* This is the source/implementation file for your safe traffic light 
* submission code.
*/
#include "safeTrafficLight.h"
#include "helpers.h"

void beforeSleep(void* stateLock){
	unlock((pthread_mutex_t*)stateLock);
}

void afterSleep(void* stateLock){
	lock((pthread_mutex_t*)stateLock);
}

void initSafeTrafficLight(SafeTrafficLight* light, int horizontal, int vertical) {
	initTrafficLight(&light->base, horizontal, vertical);

	// TODO: Add any initialization logic you need.
	for(int i=0; i<TRAFFIC_LIGHT_LANE_COUNT; i++){
		initMutex(&light->laneLock[i]);
		initConditionVariable(&light->laneCond[i]);
		light->enterCount[i] = 0;
		light->exitCount[i] = 0;
	}

	initMutex(&light->stateLock);
	initConditionVariable(&light->stateCond);
	initConditionVariable(&light->leftTurnCond);
}

void destroySafeTrafficLight(SafeTrafficLight* light) {
	destroyTrafficLight(&light->base);

	// TODO: Add any logic you need to free data structures
	for(int i=0; i<TRAFFIC_LIGHT_LANE_COUNT; i++){
		mutexDestroy(&light->laneLock[i]);
		condDestroy(&light->laneCond[i]);
	}
	mutexDestroy(&light->stateLock);
	condDestroy(&light->stateCond);
	condDestroy(&light->leftTurnCond);


}

void runTrafficLightCar(Car* car, SafeTrafficLight* light) {

	// TODO: Add your synchronization logic to this function.
	int laneIndex = getLaneIndexLight(car);
	EntryLane* lane = getLaneLight(car, &light->base);

	// Enter lane
	lock(&light->laneLock[laneIndex]);
	int carIndex = light->enterCount[laneIndex];
	enterLane(car, lane);
	light->enterCount[laneIndex]++;
	unlock(&light->laneLock[laneIndex]);


	// Enter and act are separate calls because a car turning left can first
	// enter the intersection before it needs to check for oncoming traffic.
	
	lock(&light->stateLock);
	// Validate that the light is green for the car.
	if (car->position == EAST || car->position == WEST) {
		while(getLightState(&light->base) != EAST_WEST){
			condWait(&light->stateCond, &light->stateLock);
		}
	} else if (car->position == NORTH || car->position == SOUTH) {
		while(getLightState(&light->base)!= NORTH_SOUTH){
			condWait(&light->stateCond, &light->stateLock);}
	}
	enterTrafficLight(car, &light->base);

	// Validate that there is no car going straight from opposite direction
	if (car->action == LEFT_TURN) {
		CarPosition opposite = getOppositePosition(car->position);
		while(getStraightCount(&light->base, opposite) > 0) {
			condWait(&light->leftTurnCond, &light->stateLock);
		}
	}
	actTrafficLight(car, &light->base, beforeSleep, afterSleep, &light->stateLock);
	if(car->action == STRAIGHT){
		condBroadcast(&light->leftTurnCond);
	}
	condBroadcast(&light->stateCond);
	unlock(&light->stateLock);

	
	// Exit intersection
	lock(&light->laneLock[laneIndex]);
	while(carIndex != light->exitCount[laneIndex]){
		condWait(&light->laneCond[laneIndex], &light->laneLock[laneIndex]);
	}
	exitIntersection(car, lane);
	light->exitCount[laneIndex]++;
	condBroadcast(&light->laneCond[laneIndex]);
	unlock(&light->laneLock[laneIndex]);

}

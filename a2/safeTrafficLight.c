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
static int can_go(Car* car, LightState state) {
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

void before_sleep(void* lock_ptr) {
	unlock((pthread_mutex_t*)lock);
}

void after_sleep(void* lock_ptr) {
	lock((pthread_mutex_t*) lock);
}

void initSafeTrafficLight(SafeTrafficLight* light, int horizontal, int vertical) {
	initTrafficLight(&light->base, horizontal, vertical);

	// TODO: Add any initialization logic you need.
	for (int i =0;i< TRAFFIC_LIGHT_LANE_COUNT;i++) {
		initMutex(&light->lane_lock[i]);
		initConditionVariable(&light->exit_cond[i]);
		light->enter_count[i] = 0;
		light->exit_count[i] = 0;
	}
	initMutex(&light->light_lock);
	initConditionVariable(&light->light_cond);
	initConditionVariable(&light->left_cond);
}

void destroySafeTrafficLight(SafeTrafficLight* light) {
	destroyTrafficLight(&light->base);

	// TODO: Add any logic you need to free data structures
	for (int i = 0; i< TRAFFIC_LIGHT_LANE_COUNT;i++) {
		initMutex(&light->lane_lock[i]);
		initConditionVariable(&light->exit_cond[i]);
	}
	mutex_destroy(&light->light_lock);
	cond_destroy(&light->light_cond);
	cond_destroy(&light->left_cond);
}

void runTrafficLightCar(Car* car, SafeTrafficLight* light) {

	// TODO: Add your synchronization logic to this function.
	int lane_index = getLaneIndexLight(car);
	lock(&light->lane_lock[lane_index]);
	EntryLane* lane = getLaneLight(car, &light->base);
	enterLane(car, lane);
	int index = light->enter_count[lane_index];
	light->enter_count[lane_index]++;
	unlock(&light->lane_lock[lane_index]);

	// Enter and act are separate calls because a car turning left can first
	// enter the intersection before it needs to check for oncoming traffic.
	lock(&light->light_lock);
	while (can_go(car, getLightState(&light->base)) == 0) {
		cond_wait(&light->light_cond, &light->light_lock);
	}
	enterTrafficLight(car, &light->base);
	if (car->action == STRAIGHT || car->action == RIGHT_TURN) {
		actTrafficLight(car, &light->base, before_sleep, after_sleep, 
		(void*)&light->light_lock);
		cond_broadcast(&light->left_cond);
	} else {
		while (getStraightCount(&light->base, get_oppo_pos(car)) != 0) {
			cond_wait(&light->left_cond, &light->light_lock);
		}
		actTrafficLight(car, &light->base, before_sleep, after_sleep
		, (void*)&light->light_lock);
	}
	cond_broadcast(&light->light_cond);
	unlock(&light->light_lock);

	lock(&light->lane_lock[lane_index]);
	while (light->exit_count[lane_index] != index) {
		cond_wait(&light->exit_cond[lane_index], &light->lane_lock[lane_index]);
	}
	exitIntersection(car, lane);
	light->exit_count[lane_index]++;
	cond_broadcast(&light->exit_cond[lane_index]);
	unlock(&light->lane_lock[lane_index]);

}

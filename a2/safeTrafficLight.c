/**
* CSC369 Assignment 2
*
* This is the source/implementation file for your safe traffic light 
* submission code.
*/
#include "safeTrafficLight.h"
#include "common.h"
#include "helpers.h"

/**
 * Return whether the car can go in the given light state.
 * Return 1 for okay, 0 otherwise,
 */ 
static int can_go(Car* car, LightState state) {
	if (car->position == NORTH || car ->position == SOUTH) {
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
/**
 * Return the opposite position of this car
 */  
static CarPosition get_oppo_pos(Car* car) {
	if (car->position == EAST) {
		return WEST;
	} else if (car->position == WEST) {
		return EAST;
	} else if (car->position == SOUTH) {
		return NORTH;
	} else {
		return SOUTH;
	}
}

void initSafeTrafficLight(SafeTrafficLight* light, int horizontal, int vertical) {
	initTrafficLight(&light->base, horizontal, vertical);

	// TODO: Add any initialization logic you need.
	for (int i=0; i<TRAFFIC_LIGHT_LANE_COUNT; i++) {
		light->enter_count[i] = 0;
		light->exit_count[i] = 0;
		initConditionVariable(&light->exit_cond[i]);
		initMutex(&light->lane_lock[i]);
		initMutex(&light->enter_lock[i]);
		initMutex(&light->move_lock[i]);
	}
	initConditionVariable(&light->left_cond1);
	initConditionVariable(&light->left_cond2);
	initConditionVariable(&light->light_cond);
	initMutex(&light->left_lock1);
	initMutex(&light->left_lock2);
}

void destroySafeTrafficLight(SafeTrafficLight* light) {
	destroyTrafficLight(&light->base);

	// TODO: Add any logic you need to free data structures
	for (int i=0; i<TRAFFIC_LIGHT_LANE_COUNT; i++) {
		cond_destroy(&light->exit_cond[i]);
		mutex_destroy(&light->lane_lock[i]);
		mutex_destroy(&light->enter_lock[i]);
		mutex_destroy(&light->move_lock[i]);
	}
	cond_destroy(&light->left_cond1);
	cond_destroy(&light->left_cond2);
	cond_destroy(&light->light_cond);
	mutex_destroy(&light->left_lock1);
	mutex_destroy(&light->left_lock2);
}

void runTrafficLightCar(Car* car, SafeTrafficLight* light) {

	// TODO: Add your synchronization logic to this function.
	int lane_index = getLaneIndexLight(car);
	lock(&light->lane_lock[lane_index]);
	int index = light->enter_count[lane_index];
	light->enter_count[lane_index]++;
	EntryLane* lane = getLaneLight(car, &light->base);
	enterLane(car, lane);
	unlock(&light->lane_lock[lane_index]);

	// Enter and act are separate calls because a car turning left can first
	// enter the intersection before it needs to check for oncoming traffic.
	lock(&light->enter_lock[lane_index]);
	while (can_go(car, getLightState(&light->base)) == 0) {
		cond_wait(&light->light_cond, &light->enter_lock[lane_index]);
	}
	enterTrafficLight(car, &light->base);
	unlock(&light->enter_lock[lane_index]);

	pthread_mutex_t *this_lock;
	pthread_cond_t *this_cond;
	if (car->action == RIGHT_TURN) {
		lock(&light->move_lock[lane_index]);
		actTrafficLight(car, &light->base, NULL, NULL, NULL);
		unlock(&light->move_lock[lane_index]);
	} else if (car->action == STRAIGHT){
		lock(&light->move_lock[lane_index]);
		actTrafficLight(car, &light->base, NULL, NULL, NULL);
		if (car->position == NORTH || car->position == EAST) {
			cond_broadcast(&light->left_cond2);
		} else {
			cond_broadcast(&light->left_cond1);
		}
		unlock(&light->move_lock[lane_index]);
	} else {
		if (car->position == NORTH || car->position == EAST) {
			this_lock = &light->left_lock1;
			this_cond = &light->left_cond1;
		} else {
			this_lock = &light->left_lock2;
			this_cond = &light->left_cond2;
		}
		lock(this_lock);
		CarPosition oppo_pos = get_oppo_pos(car);
		while (getStraightCount(&light->base, oppo_pos) != 0) {
			cond_wait(this_cond, this_lock);
		}
		actTrafficLight(car, &light->base, NULL, NULL, NULL);
		unlock(this_lock);
	}
	cond_broadcast(&light->light_cond);

	lock(&light->lane_lock[lane_index]);
	while (index != light->exit_count[lane_index]) {
		cond_wait(&light->exit_cond[lane_index], &light->lane_lock[lane_index]);
	}
	exitIntersection(car, lane);
	light->exit_count[lane_index]++;
	cond_broadcast(&light->exit_cond[lane_index]);
	unlock(&light->lane_lock[lane_index]);
}

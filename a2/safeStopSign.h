#pragma once
/**
* CSC369 Assignment 2
*
* This is the header file for your safe stop sign submission code.
*/
#include "car.h"
#include "stopSign.h"

/**
* @brief Structure that you can modify as part of your solution to implement
* proper synchronization for the stop sign intersection.
*
* This is basically a wrapper around StopSign, since you are not allowed to 
* modify or directly access members of StopSign.
*/
typedef struct _SafeStopSign {

	/**
	* @brief The underlying stop sign.
	*
	* You are not allowed to modify the underlying stop sign or directly
	* access its members. All interactions must be done through the functions
	* you have been provided.
	*/
	StopSign base;

	// TODO: Add any members you need for synchronization here.
	//Mutexes used to synchronize the cars entering the lanes
	pthread_mutex_t laneLock[DIRECTION_COUNT];
	//Mutexes used to synchronize modifying number of cars staying in a quadrant
	pthread_mutex_t	quadLock;

	pthread_cond_t quadCond;
	pthread_cond_t laneCond[DIRECTION_COUNT];

	// number of cars entered a lane
	int enterCount[DIRECTION_COUNT];
	//number of cars exited a lane
	int exitCount[DIRECTION_COUNT];
	// number of cars staying in a quadrant
	int quadCount[QUADRANT_COUNT];

} SafeStopSign;

/**
* @brief Initializes the safe stop sign.
*
* @param sign pointer to the instance of SafeStopSign to be initialized.
* @param carCount number of cars in the simulation.
*/
void initSafeStopSign(SafeStopSign* sign, int carCount);

/**
* @brief Destroys the safe stop sign.
*
* @param sign pointer to the instance of SafeStopSign to be freed
*/
void destroySafeStopSign(SafeStopSign* sign);

/**
* @brief Runs a car-thread in a stop-sign scenario.
*
* @param car pointer to the car.
* @param sign pointer to the stop sign intersection.
*/
void runStopSignCar(Car* car, SafeStopSign* sign);


/**
* @brief Checks if there are any cars moving through the quadrants.
*
* @param quadrants pointer the list of quadrants a car will move through.
* @param quadrantCount the length of quadrants.
* @param sign pointer to the stop sign a car will move through.
*/
int check_road_clear(int *quadrants, int quadrantCount, SafeStopSign* sign);

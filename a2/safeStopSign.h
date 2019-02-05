#pragma once
/**
* CSC369 Assignment 2
*
* This is the header file for your safe stop sign submission code.
*/
#include "car.h"
#include "stopSign.h"
#include <pthread.h>

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
	// Record whether the quadrant is occupied. 1 denote occupied, 0 otherwist.
	int occupy[QUADRANT_COUNT];
	//Mutex to protext the occupy array
	pthread_mutex_t occupyLock;
	//Mutexes used to synchronize the cars enter the lanes
	pthread_mutex_t laneLock[DIRECTION_COUNT];
	//Mutexes used to enter the stop sign from different direction
	pthread_mutex_t enterLock[DIRECTION_COUNT];
	pthread_cond_t waitCond;

	//Count the number of cars enter from every direction
	int enterCount[DIRECTION_COUNT];
	int exitCount[DIRECTION_COUNT];
	//Conditional variable for cars to exit stop sign in order
	pthread_cond_t exitCond[DIRECTION_COUNT];

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

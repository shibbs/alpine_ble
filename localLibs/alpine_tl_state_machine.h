//Michron TL State Machine .h

#ifndef ALPINE_TL_SM_H_
#define ALPINE_TL_SM_H_

#include "app_timer.h"
#include "alpine_includes.h"
#include "alpine_boards.h"



#define APP_TIMER_PRESCALER             8    /**< Value of the RTC1 PRESCALER register. Runs from 32.768KHz internal RC We want to run in units of .1ms, so use 32K /   = 10^4 Hz = 10^-4s */
#define TICKS_PER_MS										4		 /**< # ticks/ms based on the prescaler above. with 8 prescale it's ~4 ticks/ms. We need to also compensate for error inthe timer */


//definition of states
#define TAKING_PHOTO_STATE	1
#define TURNING_OFF_STATE 	2
#define FRONT_DELAY_STATE		3
#define MOVING_STATE				4
#define CHARGING_ONLY_STATE	5
#define PROCESSING_PACKET_STATE	6

//moving sub states
#define START_STEP_SUB	1
#define STEP_HOLD_SUB		2

//taking photo sub states
#define	SHUTTER_LED_SUB			0 //turn on the LED momentarily before taking photo
#define OPEN_SHUTTER_SUB		1
#define	WAITING_FOR_PC_SUB	2
#define	CLOSE_SHUTTER_SUB		3

//Event Type Definitions
#define NULL_EVT							0
#define TIMER1_EVT						1
#define	PC_SYNC_CHANGE_EVT		2
#define NEW_PACKET_EVT				3
#define	POWER_TOGGLE_EVT			4
#define TIMER2_EVT						5

//various timers
#define MIN_STEP_ON_MS			4 //minimum number of ms we can have as our step
#define NUM_STEPS_PER_DEGREE				57 //need to set this later
#define SHUTTER_LED_MS			50
#define MAX_MOVE_TO_SHUTTER_TIME_MS		2000	//max pause between motion and shutter
#define MIN_MOVE_TO_SHUTTER_TIME_MS		100		//minimum pause time
#define MIN_SHUTTER_TO_MOVE_TIME_MS		10		//minimum shutter time in ms
#define CW														1

#define EVENT_TIMER_TIME 		100 //how often, in ms, we should check the event queue. 

//various timeouts
#define PC_SYNC_TIMEOUT						100 // ms timeout while waiting for change on PC Sync line

void StartupStateMachine();//function called on startup, gets the SM rolling.


//these are out timer callback functions
void RegularTimerDone(void * nil);
void PeripheralTimerDone(void * nil);
void ProcessEvents(void* nil);

//App timer id's, need to be non-static so that main.c can access them. 
extern app_timer_id_t            				Regular_sm_timer;
extern app_timer_id_t            				Secondary_sm_timer;
extern app_timer_id_t										EventClearTimer; //called periodically to make sure we have cleared our event buffer

#endif //ALPINE_TL_SM_H_
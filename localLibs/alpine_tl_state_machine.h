//Michron TL State Machine .h

#ifndef ALPINE_TL_SM_H_
#define ALPINE_TL_SM_H_

#include "app_timer.h"
#include "alpine_includes.h"
#include "alpine_boards.h"



// This struct contains our event structures that can be passed. They contain the tpe of event, some event data, and the state from which this event was called
typedef struct Evt_struct {
	uint8_t event_type; 
	uint8_t last_state; 
	uint16_t val1; 
	uint8_t val2;
};




#define APP_TIMER_PRESCALER             2    /**< Value of the RTC1 PRESCALER register. Runs from 32.768KHz internal RC We want to run in units of .1ms, so use 32K /   = 10^4 Hz = 10^-4s */
#define TICKS_PER_MS										16		 /**< # ticks/ms based on the prescaler above. with 8 prescale it's ~4 ticks/ms. We need to also compensate for error inthe timer */


//definition of states
#define TAKING_PHOTO_STATE	1
#define TURNING_OFF_STATE 	2
#define FRONT_DELAY_STATE		3
#define MOVING_STATE				4
#define CHARGING_ONLY_STATE	5
#define PROCESSING_PACKET_STATE	6
#define REMOTE_CNTRL_STATE	7	//under control from smartphone, not executing a TL

//moving sub states
#define START_STEP_SUB	1
#define STEP_HOLD_SUB		2

//taking photo sub states
#define	SHUTTER_LED_SUB			0 //turn on the LED momentarily before taking photo
#define OPEN_SHUTTER_SUB		1
#define	WAITING_FOR_PC_SUB	2
#define	CLOSE_SHUTTER_SUB		3
#define EXTERN_EVT_STATE		4  //used for when the event was called from outside of the state machine

//Event Type Definitions
#define NULL_EVT							0
#define TIMER1_EVT						1
#define	PC_SYNC_CHANGE_EVT		2
#define NEW_TL_PACKET_EVT				3
#define	POWER_TOGGLE_EVT			4
#define TIMER2_EVT						5
#define SHUTTER_CMD_EVT				6

#define GOOD_PKT							1
#define BAD_PKT								2

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



//Outward-facing SM utilities
void StartupStateMachine();//function called on startup, gets the SM rolling.
bool Tl_pkt_is_good(uint8_t * tl_pkt_in); //checks if a tl packet is good
void AddEventToTlSmQueue_extern( uint8_t event_type, uint16_t data1, uint8_t data2);
void UpdateCurrentTlPacket( uint8_t* new_pkt, uint8_t length);



//these are out timer callback functions
void RegularTimerDone(void * nil);
void PeripheralTimerDone(void * nil);
void ProcessEvents(void* nil);


//App timer id's, need to be non-static so that main.c can access them. 
extern app_timer_id_t            				Regular_sm_timer;
extern app_timer_id_t            				Secondary_sm_timer;
extern app_timer_id_t										EventClearTimer; //called periodically to make sure we have cleared our event buffer

#endif //ALPINE_TL_SM_H_
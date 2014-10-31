//
//  Radian BLE Pseudo Code 2-0.c
//  
//
//  Created by stephen hibbs on 8/15/14.
//
//
#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include <stdio.h>
#include "alpine_tl_state_machine.h"
#include "alpine_includes.h"
#include "alpine_boards.h"
#include "app_state_machine.h"

#define EVENT_QUEUE_SIZE	10


//start us out with a null queue, except for a power toggle event to kick us off
unsigned char Event_queue [EVENT_QUEUE_SIZE] = { POWER_TOGGLE_EVT,NULL_EVT,NULL_EVT,NULL_EVT,NULL_EVT,NULL_EVT,NULL_EVT,NULL_EVT,NULL_EVT,NULL_EVT};
unsigned char Event_q_index=0;

unsigned char Current_packet [TL_PACKET_MAX_LEN];

//state variables
unsigned char Curr_state = PROCESSING_PACKET_STATE ;//state machine current state
unsigned char Next_state; //state machine next state, usually not used
unsigned char Curr_p_state; //peripheral state machine current state
unsigned char Next_p_state; //peripheral state machine next state, usually not used

//Time-lapse packet front end variables
unsigned char Num_timelapses = 1;
unsigned char Preload_motion1 = 60; //units of .1 degrees
unsigned char Preload_motion2 = 80; 
bool Execute_on_start=true;
bool Looping = false;
bool Starting_up =true;

//time-lapse cycle-specific variables
/*
For Michron, Shutter_on_time_ms, and move_to_shutter_time_ms are the only relevant variables
For Radian, the group of variables below are all important
*/
long Shutter_on_time_ms = 100; //ms shutter is on for
long Shutter_to_move_time_ms = 1000; //ms between shutter closing and motion
long Step_on_time_ms = 4;//ms that motor is in on value after stepping
long Step_idle_time_ms = 0; //ms that motor is in idle value between steps
long Move_to_shutter_time_ms = 1000; //ms between moving ending and shutter starting
unsigned int Num_steps_per_move = 5; //number of motor steps to take per motionsfron
long Set_cycle_time_ms = 2000; //cycle time in ms

//motor variables
char Direction = CW; //either 1 or -1, needs to be signed
unsigned char Drive_duty = 100; //duty cycle when stepping
unsigned char Idle_duty = 0; //duty cycle while idling

//non-standard tracking variables
bool	Bramping_on = false;
bool	Sramping_on = false;
bool 	Iramping_on = false;
bool	Pc_sync_on = false;
bool	Hdr_on			= false;	
bool	Usb_cntrl_on	= false; //flag for if we're doing usb control of camera

//time-lapse tracking variables
unsigned long Num_photos_to_take = 100;
unsigned long Num_photos_taken= 0 ;

//time lapse variables
long Front_delay_time_ms = 0;

//more temporary variables
unsigned char Preload_flag = 0;

//App timer id's, declared as externs in alpine_tl_State_machine.h need to be non-static so that main.c can access them. 
app_timer_id_t							Regular_sm_timer;
app_timer_id_t							Secondary_sm_timer;
app_timer_id_t										EventClearTimer; //called periodically to make sure we have cleared our event buffer

//function declarations
static void InitForNewTL(void);
static void AddEventToQueue( char event);
static void HandleStateMachineEvent( char event);


/* ------------ TIMERS -------------------------------*/


//regular timer callback functtion
 void RegularTimerDone(void * nil){
	AddEventToQueue(TIMER1_EVT);
	
}
//peripheral timer callback functtion
void PeripheralTimerDone(void * nil){
	AddEventToQueue(TIMER2_EVT);
}

/*This is the timer function
Needs to be able to hanle huge #s, long may not be enough
if it gets a 0, should just fire event right away
*/
static void SetTimer(unsigned long time){
	uint32_t err_code;
	if(time == 0){
		AddEventToQueue(TIMER1_EVT);
		return;
	}
	//we will want to add some code to correct for the slight error in timing. Currently we assume a 32Khz clock, but really it's 32.768KHz
	//need to error check for overly-long times and dice up time chunks based on this. 
	err_code = app_timer_start(Regular_sm_timer, time * TICKS_PER_MS, NULL); //set the main sm timer. Boost up the time so that it converts to ticks from ms
	APP_ERROR_CHECK(err_code);
}

/*This is the timer function
Needs to be able to hanle huge #s, long may not be enough
if it gets a 0, should just fire event right away
*/
static void SetPeripheralTimer(unsigned long time){
	if(time == 0){
		AddEventToQueue(TIMER1_EVT);
		return;
	}
}

/* -----------PACKET PROCESSING -----------------------*/

//handles the bytes at the start of the packet, that apply to all TLs in packet
static void ParsePacketPreamble(){
	
}

// processes time-lapse settings for a specific TL
static void ProcessTLSettings(){
	//process settings for this timelapse
	InitForNewTL(); //reset timelapse variables for startup
}

//initialize variables relevant to a new timelapse being run
static void InitForNewTL(){
	
	
}


/* -------------------------Cycle Processing ----------------------*/

/**
	This will initialize for our new timelapse, getting us ready for the start
*/
static void InitNewTL(){
	//ProcessBrampSettings();
	//ProcessIrampSettings();
	Num_photos_taken = 0;
}

//takes care of the preload settings
static void HandlePreloadSettings(){
	static unsigned char step_on_carbon;
	static unsigned char step_idle_carbon;
	static unsigned int num_steps;
	if(Preload_flag ==1){
		//grab carbons of our set TL values
		step_on_carbon = Step_on_time_ms;
		step_idle_carbon = Step_idle_time_ms;
		num_steps = Num_steps_per_move;
		//set us up to do the first preload motion
		Step_on_time_ms = MIN_STEP_ON_MS;
		Step_idle_time_ms = 0;
	}else if(Preload_flag ==2){ //else we are starting second run
		
	}else{ //for value of 3 we are done and should reset vals
		Preload_flag = 0;
		//reset the carboned values
		Step_on_time_ms = step_on_carbon;
		Step_idle_time_ms = step_idle_carbon;
		Num_steps_per_move = num_steps;
	}
	
}

//comes in and updates the cycle settings between cycles
static void UpdateCycleSettings(){
	static unsigned long cycle_time_ms = 1000;
	static unsigned long step_time_ms = 0;
	static long temp = 0;//general temp value need to make sure this doesn't limit our dynamic range
	cycle_time_ms = Set_cycle_time_ms; //we want this to account for error later on FLAG SAH
	if(Preload_flag !=0){
		HandlePreloadSettings();
		return;
	}
	
	Num_photos_taken ++; 
	// use > since we start with 1 before anything is taken
	if(Num_photos_taken > Num_photos_to_take){ //check if we're done with this TL
		//shut us down or move us to next TL in the queue
	}
	
	//Shutter_on_time_ms = UpdateBramp();
	//Set_cycle_time_ms = UpdateIramp();
	//something = UpdateHdr(Shutter_on_time_ms);
	//Num_steps_per_shot = UpdateSramp();
	
	//Re-evaluate cycle based on settings and error
	
	//constraints to this are: cycle_time_ms, Shutter_on_time_ms, Step_idle_time_ms, Num_steps_per_move, SHUTTER_LED_MS, 
	step_time_ms = Num_steps_per_move *( Step_idle_time_ms + Step_on_time_ms); //compute time spent moving
	temp = (Shutter_on_time_ms + step_time_ms + SHUTTER_LED_MS );
	if(temp > cycle_time_ms ) Move_to_shutter_time_ms = MIN_MOVE_TO_SHUTTER_TIME_MS; //if we don't have enough time, need to use the min value
	else Move_to_shutter_time_ms = cycle_time_ms - temp;
	
	if(Move_to_shutter_time_ms > MAX_MOVE_TO_SHUTTER_TIME_MS) Move_to_shutter_time_ms = MAX_MOVE_TO_SHUTTER_TIME_MS; //if time is too long, cap it
	//now compute time between shutter and motion. This is lowest importance variable
	//we need to ensure this doesn't go negative. 
	temp += Move_to_shutter_time_ms; 
	if(temp > cycle_time_ms) Shutter_to_move_time_ms = MIN_SHUTTER_TO_MOVE_TIME_MS;
	else Shutter_to_move_time_ms = cycle_time_ms - temp ;
}


/*
peripheral state machine handles non-TL things like : 
battery LED status, connection made status, display upload success
Events we can expect are TIMER2_EVT , GOOD_PACKET_EVT , BLE_CONN_EVT
*/
static void HandlePeripheralEvent( char event);



/*
state machine handler for the processing packet state
we  get here  after power toggled on, or when new packet comes in. 
we want to either go to sleep if we are not suppsed to execute on start or we should
process the current timelapse settings, and set up the front delay timer and go into front delay state
*/
static void ProcessingPacketState(char event){
	//run basic check on packet
	//if not startup then save to EEprom
	if(event != NEW_PACKET_EVT && event != POWER_TOGGLE_EVT) return;
	
	if(Current_packet[0] == TL_PACKET_START_FLAG) { // if we seem to have a valid packet : 
		ParsePacketPreamble(); //parse the values at the front of the packet
		if(Starting_up && !Execute_on_start){ //if we should not execute on startup, go to sleep
			Curr_state = TURNING_OFF_STATE;
			SetTimer(0);
			return;
		}
		ProcessTLSettings();
	}
	Starting_up = false;
	Curr_state = FRONT_DELAY_STATE;
	//set up front delay timer
	SetTimer( Front_delay_time_ms );
	
}

//we get here after waiting the front delay time, at the start of a new TL
//we need to update the cycle settings, and move us to the take a photo state
static void FrontDelayState(char event){
	Curr_state = TAKING_PHOTO_STATE;
	Preload_flag = 0; //preload set off
	//FLAG SAH removed below line since currently the preload logic is all wrong
//	if(!MICHRON) Preload_flag ++;//set preload flag to 1 if we're not a michron
	if(1!=1) ; 
	else InitNewTL();

	
	UpdateCycleSettings(); //update cycle settings
	SetTimer(0);//set zero timer so we get right to it
}


//this handles what to do after taking a photo
static void HandleShutterDone(){
	//if we are supposed to take mulptiple photos, handle the math here and update Shutter_open_time_ms as appropriate
	
	//if we are in Radian mode and are done taking photos, move us to moving state
	if(!MICHRON){
		Curr_state = MOVING_STATE;
		SetTimer(Shutter_to_move_time_ms);
	}
	else{//if we are in Michron mode and are done taking photos, send us back to taking photo state
		Curr_state = TAKING_PHOTO_STATE;
		UpdateCycleSettings();
		SetTimer(Move_to_shutter_time_ms);
	}
}

/*
This is the state in which we control the taking of a photo.

*/
static void TakingPhotoState(char event){
	//this is our substate which will track where we are in this state. When we are done we should always revert it to OPEN_SHUTTER_SUB
	static char sub_state = SHUTTER_LED_SUB;

	//check if this event is relevant first, if not then exit out
	if(event != TIMER1_EVT && event != PC_SYNC_CHANGE_EVT) return;
			
	if( sub_state == SHUTTER_LED_SUB){
		G_STAT_LED_ON;
		sub_state = OPEN_SHUTTER_SUB;//send us back to first sub at end
		SetTimer(SHUTTER_LED_MS);
	}else if(sub_state == OPEN_SHUTTER_SUB){
		OPEN_SHUTTER_MACRO //open the shutter
	
		if(Pc_sync_on){//set up PC sync port interrupt
			SetTimer(PC_SYNC_TIMEOUT); //set up the timeout for PC Sync port
			sub_state = WAITING_FOR_PC_SUB;
		}else{
			G_STAT_LED_OFF;
			SetTimer(Shutter_on_time_ms); //otherwise just set the shutter time
			sub_state = CLOSE_SHUTTER_SUB;
		}
	 
	}else if(sub_state == WAITING_FOR_PC_SUB){ //whether due to timeout or PC Sync change we now start the shutter timer
		G_STAT_LED_OFF;
		SetTimer(Shutter_on_time_ms); 
	}else{//else it's CLOSE_SHUTTER_SUB
		CLOSE_SHUTTER_MACRO
		sub_state = SHUTTER_LED_SUB;//send us back to first sub at end
		HandleShutterDone();
	}
}

//handler for moving state
/*
	This state operates in the following fashion : 
	The start step sub state turns the motor on to full, and sets a timer for the amount of time to keep the motor on full
	The hold step sub state turns the motor to idle for the remainder of ther step time. If we are done with steps the next sub is Exit sub, else we go back to start step sub
*/
static void MovingState(char event){
	static char sub_state = START_STEP_SUB; //start us with start step
	static char num_steps_taken = 0;
	
	//check if this event is relevant first, if not then exit out
	if(event != TIMER1_EVT) return;
  
	if(sub_state == START_STEP_SUB){
		SetStepperPWM(Drive_duty);//set motor pwm for stepping
		Step(Direction);//step motor
		sub_state = STEP_HOLD_SUB;
		SetTimer(Step_on_time_ms);
	}else if (sub_state == STEP_HOLD_SUB){ //set the motor to idle
		SetStepperPWM(Idle_duty);//set motor to idle pwm
		sub_state = START_STEP_SUB; //send us back to start step
		num_steps_taken ++;//increment num steps taken
		
		//check if we are done and should exit out
		if(num_steps_taken >= Num_steps_per_move) {
			num_steps_taken = 0;
			Curr_state = TAKING_PHOTO_STATE;
			SetTimer(Move_to_shutter_time_ms);
			UpdateCycleSettings();
		}else {
			SetTimer( Step_idle_time_ms );
		}
	}//end sub_state if/else
	
}//end MovingState()

/* -------------STATE MACHINE HANDLING-------------------------------*/


static void AddEventToQueue( char event){
	static 	void * nil; //needed for calling processEvents
	if(Event_queue[Event_q_index] != NULL_EVT) Event_q_index ++; //check for if we're on 0th val and it's null
	Event_queue[Event_q_index] = event;
	ProcessEvents( nil);
}

/*
handles regular state machine events.
Events we can expect are : NEW_PACKET_EVT , TIMER1_EVT , PC_SYNC_EVT , POWER_TOGGLE_EVT
will just be a large switch statement that calls handler functions
*/
static void HandleStateMachineEvent( char event){
	
	if(Curr_state == TURNING_OFF_STATE){
		//TurningOffState(event);
	}else if(Curr_state == TAKING_PHOTO_STATE){
		TakingPhotoState(event);
	}else if(Curr_state == FRONT_DELAY_STATE ){
		FrontDelayState(event);
	}else if(Curr_state == MOVING_STATE ){
		MovingState(event);
	}else if(Curr_state == CHARGING_ONLY_STATE ){
		//ChargingSate(event)
	}else if(Curr_state == PROCESSING_PACKET_STATE){
		ProcessingPacketState(event);
	}
	
}

/*
	Called periodically to push out any unhandled events from our FIFO queue
*/
void ProcessEvents(void* nil){
	char event; 
	static char i;
	
	event = Event_queue[0]; //get the event
	//move through the queue and slide everything down
	for( i = 0; i < Event_q_index; i++){
			Event_queue[i] = Event_queue[i+1];
	}
	Event_queue[Event_q_index] = NULL_EVT; //set this slot to now be a null event to be sure we clear it
	if(Event_q_index > 0) Event_q_index--;
	if(event ==NULL_EVT) return; //if our event is null then exit out
	HandleStateMachineEvent( event );
}

/* --------------------------STARTUP  ----------------------------------*/

//Loads the Values from EEPROM
static void GetEepromValues(){
	
	
}

//First fucntion called, gets our state machine started off on power up
void StartupStateMachine(){
	GetEepromValues();
	Curr_state = PROCESSING_PACKET_STATE;
	AddEventToQueue( POWER_TOGGLE_EVT ); //adds power toggle event to the queue
}












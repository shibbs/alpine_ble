//
//  Radian_bramp.cpp
//  
//
//  Created by Stephen Hibbs on 9/27/12.
//  Copyright (c) 2012 Stanford. All rights reserved.
//
/*

 
 
 */

#include <../RadianIncludes/RadianIncludes.h>
#include <radian_bramp.h>

#define SPEED_UP  false
#define DEBUGB false
#define DEBUGB2 false

#define MAX_NUM_BRAMPS 4
#define BRAMP_ON    2
#define BRAMP_OFF   0
#define OVERRIDE  false
#define OVERRIDE_STARTMS    1

int BrampStatus = BRAMP_ON; //1 for bramp
unsigned long CurrentShutterMs;
float StartShutterMs;
float InterpPow;
float ExponentialTimeSeconds; //in seconds
unsigned int BrampDurationMinutes; //total time in minutes that the timelapse will run for
float FrontDelaySeconds; 
unsigned long BrampStartTime = 0;

Radian_bramp::Radian_bramp(int dummy){
    int x = dummy*2;

}

/*
 takes in the array of values from the total input packet, those associated with the bramping start at BRAMPING_UNIT_START_BYTE
 
 shutter time ( time elapsed )  = original shutter time *
     [  2 ^ (  f-stop# * ( time elapsed / step increase time)) ]
 byte 0 : bramping on/off 0 or 1 is off, 2 + is on
 byte 1 : exponential power value, it's signed and sent in .1 sec increments so  powerVal = ( valIn - MaxVal/2 ) / 10
 byte 2 : Doubling/tripling Time in minutes or steps. Formatted as MAX_PACKET_VALUE/2 + (number of minutes (if negative) or frames (if positive))
 byte 3 : Total Time over which to Bramp in 1 minute intervals
 byte 4 : Front Delay Time 5 minute intervals
 */
void Radian_bramp::SetBrampSettings(byte* valuesIn,unsigned  int intervalSeconds, unsigned long startShutterMs){
    BrampStartTime = millis(); 
    BrampStatus = valuesIn[0+BRAMPING_UNIT_START_BYTE];
    StartShutterMs = startShutterMs;
    //sets starting shuttervalue, it's set in 1ms intervals
     
     CurrentShutterMs = StartShutterMs;
     if(OVERRIDE ){
         StartShutterMs = OVERRIDE_STARTMS;
         CurrentShutterMs = OVERRIDE_STARTMS; //start us off at exposure
    }
    if(DEBUGB) Serial.println(" startup currentShutterms:");
    if(DEBUGB) Serial.println(CurrentShutterMs);
     
     //if bramp turned off, just leave currentT as the minTime value
     if(BrampStatus >= BRAMP_ON){
         InterpPow = ((float)(valuesIn[1+BRAMPING_UNIT_START_BYTE] - (float)(MAX_PACKET_VAL/2 ))) /10.0;
         if(DEBUGB) { Serial.print("Bramp Interp Power : ");Serial.println(InterpPow);  }
         ExponentialTimeSeconds = valuesIn[2+BRAMPING_UNIT_START_BYTE];//grab the value now, parse it next
         if(ExponentialTimeSeconds < MAX_PACKET_VAL/2){ //check if we're doing it with respect to x steps or x time
             //figure out number of seconds over which to increase
             ExponentialTimeSeconds  = (MAX_PACKET_VAL/2 -ExponentialTimeSeconds)*60;
         }else {
            // figure out the time associated with increase every x steps
             ExponentialTimeSeconds  = ( ExponentialTimeSeconds - MAX_PACKET_VAL/2 ) * intervalSeconds; 
         } //end exponential time if statement
             
             FrontDelaySeconds = 300*valuesIn[4+BRAMPING_UNIT_START_BYTE];
             BrampDurationMinutes = FrontDelaySeconds/60 + valuesIn[3+BRAMPING_UNIT_START_BYTE];
             if(DEBUGB){
                 Serial.print("Bramp Duration Minutes : "); Serial.println(BrampDurationMinutes);
                 Serial.print("Front Delay Seconds : "); Serial.println(FrontDelaySeconds);
            }
        }
   
}
                     
/*
update the settings for the shutter time. Takes in a reference to the shutterTime
vaiable, so that it can update the value of the variable 
If the 
             
*/
unsigned long Radian_bramp::UpdateBulbSettings(unsigned long maxShutterT){
    unsigned long rtn = CurrentShutterMs;
    unsigned long currTimeSec;
    
    if(!SPEED_UP)  currTimeSec = ( millis()-BrampStartTime )/(long)1000;
    //speed things up by 10x
    else currTimeSec = ( millis()-BrampStartTime )/(long)100;
    if(DEBUGB2) Serial.print("time(s) : "); 
    if(DEBUGB2) Serial.println(currTimeSec);
    
    if(currTimeSec/60 >= BrampDurationMinutes ){ //if we are done bramping, just sit there
        if(DEBUGB) Serial.println("Done Bramping");
        rtn = CurrentShutterMs;
    }
    else if(currTimeSec > FrontDelaySeconds){ //if we should be bramping, compute the shutter time
        if(DEBUGB2)Serial.println("FrontDelayDone");
        rtn= (unsigned long) ( StartShutterMs * pow(2, InterpPow * (currTimeSec -FrontDelaySeconds ) / ExponentialTimeSeconds  ) ) ;
    }else{
        if(DEBUGB) Serial.println("Delay On still");
    }
    
    if( rtn >= maxShutterT){
       rtn = maxShutterT; 
        if(DEBUGB) Serial.println("M");
    }
    
    CurrentShutterMs = rtn;
    if(DEBUGB) Serial.print("shutter : ");
    if(DEBUGB) Serial.println(rtn);
    return rtn; 
    
}
//
//  Radian_sramp.cpp
//  
//
//  Created by Stephen Hibbs on 9/27/12.
//  Copyright (c) 2012 Stanford. All rights reserved.
//
/*

 9/2/2014 major revision made to add virtual point from app to improve Sramp performance. Also changed it so that we include NumSrampVals as the last point in the array, not NumSrampVals-1
 
 */

#define GEARBOX     75
#include <../RadianIncludes/RadianIncludes.h>
#include <radian_sramp.h>
#include <math.h>

#define SPEED_UP  false  //speed it up 10x
#define DEBUG       false
#define DEBUG2      false //controls output of updated angle

#define SRAMP_ON    1
#define SRAMP_OFF   0
#define OVERRIDE  false
#define MAX_NUM_SRAMP_POINTS          7 //n, number of timelapses max will be less in reality, need to add 1 for 0,0 val at start
#define MONOTONIC_CUBIC_SPLINE  1
#define LINEAR    0

int SrampStatus = SRAMP_ON; //1 for sramp
int CurrentAngle =0;
int SrampInterpolationType = MONOTONIC_CUBIC_SPLINE; 
float m[MAX_NUM_SRAMP_POINTS] ;//= new float[];
long startTime = 0; //marks when the Sramp started, in seconds
int ref; float temp;int NumSrampVals = 0;



//This is the vector to simulate the user-generated points. Feel free to
//screw around with this to get an idea of what to expect. Goes to hell if
//you have x values that aren't always increasing
unsigned int angleArr[MAX_NUM_SRAMP_POINTS] ={ 0,  10,   30,    40,  50, 60 };  //angluar values of points relative to start angle of 0
unsigned int timeArr[MAX_NUM_SRAMP_POINTS] = { 0 , 10, 30 , 40 ,  70 , 80  };  //time in seconds for points to occur relative to start time

Radian_sramp::Radian_sramp(int dummy){
    int x = dummy*2;
    
}
/*
 creates the array of values associated with the sramping
 we need the angular measurements to be "signed" so we'll use CCW as negative and CW as positive
 byte 0 : sramping number of sramp packet bytes, including that one 0 means sramp is off
 byte 1 : sramp interpolation type. 0 is linear, 1 is Monotonic cubic spline interpoation
 it is assumed that all sramp settings start a 0 degrees, 0 seconds
 byte 2 : sramp angle 1 in 5 degree increments. This needs to be an unsigned value relative to the last value
 byte 3 : sramp point 1, time from start in 5 minute increments, max value is 230. 
 Each even byte is the sramp angle relatvie to the start angle of 0
 each odd byte is the time value, relative to the last
 */
//returns the number of seconds that the timelapse will run for
long Radian_sramp::SetSrampSettings(byte* valuesIn, unsigned long durationSeconds,unsigned int angle ){
    if(valuesIn[SRAMPING_UNIT_START_BYTE] == 0){
        SrampStatus = SRAMP_OFF;   
        return 0;
    }
    
    static int offset = MAX_PACKET_VAL/2;
    NumSrampVals = ( valuesIn[SRAMPING_UNIT_START_BYTE ]-2) /2 ;//figure out how many SRAMP points have been set
    SrampInterpolationType = valuesIn[SRAMPING_UNIT_START_BYTE+1];//the type of interpolation to perform
    if(DEBUG){ Serial.print("Number of Sramp Values : "); Serial.println(NumSrampVals);}
    int totalAngle=0;
    unsigned long totalTime = 0;
    int srampIndex = 0;
    int arrIndex = 0;
    startTime = millis()/1000; // mark the start of the bramping
    for(int i = SRAMPING_UNIT_START_BYTE+2 ; i < (SRAMPING_UNIT_START_BYTE+valuesIn[SRAMPING_UNIT_START_BYTE ]);i++ )
    {
       if(DEBUG)Serial.println(valuesIn[i]);
        if(srampIndex%2 ==0){ //even values are angle in 5 degree increments relative to last value
            arrIndex ++; 
            totalAngle += (valuesIn[ i ] )*5; //values are relative to 0
            angleArr[arrIndex] = totalAngle;
        //    if(DEBUG)Serial.println(totalAngle);
        }else{//odd values are time in minutes in 5 minute increments relative to the last time
            totalTime += valuesIn[i]*5;
            if(DEBUG) Serial.println(totalTime);
            timeArr[arrIndex] = totalTime*60; //timeArr stored in seconds
         //   if(DEBUG)Serial.println(totalTime);
            
        }
        srampIndex++;
        //if(DEBUG)Serial.println(arrIndex);
                                                                            
    }
    //set the new last point which is a virtual pointin the app. This point is mirored about the set time and angle
    //so the new last point is the duration + (duration - sent last) = 2*duration - sent last
    //it is simlar for the angle
    arrIndex++;
    timeArr[arrIndex] = 2*durationSeconds - timeArr[NumSrampVals];
    angleArr[arrIndex] =  2*angle - angleArr[NumSrampVals];
    arrIndex++;
    NumSrampVals ++ ; //this is important since we just added a new point
    
    //fill in the rest of the values with the final value so that the array finishes properly.
    for( arrIndex; arrIndex < MAX_NUM_SRAMP_POINTS; arrIndex++){
        timeArr[arrIndex] = timeArr[ NumSrampVals]; //will copy previous value
        angleArr[arrIndex] =  angleArr[NumSrampVals];
    }
    if(DEBUG){
        Serial.print("timeArr : ");
        for(int i = 0; i < MAX_NUM_SRAMP_POINTS; i++){
            Serial.print(timeArr[i]);
            Serial.print(", ");
        }
        Serial.println(" " );
        Serial.print("angleArr : ");
        for(int i = 0; i < MAX_NUM_SRAMP_POINTS; i++){
            Serial.print(angleArr[i]);
            Serial.print(", ");
        }
    }
    
    //do the monotonic stuff if that's our interpolation type
    if(SrampInterpolationType ==MONOTONIC_CUBIC_SPLINE )MonotonicCubicSplineInit();
    
    if(DEBUG) {Serial.print("Sramp Time in Sramp.cpp :  "); Serial.println(timeArr[NumSrampVals]);}
    if(SPEED_UP) return timeArr[NumSrampVals]/10; //we want to take fewe steps when sped up
    else return (long) timeArr[NumSrampVals]; //return the max time value in seconds
}

//returns a value <0 when the Sramp is done. 
//returns the nubmer of stapes to take and sign gives what direction
int Radian_sramp::UpdateSrampValue(){
    
    static float lastAngle = 0;
    static float numSteps = 0; //changed to static so that when we are at end of tl we can just return same value repeatedly
    
    float currentAngle;
    if(SrampInterpolationType == LINEAR){
        currentAngle = PerformLinearInterpolation();
        if(DEBUG2) Serial.println("Linearf interp");
    }
    else{
        currentAngle = PerformCubicSplineInterpolation(); //get the angle we should move to
        if(DEBUG2) Serial.println("Monotonic interp");
    }

    if(DEBUG2){ Serial.print("currAngle : ");Serial.println(currentAngle);}

    //new as og 1.02
    if(currentAngle == 0) return 0; //if we got 0 back, that means we are stopped at the end
    else if (currentAngle == -1) return numSteps; //if it's -1 then we're off the end and need to just keep returning the same value
    
    numSteps = currentAngle-lastAngle;//compute angular difference
    if(numSteps < 0) numSteps = 0; 
    numSteps = round(numSteps * (float)STEPS_PER_DEGREE ) ;//compute number of steps to take based on that
    if(DEBUG){ Serial.print("numSteps : "); Serial.println(numSteps);}
    
    lastAngle =currentAngle;//update lastAngle
    
    return (int)numSteps;
}
/*
 
             
*/

//// now perform the interpolations stage
//returns -1 if we are off the end of our Sramp and should not stop at end
//reurns 0 if we are off the end of our Sramp and should stop at end
float Radian_sramp::PerformCubicSplineInterpolation(){
    int index = -1;
    static double t =0;
    static float h[4];
    float currTime = ( (float)millis()/1000 ) -(float)startTime;
    if(SPEED_UP) currTime*=10; //speed it up 10x
    
    //check if we are past the last point. If we are, and the last 2 time vals are the same (indicating stop at end) then return 0
    //else return -1
    if(currTime > timeArr[NumSrampVals]){
        if(timeArr[NumSrampVals-1] == timeArr[NumSrampVals])
            return 0;
        else
            return -1;
        
    }
    
    static int i=0;
    while(currTime > timeArr[i] && i < NumSrampVals) i++; //figure out what index we're on
    i--; //go back one so we fall into the correct bracket
    if(i<0) i=0;
    if(DEBUG) { 
        Serial.print("T ");Serial.println(currTime); 
        Serial.print("index "); Serial.println(i);
        Serial.print("m : ");   Serial.println(m[i]);
        Serial.print("x : ");   Serial.println(timeArr[i]);
        Serial.print("y : ");   Serial.println(angleArr[i]);
    }
    
    t = (currTime-(float)timeArr[i])/( (float)(timeArr[i+1]-timeArr[i])); //this is our fractional distance between the two points
    if(DEBUG) { Serial.print("t "); Serial.println(t); }
    //calcuate the h values on the fly
    h[ 0] =  2.0* pow(t,3) - 3.0* pow(t,2) + 1.0;
    h[ 1] =  pow(t,3) - 2.0* pow ( t,2) + t;
    h[ 2] =  -h[ 0] + 1.0 ;
    h[3] =  pow(t,3) - pow( t,2);
   /* 
    if(DEBUG) {
        Serial.println(h[0]);
        Serial.println(h[1]);
        Serial.println(h[2]);
        Serial.println(h[3]);
    }*/
    //calculate the interpolated y value, done in2 lines just for
    //readability
    float p = ( h[ 0 ]) * ((float)angleArr[i]) + ( (float) ( timeArr[i+1] - timeArr[i]  ) )*( h[1] ) *m[i] ;
    if(DEBUG) Serial.println(p);
    p = p  + (h[2] )*((float)angleArr[i+1]) +  (h[3] )*( ((float) timeArr[i+1] - timeArr[i])  )*m[i+1] ;
  //  if(DEBUG) Serial.println(p);
    return p;
}

//// Performs the linear interpolation
float Radian_sramp::PerformLinearInterpolation(){
    int index = -1;
    static double t =0;
    float currTime = ( (float)millis()/1000 ) -(float)startTime;
    if(SPEED_UP) currTime*=10; //speed it up 10x
    static int i=0;
    while(currTime > timeArr[i] && i < NumSrampVals) i++; //figure out what index we're on
    i--; //go back one so we fall into the correct bracket
    if(i<0) i=0;
    if(DEBUG) { 
        Serial.print("T ");Serial.println(currTime); 
        Serial.print("index "); Serial.println(i);
        Serial.print("x : ");   Serial.println(timeArr[i]);
        Serial.print("y : ");   Serial.println(angleArr[i]);
    }
    
    t = (currTime-(float)timeArr[i])/( (float)(timeArr[i+1]-timeArr[i])); //this is our fractional distance between the two points
    if(DEBUG) { Serial.print("t "); Serial.println(t); }
    
    //perform a linear interpolations of so that angle = angle pnt + fraction between * (next angle point - angle pnt)
    float p = ((float)angleArr[i]) + ((float)(angleArr[i+1] - angleArr[i])) *t   ;
    return p;
}


// /*
// This function initializes the m array that is later used for the cubic spline
// the m array is set up here to ensure monotonicity in the cubic spline later
// */
void Radian_sramp::MonotonicCubicSplineInit( ){
    float alpha1[MAX_NUM_SRAMP_POINTS];// = new float[n];
    float beta[MAX_NUM_SRAMP_POINTS];// = new float[n];
    //  multiplier = 10;
    float delta[MAX_NUM_SRAMP_POINTS] ;//= new float[n];
    float xdiff;
    ref = NumSrampVals; int i;
    //set up and populate the m array
    for ( i = 0 ; i < ref; i++) {
        xdiff = ((float) ( timeArr[i + 1] - timeArr[i] ));
        if( xdiff ==0) delta[i] = 0;
        else delta[i] = ((float) (angleArr[i + 1] - angleArr[i]) ) / (xdiff );
        //println(delta[i]);
        if ( i > 0) //i=1 is an edge case error, but edges are set below
            m[i] = ((delta[i - 1] + delta[i])) / 2.0;  //m array used in cubic spline interpolation
        
    } //end for loop over i
    
    //set the first and last values of the m array
    m[0] = delta[0];
    m[ref ] = delta[ref-1];
    
    
    //find all deltas of value 0 and set the m values to 0 accordingly, this
    //marks flat sections of the curve
    for( i = 0; i<  ref; i++){
        if ( delta[i] == 0 ) {
            m[i] = 0;
            m[i+1] = 0;
        }
    } //end looping through deltas
    //Find alpha and beta values
    for ( i = 0; i< ref; i++){
        alpha1[i] = m[i] / delta[i];
        beta[i] = m[i+1] / delta[i];
    }
    
    float tau;
    
    //find all alpha and beta for which a^2 + b^2 > 9 and adjust those m's
    //back down, this helps maintain monotonicity
    for (i = 0; i <  ref; i++){
        temp = ( pow ( alpha1[i], 2) + pow (beta[i], 2));
        if ( alpha1[i] < 0 || beta[i] < 0 ) {
            m[i] = 0;
            m[i+1] = 0;
        }
        else if( temp > 9 ) {
            tau = ( (float)3 / sqrt(temp)) ;
            m[i] = ( tau*alpha1[i]*delta[i] );
            m[i+1] =  ( tau * beta[i] * delta[i] ) ; 
        }
        //Serial.print(i); Serial.print(" " );Serial.println(m[i]);
    } //ennd for loop normalizing alphas and  betas
    //println(m[ref]);
}



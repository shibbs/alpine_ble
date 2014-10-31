//
//  Radian_iramp.cpp
//  
//
//  Created by Stephen Hibbs on 9/27/12.
//  Copyright (c) 2012 Stanford. All rights reserved.
//
/*
This is based on radian_sramp.cpp
This was created on April 28
This file has not yet been materially altered from the sramp version
 
 
 */

#include <../RadianIncludes/RadianIncludes.h>
#include <radian_iramp.h>
#include <math.h>

#define SPEED_UP  false  //speed it up 10x
#define DEBUG       true
#define DEBUG2      false //controls output of updated angle

#define IRAMP_ON    1
#define IRAMP_OFF   0
#define OVERRIDE  false
#define MAX_NUM_IRAMP_POINTS          6 //n, number of timelapses max will be less in reality, but 15 is the absolute max
#define MONOTONIC_CUBIC_SPLINE  1
#define LINEAR    2

int IrampStatus = IRAMP_ON; //1 for iramp
int IrampInterpolationType = MONOTONIC_CUBIC_SPLINE;
float m2[MAX_NUM_IRAMP_POINTS] ;//= new float[];
long startTime2 = 0; //marks when the Iramp started, in seconds
int ref2; float temp2;int NumIrampVals = 0;



//This is the vector to simulate the user-generated points.
//These are the interval values in seconds, relative to 0
unsigned int intervalArr[MAX_NUM_IRAMP_POINTS] ={ 3,  10,   5,    3,  7, 10 };
//time in seconds for points to occur relative to start time
unsigned int timeArr2[MAX_NUM_IRAMP_POINTS] = { 0 , 10, 30 , 40 ,  70 , 80  };

Radian_iramp::Radian_iramp(int dummy){
    int x = dummy*2;
    
}
/*
 creates the array of values associated with the iramping
 we need the angular measurements to be "signed" so we'll use CCW as negative and CW as positive
 byte 0 : iramping number of iramp packet bytes, including that one 0 means iramp is off
 byte 1 : iramp interpolation type. 0 is linear, 1 is Monotonic cubic spline interpoation
 it is assumed that all iramp settings start a 0 degrees, 0 seconds
 byte 2 : iramp angle 1 in 5 degree increments. This needs to be an unsigned value relative to the last value
 byte 3 : iramp point 1, time from start in 5 minute increments, max value is 230. 
 Each even byte is the iramp interval relatvie to 0
 each odd byte is the time value, relative to the last
 Once an interval value is 0, that marks that we are done with the intervals array
 */
void Radian_iramp::SetIrampSettings(byte* valuesIn, int setInterval ){
    if(valuesIn[IRAMPING_UNIT_START_BYTE] == 0){
        IrampStatus = IRAMP_OFF;
        return;
    }
    
    IrampInterpolationType = valuesIn[IRAMPING_UNIT_START_BYTE];//the type of interpolation to perform
    int irampIndex = 0;
    int arrIndex = 0;
    int totalTime = 0;
    NumIrampVals = 0;
    //move through until we find an interval of length 0, which marks the last value
    for(int i = IRAMPING_UNIT_START_BYTE+1; i < IRAMPING_UNIT_START_BYTE + MAX_NUM_IRAMP_POINTS*2;i+=2 ){
        NumIrampVals++;
        if(valuesIn[i]==0) break;
    }
    if(DEBUG){Serial.println("NumIrampVals: ");Serial.println(NumIrampVals);}
    
    startTime2 = millis()/1000; // mark the start of the bramping
    intervalArr[0] = setInterval;//the first value is the interval set in the regular part of the app
    timeArr2[0]=0;
    
    for(int i = IRAMPING_UNIT_START_BYTE+1 ; i < IRAMPING_UNIT_START_BYTE + NumIrampVals * 2; i++ )
    {
       if(DEBUG)Serial.println(valuesIn[i]);
        if(irampIndex%2 ==0){ //even values are interval in 1 second resolution relative to 0
            arrIndex ++;
            intervalArr[arrIndex] = (valuesIn[ i ]);
        }else{//odd values are time in minutes in 5 minute increments relative to the last time
            totalTime += valuesIn[i]*5;
            if(DEBUG) Serial.println(totalTime);
            timeArr2[arrIndex] = totalTime*60; //timeArr2 stored in seconds
        }
        irampIndex++;
                                                                            
    }
    //fill in the rest of the values with the final value so that the array finishes properly.
    for( arrIndex; arrIndex < MAX_NUM_IRAMP_POINTS; arrIndex++){
        timeArr2[arrIndex] = timeArr2[NumIrampVals];
        intervalArr[arrIndex] =  intervalArr[NumIrampVals];
    }
    if(DEBUG){
        Serial.print("timeArr2 : ");
        for(int i = 0; i < MAX_NUM_IRAMP_POINTS; i++){
            Serial.print(timeArr2[i]);
            Serial.print(", ");
        }
        Serial.println(" " );
        Serial.print("intervalArr : ");
        for(int i = 0; i < MAX_NUM_IRAMP_POINTS; i++){
            Serial.print(intervalArr[i]);
            Serial.print(", ");
        }
        
        
    }
    //do the monotonic stuff if that's our interpolation type
    if(IrampInterpolationType ==MONOTONIC_CUBIC_SPLINE )MonotonicCubicSplineInit();

}

//returns a value <0 when the Iramp is done. 
//returns the nubmer of stapes to take and sign gives what direction
int Radian_iramp::UpdateIrampValue(){
    
    static float lastAngle = 0;
    float numSteps;
    
    float rtnVal;
    if(IrampInterpolationType == LINEAR){
        rtnVal = PerformLinearInterpolation();
        if(DEBUG2) Serial.println("Linearf interp");
    }
    else{
        rtnVal = PerformCubicSplineInterpolation(); //get the angle we should move to
        if(DEBUG2) Serial.println("Monotonic interp");
    }

    if(DEBUG2)Serial.print("iramp val ");
    if(DEBUG2)Serial.println(rtnVal);
    return (int)rtnVal;
}
/*
 
             
*/

//// now perform the interpolations stage
float Radian_iramp::PerformCubicSplineInterpolation(){
    int index = -1;
    static double t =0;
    static float h[4];
    float currTime = ( (float)millis()/1000 ) -(float)startTime2;
    if(SPEED_UP) currTime*=10; //speed it up 10x
    static int i=0;
    while(currTime > timeArr2[i] && i < NumIrampVals-1) i++; //figure out what index we're on
    i--; //go back one so we fall into the correct bracket
    if(i<0) i=0;
    if(DEBUG) { 
        Serial.print("T ");Serial.println(currTime); 
        Serial.print("index "); Serial.println(i);
        Serial.print("m : ");   Serial.println(m2[i]);
        Serial.print("x : ");   Serial.println(timeArr2[i]);
        Serial.print("y : ");   Serial.println(intervalArr[i]);
    }
    
    t = (currTime-(float)timeArr2[i])/( (float)(timeArr2[i+1]-timeArr2[i])); //this is our fractional distance between the two points
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
    float p = ( h[ 0 ]) * ((float)intervalArr[i]) + ( (float) ( timeArr2[i+1] - timeArr2[i]  ) )*( h[1] ) *m2[i] ;
    if(DEBUG) Serial.println(p);
    p = p  + (h[2] )*((float)intervalArr[i+1]) +  (h[3] )*( ((float) timeArr2[i+1] - timeArr2[i])  )*m2[i+1] ;
    if(DEBUG) Serial.println(p);
    return p;
}

//// Performs the linear interpolation
float Radian_iramp::PerformLinearInterpolation(){
    int index = -1;
    static double t =0;
    float currTime = ( (float)millis()/1000 ) -(float)startTime2;
    if(SPEED_UP) currTime*=10; //speed it up 10x
    static int i=0;
    while(currTime > timeArr2[i] && i < NumIrampVals-1) i++; //figure out what index we're on
    i--; //go back one so we fall into the correct bracket
    if(i<0) i=0;
    if(DEBUG) { 
        Serial.print("T ");Serial.println(currTime); 
        Serial.print("index "); Serial.println(i);
        Serial.print("x : ");   Serial.println(timeArr2[i]);
        Serial.print("y : ");   Serial.println(intervalArr[i]);
    }
    
    t = (currTime-(float)timeArr2[i])/( (float)(timeArr2[i+1]-timeArr2[i])); //this is our fractional distance between the two points
    if(DEBUG) { Serial.print("t "); Serial.println(t); }
    
    //perform a linear interpolations of so that angle = angle pnt + fraction between * (next angle point - angle pnt)
    float p = ((float)intervalArr[i]) + ((float)(intervalArr[i+1] - intervalArr[i])) *t   ;
    return p;
}


// /*
// This function initializes the m array that is later used for the cubic spline
// the m array is set up here to ensure monotonicity in the cubic spline later
// */
void Radian_iramp::MonotonicCubicSplineInit( ){
    float alpha1[MAX_NUM_IRAMP_POINTS];// = new float[n];
    float beta[MAX_NUM_IRAMP_POINTS];// = new float[n];
    //  multiplier = 10;
    float delta[MAX_NUM_IRAMP_POINTS] ;//= new float[n];
    float xdiff;
    ref2 = NumIrampVals-1; int i;
    //set up and populate the m array
    for ( i = 0 ; i < ref2; i++) {
        xdiff = ((float) ( timeArr2[i + 1] - timeArr2[i] ));
        if( xdiff ==0) delta[i] = 0;
        else delta[i] = ((float) (intervalArr[i + 1] - intervalArr[i]) ) / (xdiff );
        //println(delta[i]);
        if ( i > 0) //i=1 is an edge case error, but edges are set below
            m2[i] = ((delta[i - 1] + delta[i])) / 2.0;  //m array used in cubic spline interpolation
        
    } //end for loop over i
    
    //set the first and last values of the m array
    m2[0] = delta[0];
    m2[ref2 ] = delta[ref2-1];
    
    
    //find all deltas of value 0 and set the m values to 0 accordingly, this
    //marks flat sections of the curve
    for( i = 0; i<  ref2; i++){
        if ( delta[i] == 0 ) {
            m2[i] = 0;
            m2[i+1] = 0;
        }
    } //end looping through deltas
    //Find alpha and beta values
    for ( i = 0; i< ref2; i++){
        alpha1[i] = m2[i] / delta[i];
        beta[i] = m2[i+1] / delta[i];
    }
    
    float tau;
    
    //find all alpha and beta for which a^2 + b^2 > 9 and adjust those m's
    //back down, this helps maintain monotonicity
    for (i = 0; i <  ref2; i++){
        temp2 = ( pow ( alpha1[i], 2) + pow (beta[i], 2));
        if ( alpha1[i] < 0 || beta[i] < 0 ) {
            m2[i] = 0;
            m2[i+1] = 0;
        }
        else if( temp2 > 9 ) {
            tau = ( (float)3 / sqrt(temp2)) ;
            m2[i] = ( tau*alpha1[i]*delta[i] );
            m2[i+1] =  ( tau * beta[i] * delta[i] ) ; 
        }
        //Serial.print(i); Serial.print(" " );Serial.println(m2[i]);
    } //ennd for loop normalizing alphas and  betas
    //println(m2[ref2]);
}



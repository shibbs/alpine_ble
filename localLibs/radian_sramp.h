//
//  radian_Sramp.h
//  
//
//  Created by Stephen Hibbs on 9/27/12.
//  Copyright (c) 2012 Stanford. All rights reserved.
//

#ifndef _radian_sramp_h
#define _radian_sramp_h



#include <../RadianIncludes/RadianIncludes.h>

class Radian_sramp
{
public: 
    Radian_sramp(int dummy); //init function
    long SetSrampSettings(byte* valuesIn, unsigned long durationSeconds,unsigned int angle); //takes in array used for setting the Sramp settings. Returns the number of seconds it will run for
    int UpdateSrampValue( ); //returns number of steps to take
private:
    float PerformCubicSplineInterpolation();
    float PerformLinearInterpolation();
    void MonotonicCubicSplineInit();
    
};
    
#endif

//
//  radian_iramp.h
//  
//
//  Created by Stephen Hibbs on 9/27/12.
//  Copyright (c) 2012 Stanford. All rights reserved.
//

#ifndef _radian_iramp_h
#define _radian_iramp_h



#include <../RadianIncludes/RadianIncludes.h>

class Radian_iramp
{
public: 
    Radian_iramp(int dummy); //init function
    void SetIrampSettings(byte* valuesIn, int setInterval); //takes in array used for setting the Iramp settings.
    int UpdateIrampValue( ); //returns new interval length in ms
private:
    float PerformCubicSplineInterpolation();
    float PerformLinearInterpolation();
    void MonotonicCubicSplineInit();
    
};
    
#endif

//
//  radian_power_saver.h
//  
//
//  Created by Stephen Hibbs on 9/27/12.
//  Copyright (c) 2012 Stanford. All rights reserved.
//

#ifndef _radian_power_saver_h
#define _radian_power_saver_h

#include <avr/sleep.h>
#include <Arduino.h>

class Radian_power_saver
{
public: 
    Radian_power_saver(int dummy);
    void InitForPowerSavings(); //sets up system on initialization to turn off unneeded modules
    void EnterSimpleSleep(); 
    void EnterSimpleSleep(unsigned int IdlePWM); //sets up system for sleeping in between timer events
    void EnterDeepSleepMode(); //enters a deep sleep mode when there is a low battery situation
    void WakeUpPins(int tiltOrPan);
    void SetIdlePins(int tiltOrPan);
    
};
    
#endif

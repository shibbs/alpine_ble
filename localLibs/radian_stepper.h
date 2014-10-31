//
//  radian_power_saver.h
//  
//
//  Created by Stephen Hibbs on 9/27/12.
//  Copyright (c) 2012 Stanford. All rights reserved.
//

#ifndef _radian_stepper_h
#define _radian_stepper_h



#include "alpine_includes.h"


void InitStepperPins(); //initializes the stepper motor pins
void Step(int direction); //steps the stepper one step
void Brake(); //puts the h-bridge into brake mode
void DisableStepper(); 
void EnableStepper();
void SetStepperPWM(int PWM); //sets stepper pwm form 0-100

    
#endif

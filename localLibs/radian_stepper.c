//
//	Radian_stepper.cpp
//	
//
//	Created by Stephen Hibbs on 9/27/12.
//	Copyright (c) 2012 Stanford. All rights reserved.
//
/*

 
 
 */

#include "alpine_boards.h"
#include "radian_stepper.h"
#include "./Simple_PWM/nrf_pwm.h" 

/*
 initializes the pins associated with riving the stepper motor
 */
void InitStepperPins(){
	nrf_gpio_cfg_output(STEPPER_EN);
	nrf_gpio_cfg_output(STEPPER_A1);
	nrf_gpio_cfg_output(STEPPER_A2);
	nrf_gpio_cfg_output(STEPPER_B1);
	nrf_gpio_cfg_output(STEPPER_B2);
	nrf_gpio_cfg_output(STEPPER_PWM);
	nrf_gpio_cfg_output(BOOST_EN);
	nrf_gpio_pin_clear(STEPPER_EN); //make sure stepper is off
	nrf_gpio_pin_clear(STEPPER_PWM);
}

/*
 brakes the motor and reduces power consumption
 */
void Brake( ){
		nrf_gpio_pin_set(STEPPER_EN);
		SetStepperPWM(0);
		nrf_gpio_pin_set(STEPPER_A1);
		nrf_gpio_pin_clear(STEPPER_A2);
		nrf_gpio_pin_set(STEPPER_B1);
		nrf_gpio_pin_clear(STEPPER_B2);
}

/*
 steps the radian forward one step in the specified direction
 */
void Step(int StepDirection){

	static unsigned int stepNum = 0; 
	stepNum+= StepDirection;
	stepNum = stepNum%4;
	
	//if(DEBUG) Serial.println(stepNum);
	
	if(stepNum==0){
			nrf_gpio_pin_set(STEPPER_A1);
			nrf_gpio_pin_clear(STEPPER_A2);
			nrf_gpio_pin_set(STEPPER_B1);
			nrf_gpio_pin_clear(STEPPER_B2);
	}
	else if(stepNum==1){
			nrf_gpio_pin_set(STEPPER_A1);
			nrf_gpio_pin_clear(STEPPER_A2);
			nrf_gpio_pin_clear(STEPPER_B1);
			nrf_gpio_pin_set(STEPPER_B2);
	}
	else if(stepNum==2){
			nrf_gpio_pin_clear(STEPPER_A1);
			nrf_gpio_pin_set(STEPPER_A2);
			nrf_gpio_pin_clear(STEPPER_B1);
			nrf_gpio_pin_set(STEPPER_B2);
	}
	else{
			nrf_gpio_pin_clear(STEPPER_A1);
			nrf_gpio_pin_set(STEPPER_A2);
			nrf_gpio_pin_set(STEPPER_B1);
			nrf_gpio_pin_clear(STEPPER_B2);
	}
		
}

/*
 takes in a pwm between 0 and 100, and sets the stepper PWM line to that duty cycle
 */
void SetStepperPWM(int PWM){
	//	if(PWM != OCR4B)
	//			OCR4B = PWM;
}

//enable the stepper by turning on the boost and stepper chip
//The Stepper PWM is not set by this though, and will be set to whatever it was last set to
void EnableStepper(){
	nrf_gpio_pin_set(STEPPER_EN);
	nrf_gpio_pin_set(BOOST_EN);
}
							 
//diable the stepper by turning off the boost and stepper chip
void DisableStepper(){
	nrf_gpio_pin_clear(STEPPER_EN);
	nrf_gpio_pin_clear(BOOST_EN);
}					 

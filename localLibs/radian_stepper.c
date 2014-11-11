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
	//#define PWM_DEFAULT_CONFIG  {.num_channels   = 1,  .gpio_num = {STEPPER_PWM}, .ppi_channel = {0,1,2,3,4,5},.gpiote_channel = {2,3,0},   .mode = PWM_MODE_LED_100};

	nrf_pwm_config_t init_config = PWM_DEFAULT_CONFIG; //create the initialized struct
	init_config.num_channels = 1; 
	init_config.gpio_num[0] = STEPPER_PWM;
	init_config.ppi_channel[0] = 0;//{0,1,2,3,4,5};
	init_config.ppi_channel[1] = 1;
	init_config.gpiote_channel[0] = 2;
	init_config.mode = PWM_MODE_LED_100;
	nrf_pwm_init(&init_config); //init the pwm module
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
void SetStepperPWM(int duty){
	//	if(PWM != OCR4B)
	//			OCR4B = PWM;
	nrf_gpio_pin_set(STEPPER_PWM);
//	nrf_pwm_set_value(0,duty); //our pwm channel is channel 0
}

//enable the stepper by turning on the boost and stepper chip
//The Stepper PWM is not set by this though, and will be set to whatever it was last set to
void EnableStepper(){
	
	nrf_gpio_pin_set(PWR_CNTRL); //turn on the main power channel
	nrf_gpio_pin_set(STEPPER_EN); //enable the stepper chip
	nrf_gpio_pin_set(BOOST_EN); //enable the boost
}
							 
//diable the stepper by turning off the boost and stepper chip
void DisableStepper(){
	nrf_gpio_pin_clear(STEPPER_EN); //disable the stepper
	//nrf_gpio_pin_clear(BOOST_EN); //disable the boost
}					 

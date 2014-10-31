//
//  Radian_stepper.cpp
//  
//
//  Created by Stephen Hibbs on 9/27/12.
//  Copyright (c) 2012 Stanford. All rights reserved.
//
/*

 
 
 */

#include "alpine_boards.h"
#include "radian_stepper.h"


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
	nrf_gpio_pin_clear(STEPPER_EN);
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
  //  if(PWM != OCR4B)
  //      OCR4B = PWM;
}

//enable the stepper by turning on the boost and stepper chip as well as 
//setting up the timer4 pwm system
void EnableStepper(){
    /* settings that work    
     OCR4C =  100 ;//set the top value
     OCR4B = 70; //set the duty cycle, since OCR4C is 100 this is just %DC
     TCCR4A |= (1<<COM4B0) | (PWM4B);
     TCCR4A &= ~(1<<COM4B1);     // COM4B1 = 0, COM4B0=1 so that OC4B is connected and cleared on compare match and then set high on reset of counter . Also need to turn on PWM enable for module B
     TCCR4B = B00000010; //set the timer version off, with prescaler of 1024 to set PWM frequency to ~8KHz /
     TCCR4D &= ~( (1<<WGM41)|(1<<WGM40)); //set WGM40 to 0
*/
   /*
    OCR4C =  100 ;//set the top value
   // OCR4B = 60; //set the duty cycle, since OCR4C is 100 this is just %DC
    TCCR4A = 0;
    TCCR4C = 0;
    TCCR4A |= (1<<COM4B1) | (1<<PWM4B);
    TCCR4A &= ~(1<<COM4B0);     // COM4B1 = 0, COM4B0=1 so that OC4B is connected and cleared on compare match and then set high on reset of counter . Also need to turn on PWM enable for module B
    TCCR4C |= (1<<COM4B1S);
    
    TCCR4B = B00000010; //set the timer version off, with prescaler of 1024 to set PWM frequency to ~8KHz /
   
    // TCCR4D &= ~( (1<<WGM41)|(1<<WGM40)); //set WGM40 to 0
    TCCR4D=0;
    
    TCCR4E = (1<<OC4OE3); //enable output to output compare
    TIMSK4= 0;
    TIFR4 = 0; //turn off interrupt flags
    DT4 = 0;
    
    */
   // digitalWrite(STEPPER_PWM);
    nrf_gpio_pin_set(STEPPER_EN);
    nrf_gpio_pin_set(BOOST_EN);
}
               
//diable the stepper by turning off the boost and stepper chip
void DisableStepper(){
    nrf_gpio_pin_clear(STEPPER_EN);
    nrf_gpio_pin_clear(BOOST_EN);
}           

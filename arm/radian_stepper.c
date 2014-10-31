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

#define DEBUG false

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
	nrf_gpio_clear(STEPPER_EN);
}

/*
 brakes the motor and reduces power consumption
 */
void Brake( ){
    digitalWrite(STEPPER_EN, HIGH);
    SetStepperPWM(0);
    digitalWrite(STEPPER_A1,HIGH);
    nrf_gpio_clear(STEPPER_A2);
    digitalWrite(STEPPER_B1,HIGH);
    nrf_gpio_clear(STEPPER_B2);
}

/*
 steps the radian forward one step in the specified direction
 */
void Step(int StepDirection){
    if(DEBUG) Serial.println("Stepping");
    static unsigned int stepNum = 0; 
    stepNum+= StepDirection;
    stepNum = stepNum%4;
    
    //if(DEBUG) Serial.println(stepNum);
    
    if(stepNum==0){
        digitalWrite(STEPPER_A1,HIGH);
        digitalWrite(STEPPER_A2,LOW);
        digitalWrite(STEPPER_B1,HIGH);
        digitalWrite(STEPPER_B2,LOW);
    }
    else if(stepNum==1){
        digitalWrite(STEPPER_A1,HIGH);
        digitalWrite(STEPPER_A2,LOW);
        digitalWrite(STEPPER_B1,LOW);
        digitalWrite(STEPPER_B2,HIGH);
    }
    else if(stepNum==2){
        digitalWrite(STEPPER_A1,LOW);
        digitalWrite(STEPPER_A2,HIGH);
        digitalWrite(STEPPER_B1,LOW);
        digitalWrite(STEPPER_B2,HIGH);
        
        
    }
    else{
        digitalWrite(STEPPER_A1,LOW);
        digitalWrite(STEPPER_A2,HIGH);
        digitalWrite(STEPPER_B1,HIGH);
        digitalWrite(STEPPER_B2,LOW);
    }
    
}

/*
 takes in a pwm between 0 and 100, and sets the stepper PWM line to that duty cycle
 */
void SetStepperPWM(int PWM){
    if(PWM != OCR4B)
        OCR4B = PWM;
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
    
    
   // digitalWrite(STEPPER_PWM, HIGH);
    digitalWrite(STEPPER_EN, HIGH);
    digitalWrite(BOOST_EN, HIGH);
}
               
//diable the stepper by turning off the boost and stepper chip
void DisableStepper(){
    digitalWrite(STEPPER_EN, LOW);
    digitalWrite(BOOST_EN, LOW);
}           

//
//  radian_power_saver.cpp
//  
//
//  Created by Stephen Hibbs on 9/27/12.
//  Copyright (c) 2012 Stanford. All rights reserved.
//
/*
 void InitForPowerSavings() //sets up system on initialization to turn off unneeded modules
 void EnterSimpleSleep() //sets up system for sleeping in between timer events
 void EnterDeepSleepMode() //enters a deep sleep mode when there is a low battery situation
 
 */
#include <../RadianIncludes/RadianIncludes.h>
#include <avr/sleep.h>
#include "radian_power_saver.h"
#include <Arduino.h>
#include <avr/wdt.h>
#include <avr/power.h>

#define DEBUG false

Radian_power_saver::Radian_power_saver(int dummy){
    int x = dummy*2;
}

/*function to turn off all periferlas of the radian as well as to send uC into low power mode
in the event of a low battery condition
 */
void Radian_power_saver::EnterDeepSleepMode(){
    POWER_OFF;
    if(DEBUG) Serial.println("Entering Deep Sleep Mode");
    //Disable ADC and internal Vref
    digitalWrite(BOOST_EN, LOW);
    POWER_OFF;
    ADMUX &= ~( (1<<REFS1) | (1<<REFS0) );
    ADCSRA &= ~( (1<<ADEN) );
    
    //Disable AnalogComparator
    ACSR |= (1<<ACD); 
    
    DIDR0 = 0xFF;
    //  DIDR1 = 0xFF;
    DIDR2 = 0xFF;
    
    
    
    //disable watchdog timer
    cli();
    wdt_reset();
    // Clear WDRF in MCUSR 
    MCUSR &= ~(1<<WDRF);
    MCUCR |= (1<<JTD);//disable on chip debug system by writing 1 to JTD bit in MCUCR
    // Write logical one to WDCE and WDE 
    // Keep old prescaler setting to prevent unintentional time-out
     
    WDTCSR |= (1<<WDCE) | (1<<WDE);
    // Turn off WDT 
    WDTCSR = 0x00;
    
    //disable port pins by writing to digital input disable register DIDR1
    
    //disable USB
    PRR0 = 0xFF; //turn off all periferals in the power reduction registers
    PRR1 = 0xFF; 
    
    wdt_disable();
    //bod_disable();
    power_usart0_disable();
    power_usart1_disable();
    power_spi_disable();
    power_adc_disable();
    power_all_disable();
    POWER_OFF;
    cli(); //disable all interrupts
    SMCR = B00000101; //enable sleep mode (bit0 set) and set to POWER DOWN mode (lowest consumption)
    sleep_mode(); //go into sleep
    
    
    
}

void Radian_power_saver::EnterSimpleSleep( ) {
    
    

}

/*this mode turns off everything not needed during the time in which only the main counter is running
 The only things needed during simple sleep are the operation of basic pins and the timer1 interrupt
 */
void Radian_power_saver::EnterSimpleSleep(unsigned int IdlePWM ){
    
  /*  
    
    //Disable ADC and internal Vref
    POWER_OFF;
    ADMUX &= ~( (1<<REFS1) | (1<<REFS0));
    ADCSRA &= ~( (1<<ADEN) );
    //Disable AnalogComparator
    ACSR |= (1<<ACD); 
    
    
    
    SMCR = 1; //go into idle mode
    sleep_mode();
    POWER_ON;
   
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);  
    sleep_enable();
    interrupts();
 //   attachInterrupt(0,sleepHandler, FALLING);
    sleep_mode();  //sleep now
    //--------------- ZZZZZZ sleeping here
    sleep_disable(); //fully awake now
    POWER_ON; //turn the power back on
   
  */
    
    
    if(DEBUG) Serial.println("Entering simple Sleep Mode");
    if(IdlePWM == 0) SetIdlePins(PAN);
    else SetIdlePins(TILT);
//    power_all_disable(); //turn off all modules
    
    //turn necessary idle ports back on
    power_usart0_enable();
    power_timer1_enable();
 //   power_timer4_enable(); //leave PWM module on, should make something smarter for future
    
    set_sleep_mode(SLEEP_MODE_IDLE);  
    sleep_enable();
    //  interrupts();
    //  attachInterrupt(0,sleepHandler, FALLING);
    
    sleep_mode();  //sleep now
    //--------------- ZZZZZZ sleeping here
    sleep_disable(); //fully awake now
   // Serial.println("Woke up");
    POWER_ON;
    power_all_enable();
    if(IdlePWM == 0) WakeUpPins(PAN);
    else WakeUpPins(TILT);

    
    /*power_adc_enable();
    power_timer0_enable();
    power_usart1_enable();
    power_useart0_enable();
     
     */
    
}
   
void Radian_power_saver::WakeUpPins(int tiltOrPan){
    
    

}

void Radian_power_saver::SetIdlePins(int tiltOrPan){
    
    //  R_LED_OFF;
  //  G_LED1_OFF;
  //  G_LED2_OFF;
  //  B_LED_OFF;
    
  //  digitalWrite(TRIGGER_CNTRL, LOW);
  //  digitalWrite(TRIGGER_STEREO_CNTRL, LOW);
    
    //turn off pullups on charger chip
    digitalWrite(CH_PG, LOW); //set internal pullup
    digitalWrite(CH_STAT1, LOW); //set internal pullup
    digitalWrite(CH_STAT2, LOW); //set internal pullup
    
  //  ADMUX &= ~( (1<<REFS0) | (1<<REFS1)); //to turn reference off
    
    //turn off the comparator 
    
    if(tiltOrPan == TILT){
        if(DEBUG)Serial.println("TILT");
        POWER_ON;
    }else{ //if we're panning, turn down all stepper pins
        if(DEBUG)Serial.println("PAN");
        POWER_OFF;
        //DDRB &= ~1;
        
        digitalWrite(STEPPER_EN, LOW);
        //this comment as of sept 2014 should help avoid slippage
  /*      digitalWrite(STEPPER_A1,LOW);
        digitalWrite(STEPPER_A2,LOW);
        digitalWrite(STEPPER_B1,LOW);
        digitalWrite(STEPPER_B2,LOW); */
        digitalWrite(STEPPER_PWM,LOW);
        digitalWrite(BOOST_EN, LOW);
        
    }
    
    
    
}

/*
 turn off unneeded periferals 
 turn off : SPI, ADC, Timer0, timer4, timer4, and the usart module, and WDT
 */
void Radian_power_saver::InitForPowerSavings(){
   
    if(DEBUG) Serial.println("Initializing power saver Mode");
    
    //commented out sicne I can't get comparator working after this has been called
//    PRR0 |= (1<<PRADC) | (1<<PRSPI) | (1<<PRTIM0) ;
//    PRR1 |=  (1<<PRTIM3) | (1<<PRUSART1);// | (1<<PRTIM4); 
    //disable watchdog timer
    cli();
    wdt_reset();
    // Clear WDRF in MCUSR 
    MCUSR &= ~(1<<WDRF);
    // Write logical one to WDCE and WDE 
    // Keep old prescaler setting to prevent unintentional time-out
     
    WDTCSR |= (1<<WDCE) | (1<<WDE);
    // Turn off WDT 
    WDTCSR = 0x00;
    sei();
    
    
}

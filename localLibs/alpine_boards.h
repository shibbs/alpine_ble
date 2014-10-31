
// alpine_boards.h definition
#ifndef ALPINE_BOARDS_H_
#define ALPINE_BOARDS_H_
#include "nrf_gpio.h"
#include "alpine_includes.h"

#ifdef MICHRON //defines for michron-specific things

#define G_BATT_LED			0				//NC on michron
#define R_BATT_LED			0				//NC on michron
#define G_STAT_LED			22			//P03: G status LED
#define R_STAT_LED			22			//P04: R status LED

#define PC_SYNC_FEEDBACK	30		//P30 : PC Sync Feedback
#define FOC_CNTRL		0			//P00 : Tigger Stereo Cntrl
#define TRIG_PWR_CNTRL		1			//P01 : Trigger Power Cntrl
#define TRIG_CNTRL				2			//P02: Trigger Cntrl

#else //else if Radian

// Radian R2 Pin Mappings
#define G_BATT_LED			21			//P21: Green Battery LED
#define R_BATT_LED			22			//P22 Red Batt LED
#define G_STAT_LED			3				//P03: G status LED
#define R_STAT_LED			4				//P04: R status LED

#define PC_SYNC_FEEDBACK	30		//P30 : PC Sync Feedback
#define FOC_CNTRL		0			//P00 : Tigger Stereo Cntrl
#define TRIG_PWR_CNTRL		1			//P01 : Trigger Power Cntrl
#define TRIG_CNTRL				2			//P02: Trigger Cntrl

#endif

#define BOOST_EN				5				//P05: Boost enable
#define STEPPER_A1			6				//P06: Stepper AI1
#define STEPPER_A2			7				//P07: Stepper AI2
#define STEPPER_B1			8				//P08: Stepper BI1
#define STEPPER_B2			9				//P09: Stepper BI2
#define STEPPER_PWM			10			//P10: Motor PWM
#define STEPPER_EN			11			//P11: Motor Enable

#define USB_V_EN				12			//P12: USB Voltage Enable
#define USB_V_FLAG			13			//P13: USB VOltage Chip Flag

#define CH_IUSB2				14			//P14: IUSB2 - something to do with charge chip
#define CH_CHG					15			//P15: CHG - to do with charge chip
#define	PWR_CNTRL				19			//P19: Circtuit Power Control - high = power on
#define BATT_MEAS				20			//P20: Battery measure Point

#define PWR_SWITCH			23			//P23: Power Switch monitoring pin
#define USB_INT					24			//P24: USB chip interrupt
#define USB_GPX					25			//P25: USB Chip GPX
#define USB_MOSI				26			//P26: USB Chip MOSI
#define USB_MISO				27			//P27: USB Chip MISO
#define USB_SS					28			//P28: USB SS
#define USB_SCLK				29			//P29: USB SCLK
#define USB_RST					2				//P2 : USB RST //probavbly need to move this once since it looks like it's not on a valid pin ! FLAG SAH


#define OPEN_SHUTTER_MACRO { 	nrf_gpio_pin_clear(TRIG_CNTRL);	nrf_gpio_pin_clear(FOC_CNTRL);  } 
#define CLOSE_SHUTTER_MACRO { nrf_gpio_pin_set(TRIG_CNTRL);   nrf_gpio_pin_set(FOC_CNTRL);   } 

#define R_STAT_LED_ON			{	nrf_gpio_pin_clear(R_STAT_LED);			}
#define R_STAT_LED_OFF		{	nrf_gpio_pin_set(R_STAT_LED);				}
#define G_STAT_LED_ON			{	nrf_gpio_pin_clear(G_STAT_LED);			}
#define G_STAT_LED_OFF		{	nrf_gpio_pin_set(G_STAT_LED);				}

#define R_BATT_LED_ON			{	nrf_gpio_pin_clear(R_BATT_LED);			}
#define R_BATT_LED_OFF		{	nrf_gpio_pin_set(R_BATT_LED);				}
#define G_BATT_LED_ON			{	nrf_gpio_pin_clear(G_BATT_LED);			}
#define G_BATT_LED_OFF		{	nrf_gpio_pin_set(G_BATT_LED);				}


void init_alpine_pins(void);


#endif //ALPINE_BOARDS_H_

//alpine_boards.c

#include "alpine_boards.h"

/**initializes pins for Radian and Michron boards
GPI : 
#define USB_V_FLAG			13			//P13: USB VOltage Chip Flag
#define USB_INT					24			//P24: USB chip interrupt
#define USB_MISO				27			//P27: USB Chip MISO
#define CH_CHG					15			//P15: CHG - to do with charge chip
#define PWR_SWITCH			23			//P23: Power Switch monitoring pin
#define USB_GPX					25			//P25: USB Chip GPX
#define PC_SYNC_FEEDBACK	30		//P30 : PC Sync Feedback

GPO : 
#define BOOST_EN				5				//P05: Boost enable
#define STEPPER_AI1			6				//P06: Stepper AI1
#define STEPPER_AI2			7				//P07: Stepper AI2
#define STEPPER_BI1			8				//P08: Stepper BI1
#define STEPPER_BI2			9				//P09: Stepper BI2
#define STEPPER_PWM			10			//P10: Motor PWM
#define STEPPER_EN			11			//P11: Motor Enable
#define USB_V_EN				12			//P12: USB Voltage Enable
#define CH_IUSB2				14			//P14: IUSB2 - used to set charge current in the chip
#define	PWR_CNTRL				19			//P19: Circtuit Power Control - high = power on

#define USB_MOSI				26			//P26: USB Chip MOSI
#define USB_SS					28			//P28: USB SS
#define USB_SCLK				29			//P29: USB SCLK
#define USB_RST					2				//P2 : USB RST //probavbly need to move this once since it looks like it's not on a valid pin ! FLAG SAH
#define G_BATT_LED			21			//P21: Green Battery LED
#define R_BATT_LED			22			//P22 Red Batt LED
#define G_STAT_LED			3				//P03: G status LED
#define R_STAT_LED			4				//P04: R status LED
#define FOC_CNTRL				0			//P00 : Tigger Stereo Cntrl
#define TRIG_PWR_CNTRL		1			//P01 : Trigger Power Cntrl
#define TRIG_CNTRL				2			//P02: Trigger Cntrl

Analog Input : 
#define BATT_MEAS				20			//P20: Battery measure Point

**/
void init_alpine_pins(){

	//configure inputs, need to decide plan for pullup values, currently all pulled up
		nrf_gpio_cfg_input(USB_V_FLAG,NRF_GPIO_PIN_PULLUP);	
		nrf_gpio_cfg_input(USB_INT,NRF_GPIO_PIN_PULLUP);	
		nrf_gpio_cfg_input(USB_MISO,NRF_GPIO_PIN_PULLUP);	
		nrf_gpio_cfg_input(CH_CHG,NRF_GPIO_PIN_PULLUP);	
		nrf_gpio_cfg_input(PWR_SWITCH,NRF_GPIO_PIN_PULLUP);	
		nrf_gpio_cfg_input(USB_GPX,NRF_GPIO_PIN_PULLUP);	
		nrf_gpio_cfg_input(PC_SYNC_FEEDBACK,NRF_GPIO_PIN_PULLUP);	
	
	//configure outputs

		nrf_gpio_cfg_output(USB_V_EN);
		nrf_gpio_cfg_output(CH_IUSB2);
		nrf_gpio_cfg_output(PWR_CNTRL);
		nrf_gpio_cfg_output(USB_MOSI);
		nrf_gpio_cfg_output(USB_SS);
		nrf_gpio_cfg_output(USB_SCLK);
		nrf_gpio_cfg_output(USB_RST);
		nrf_gpio_cfg_output(G_BATT_LED);
		nrf_gpio_cfg_output(R_BATT_LED);
		nrf_gpio_cfg_output(R_STAT_LED);
		nrf_gpio_cfg_output(G_STAT_LED);
		nrf_gpio_cfg_output(FOC_CNTRL);
		nrf_gpio_cfg_output(TRIG_PWR_CNTRL);
		nrf_gpio_cfg_output(TRIG_CNTRL);
		InitStepperPins();
}
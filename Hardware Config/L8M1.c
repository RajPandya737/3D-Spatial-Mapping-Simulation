
/*
Written by Raj Pandya
*/

#include <stdint.h>
#include "PLL.h"
#include "SysTick.h"
#include "uart.h"
#include "onboardLEDs.h"
#include "tm4c1294ncpdt.h"
#include "VL53L1X_api.h"

#define I2C_MCS_ACK 0x00000008	  // Data Acknowledge Enable
#define I2C_MCS_DATACK 0x00000008 // Acknowledge Data
#define I2C_MCS_ADRACK 0x00000004 // Acknowledge Address
#define I2C_MCS_STOP 0x00000004	  // Generate STOP
#define I2C_MCS_START 0x00000002  // Generate START
#define I2C_MCS_ERROR 0x00000002  // Error
#define I2C_MCS_RUN 0x00000001	  // I2C Master Enable
#define I2C_MCS_BUSY 0x00000001	  // I2C Busy
#define I2C_MCR_MFE 0x00000010	  // I2C Master Function Enable

#define MAXRETRIES 5 // number of receive attempts before giving up
void I2C_Init(void)
{
	SYSCTL_RCGCI2C_R |= SYSCTL_RCGCI2C_R0;	 // activate I2C0
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1; // activate port B
	while ((SYSCTL_PRGPIO_R & 0x0002) == 0)
	{
	}; // ready?

	GPIO_PORTB_AFSEL_R |= 0x0C; // 3) enable alt funct on PB2,3       0b00001100
	GPIO_PORTB_ODR_R |= 0x08;	// 4) enable open drain on PB3 only

	GPIO_PORTB_DEN_R |= 0x0C; // 5) enable digital I/O on PB2,3
							  //    GPIO_PORTB_AMSEL_R &= ~0x0C;          																// 7) disable analog functionality on PB2,3

	// 6) configure PB2,3 as I2C
	//  GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0xFFFF00FF)+0x00003300;
	GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R & 0xFFFF00FF) + 0x00002200; // TED
	I2C0_MCR_R = I2C_MCR_MFE;										   // 9) master function enable
	I2C0_MTPR_R = 0b0000000000000101000000000111011;				   // 8) configure for 100 kbps clock (added 8 clocks of glitch suppression ~50ns)
	//    I2C0_MTPR_R = 0x3B;                                        						// 8) configure for 100 kbps clock
}

// The VL53L1X needs to be reset using XSHUT.  We will use PG0
void PortG_Init(void)
{
	// Use PortG0
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R6; // activate clock for Port N
	while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R6) == 0)
	{
	};							 // allow time for clock to stabilize
	GPIO_PORTG_DIR_R &= 0x00;	 // make PG0 in (HiZ)
	GPIO_PORTG_AFSEL_R &= ~0x01; // disable alt funct on PG0
	GPIO_PORTG_DEN_R |= 0x01;	 // enable digital I/O on PG0
								 // configure PG0 as GPIO
	// GPIO_PORTN_PCTL_R = (GPIO_PORTN_PCTL_R&0xFFFFFF00)+0x00000000;
	GPIO_PORTG_AMSEL_R &= ~0x01; // disable analog functionality on PN0

	return;
}

void PortH_Init(void)
{
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R7; // Activate the clock for Port H
	while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R7) == 0)
	{
	}; // Allow time for clock to stabilize

	GPIO_PORTH_DIR_R |= 0b00001111;	   // Enable PH0 and PH1 as inputs
	GPIO_PORTH_AFSEL_R &= ~0b00001111; // disable alt funct on Port M pins (PM0-PM3)
	GPIO_PORTH_DEN_R |= 0b00001111;	   // Enable PH0, PH1, PH2, and PH3 as digital pins
	GPIO_PORTH_AMSEL_R &= ~0b00001111; // disable analog functionality on Port M	pins (PM0-PM3)

	return;
}

void PortF_Init(void)
{
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5; // Activate the clock for Port M
	while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R5) == 0)
	{
	}; // Allow time for clock to stabilize

	GPIO_PORTF_DIR_R = 0b00010000; // Enable PM4 output
	GPIO_PORTF_DEN_R = 0b00010000; // Enable PM4 as digital pin
}


// Initialize onboard LEDs
void PortN_Init(void){
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R12;				// Activate clock for Port N
	while((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R12) == 0){};	// Allow time for clock to stabilize
	GPIO_PORTN_DIR_R |= 0x03;        								// Make PN0 and PN1 output (Built-in LEDs: D1 (PN1) and D2 (PN0))
  GPIO_PORTN_AFSEL_R &= ~0x03;     								// Disable alt funct on PN0 and PN1
  GPIO_PORTN_DEN_R |= 0x03;        								// Enable digital I/O on PN0 and PN1
																									
  GPIO_PORTN_AMSEL_R &= ~0x03;     								// Disable analog functionality on PN0 and PN1
	return;
}




// XSHUT     This pin is an active-low shutdown input;
//					the board pulls it up to VDD to enable the sensor by default.
//					Driving this pin low puts the sensor into hardware standby. This input is not level-shifted.
void VL53L1X_XSHUT(void)
{
	GPIO_PORTG_DIR_R |= 0x01;		 // make PG0 out
	GPIO_PORTG_DATA_R &= 0b11111110; // PG0 = 0
	FlashAllLEDs();
	SysTick_Wait10ms(10);
	GPIO_PORTG_DIR_R &= ~0x01; // make PG0 input (HiZ)
}

int totalSteps = 0;
char motorSteps[] = {0b00000011, 0b00000110, 0b00001100, 0b00001001};

//*********************************************************************************************************
//*********************************************************************************************************
//***********					MAIN Function				*****************************************************************
//*********************************************************************************************************
//*********************************************************************************************************
uint16_t dev = 0x29; // address of the ToF sensor as an I2C slave peripheral
int status = 0;


void spin(int direction){
	// if direction is 1, spin the shit clockwise, otherwise spin ccw
	
	
	uint16_t Distance;
	if (direction == 1){
			GPIO_PORTN_DATA_R |= 0b0000001;
	}
	else {
			GPIO_PORTN_DATA_R &= 0b1111110;
	}
	
	
	for (int m = 0; m < 32; m++)
		{
			int spinMotor = 0;

			int delay = 300000;
			int angle = 64;
			for (int j = 0; j < 16; j++)
			{
				GPIO_PORTF_DATA_R ^= 0b0010000;
				
				// Clockwise and Counter Clock Wise
				if (direction == 1) {
					for (int k = 0; k < 4; k++)
					// for (int k = 4; k >=0; k--)
					{
						GPIO_PORTH_DATA_R = motorSteps[k];
						SysTick_Wait(delay);
					}
				}
				else{
					for (int k = 3; k >=0; k--)
					{
						GPIO_PORTH_DATA_R = motorSteps[k];
						SysTick_Wait(delay);
					}
				}
				
				totalSteps++;
				//turn the led shit on
				GPIO_PORTF_DATA_R ^= 0b0010000;
			}
			SysTick_Wait10ms(3);

			status = VL53L1X_GetDistance(dev, &Distance);
			sprintf(printf_buffer, "%u\r\n", Distance);

			UART_printf(printf_buffer);
			// reset the steps and stop the motor
			if (totalSteps == 512)
			{
				spinMotor = 0;
				totalSteps = 0;
			}
		}
}

void bus_pulse(){
	while (1) {
		GPIO_PORTH_DATA_R=0b00001111;
		SysTick_Wait10ms(1);
		GPIO_PORTH_DATA_R=0b00000000;
		SysTick_Wait10ms(1);  
 
	}
}




int main(void)
{
	uint16_t wordData;
	uint8_t ToFSensor = 1; // 0=Left, 1=Center(default), 2=Right
	uint16_t Distance;
	uint16_t SignalRate;
	uint16_t AmbientRate;
	uint16_t SpadNum;
	uint8_t RangeStatus;

	
	// initialize
	PLL_Init();
	PortH_Init();
	PortF_Init();
	PortN_Init();
	SysTick_Init();
	onboardLEDs_Init();
	
	//bus_pulse();
	
	I2C_Init();
	UART_Init();

	char j;
	char TxChar;
	int input = 0;
	int num_scans = 2; // must be an even number

	status = VL53L1X_ClearInterrupt(dev); /* clear interrupt has to be called to enable next interrupt*/
	status = VL53L1X_SensorInit(dev);
	Status_Check("SensorInit", status);

	status = VL53L1X_StartRanging(dev); /* This function has to be called to enable the ranging */
	Status_Check("StartRanging", status);
	while (1)
	{
		// wait for the right transmition initiation code
		while (1)
		{
			input = UART_InChar();
			if (input == 's')
				break;
		}
		
		for (int i=0; i<num_scans/2; i++){
			spin(1);
			SysTick_Wait10ms(25);
			spin(-1);
			SysTick_Wait10ms(25);
		}
		spin(1);
	
		
	}
}
/*================================= DSP - LAB5_2 ==============================*/
// Version with WD timer
#include "DSP2833x_Device.h"

// Prototype statements for functions found within this file.
void Gpio_select(void);
void InitSystem(void);
void delay_loop(long);

//###########################################################################
//						main code
//###########################################################################
void main(void)
{
	InitSystem();
	DINT;
	Gpio_select();

	while(1)
	{
		//==== "Knight Rider": Sets Led in Sequence === //
		
        // 10
        GpioDataRegs.GPASET.bit.GPIO31 = 1;         // Sets LED 1
		GpioDataRegs.GPBCLEAR.bit.GPIO34 = 1;       // Resets LED 2
	  	delay_loop(1000000);

		// 01
		GpioDataRegs.GPBSET.bit.GPIO34 = 1;         // Sets LED 2
        GpioDataRegs.GPACLEAR.bit.GPIO31 = 1;       // Resets LED 1

	  	delay_loop(1000000);
	}
}

void delay_loop(long end)
{
	long i;
	for (i = 0; i < end; i++)
	{
		asm(" NOP");
		EALLOW;
		// Feeds WD timer so i doesnt crash the code
		SysCtrlRegs.WDKEY = 0x55;
		SysCtrlRegs.WDKEY = 0xAA;
		EDIS;
	}
}

void Gpio_select(void)
{
	EALLOW;

	// Set all pins as Digital IOs [Not PWM, CAN, SPI, etc]
	GpioCtrlRegs.GPAMUX1.all = 0;			// [ GPIO15 ... GPIO0  ]
	GpioCtrlRegs.GPAMUX2.all = 0;			// [ GPIO31 ... GPIO16 ]

	GpioCtrlRegs.GPBMUX1.all = 0;			// [ GPIO47 ... GPIO32 ]
	GpioCtrlRegs.GPBMUX2.all = 0;			// [ GPIO63 ... GPIO48 ]

	GpioCtrlRegs.GPCMUX1.all = 0;			// [ GPIO79 ... GPIO64 ]
	GpioCtrlRegs.GPCMUX2.all = 0;			// [ GPIO87 ... GPIO80 ]


	// Port A Setup
	GpioCtrlRegs.GPADIR.all = 0;			// Sets all pins in Port B [ GPIO31 ... 16] as inputs
	GpioCtrlRegs.GPADIR.bit.GPIO31 = 1;		// Set LED LD3 at GPIO31 as an digital Out

	// Port B Setup
	GpioCtrlRegs.GPBDIR.all = 0;			// Sets all pins in Port B [ GPIO63 ... 32] as inputs
	GpioCtrlRegs.GPBDIR.bit.GPIO34 = 1;		// Set LED LD3 at GPIO34 as an digital Out
	GpioCtrlRegs.GPBDIR.bit.GPIO49 = 1; 	// Set LED LD4 at GPIO49 an an digital Out

	EDIS;
}

void InitSystem(void)
{
	EALLOW;
    SysCtrlRegs.WDCR 				= 0x0028;		// Enabled WD [Timeout of 4.3 ms]

	// Set internal frequency of the DSP to 150MHz (30M*10/2)
	SysCtrlRegs.PLLSTS.bit.DIVSEL 	= 2;			// n =2
	SysCtrlRegs.PLLCR.bit.DIV 		= 10;			// CLKIN = OSCCLK x 10 / n

	// Peripherals
	SysCtrlRegs.HISPCP.all 			= 0x0001;		// Clk divider for ADC,(001 = SYSCLKOUT/2)
   	SysCtrlRegs.LOSPCP.all 			= 0x0002;		// Clk divider for ADC,(010 = SYSCLKOUT/4)
	
	// Disable clock for all peripherals, except GPIO
	SysCtrlRegs.PCLKCR0.all 		= 0x0000;		// Disable Clock
	SysCtrlRegs.PCLKCR1.all 		= 0x0000;		// Disable Clock
	SysCtrlRegs.PCLKCR3.all 		= 0x0000;		// Disable Clock
	SysCtrlRegs.PCLKCR3.bit.GPIOINENCLK = 1;		// Enable Clock for GPIO
    EDIS;
}
//===========================================================================
// End of SourceCode.
//===========================================================================

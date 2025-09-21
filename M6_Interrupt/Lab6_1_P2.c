/* CTG - UFPE*/
/* LAB 6.1:  Interrupt Routine for DSP */

//
// FILE:	Lab6.c


#include "DSP281x_Device.h"
// Prototype statements for functions found within this file.
// 08. Add External Function
extern void InitSysCtrl(void);
interrupt void cpu_timer0_isr(void);
extern void InitPieCtrl(void);

void Gpio_select(void);
void SpeedUpRevA(void);
void show_ADC(int result); 
interrupt void adc_isr(void);	// Prototype for ADC result ISR
 
// Global variables used in this example:

int Voltage_A0;
int Voltage_B0;    

void main(void)
{
	unsigned long i;
	InitSysCtrl();		// Initialize the DSP's core Registers
	// 11. Reenables Watchdog
    EALLOW;
    SysCtrlRegs.WDCR = 0x00AF;
    EDIS;

	// Speed_up the silicon A Revision. 
	// No need to call this function for Rev. C  later silicon versions
	SpeedUpRevA();
	
	Gpio_select();		// Setup the GPIO Multiplex Registers


	InitPieCtrl();		// Function Call to init PIE-unit ( code : DSP281x_PieCtrl.c)
	
	InitPieVectTable(); // Function call to init PIE vector table ( code : DSP281x_PieVect.c )
	 
	InitAdc();			// Function call for basic ADC initialisation
	   
	// re-map PIE - entry for GP Timer 1 Compare Interrupt 
	EALLOW;  // This is needed to write to EALLOW protected registers
   	PieVectTable.ADCINT = &adc_isr;
   	EDIS;    // This is needed to disable write to EALLOW protected registers
	
	// Enable ADC interrupt: PIE-Group1 , interrupt 6
	PieCtrlRegs.PIEIER1.bit.INTx6 = 1;	                    
	
	// Enable CPU INT1 which is connected to ADC interrupt:
    IER = 1;
    
	// Enable global Interrupts and higher priority real-time debug events:
   	EINT;   // Enable Global interrupt INTM
   	ERTM;   // Enable Global realtime interrupt DBGM
   	 
   	// Configure ADC
   	AdcRegs.ADCTRL1.bit.SEQ_CASC = 0;	   // Dual Sequencer Mode
   	AdcRegs.ADCTRL1.bit.CONT_RUN = 0;	   // No Continuous run
   	AdcRegs.ADCTRL1.bit.CPS = 0;		   // prescaler = 1	
   	AdcRegs.ADCMAXCONV.all = 0x0001;       // Setup 2 conv's on SEQ1  
    AdcRegs.ADCCHSELSEQ1.bit.CONV00 = 0x0; // Setup ADCINA0 as 1st SEQ1 conv.// Assumes EVA Clock is already enabled in InitSysCtrl();
    AdcRegs.ADCCHSELSEQ1.bit.CONV01 = 0x8; // Setup ADCINB0 as 2nd SEQ1 conv.// Drive T1PWM / T2PWM by T1/T2 - logic
   	AdcRegs.ADCTRL2.bit.EVA_SOC_SEQ1 = 1;  // Enable EVASOC to start SEQ1// Polarity of GP Timer 1 Compare = Active low
	AdcRegs.ADCTRL2.bit.INT_ENA_SEQ1 = 1;  // Enable SEQ1 interrupt (every EOEvaRegs.GPTCONA.bit.T1PIN = 1;
    AdcRegs.ADCTRL3.bit.ADCCLKPS = 2;	   // Divide HSPCLK by 4

  	// Configure EVA
    // Assumes EVA Clock is already enabled in InitSysCtrl();
    // Disable T1PWM / T2PWM outputs
    EvaRegs.GPTCONA.bit.TCMPOE = 0;
   	// Polarity of GP Timer 1 Compare = forced low
	EvaRegs.GPTCONA.bit.T1PIN = 0;
	EvaRegs.GPTCONA.bit.T1TOADC = 2;       // Enable EVASOC in EVA
	
	
	EvaRegs.T1CON.bit.FREE = 0;				// Stop on emulation suspend
    EvaRegs.T1CON.bit.SOFT = 0;				// Stop on emulation suspend
    EvaRegs.T1CON.bit.TMODE = 2;			// Continuous up count mode
    EvaRegs.T1CON.bit.TPS = 7;				// prescaler = 128
    EvaRegs.T1CON.bit.TENABLE = 1;			// enable GP Timer 1 
   	EvaRegs.T1CON.bit.TCLKS10 = 0;			// internal clock
   	EvaRegs.T1CON.bit.TCLD10 = 0;			// Compare Reload when zero
   	EvaRegs.T1CON.bit.TECMPR = 0;			// Disable Compare operation
                                         	
   	EvaRegs.T1PR = 58594;     

   	while(1)
	{    
  	    for(i=0;i<1500000;i++)
  	    {
  	    EALLOW;
    	SysCtrlRegs.WDKEY = 0xAA;		// and serve watchdog #2		
	    EDIS;
     	show_ADC(Voltage_A0>>8);	/* displays the latest result on LED's */
    	}
    	for(i=0;i<1500000;i++)
  	    {
  	    EALLOW;
    	SysCtrlRegs.WDKEY = 0xAA;		// and serve watchdog #2		
	    EDIS;
     	show_ADC(Voltage_B0>>8);  
  		}
    }
} 		
   


   
void Gpio_select(void)
{
	EALLOW;
	GpioMuxRegs.GPAMUX.all = 0x0;	// all GPIO port Pin's to I/O   
    GpioMuxRegs.GPBMUX.all = 0x0;   
    GpioMuxRegs.GPDMUX.all = 0x0;
    GpioMuxRegs.GPFMUX.all = 0x0;		 
    GpioMuxRegs.GPEMUX.all = 0x0; 
    GpioMuxRegs.GPGMUX.all = 0x0;			
										
    GpioMuxRegs.GPADIR.all = 0x0;	// GPIO PORT  as input
    GpioMuxRegs.GPBDIR.all = 0x00FF;// GPIO Port B15-B8 input , B7-B0 output
    GpioMuxRegs.GPDDIR.all = 0x0;	// GPIO PORT  as input
    GpioMuxRegs.GPEDIR.all = 0x0;	// GPIO PORT  as input
    GpioMuxRegs.GPFDIR.all = 0x0;	// GPIO PORT  as input
    GpioMuxRegs.GPGDIR.all = 0x0;	// GPIO PORT  as input

    GpioMuxRegs.GPAQUAL.all = 0x0;	// Set GPIO input qualifier values to zero
    GpioMuxRegs.GPBQUAL.all = 0x0;
    GpioMuxRegs.GPDQUAL.all = 0x0;
    GpioMuxRegs.GPEQUAL.all = 0x0;
    EDIS;
}     

void SpeedUpRevA(void)
{
// On TMX samples, to get the best performance of on chip RAM blocks M0/M1/L0/L1/H0 internal
// control registers bit have to be enabled. The bits are in Device emulation registers.
   	EALLOW;
   	DevEmuRegs.M0RAMDFT = 0x0300;
   	DevEmuRegs.M1RAMDFT = 0x0300;
   	DevEmuRegs.L0RAMDFT = 0x0300;
   	DevEmuRegs.L1RAMDFT = 0x0300;
   	DevEmuRegs.H0RAMDFT = 0x0300;
   	EDIS;
}

void InitSystem(void)
{
   	EALLOW;
   	SysCtrlRegs.WDCR= 0x00AF;		// Setup the watchdog 
   									// 0x00E8  to disable the Watchdog , Prescaler = 1
   									// 0x00AF  to NOT disable the Watchdog, Prescaler = 64
   	SysCtrlRegs.SCSR = 0; 			// Watchdog generates a RESET	
   	SysCtrlRegs.PLLCR.bit.DIV = 10;	// Setup the Clock PLL to multiply by 5
    
   	SysCtrlRegs.HISPCP.all = 0x1; // Setup Highspeed Clock Prescaler to divide by 2
   	SysCtrlRegs.LOSPCP.all = 0x2; // Setup Lowspeed CLock Prescaler to divide by 4
      	
   	// Peripheral clock enables set for the selected peripherals.   
   	SysCtrlRegs.PCLKCR.bit.EVAENCLK=1;
   	SysCtrlRegs.PCLKCR.bit.EVBENCLK=0;
   	SysCtrlRegs.PCLKCR.bit.SCIAENCLK=0;
   	SysCtrlRegs.PCLKCR.bit.SCIBENCLK=0;
   	SysCtrlRegs.PCLKCR.bit.MCBSPENCLK=0;
   	SysCtrlRegs.PCLKCR.bit.SPIENCLK=0;
   	SysCtrlRegs.PCLKCR.bit.ECANENCLK=0;
   	SysCtrlRegs.PCLKCR.bit.ADCENCLK=1;
   	EDIS;
}
        
void show_ADC(int result)
/* show the result of the AD-conversion on 8 LED's on GPIO B0-B7	*/
/* the result will be show as light-beam							*/
{
	switch(result) {
		case 0 : GpioDataRegs.GPBDAT.all=0x0000;break;
		case 1 : GpioDataRegs.GPBDAT.all=0x0001;break;
		}
	result>>=1;
	switch(result) {
		case 1 : GpioDataRegs.GPBDAT.all=0x0003;break;
		case 2 : GpioDataRegs.GPBDAT.all=0x0007;break;
		case 3 : GpioDataRegs.GPBDAT.all=0x000F;break;
		case 4 : GpioDataRegs.GPBDAT.all=0x001F;break;
		case 5 : GpioDataRegs.GPBDAT.all=0x003F;break;
		case 6 : GpioDataRegs.GPBDAT.all=0x007F;break;
		case 7 : GpioDataRegs.GPBDAT.all=0x00FF;break;
		}
}	    

interrupt void adc_isr(void)  
{
   	// Serve the watchdog every Timer 0 interrupt
   	EALLOW;
	SysCtrlRegs.WDKEY = 0x55;		// Serve watchdog #1
	EDIS;  
    
    Voltage_A0 = AdcRegs.ADCRESULT0>>4;
    Voltage_B0 = AdcRegs.ADCRESULT1>>4;

  // Reinitialize for next ADC sequence
  AdcRegs.ADCTRL2.bit.RST_SEQ1 = 1;         // Reset SEQ1
  AdcRegs.ADCST.bit.INT_SEQ1_CLR = 1;		// Clear INT SEQ1 bit
  PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;   // Acknowledge interrupt to PIE
  }          
//===========================================================================
// End of SourceCode.
//===========================================================================


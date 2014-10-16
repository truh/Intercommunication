#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/timer.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/can.h"
#include "buttons.h"

#include "protocol.h"
#include "util.h"

#define LED_RED GPIO_PIN_1
#define LED_BLUE GPIO_PIN_2
#define LED_GREEN GPIO_PIN_3

uint32_t g_iTime = 0;

void StartTimer()
{
	g_iTime = 0;
	ROM_TimerEnable(TIMER0_BASE, TIMER_A);
}

void StopTimer()
{
	ROM_TimerDisable(TIMER0_BASE, TIMER_A);
}

void SetupSSI()
{
    // The SSI2 peripheral must be enabled for use.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI2);

    // SSI2 -> PortA[5:2].
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    // Configure the pin muxing for SSI2 functions on port B4, B5, B6, and B7.
    GPIOPinConfigure(GPIO_PB4_SSI2CLK);
    GPIOPinConfigure(GPIO_PB5_SSI2FSS);
    GPIOPinConfigure(GPIO_PB6_SSI2RX);
    GPIOPinConfigure(GPIO_PB7_SSI2TX);

    // Configure the GPIO settings for the SSI pins.
    //      PB7 - SSI2Tx
    //      PB6 - SSI2Rx
    //      PB5 - SSI2Fss
    //      PB4 - SSI2CLK
    GPIOPinTypeSSI(GPIO_PORTB_BASE, GPIO_PIN_7 | GPIO_PIN_6 | GPIO_PIN_5 | GPIO_PIN_4);

    // Configure and enable the SSI port for TI master mode.  Use SSI2, system
    // clock supply, master mode, 1MHz SSI frequency, and 8-bit data.
    SSIConfigSetExpClk(SSI2_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0,
                       SSI_MODE_MASTER, 2000000, 8);

    // Enable the SSI2 module.
    SSIEnable(SSI2_BASE);
}

void OnDataReceived(void)
{
	SSIIntClear(SSI2_BASE, SSIIntStatus(SSI2_BASE, true));
}

void OnButtonPressed(void)
{
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, LED_RED|LED_GREEN|LED_BLUE, 0);
	
	uint8_t buttonStates = (uint8_t)~ROM_GPIOPinRead(BUTTONS_GPIO_BASE, ALL_BUTTONS);
    if(buttonStates & LEFT_BUTTON)
	{
		GPIOIntClear(GPIO_PORTF_BASE, LEFT_BUTTON);
		UARTprintf("\nleft button was pressed");
		
		StartTimer();
		
	}
	if(buttonStates & RIGHT_BUTTON)
	{
		GPIOIntClear(GPIO_PORTF_BASE, RIGHT_BUTTON);
		UARTprintf("\nright button was pressed");
		
		StartTimer();
	}
}

void OnTimerInterrupt(void)
{
	// Clear the timer interrupt
	ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	
	g_iTime += 1;
	
	uint8_t buttonStates = (uint8_t)~ROM_GPIOPinRead(BUTTONS_GPIO_BASE, ALL_BUTTONS);
	
	if(!(buttonStates & LEFT_BUTTON) && !(buttonStates & RIGHT_BUTTON)) StopTimer();
	
	if(g_iTime >= 2000)
	{
		StopTimer();
		
		uint32_t led = 0;
		if(buttonStates & LEFT_BUTTON) led = P_LED_GREEN;
		if(buttonStates & RIGHT_BUTTON) led = P_LED_RED;
		
		for(int i = 0;i<4;i++)
		{
        	SSIDataPut(SSI2_BASE, led);
			UARTprintf("sent data");
    	}   

    	// Wait until SSI2 is done transferring all the data in the transmit FIFO.
    	while(SSIBusy(SSI2_BASE))
		{
			ROM_GPIOPinWrite(GPIO_PORTF_BASE, LED_RED|LED_GREEN|LED_BLUE, LED_BLUE);
		}
	
		ROM_GPIOPinWrite(GPIO_PORTF_BASE, LED_RED|LED_GREEN|LED_BLUE, LED_GREEN);
	}
}

void EnableButtons(void)
{
	// Enable the GPIO port to which the push buttons are connected.
	ROM_SysCtlPeripheralEnable(BUTTONS_GPIO_PERIPH);
	
	// Unlock PF0 to set the buttons to GPIO input
  	HWREG(BUTTONS_GPIO_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
  	HWREG(BUTTONS_GPIO_BASE + GPIO_O_CR) |= 0x01;
  	HWREG(BUTTONS_GPIO_BASE + GPIO_O_LOCK) = 0;
	
  	ROM_GPIODirModeSet(BUTTONS_GPIO_BASE, ALL_BUTTONS, GPIO_DIR_MODE_IN);
  	ROM_GPIOPadConfigSet(BUTTONS_GPIO_BASE, ALL_BUTTONS,
      GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
	  
	// Enable the port/pins for interrupts
	ROM_GPIOIntTypeSet(GPIO_PORTF_BASE, ALL_BUTTONS, GPIO_FALLING_EDGE);
	GPIOIntEnable(BUTTONS_GPIO_BASE, ALL_BUTTONS);
	ROM_IntEnable(INT_GPIOF);
}

void EnableInterrupt(void)
{
    // Enable SPI interrupt 
    IntEnable(INT_SSI2);
    
    // SPI interrupt from receiving data
    SSIIntEnable(SSI2_BASE, SSI_RXFF);
}

void EnableLED(void)
{
	// Enable the LEDs
  	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
  	ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, LED_RED|LED_BLUE|LED_GREEN);
}

void EnableTimer(void)
{
	// Enable the Timer Port used in this program
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    // Configure the 32 bit periodic timer
    ROM_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    
	ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, ROM_SysCtlClockGet() / 1000);

    // Setup the interrupts for the timer timeouts.
    ROM_IntEnable(INT_TIMER0A);
    ROM_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}

int main(void)
{
	// Set the clocking to run directly from the external crystal/oscillator.
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ);
				   
	// Set up the serial console to use for displaying messages.  This is
    // just for this example program and is not needed for SSI operation.
    InitConsole();
  	
	EnableButtons();
	EnableLED();
	EnableTimer();
	SetupSSI();
	
	UARTprintf("\nInitialized\n");

    // Read any residual data from the SSI port.  This makes sure the receive
    // FIFOs are empty, so we don't read any unwanted junk.  This is done here
    // because the TI SSI mode is full-duplex, which allows you to send and
    // receive at the same time.  The SSIDataGetNonBlocking function returns
    // "true" when data was returned, and "false" when no data was returned.
    // The "non-blocking" function checks if there is any data in the receive
    // FIFO and does not "hang" if there isn't.
    while(SSIDataGetNonBlocking(SSI2_BASE, NULL));
	
	//int i = 0;
	//while(i <= (NUM_DATA + 1))
	//{
	//	if(blubb[i] == 0x00) break;
	//	SSIDataPut(SSI2_BASE, blubb[i]);
	//	UARTprintf("\nChar sent: %c", blubb[i]);
	//	i++;
	//}

    EnableInterrupt();

    OS();  // start the operating system
	
    return(0);
}
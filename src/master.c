/*
  Name of file  : master.c
  Author        : Andreas Willinger
  Version       : 20141020.1
  Description   : Handles incoming data from the slave and turns the LEDs on/off.
*/
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"

#include "protocol.h"
#include "util.h"

#define LED_RED GPIO_PIN_1
#define LED_BLUE GPIO_PIN_2
#define LED_GREEN GPIO_PIN_3

void OnDataReceived(void)
{
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, LED_RED|LED_GREEN|LED_BLUE, LED_BLUE);	
	unsigned long int_source = SSIIntStatus(SSI2_BASE, true);

	if(int_source & SSI_RXFF)
	{
		uint32_t data;

		for(int_source = 0; int_source < NUM_DATA; int_source++)
		{
			SSIDataGet(SSI2_BASE, &data);
		}
		
		// reset LED's
		ROM_GPIOPinWrite(GPIO_PORTF_BASE, LED_RED|LED_GREEN|LED_BLUE, 0);
		uint32_t leds;
		
		if(data & P_LED_GREEN) leds |= LED_GREEN;
		if(data & P_LED_RED) leds |= LED_RED;
		if(data & P_LED_BLUE) leds |= LED_BLUE;
		ROM_GPIOPinWrite(GPIO_PORTF_BASE, LED_RED|LED_GREEN|LED_BLUE, leds);
	}
	
	SSIIntClear(SSI2_BASE, int_source);
}

void OnButtonPressed(void)
{
}

void OnTimerInterrupt(void)
{
}

void EnableInterrupt(void)
{
    // Enable SPI interrupt 
    IntEnable(INT_SSI2);
    
    // SPI interrupt from receiving data
    SSIIntEnable(SSI2_BASE, SSI_RXFF);
}

int main(void)
{
    // Set the clocking to run directly from the external crystal/oscillator.
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ);

	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
  	ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, LED_RED|LED_BLUE|LED_GREEN);
	
    // Set up the serial console to use for displaying messages.  This is
    // just for this example program and is not needed for SSI operation.
    InitConsole();
	
    SetupSSI(SSI_MODE_SLAVE);

    // Read any residual data from the SSI port.  This makes sure the receive
    // FIFOs are empty, so we don't read any unwanted junk.  This is done here
    // because the TI SSI mode is full-duplex, which allows you to send and
    // receive at the same time.  The SSIDataGetNonBlocking function returns
    // "true" when data was returned, and "false" when no data was returned.
    // The "non-blocking" function checks if there is any data in the receive
    // FIFO and does not "hang" if there isn't.
    while(SSIDataGetNonBlocking(SSI2_BASE, NULL));
	
	EnableInterrupt();
	
    OS();  // start the operating system
	
    return(0);
}
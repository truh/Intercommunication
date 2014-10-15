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
                       SSI_MODE_SLAVE, 2000000, 8);
	
	SSIAdvModeSet(SSI2_BASE, SSI_ADV_MODE_READ_WRITE);

    // Enable the SSI2 module.
    SSIEnable(SSI2_BASE);
}

void OnDataReceived(void)
{
	UARTprintf("\ndata received");
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
	
    SetupSSI();

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
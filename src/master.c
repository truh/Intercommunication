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
    GPIOPinTypeSSI(GPIO_PORTB_BASE, GPIO_PIN_7 | GPIO_PIN_6 | GPIO_PIN_5 |
                   GPIO_PIN_4);

    // Configure and enable the SSI port for TI master mode.  Use SSI2, system
    // clock supply, master mode, 1MHz SSI frequency, and 8-bit data.
    SSIConfigSetExpClk(SSI2_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0,
                       SSI_MODE_MASTER, 2000000, 8);

    // Enable the SSI2 module.
    SSIEnable(SSI2_BASE);
}

void OnDataReceived(void)
{
	UARTprintf("\n\n*** DATA RECEIVED ***");
	
	unsigned long int_source = SSIIntStatus(SSI2_BASE, true);
	unsigned long rx_data_size;
	
	SSIIntClear(SSI2_BASE, int_source);
	
	if(int_source & SSI_RXTO)
	{
		uint32_t received;
		rx_data_size = SSIDataGetNonBlocking(SSI2_BASE, &received);
		
		UARTprintf("\n%d bits of data received via SSI: %d", rx_data_size, received);
	}
	else
	{
		UARTprintf("\nsent stuff");
	}
}

void EnableInterrupt(void)
{
    // Enable SPI interrupt 
    IntEnable(INT_SSI2);
    
    // SPI interrupt from receiving data
    // TODO Set when to call the interrupt
    SSIIntEnable(SSI2_BASE, SSI_RXFF);
}

int main(void)
{
    // Set the clocking to run directly from the external crystal/oscillator.
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ);

	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
  	ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, LED_RED|LED_BLUE|LED_GREEN);
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, LED_RED|LED_GREEN|LED_BLUE, LED_RED);
	
    // Set up the serial console to use for displaying messages.  This is
    // just for this example program and is not needed for SSI operation.
    //InitConsole();
	
    SetupSSI();

    // Read any residual data from the SSI port.  This makes sure the receive
    // FIFOs are empty, so we don't read any unwanted junk.  This is done here
    // because the TI SSI mode is full-duplex, which allows you to send and
    // receive at the same time.  The SSIDataGetNonBlocking function returns
    // "true" when data was returned, and "false" when no data was returned.
    // The "non-blocking" function checks if there is any data in the receive
    // FIFO and does not "hang" if there isn't.
    while(SSIDataGetNonBlocking(SSI2_BASE, NULL));
	
	char *blubb = "* Hallo welt, bitte funktier endlich du scheiss";
	
	int i = 0;
	while(blubb[i])
	{
		SSIDataPut(SSI2_BASE, blubb[i]);
		i++;
	}
	//SSIDataPut(SSI2_BASE, 0xFF);

    // Wait until SSI2 is done transferring all the data in the transmit FIFO.
    while(SSIBusy(SSI2_BASE))
	{
		ROM_GPIOPinWrite(GPIO_PORTF_BASE, LED_RED|LED_GREEN|LED_BLUE, LED_BLUE);
	}
	
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, LED_RED|LED_GREEN|LED_BLUE, LED_GREEN);
	
	
	//SSIDataGet(SSI2_BASE, &pui32DataRx[ui32Index]);

	// Since we are using 8-bit data, mask off the MSB.
	//pui32DataRx[ui32Index] &= 0x00FF;*/
	
    EnableInterrupt();

    OS();  // start the operating system
	
    return(0);
}
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"

#include "util.h"

void OnDataReceived(void);

void SetupSSI() {
    // The SSI0 peripheral must be enabled for use.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);

    // SSI0 -> PortA[5:2].
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // Configure the pin muxing for SSI0 functions on port A2, A3, A4, and A5.
    GPIOPinConfigure(GPIO_PA2_SSI0CLK);
    GPIOPinConfigure(GPIO_PA3_SSI0FSS);
    GPIOPinConfigure(GPIO_PA4_SSI0RX);
    GPIOPinConfigure(GPIO_PA5_SSI0TX);

    // Configure the GPIO settings for the SSI pins.
    //      PA5 - SSI0Tx
    //      PA4 - SSI0Rx
    //      PA3 - SSI0Fss
    //      PA2 - SSI0CLK
    GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_3 |
                   GPIO_PIN_2);

    // Configure and enable the SSI port for TI master mode.  Use SSI0, system
    // clock supply, master mode, 1MHz SSI frequency, and 8-bit data.
    SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_TI,
                       SSI_MODE_MASTER, 1000000, 8);

    // Enable the SSI0 module.
    SSIEnable(SSI0_BASE);
}

int main(void)
{
    // Set the clocking to run directly from the external crystal/oscillator.
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ);

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
    while(SSIDataGetNonBlocking(SSI0_BASE, NULL));
	
	/*
	SSIDataPut(SSI0_BASE, pui32DataTx[ui32Index]);

    // Wait until SSI0 is done transferring all the data in the transmit FIFO.
    while(SSIBusy(SSI0_BASE));

	SSIDataGet(SSI0_BASE, &pui32DataRx[ui32Index]);

	// Since we are using 8-bit data, mask off the MSB.
	pui32DataRx[ui32Index] &= 0x00FF;*/
	
	// OnDataReceived will be the interrupt
	IntRegister(INT_SSI0, OnDataReceived);

	// Enable SPI interrupt	
	IntEnable(INT_SSI0);
	
	// SPI interrupt from receiving data
	// TODO Set when to call the interrupt
	SSIIntEnable(SSI0_BASE, SSI_RXFF);
	
	// Interrupt enable
	IntMasterEnable();
	
    return(0);
}

void OnDataReceived(void)
{
	UARTprintf("\n\n*** DATA RECEIVED ***");
	
	unsigned long int_source = SSIIntStatus(SSI0_BASE, true);
	unsigned long rx_data_size;
	
	SSIIntClear(SSI0_BASE, int_source);
	
	if(int_source & SSI_RXTO)
	{
		bool received = false;
		rx_data_size = SSIDataGetNonBlocking(SSI0_BASE, &received);
		
		UARTprintf("\nData received via SSI: %s", (received)?"true":"false");
	}
}
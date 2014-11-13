//*****************************************************************************
//
// spi_slave.c - Example demonstrating how to configure RX timeout interrupt in
// SPI slave mode.
//
// Copyright (c) 2013 Texas Instruments Incorporated.  All rights reserved.
// TI Information - Selective Disclosure
//
//*****************************************************************************
#include <stdbool.h>
#include <stdint.h>

#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/hw_ssi.h"

#include "driverlib/ssi.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom_map.h"
#include "driverlib/uart.h"

#include "utils/uartstdio.h"

#define LED_RED GPIO_PIN_1
#define LED_BLUE GPIO_PIN_2
#define LED_GREEN GPIO_PIN_3


//*****************************************************************************
//
//! \addtogroup ssi_examples_list
//! <h1>SPI Slave (spi_slave)</h1>
//!
//! This example configures the SSI0 as SPI Master, SSI2 as SPI Slave on an
//! EK-LM4F232 evaluation board.  RX timeout interrupt is configured for SSI2.
//! Three characters are sent on the master TX, then SSI2 RX timeout interrupt
//! is enabled. The code then waits for the interrupt to fire.  Once the
//! interrupt is fired the data from slave RX FIFO is read and compared to the
//! transmitted packet and the appropriate status is displayed.  If everything
//! goes well you should see a "Test Passed." message on the terminal window.
//! The status messages are transmitted over UART0 at 115200 baud and 8-n-1
//! mode.
//!
//! This example uses the following peripherals and I/O signals on EK-TM4C123GXL.
//! You must review these and change as needed for your own board:
//! - SSI0 peripheral
//! - GPIO Port A peripheral (for SSI0 pins) (available on the right side)
//! - SSI0CLK - PA2
//! - SSI0Fss - PA3
//! - SSI0Rx  - PA4
//! - SSI0Tx  - PA5
//!
//! - SSI2 peripheral
//! - GPIO Port M peripheral (for SSI2 pins) (mostly on the left side)
//! - SSI2CLK - PB4
//! - SSI2Fss - PB5
//! - SSI2Rx  - PB6
//! - SSI2Tx  - PB7
//!
//! For this example to work, the following connections are needed on the
//! EK-LM4F232 evaluation board.
//! - SSI0CLK(PA2) - SSI2CLK(PB4)
//! - SSI0Fss(PA3) - SSI0Fss(PB5)
//! - SSI0Rx(PA4)  - SSI2Tx(PB7)
//! - SSI0Tx(PA5)  - SSI2Rx(Pb6)
//!
//! The following UART signals are configured only for displaying console
//! messages for this example.  These are not required for operation of SSI0.
//! - UART0 peripheral
//! - GPIO Port A peripheral (for UART0 pins)
//! - UART0RX - PA0
//! - UART0TX - PA1
//!
//! This example uses the following interrupt handlers.  To use this example
//! in your own application you must add these interrupt handlers to your
//! vector table.
//! - SSI2IntHandler.
//!
//
//*****************************************************************************

//*****************************************************************************
//
// Number of bytes to send and receive.
//
//*****************************************************************************
#define NUM_SSI_DATA 4

//*****************************************************************************
//
// Global variables used in interrupt handler and the main loop.
//
//*****************************************************************************
volatile unsigned long g_ulSSI2RXTO = 0;
unsigned long g_ulDataRx2[NUM_SSI_DATA];

//*****************************************************************************
//
// Interrupt handler for SSI2 peripheral in slave mode.  It reads the interrupt
// status and if the interrupt is fired by a RX time out interrupt it reads the
// SSI2 RX FIFO and increments a counter to tell the main loop that RX timeout
// interrupt was fired.
//
//*****************************************************************************
	void
SSI2IntHandler(void)
{
	unsigned long ulStatus, ulIndex;

	//
	// Read interrupt status.
	//
	ulStatus = SSIIntStatus(SSI2_BASE, 1);

	//
	// Check the reason for the interrupt.
	//
	if(ulStatus & SSI_RXTO)
	{
		//
		// Interrupt is because of RX time out.  So increment counter to tell
		// main loop that RX timeout interrupt occurred.
		//
		g_ulSSI2RXTO++;

		//
		// Read NUM_SSI_DATA bytes of data from SSI2 RX FIFO.
		//
		for(ulIndex = 0; ulIndex < NUM_SSI_DATA; ulIndex++)
		{
			SSIDataGet(SSI2_BASE, &g_ulDataRx2[ulIndex]);
		}
	}

	//
	// Clear interrupts.
	//
	SSIIntClear(SSI2_BASE, ulStatus);
}

//*****************************************************************************
//
// This function sets up UART0 to be used for a console to display information
// as the example is running.
//
//*****************************************************************************
	void
InitConsole(void)
{
	//
	// Enable GPIO port A which is used for UART0 pins.
	//
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

	//
	// Configure the pin muxing for UART0 functions on port A0 and A1.
	// This step is not necessary if your part does not support pin muxing.
	//
	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);

	//
	// Select the alternate (UART) function for these pins.
	//
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	//
	// Initialize the UART for console I/O.
	//
	UARTStdioConfig(0, 115200, 16000000);
}

//*****************************************************************************
//
// This function sets up SPI0 to be used as Master in freescale mode.
//
//*****************************************************************************
	void
InitSPI0(void)
{
	//
	// The SSI0 peripheral must be enabled for use.
	//
	SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);

	//
	// For this example SSI0 is used with PortA[5:2].  GPIO port A needs to be
	// enabled so these pins can be used.
	//
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

	//
	// Configure the pin muxing for SSI0 functions on port A2, A3, A4, and A5.
	// This step is not necessary if your part does not support pin muxing.
	//
	GPIOPinConfigure(GPIO_PA2_SSI0CLK);
	GPIOPinConfigure(GPIO_PA3_SSI0FSS);
	GPIOPinConfigure(GPIO_PA4_SSI0RX);
	GPIOPinConfigure(GPIO_PA5_SSI0TX);

	//
	// Configure the GPIO settings for the SSI pins.  This function also gives
	// control of these pins to the SSI hardware.  Consult the data sheet to
	// see which functions are allocated per pin.
	// The pins are assigned as follows:
	//      PA5 - SSI0Tx
	//      PA4 - SSI0Rx
	//      PA3 - SSI0Fss
	//      PA2 - SSI0CLK
	//
	GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_3 |
			GPIO_PIN_2);

	//
	// Configure and enable the SSI0 port for SPI master mode.
	//
	SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_2,
			SSI_MODE_MASTER, 660000, 8);

	//
	// Enable the SSI0 module.
	//
	SSIEnable(SSI0_BASE);
}

//*****************************************************************************
//
// This function sets up SPI2 to be used as slave in freescale mode.
//
//*****************************************************************************
	void
InitSPI2(void)
{
	//
	// The SSI0 peripheral must be enabled for use.
	//
	SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI2);

	//
	// For this example SSI2 is used with PortB[7:4].  GPIO port H needs to be
	// enabled so these pins can be used.
	//
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

	//
	// Configure the pin muxing for SSI2 functions on port B4, B5, B6 and B7.
	// This step is not necessary if your part does not support pin muxing.
	//
	GPIOPinConfigure(GPIO_PB4_SSI2CLK);
	GPIOPinConfigure(GPIO_PB5_SSI2FSS);
	GPIOPinConfigure(GPIO_PB6_SSI2RX);
	GPIOPinConfigure(GPIO_PB7_SSI2TX);

	//
	// Configure the GPIO settings for the SSI pins.  This function also gives
	// control of these pins to the SSI hardware.  Consult the data sheet to
	// see which functions are allocated per pin.
	// The pins are assigned as follows:
	//      PB7 - SSI2Tx
	//      PB6 - SSI2Rx
	//      PB5 - SSI2Fss
	//      PB4 - SSI2CLK
	//
	GPIOPinTypeSSI(GPIO_PORTB_BASE, GPIO_PIN_7 | GPIO_PIN_6 | GPIO_PIN_5 |
			GPIO_PIN_4);

	//
	// Configure and enable the SSI2 port for SPI slave mode.
	//
	SSIConfigSetExpClk(SSI2_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_2,
			SSI_MODE_SLAVE, 660000, 8);

	//
	// Enable the SSI2 module.
	//
	SSIEnable(SSI2_BASE);
}

//*****************************************************************************
//
// This example will send out 3 bytes of data from master, then waits for slave
// RX timeout interrupt to fire (where these 3 bytes are read).  Then the sent
// and returned data are compared to give out appropriate status messages on
// UART0.
//
//*****************************************************************************
	int
main(void)
{
	unsigned long ulDataTx0[NUM_SSI_DATA];
	unsigned long ulDataRx0[NUM_SSI_DATA];
	unsigned long ulindex;

	//
	// Set the clocking to run directly from the external crystal/oscillator.
	//
	SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
			SYSCTL_XTAL_16MHZ);

	//
	// Set up the serial console to use for displaying messages.  This is
	// just for this example program and is not needed for SSI operation.
	//
	InitConsole();

	// Enable LEDs
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, LED_RED|LED_BLUE|LED_GREEN);

	//
	// Display the setup on the console.
	//
	UARTprintf("SSI ->\n");
	UARTprintf("  Mode: SPI\n");
	UARTprintf("  Data: 16-bit\n\n");

	//
	// Init SPI0 as master.
	//
	InitSPI0();

	//
	// Read any residual data from the SSI port.  This makes sure the receive
	// FIFOs are empty, so we don't read any unwanted junk.  This is done here
	// because the SPI SSI mode is full-duplex, which allows you to send and
	// receive at the same time.  The SSIDataGetNonBlocking function returns
	// "true" when data was returned, and "false" when no data was returned.
	// The "non-blocking" function checks if there is any data in the receive
	// FIFO and does not "hang" if there isn't.  This might not be needed here.
	//
	while(SSIDataGetNonBlocking(SSI0_BASE, &ulDataRx0[0]))
	{
	}

	//
	// Init SPI2 as slave.
	//
	InitSPI2();

	//
	// Enable RX timeout interrupt.
	//
	SSIIntEnable(SSI2_BASE, SSI_RXTO);

	//
	// Read any residual data from the SSI port.  This makes sure the receive
	// FIFOs are empty, so we don't read any unwanted junk.  This is done here
	// because the SPI SSI mode is full-duplex, which allows you to send and
	// receive at the same time.  The SSIDataGetNonBlocking function returns
	// "true" when data was returned, and "false" when no data was returned.
	// The "non-blocking" function checks if there is any data in the receive
	// FIFO and does not "hang" if there isn't.
	//
	while(SSIDataGetNonBlocking(SSI2_BASE, &g_ulDataRx2[0]))
	{
	}

	//
	// Clear any pending interrupt
	//
	SSIIntClear(SSI2_BASE, SSI_RXTO);

	//
	// Initialize the data to send.
	//
	ulDataTx0[0] = 'H';
	ulDataTx0[1] = 'e';
	ulDataTx0[2] = 'y';
	ulDataTx0[3] = '!';

	//
	// Display indication that the SSI is transmitting data.
	//
	UARTprintf("Sent:\n  ");

	//
	// Send 3 bytes of data.
	//
	for(ulindex = 0; ulindex < NUM_SSI_DATA; ulindex++)
	{
		//
		// Display the data that SSI is transferring.
		//
		UARTprintf("'%c' ", ulDataTx0[ulindex]);

		//
		// Send the data using the "blocking" put function.  This function
		// will wait until there is room in the send FIFO before returning.
		// This allows you to assure that all the data you send makes it into
		// the send FIFO.
		//
		SSIDataPut(SSI0_BASE, ulDataTx0[ulindex]);
	}

	//
	// Wait until SSI0 is done transferring all the data in the transmit FIFO.
	//
	while(SSIBusy(SSI0_BASE))
	{
	}

	//
	// Enable the SSI2 interrupts to ARM core.  This has to be done here,
	// otherwise the RX timeout interrupt will fire before all the data has
	// been transferred.  This is specific to this example as both the SSI
	// master and slave are on the same microcontroller.
	//
	IntEnable(INT_SSI2);

	//
	// Wait for the SSI2 RXTO interrupt to fire and data read from RXFIFO.
	//
	while(g_ulSSI2RXTO == 0)
	{
	}

	//
	// Display indication that slave has receiving data.
	//
	UARTprintf("\nReceived:\n  ");

	//
	// Display the 3 bytes of data that were read from RX FIFO.
	//
	for(ulindex = 0; ulindex < NUM_SSI_DATA; ulindex++)
	{
		UARTprintf("'%c' ", g_ulDataRx2[ulindex]);
	}

	//
	// Check that the data sent was the same as the data received.
	//
	for(ulindex = 0; ulindex < NUM_SSI_DATA; ulindex++)
	{
		if(ulDataTx0[ulindex] != g_ulDataRx2[ulindex])
		{
			//
			// Tell the user that the test failed.
			//
			UARTprintf("\n\nError: Data does not exactly match.\n");
			UARTprintf("Check that Tx & Rx are connected correctly.\n\n");

			ROM_GPIOPinWrite(GPIO_PORTF_BASE, LED_RED|LED_GREEN|LED_BLUE, LED_RED);

			//
			// Wait in infinite loop for debugging.
			//
			while(1)
			{
			}
		}
	}

	if(g_ulSSI2RXTO > 1)
	{
		//
		// Tell the user that the test failed and the reason.
		//
		UARTprintf("\n\nError: %d interrupt(s) fired when expecting only one."
				"\n", g_ulSSI2RXTO);

		ROM_GPIOPinWrite(GPIO_PORTF_BASE, LED_RED|LED_GREEN|LED_BLUE, LED_RED);
		ROM_SysCtlDelay(5000000);
		ROM_GPIOPinWrite(GPIO_PORTF_BASE, LED_RED|LED_GREEN|LED_BLUE, 0); 
		ROM_SysCtlDelay(5000000);
	}
	else
	{
		//
		// Tell the user that the test passed.
		//
		UARTprintf("\n\nTest Passed.\n\n");

		ROM_GPIOPinWrite(GPIO_PORTF_BASE, LED_RED|LED_GREEN|LED_BLUE, LED_GREEN);
	}

	while(1)
	{
	}
}

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"

//! This example uses the following peripherals and I/O signals.  You must
//! review these and change as needed for your own board:
//! - SSI0 peripheral
//! - GPIO Port A peripheral (for SSI0 pins)
//! - SSI0Clk - PA2
//! - SSI0Fss - PA3
//! - SSI0Rx  - PA4
//! - SSI0Tx  - PA5
//!
//! The following UART signals are configured only for displaying console
//! messages for this example.  These are not required for operation of I2C0.
//! - UART0 peripheral
//! - GPIO Port A peripheral (for UART0 pins)
//! - UART0RX - PA0
//! - UART0TX - PA1

//*****************************************************************************
//
// This function sets up UART0 to be used for a console to display information
// as the example is running.
//
//*****************************************************************************
void InitConsole(void)
{
	// Enable GPIO Port A, used for UART0 pins.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // Configure the pin muxing for UART0 functions on port A0 and A1.
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);

    // Enable UART0 so that we can configure the clock.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    // Use the internal 16MHz oscillator as the UART clock source.
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    // Select the alternate (UART) function for these pins.
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Initialize the UART for console I/O.
    UARTStdioConfig(0, 115200, 16000000);
}

int main(void)
{
    // Set the clocking to run directly from the external crystal/oscillator.
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ);

    // Set up the serial console to use for displaying messages.  This is
    // just for this example program and is not needed for SSI operation.
    InitConsole();

    // Display the setup on the console.
    UARTprintf("SSI ->\n");
    UARTprintf("  Mode: TI\n");
    UARTprintf("  Data: 8-bit\n\n");

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

    // Read any residual data from the SSI port.  This makes sure the receive
    // FIFOs are empty, so we don't read any unwanted junk.  This is done here
    // because the TI SSI mode is full-duplex, which allows you to send and
    // receive at the same time.  The SSIDataGetNonBlocking function returns
    // "true" when data was returned, and "false" when no data was returned.
    // The "non-blocking" function checks if there is any data in the receive
    // FIFO and does not "hang" if there isn't.
    while(SSIDataGetNonBlocking(SSI0_BASE, NULL));


    // Display indication that the SSI is transmitting data.
    UARTprintf("Sent:\n  ");

    // Send 3 bytes of data.
    for(ui32Index = 0; ui32Index < NUM_SSI_DATA; ui32Index++)
    {
        // Display the data that SSI is transferring.
        UARTprintf("'%c' ", pui32DataTx[ui32Index]);

        // Send the data using the "blocking" put function.  This function
        // will wait until there is room in the send FIFO before returning.
        // This allows you to assure that all the data you send makes it into
        // the send FIFO.
        SSIDataPut(SSI0_BASE, pui32DataTx[ui32Index]);
    }

    // Wait until SSI0 is done transferring all the data in the transmit FIFO.
    while(SSIBusy(SSI0_BASE))
    {
    }

    // Display indication that the SSI is receiving data.
    UARTprintf("\nReceived:\n  ");

    // Receive 3 bytes of data.
    for(ui32Index = 0; ui32Index < NUM_SSI_DATA; ui32Index++)
    {
        // Receive the data using the "blocking" Get function. This function
        // will wait until there is data in the receive FIFO before returning.
        SSIDataGet(SSI0_BASE, &pui32DataRx[ui32Index]);

        // Since we are using 8-bit data, mask off the MSB.
        pui32DataRx[ui32Index] &= 0x00FF;

        // Display the data that SSI0 received.
        UARTprintf("'%c' ", pui32DataRx[ui32Index]);
    }

    return(0);
}

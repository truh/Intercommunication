/*
  Name of file  : util.c
  Author        : Andreas Willinger
  Version       : 20141020.1
  Description   : A set of functions used in the application multiple times.
*/
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"

void InitConsole(void)
{
	// Enable GPIO Port A, used for UART0 pins.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	
	// Enable UART0 so that we can configure the clock.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	
    // Configure the pin muxing for UART0 functions on port A0 and A1.
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    
    // Use the internal 16MHz oscillator as the UART clock source.
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    // Initialize the UART for console I/O.
    UARTStdioConfig(0, 115200, 16000000);
}

void SetupSSI(uint32_t iMode)
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
                       iMode, 2000000, 8);
	
	SSIAdvModeSet(SSI2_BASE, SSI_ADV_MODE_READ_WRITE);

    // Enable the SSI2 module.
    SSIEnable(SSI2_BASE);
}

void OS() {
    for(;;);
}

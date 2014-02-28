#include <stdbool.h>
#include <stdint.h>

#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_ints.h"

#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom_map.h"
#include "driverlib/interrupt.h"

#include "drivers/buttons.h"

#include "driverlib/uart.h"
#include "utils/uartstdio.h"

#define LED_RED GPIO_PIN_1
#define LED_BLUE GPIO_PIN_2
#define LED_GREEN GPIO_PIN_3

void IntButtonsHandler(void);

int main()
{
  uint32_t ticker = 0;

  ROM_SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
  ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, LED_RED|LED_BLUE|LED_GREEN);

  ROM_SysCtlPeripheralEnable(BUTTONS_GPIO_PERIPH);
  HWREG(BUTTONS_GPIO_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
  HWREG(BUTTONS_GPIO_BASE + GPIO_O_CR) |= 0x01;
  HWREG(BUTTONS_GPIO_BASE + GPIO_O_LOCK) = 0;
  ROM_GPIODirModeSet(BUTTONS_GPIO_BASE, ALL_BUTTONS, GPIO_DIR_MODE_IN);
  ROM_GPIOPadConfigSet(BUTTONS_GPIO_BASE, ALL_BUTTONS,
      GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

  ROM_GPIOIntTypeSet(BUTTONS_GPIO_BASE, ALL_BUTTONS, GPIO_BOTH_EDGES);
  GPIOIntEnable(BUTTONS_GPIO_BASE, ALL_BUTTONS);
  ROM_IntEnable(INT_GPIOF);
  ROM_IntMasterEnable();

  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
  ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
  ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
  ROM_UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
  UARTStdioConfig(0, 115200, 16000000);

  UARTprintf("Starting the program ...\n");

  while(1)
  {
    ROM_SysCtlDelay(ROM_SysCtlClockGet() / 2);
    UARTprintf("%d",ticker++);
  }

  return 0;
}

  void
IntButtonsHandler(void)
{
  uint8_t ui8Buttons;
  uint8_t led_switch = 0;

  GPIOIntClear(BUTTONS_GPIO_BASE, ALL_BUTTONS);
  UARTprintf("ButtonsHandler was called ...\n");

  ui8Buttons = (uint8_t)~ROM_GPIOPinRead(BUTTONS_GPIO_BASE, ALL_BUTTONS);

  if(LEFT_BUTTON & ui8Buttons) led_switch |= LED_RED;
  if(RIGHT_BUTTON & ui8Buttons) led_switch |= LED_GREEN;

  ROM_GPIOPinWrite(GPIO_PORTF_BASE, LED_RED|LED_GREEN|LED_BLUE, led_switch);
}

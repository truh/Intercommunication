#include <stdbool.h>
#include <stdint.h>

#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/can.h"
#include "drivers/buttons.h"

#define LED_RED GPIO_PIN_1
#define LED_BLUE GPIO_PIN_2
#define LED_GREEN GPIO_PIN_3

int main()
{
  uint8_t ui8Buttons;
  uint8_t led_switch;
  /*
  uint8_t ui8ButtonsChanged;
  */

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
  /*
  ButtonsInit();
  */

  for (;;) {
    /*
    ui8Buttons = ButtonsPoll(&ui8ButtonsChanged, 0);

    if(BUTTON_PRESSED(LEFT_BUTTON, ui8Buttons, ui8ButtonsChanged))
      ROM_GPIOPinWrite(GPIO_PORTF_BASE, LED_RED|LED_GREEN|LED_BLUE, LED_RED);
    else if(BUTTON_PRESSED(RIGHT_BUTTON, ui8Buttons, ui8ButtonsChanged))
      ROM_GPIOPinWrite(GPIO_PORTF_BASE, LED_RED|LED_GREEN|LED_BLUE, LED_GREEN);
    else if(BUTTON_RELEASED(LEFT_BUTTON|RIGHT_BUTTON, ui8Buttons, ui8ButtonsChanged))
      ROM_GPIOPinWrite(GPIO_PORTF_BASE, LED_RED|LED_GREEN|LED_BLUE, 0);
    */

    ui8Buttons = (uint8_t)~ROM_GPIOPinRead(BUTTONS_GPIO_BASE, ALL_BUTTONS);
    led_switch = 0;
    if(LEFT_BUTTON & ui8Buttons) led_switch |= LED_RED;
    if(RIGHT_BUTTON & ui8Buttons) led_switch |= LED_GREEN;
    ROM_GPIOPinWrite(GPIO_PORTF_BASE, LED_RED|LED_GREEN|LED_BLUE, led_switch);
  }

  return 0;
}

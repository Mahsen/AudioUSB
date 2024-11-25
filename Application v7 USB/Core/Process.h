//Public library has to be included:
#include <intrinsics.h>
#include <stdio.h>
#include <string.h>

//For the LPC library has to be included:
#include "../cmsis_boot/LPC17xx.h"
#include "../lpc17xx_lib/include/types.h"
#include "../Drivers/Configuration.h"

//For the UART library has to be included:
#include "../lpc17xx_lib/include/uart.h"

//For the Timer library has to be included:
//#include "../lpc17xx_lib/include/timer.h"

//For the GPIO library has to be included:
//#include "../lpc17xx_lib/include/gpio.h"

//For the GLCD library has to be included:
//#include "../Drivers/GLCD/GLCD.h" 

//For the TP library has to be included:
//#include "../Drivers/TouchPanel/TouchPanel.h" 

//For the libcfg library has to be included:
#include "../lpc17xx_lib/include/libcfg.h"

//For the pinsel library has to be included:
#include "../lpc17xx_lib/include/pinsel.h"

//For the ADC library has to be included:
//#include "../lpc17xx_lib/include/adc.h"

//For the DAC library has to be included:
//#include "../lpc17xx_lib/include/dac.h"

//For the USB library has to be included:
#include "../Drivers/USB/usb.h"
#include "../Drivers/USB/usbcfg.h"
#include "../Drivers/USB/usbhw.h"
#include "../Drivers/USB/usbcore.h"
#include "../Drivers/USB/usbaudio.h"
#include "../Drivers/USB/usbuser.h"



//Run function :
int Run_Processes(void);
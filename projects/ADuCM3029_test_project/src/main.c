/*****************************************************************************
 * main.c
 *****************************************************************************/

/***************************** Include Files **********************************/
#include <stdio.h>

#include <sys/platform.h>
#include "adi_initialize.h"
#include "Communication.h"
#include "Timer.h"
#include "Test.h"
#include "Test_Config.h"

/************************** Variable Definitions ******************************/
/* Pin muxing */
extern int32_t adi_initpinmux(void);

/************************* Functions Definitions ******************************/

/**
  @brief Main function

**/
int main(int argc, char *argv[])
{
	timer_start(); // Start timer

	if(FUNCTION_TO_TEST != GPIO)
		adi_initpinmux(); // Init port configuration for UART, SPI and I2C

	Test_Init(); // Init Test project

	while(1) {
	    if(FUNCTION_TO_TEST == GPIO) {
	    	timer_sleep(500); // Delay 500ms, in order to see the pins toggling
	    	Test_Port(); // Test ports
	    }
	    else {
	    	Test_Periph(); // Test peripherals
	    }
	}
	return 0;
}

/*****************************************************************************
 * ADuCM3029_demo_cli.c
 *****************************************************************************/

#include <sys/platform.h>
#include "adi_initialize.h"
#include <stdio.h>
#include <stdlib.h>

#include "drivers/uart/adi_uart.h"
#include "drivers/dma/adi_dma.h"
#include "ADuCM3029_demo_cli.h"
#include "Cli.h"
#include "Communication.h"
#include "Timer.h"

uint8_t buffrx;
uint32_t error;

int main(int argc, char *argv[])
{
	uint8_t uart_word_len = 8;

	adi_initComponents();

	UART_Init(UART_BAUDRATE,
			uart_word_len);  /* UART initialization */

	timer_start();

	Cli_Prompt();

	while(1) {
		Cli_Process();    /* Command interpreter */
	}

	return 0;
}


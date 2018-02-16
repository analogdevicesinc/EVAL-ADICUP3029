/*
 * cn0410_bindings.h
 *
 *  Created on: Jul 4, 2017
 *      Author: mcaprior
 */

#ifndef CN0410_BINDINGS_H_
#define CN0410_BINDINGS_H_

extern "C"
{
	void UART_Init(void);
	int UART_WriteChar(char data);
	int UART_WriteString(char *string);
	void AppPrintf(const char *fmt, ...);
	void UART_Interrupt(void);

	void timer_start (void);
	void timer_sleep (uint32_t ticks);
}


#endif /* CN0410_BINDINGS_H_ */

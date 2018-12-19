/**
******************************************************************************
*   @file     RingBuffer.c
*   @brief    Ring buffer object source file
*   @version  V0.1
*   @author   ADI
*
*******************************************************************************
*
*Copyright 2015-2018(c) Analog Devices, Inc.
*
*All rights reserved.
*
*Redistribution and use in source and binary forms, with or without modification,
*are permitted provided that the following conditions are met:
*    - Redistributions of source code must retain the above copyright
*      notice, this list of conditions and the following disclaimer.
*    - Redistributions in binary form must reproduce the above copyright
*      notice, this list of conditions and the following disclaimer in
*      the documentation and/or other materials provided with the
*      distribution.
*    - Neither the name of Analog Devices, Inc. nor the names of its
*      contributors may be used to endorse or promote products derived
*      from this software without specific prior written permission.
*    - The use of this software may or may not infringe the patent rights
*      of one or more patent holders.  This license does not release you
*      from the requirement that you obtain separate licenses from these
*      patent holders to use this software.
*    - Use of the software either in source or binary form, must be run
*      on or directly connected to an Analog Devices Inc. component.
*
*THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
*INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
*PARTICULAR PURPOSE ARE DISCLAIMED.
*
*IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
*EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, INTELLECTUAL PROPERTY
*RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
*BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
*STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
*THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************
**/
#include "Ringbuffer.h"

static char   TXbuf [MAX_BUFLEN];       /* memory for ring buffer #1 (TXD) */
static char   RXbuf [MAX_BUFLEN];       /* memory for ring buffer #2 (RXD) */

/* define o/p and i/p ring buffer control structures */
extern  RingBuf_Create(TX);             /* static struct { ... } out; */
extern  RingBuf_Create(RX);             /* static struct { ... } in; */

/*-----------------------------------------------------------------------------
* Ring Buffer Init
*----------------------------------------------------------------------------*/
void Rb_Init(void)
{
	RingBuf_Init(&TX, TXbuf,
		     MAX_BUFLEN-1);               /* set up TX ring buffer */
	RingBuf_Init(&RX, RXbuf,
		     MAX_BUFLEN-1);               /* set up RX ring buffer */
}

/*-----------------------------------------------------------------------------
* PutChar onto Ring buffer
*----------------------------------------------------------------------------*/
unsigned char PutChar(char c)
{
	if(!RingBuf_Full(
		    &TX)) { // si le buffer n'est pas plein, on place l'octet dans le buffer
		*RingBuf_Putvalue(&TX) = c;                     /* store data in the buffer */
		RingBuf_Write_Increment(&TX);                   /* adjust write position */

// Activer la transmission
//  	if(!TXactive) {
//		TXactive = 1;                      /* indicate ongoing transmission */
// 	    TI0 = 1;//   Placer le bit TI à 1 pour provoquer le déclenchement de l'interruption
//  	}
		return 0;  // opération correctement réalisée
	} else return 1; // opération échouée
}

/*-----------------------------------------------------------------------------
* GetChar from Ring Buffer
*----------------------------------------------------------------------------*/
char GetChar(void)
{
	char c;

//if (!RingBuf_Empty(&RX))
// {                 /* wait for data */
	c = *RingBuf_Getvalue(
		    &RX);                    /* get character off the buffer */
	RingBuf_Read_Increment(&RX);                   /* adjust read position */
	return c;
//}
//else return 0;
}

void DeleteChar(int num)
{
	for (int i = 0; i < num; i++) {
		RingBuf_Read_Decrement(&RX);
		*RingBuf_Getvalue(&RX) = 0;
	}
	for (int i = 0; i < num; i++) {
		RingBuf_Read_Increment(&RX);
	}
}


/*-----------------------------------------------------------------------------
* Empty the Ring Buffer
*----------------------------------------------------------------------------*/
void Empty_Ring(void)
{
//  for (int i = 0; i < MAX_BUFLEN; i++)
//  {
//    RXbuf[i] = 0;
//  }
	volatile char c;
	do {
		c = *RingBuf_Getvalue(
			    &RX);                    /* get character off the buffer */
		(void)c;										 /* remove unused variable compiler warning */
		RingBuf_Read_Increment(&RX);                   /* adjust read position */
	} while(!RingBuf_Empty(&RX));
}





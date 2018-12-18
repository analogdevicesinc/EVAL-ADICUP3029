/**
******************************************************************************
*   @file     Ringbuffer.h
*   @brief    Ring buffer object header file
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
#ifndef _ringbuffer_h_
 #define _ringbuffer_h_

#define MAX_BUFLEN      160
#define MY_BUF	        160

//void serInit(void);
unsigned char  PutChar(char c );
char GetChar ( void); 
void Rb_Init(void);
void Empty_Ring(void);
void DeleteChar(int num);

 #define RingBuf_Create(rb) \
 struct \
    { \
	   char *rb_head; \
	   char *rb_tail; \
	   char *rb_write; \
	   char *rb_read; \
   }rb 
   
 // Initialisation of the ring buffer
 // rb is the ring buffer
 // size is the size of the buffer
 // start is the adress of the buffer
   
 #define RingBuf_Init(rb,start,size) \
    ( \
		(rb)->rb_write = (rb)->rb_read = (rb)->rb_head = start, \
        (rb)->rb_tail = &(rb)->rb_head[size] )
	   
 // If we reach the end, the pointer should go to the head
 #define RingBuf_in_Tail(rb,position) ( (position) == (rb)->rb_tail ? (rb)->rb_head:(position) )
      
 #define RingBuf_in_Head(rb,position) ( (position) == (rb)->rb_head ? (rb)->rb_tail:(position) )
	   
 // Testing if RB is empty 
 #define RingBuf_Empty(rb) ( (rb)->rb_write==(rb)->rb_read )

 // Testing if RB is full
// #define RingBuf_Full(rb)  ( RingBuf_in_Tail(rb, (rb)->rb_write+1)==(rb)->rb_read )
 #define RingBuf_Full(rb)  ( (rb)->rb_write==(rb)->rb_read )     

 // Incrementation of writing pointer
// #define RingBuf_Write_Increment(rb) ( (rb)->rb_write= RingBuf_in_Tail((rb), (rb)->rb_write+1) )
 #define RingBuf_Write_Increment(rb)  ( (rb)->rb_write = ((rb)->rb_write == (rb)->rb_tail) ? (rb)->rb_head:(rb)->rb_write+1 )
 
 // Incrémentation of reading pointer
// #define RingBuf_Read_Increment(rb)  ( (rb)->rb_read= RingBuf_in_Tail((rb), (rb)->rb_read+1) )
      
// #define RingBuf_Read_Decrement(rb)  ( (rb)->rb_read= RingBuf_in_Head((rb), (rb)->rb_read-1) )

 #define RingBuf_Read_Increment(rb)  ( (rb)->rb_read = ((rb)->rb_read == (rb)->rb_tail) ? (rb)->rb_head:(rb)->rb_read+1 )
 #define RingBuf_Read_Decrement(rb)  ( (rb)->rb_read = ((rb)->rb_read == (rb)->rb_head) ? (rb)->rb_tail:(rb)->rb_read-1 )
     
     
 // Putting value in buffer
 #define RingBuf_Putvalue(rb) ( (rb)->rb_write )

 // Getting value in buffer 
 #define RingBuf_Getvalue(rb)  ( (rb)->rb_read )

#endif

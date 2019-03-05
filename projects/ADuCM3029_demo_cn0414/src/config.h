/*****************************************************************************
 * ADuCM3029_test_hart_cn0414.h
 *****************************************************************************/

#ifndef __CONFIG_H__
#define __CONFIG_H__

/* ADC Reference voltage */
float vref = 2.5;

///* If the output needs to be raw data */
//#define _RAWDATA

/* Number of samples to average on channel read */
#define CHAN_SAMPLE_NO 10

/* To write protected memory with device data */
//#define WRITE_PROTECTED_MEMORY
/* Unique ID for each board. Change with appropriate value. */
uint32_t unique_id = 0x00112233;
/* Defines for board ID */
#define ID_MASK0 0xFF000000
#define ID_MASK1 0x00FF0000
#define ID_MASK2 0x0000FF00
#define ID_MASK3 0x000000FF
#define ID_SHIFT0 24
#define ID_SHIFT1 16
#define ID_SHIFT2 8
#define ID_SHIFT3 0

#endif /* __CONFIG_H__ */

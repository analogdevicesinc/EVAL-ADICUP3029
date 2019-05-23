/*****************************************************************************
 * config.h
 *****************************************************************************/

#ifndef __CONFIG_H__
#define __CONFIG_H__

//#define CLI_INTEFACE
#define MODBUS_INTERFACE
#if defined(CLI_INTEFACE) && defined(MODBUS_INTERFACE)
#error "Cannot use both CLI and ModBus interfaces. Only one may be active."
#elif !defined(CLI_INTEFACE) && !defined(MODBUS_INTERFACE)
#error "No interface chosen. Please choose an interface by uncommenting one of the above #defines."
#endif

/* ADC Reference voltage */
extern float vref;

/* To write protected memory with device data */
//#define WRITE_PROTECTED_MEMORY
/* Unique ID for each board. Change with appropriate value. */
extern uint32_t unique_id;
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

/***************************************************************************//**
 *   @file   can_obj_layer.h
 *   @author Andrei Drimbarean (Andrei.Drimbarean@analog.com)
********************************************************************************
 * Copyright 2019(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#ifndef CAN_OBJ_LAYER_H_
#define CAN_OBJ_LAYER_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include "spi.h"
#include "gpio.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

#define WORD_MASK				0xffff
#define BYTE_MASK				0xff

/* Controller commands */
#define CAN_CTRL_CMD_RESET		0x0
#define CAN_CTRL_CMD_READ		0x3
#define CAN_CTRL_CMD_WRITE		0x2

#define CAN_CTRL_CMD_MODE(cmd)				((cmd) << 4)
#define CAN_CTRL_ADDR_UP_NIBBLE_MODE(addr)	(((addr) & 0x0f00) >> 8)
#define CAN_CTRL_ADDR_DW_BYTE_MODE(addr)	(((addr) & 0x00ff) >> 0)

/* Register address */
/* Configuration registers */
#define CAN_CTRL_REG_CON		0x000
#define CAN_CTRL_REG_NBTCFG		0x004
#define CAN_CTRL_REG_DBTCFG		0x008
#define CAN_CTRL_REG_TDC		0x00C
#define CAN_CTRL_REG_TBC		0x010
#define CAN_CTRL_REG_TSCON		0x014

/* Interrupt and status registers */
#define CAN_CTRL_REG_VEC		0x018
#define CAN_CTRL_REG_INT		0x01C
#define CAN_CTRL_REG_RXIF		0x020
#define CAN_CTRL_REG_TXIF		0x024
#define CAN_CTRL_REG_RXOVIF		0x028
#define CAN_CTRL_REG_TXATIF		0x02C
#define CAN_CTRL_REG_TXREQ		0x030

/* Error and diagnostic registers */
#define CAN_CTRL_REG_TREC		0x034
#define CAN_CTRL_REG_BDIAG0		0x038
#define CAN_CTRL_REG_BDIAG1		0x03C

/* FIFO control and status registers */
#define CAN_CTRL_REG_TEFCON		0x040
#define CAN_CTRL_REG_TEFSTA		0x044
#define CAN_CTRL_REG_TEFUA		0x048
#define CAN_CTRL_REG_TXQCON		0x050
#define CAN_CTRL_REG_TXQSTA		0x054
#define CAN_CTRL_REG_TXQUA		0x058
#define CAN_CTRL_REG_FIFOCON(m)	(0x050 + ((m) * 3 * 4)) /* m = 1 to 31 */
#define CAN_CTRL_REG_FIFOSTA(m)	(0x054 + ((m) * 3 * 4)) /* m = 1 to 31 */
#define CAN_CTRL_REG_FIFOUA(m)	(0x058 + ((m) * 3 * 4)) /* m = 1 to 31 */

/* Filter configuration and control registers */
#define CAN_CTRL_REG_FLTCON(n)	(0x1D0 + ((n) * 4)) /* n = 0 to 7 */
#define CAN_CTRL_REG_FLTOBJ(m)	(0x1F0 + ((m) * 2 * 4)) /* m = 0 to 31 */
#define CAN_CTRL_REG_MASK(m)	(0x1F4 + ((m) * 2 * 4)) /* m = 0 to 31 */

/* MCP2517FD specific registers */
#define CAN_CTRL_REG_OSC		0xE00
#define CAN_CTRL_REG_IOCON		0xE04
#define CAN_CTRL_REG_CRC		0xE08
#define CAN_CTRL_REG_ECCCON		0xE0C
#define CAN_CTRL_REG_ECCSTA		0xE10

/* RAM */
#define CAN_CTRL_REG_RAM_ADDR	0x400
#define CAN_CTRL_REG_RAM_SIZE	2048

/* CAN_CTRL_REG_CON; CAN control register */
#define CON_TXBWS_MASK			(0x00f << 28)
#define CON_TXBWS_MODE(x)		(((x) & 0x00f) << 28)
#define CON_ABAT_MASK			(0x001 << 27)
#define CON_ABAT_MODE(x)		(((x) & 0x001) << 27)
#define CON_REQOP_MASK			(0x007 << 24)
#define CON_REQOP_MODE(x)		(((x) & 0x007) << 24)
#define CON_OPMOD_MASK			(0x007 << 21)
#define CON_OPMOD_MODE(x)		(((x) & 0x007) << 21)
#define CON_TXQEN_MASK			(0x001 << 20)
#define CON_TXQEN_MODE(x)		(((x) & 0x001) << 20)
#define CON_STEF_MASK			(0x001 << 19)
#define CON_STEF_MODE(x)		(((x) & 0x001) << 19)
#define CON_SERR2LOM_MASK		(0x001 << 18)
#define CON_SERR2LOM_MODE(x)	(((x) & 0x001) << 18)
#define CON_ESIGM_MASK			(0x001 << 17)
#define CON_ESIGM_MODE(x)		(((x) & 0x001) << 17)
#define CON_RTXAT_MASK			(0x001 << 16)
#define CON_RTXAT_MODE(x)		(((x) & 0x001) << 16)
#define CON_BRSDIS_MASK			(0x001 << 12)
#define CON_BRSDIS_MODE(x)		(((x) & 0x001) << 12)
#define CON_BUSY_MASK			(0x001 << 11)
#define CON_BUSY_MODE(x)		(((x) & 0x001) << 11)
#define CON_WFT_MASK			(0x003 << 9)
#define CON_WFT_MODE(x)			(((x) & 0x003) << 9)
#define CON_WAKFIL_MASK			(0x001 << 8)
#define CON_WAKFIL_MODE(x)		(((x) & 0x001) << 8)
#define CON_PXEDIS_MASK			(0x001 << 6)
#define CON_PXEDIS_MODE(x)		(((x) & 0x001) << 6)
#define CON_ISOCRCEN_MASK		(0x001 << 5)
#define CON_ISOCRCEN_MODE(x)	(((x) & 0x001) << 5)
#define CON_DNCNT_MASK			(0x01f << 0)
#define CON_DNCNT_MODE(x)		(((x) & 0x01f) << 0)

/* CAN_CTRL_REG_NBTCFG; Nominal bit time configuration register */
#define NBTCFG_BRP_MASK			(0x0ff << 24)
#define NBTCFG_BRP_MODE(x)		(((x) & 0x0ff) << 24)
#define NBTCFG_TSEG1_MASK		(0x0ff << 16)
#define NBTCFG_TSEG1_MODE(x)	(((x) & 0x0ff) << 16)
#define NBTCFG_TSEG2_MASK		(0x07f << 8)
#define NBTCFG_TSEG2_MODE(x)	(((x) & 0x07f) << 8)
#define NBTCFG_SJW_MASK			(0x07f << 0)
#define NBTCFG_SJW_MODE(x)		(((x) & 0x07f) << 0)

/* CAN_CTRL_REG_DBTCFG; Data bit time configuration register */
#define DBTCFG_BRP_MASK			(0x0ff << 24)
#define DBTCFG_BRP_MODE(x)		(((x) & 0x0ff) << 24)
#define DBTCFG_TSEG1_MASK		(0x01f << 16)
#define DBTCFG_TSEG1_MODE(x)	(((x) & 0x01f) << 16)
#define DBTCFG_TSEG2_MASK		(0x00f << 8)
#define DBTCFG_TSEG2_MODE(x)	(((x) & 0x00f) << 8)
#define DBTCFG_SJW_MASK			(0x00f << 0)
#define DBTCFG_SJW_MODE(x)		(((x) & 0x00f) << 0)

/* CAN_CTRL_REG_TDC; Transmitter delay compensation register */
#define TDC_EDGFLTEN_MASK		(0x001 << 25)
#define TDC_EDGFLTEN_MODE(x)	(((x) & 0x001) << 25)
#define TDC_SID11EN_MASK		(0x001 << 24)
#define TDC_SID11EN_MODE(x)		(((x) & 0x001) << 24)
#define TDC_TDCMOD_MASK			(0x003 << 16)
#define TDC_TDCMOD_MODE(x)		(((x) & 0x003) << 16)
#define TDC_TDCO_MASK			(0x07f << 8)
#define TDC_TDCO_MODE(x)		(((x) & 0x07f) << 8)
#define TDC_TDCV_MASK			(0x03f << 0)
#define TDC_TDCV_MODE(x)		(((x) & 0x03f) << 0)

/* CAN_CTRL_REG_INT; Interrupt register */
#define INT_IVMIE_MASK			(0x001 << 31)
#define INT_IVMIE_MODE(x)		(((x) & 0x001) << 31)
#define INT_WAKIE_MASK			(0x001 << 30)
#define INT_WAKIE_MODE(x)		(((x) & 0x001) << 30)
#define INT_CERRIE_MASK			(0x001 << 29)
#define INT_CERRIE_MODE(x)		(((x) & 0x001) << 29)
#define INT_SERRIE_MASK			(0x001 << 28)
#define INT_SERRIE_MODE(x)		(((x) & 0x001) << 28)
#define INT_RXOVIE_MASK			(0x001 << 27)
#define INT_RXOVIE_MODE(x)		(((x) & 0x001) << 27)
#define INT_TXATIE_MASK			(0x001 << 26)
#define INT_TXATIE_MODE(x)		(((x) & 0x001) << 26)
#define INT_SPICRCIE_MASK		(0x001 << 25)
#define INT_SPICRCIE_MODE(x)	(((x) & 0x001) << 25)
#define INT_ECCIE_MASK			(0x001 << 24)
#define INT_ECCIE_MODE(x)		(((x) & 0x001) << 24)
#define INT_TEFIE_MASK			(0x001 << 20)
#define INT_TEFIE_MODE(x)		(((x) & 0x001) << 20)
#define INT_MODIE_MASK			(0x001 << 19)
#define INT_MODIE_MODE(x)		(((x) & 0x001) << 19)
#define INT_TBCIE_MASK			(0x001 << 18)
#define INT_TBCIE_MODE(x)		(((x) & 0x001) << 18)
#define INT_RXIE_MASK			(0x001 << 17)
#define INT_RXIE_MODE(x)		(((x) & 0x001) << 17)
#define INT_TXIE_MASK			(0x001 << 16)
#define INT_TXIE_MODE(x)		(((x) & 0x001) << 16)
#define INT_IVMIF_MASK			(0x001 << 15)
#define INT_IVMIF_MODE(x)		(((x) & 0x001) << 15)
#define INT_WAKIF_MASK			(0x001 << 14)
#define INT_WAKIF_MODE(x)		(((x) & 0x001) << 14)
#define INT_CERRIF_MASK			(0x001 << 13)
#define INT_CERRIF_MODE(x)		(((x) & 0x001) << 13)
#define INT_SERRIF_MASK			(0x001 << 12)
#define INT_SERRIF_MODE(x)		(((x) & 0x001) << 12)
#define INT_RXOVIF_MASK			(0x001 << 11)
#define INT_RXOVIF_MODE(x)		(((x) & 0x001) << 11)
#define INT_TXATIF_MASK			(0x001 << 10)
#define INT_TXATIF_MODE(x)		(((x) & 0x001) << 10)
#define INT_SPICRCIF_MASK		(0x001 << 9)
#define INT_SPICRCIF_MODE(x)	(((x) & 0x001) << 9)
#define INT_ECCIF_MASK			(0x001 << 8)
#define INT_ECCIF_MODE(x)		(((x) & 0x001) << 8)
#define INT_TEFIF_MASK			(0x001 << 4)
#define INT_TEFIF_MODE(x)		(((x) & 0x001) << 4)
#define INT_MODIF_MASK			(0x001 << 3)
#define INT_MODIF_MODE(x)		(((x) & 0x001) << 3)
#define INT_TBCIF_MASK			(0x001 << 2)
#define INT_TBCIF_MODE(x)		(((x) & 0x001) << 2)
#define INT_RXIF_MASK			(0x001 << 1)
#define INT_RXIF_MODE(x)		(((x) & 0x001) << 1)
#define INT_TXIF_MASK			(0x001 << 0)
#define INT_TXIF_MODE(x)		(((x) & 0x001) << 0)

/* CAN_CTRL_REG_TEFCON; Transmit event FIFO control register */
#define TEFCON_FSIZE_MASK		(0x01f << 24)
#define TEFCON_FSIZE_MODE(x)	(((x) & 0x01f) << 24)
#define TEFCON_FRESET_MASK		(0x001 << 10)
#define TEFCON_FRESET_MODE(x)	(((x) & 0x001) << 10)
#define TEFCON_UINC_MASK		(0x001 << 8)
#define TEFCON_UINC_MODE(x)		(((x) & 0x001) << 8)
#define TEFCON_TEFTSEN_MASK		(0x001 << 5)
#define TEFCON_TEFTSEN_MODE(x)	(((x) & 0x001) << 5)
#define TEFCON_TEFOVIE_MASK		(0x001 << 3)
#define TEFCON_TEFOVIE_MODE(x)	(((x) & 0x001) << 3)
#define TEFCON_TEFFIE_MASK		(0x001 << 2)
#define TEFCON_TEFFIE_MODE(x)	(((x) & 0x001) << 2)
#define TEFCON_TEFHIE_MASK		(0x001 << 1)
#define TEFCON_TEFHIE_MODE(x)	(((x) & 0x001) << 1)
#define TEFCON_TEFNEIE_MASK		(0x001 << 0)
#define TEFCON_TEFNEIE_MODE(x)	(((x) & 0x001) << 0)

/* CAN_CTRL_REG_TXQCON; Transmit queue control register */
#define TXQCON_PLSIZE_MASK		(0x007 << 29)
#define TXQCON_PLSIZE_MODE(x)	(((x) & 0x007) << 29)
#define TXQCON_FSIZE_MASK		(0x01f << 24)
#define TXQCON_FSIZE_MODE(x)	(((x) & 0x01f) << 24)
#define TXQCON_TXAT_MASK		(0x003 << 21)
#define TXQCON_TXAT_MODE(x)		(((x) & 0x003) << 21)
#define TXQCON_TXPRI_MASK		(0x01f << 16)
#define TXQCON_TXPRI_MODE(x)	(((x) & 0x01f) << 16)
#define TXQCON_FRESET_MASK		(0x001<< 10)
#define TXQCON_FRESET_MODE(x)	(((x) & 0x001) << 10)
#define TXQCON_TXREQ_MASK		(0x001 << 9)
#define TXQCON_TXREQ_MODE(x)	(((x) & 0x001) << 9)
#define TXQCON_UINC_MASK		(0x001 << 8)
#define TXQCON_UINC_MODE(x)		(((x) & 0x001) << 8)
#define TXQCON_TXEN_MASK		(0x001 << 7)
#define TXQCON_TXEN_MODE(x)		(((x) & 0x001) << 7)
#define TXQCON_TXATIE_MASK		(0x001 << 4)
#define TXQCON_TXATIE_MODE(x)	(((x) & 0x001) << 4)
#define TXQCON_TXQEIE_MASK		(0x001 << 2)
#define TXQCON_TXQEIE_MODE(x)	(((x) & 0x001) << 2)
#define TXQCON_TXQNIE_MASK		(0x001 << 0)
#define TXQCON_TXQNIE_MODE(x)	(((x) & 0x001) << 0)

/* CAN_CTRL_REG_FIFOCON(m); FIFO control register m */
#define FIFOCON_PLSIZE_MASK			(0x007 << 29)
#define FIFOCON_PLSIZE_MODE(x)		(((x) & 0x007) << 29)
#define FIFOCON_FSIZE_MASK			(0x01f << 24)
#define FIFOCON_FSIZE_MODE(x)		(((x) & 0x01f) << 24)
#define FIFOCON_TXAT_MASK			(0x003 << 21)
#define FIFOCON_TXAT_MODE(x)		(((x) & 0x003) << 21)
#define FIFOCON_TXPRI_MASK			(0x01f << 16)
#define FIFOCON_TXPRI_MODE(x)		(((x) & 0x01f) << 16)
#define FIFOCON_FRESET_MASK			(0x001<< 10)
#define FIFOCON_FRESET_MODE(x)		(((x) & 0x001) << 10)
#define FIFOCON_TXREQ_MASK			(0x001 << 9)
#define FIFOCON_TXREQ_MODE(x)		(((x) & 0x001) << 9)
#define FIFOCON_UINC_MASK			(0x001 << 8)
#define FIFOCON_UINC_MODE(x)		(((x) & 0x001) << 8)
#define FIFOCON_TXEN_MASK			(0x001 << 7)
#define FIFOCON_TXEN_MODE(x)		(((x) & 0x001) << 7)
#define FIFOCON_RTREN_MASK			(0x001 << 6)
#define FIFOCON_RTREN_MODE(x)		(((x) & 0x001) << 6)
#define FIFOCON_RXTSEN_MASK			(0x001 << 5)
#define FIFOCON_RXTSEN_MODE(x)		(((x) & 0x001) << 5)
#define FIFOCON_TXATIE_MASK			(0x001 << 4)
#define FIFOCON_TXATIE_MODE(x)		(((x) & 0x001) << 4)
#define FIFOCON_RXOVIE_MASK			(0x001 << 3)
#define FIFOCON_RXOVIE_MODE(x)		(((x) & 0x001) << 3)
#define FIFOCON_TFERFFIE_MASK		(0x001 << 2)
#define FIFOCON_TFERFFIE_MODE(x)	(((x) & 0x001) << 2)
#define FIFOCON_TFHRFHIE_MASK		(0x001 << 1)
#define FIFOCON_TFHRFHIE_MODE(x)	(((x) & 0x001) << 1)
#define FIFOCON_TFNRFNIE_MASK		(0x001 << 0)
#define FIFOCON_TFNRFNIE_MODE(x)	(((x) & 0x001) << 0)

/* CAN_CTRL_REG_FIFOSTA(m); FIFO status register m */
#define FIFOSTA_FIFOCI_MASK			(0x01f << 8)
#define FIFOSTA_FIFOCI_MODE(x)		(((x) & 0x01f) << 8)
#define FIFOSTA_TXABT_MASK			(0x001 << 7)
#define FIFOSTA_TXABT_MODE(x)		(((x) & 0x001) << 7)
#define FIFOSTA_TXLARB_MASK			(0x001 << 6)
#define FIFOSTA_TXLARB_MODE(x)		(((x) & 0x001) << 6)
#define FIFOSTA_TXERR_MASK			(0x001 << 5)
#define FIFOSTA_TXERR_MODE(x)		(((x) & 0x001) << 5)
#define FIFOSTA_TXATIF_MASK			(0x001 << 4)
#define FIFOSTA_TXATIF_MODE(x)		(((x) & 0x001) << 4)
#define FIFOSTA_RXOVIF_MASK			(0x001 << 3)
#define FIFOSTA_RXOVIF_MODE(x)		(((x) & 0x001) << 3)
#define FIFOSTA_TFERFFIF_MASK		(0x001 << 2)
#define FIFOSTA_TFERFFIF_MODE(x)	(((x) & 0x001) << 2)
#define FIFOSTA_TFHRFHIF_MASK		(0x001 << 1)
#define FIFOSTA_TFHRFHIF_MODE(x)	(((x) & 0x001) << 1)
#define FIFOSTA_TFNRFNIF_MASK		(0x001 << 0)
#define FIFOSTA_TFNRFNIF_MODE(x)	(((x) & 0x001) << 0)

/* CAN_CTRL_REG_FLTCON(n); Filter control register n */
#define FLTCON_FLTEN_MASK(flt)		(0x001 << (7 + ((flt) * 8)))
#define FLTCON_FLTEN_MODE(x, flt)	(((x) & 0x001) << (7 + ((flt) * 8)))
#define FLTCON_FiBP_MASK(flt)		(0x01f << ((flt) * 8))
#define FLTCON_FiBP_MODE(x, flt)	(((x) & 0x01f) << ((flt) * 8))

/* CAN_CTRL_REG_FLTOBJ(m); Filter object register m */
#define FLTOBJ_EXIDE_MASK		(0x001 << 30)
#define FLTOBJ_EXIDE_MODE(x)	(((x) & 0x001) << 30)
#define FLTOBJ_SID11_MASK		(0x001 << 29)
#define FLTOBJ_SID11_MODE(x)	(((x) & 0x001) << 29)
#define FLTOBJ_EID_MASK			(0x3ffff << 11)
#define FLTOBJ_EID_MODE(x)		(((x) & 0x3ffff) << 11)
#define FLTOBJ_SID_MASK			(0x7ff << 0)
#define FLTOBJ_SID_MODE(x)		(((x) & 0x7ff) << 0)

/* CAN_CTRL_REG_MASK(m); Mask register m */
#define MASK_MIDE_MASK			(0x001 << 30)
#define MASK_MIDE_MODE(x)		(((x) & 0x001) << 30)
#define MASK_MSID11_MASK		(0x001 << 29)
#define MASK_MSID11_MODE(x)		(((x) & 0x001) << 29)
#define MASK_MEID_MASK			(0x3ffff << 11)
#define MASK_MEID_MODE(x)		(((x) & 0x3ffff) << 11)
#define MASK_MSID_MASK			(0x7ff << 0)
#define MASK_MSID_MODE(x)		(((x) & 0x7ff) << 0)

/* CAN_CTRL_REG_OSC; Oscillator control register */
#define OSC_SCLKRDY_MASK		(0x001 << 12)
#define OSC_SCLKRDY_MODE(x)		(((x) & 0x001) << 12)
#define OSC_OSCRDY_MASK			(0x001 << 10)
#define OSC_OSCRDY_MODE(x)		(((x) & 0x001) << 10)
#define OSC_PLLRDY_MASK			(0x001 << 8)
#define OSC_PLLRDY_MODE(x)		(((x) & 0x001) << 8)
#define OSC_CLKODIV_MASK		(0x003 << 5)
#define OSC_CLKODIV_MODE(x)		(((x) & 0x003) << 5)
#define OSC_SCLKDIV_MASK		(0x001 << 4)
#define OSC_SCLKDIV_MODE(x)		(((x) & 0x001) << 4)
#define OSC_OSCDIS_MASK			(0x001 << 2)
#define OSC_OSCDIS_MODE(x)		(((x) & 0x001) << 2)
#define OSC_PLLEN_MASK			(0x001 << 2)
#define OSC_PLLEN_MODE(x)		(((x) & 0x001) << 2)

/* CAN_CTRL_REG_IOCON; Input/Output control register */
#define IOCON_INTOD_MASK		(0x001 << 30)
#define IOCON_INTOD_MODE(x)		(((x) & 0x001) << 30)
#define IOCON_SOF_MASK			(0x001 << 29)
#define IOCON_SOF_MODE(x)		(((x) & 0x001) << 29)
#define IOCON_TXCANOD_MASK		(0x001 << 28)
#define IOCON_TXCANOD_MODE(x)	(((x) & 0x001) << 28)
#define IOCON_PM1_MASK			(0x001 << 25)
#define IOCON_PM1_MODE(x)		(((x) & 0x001) << 25)
#define IOCON_PM0_MASK			(0x001 << 24)
#define IOCON_PM0_MODE(x)		(((x) & 0x001) << 24)
#define IOCON_GPIO1_MASK		(0x001 << 17)
#define IOCON_GPIO1_MODE(x)		(((x) & 0x001) << 17)
#define IOCON_GPIO0_MASK		(0x001 << 16)
#define IOCON_GPIO0_MODE(x)		(((x) & 0x001) << 16)
#define IOCON_LAT1_MASK			(0x001 << 9)
#define IOCON_LAT1_MODE(x)		(((x) & 0x001) << 9)
#define IOCON_LAT0_MASK			(0x001 << 8)
#define IOCON_LAT0_MODE(x)		(((x) & 0x001) << 8)
#define IOCON_XSTBYEN_MASK		(0x001 << 6)
#define IOCON_XSTBYEN_MODE(x)	(((x) & 0x001) << 6)
#define IOCON_TRIS1_MASK		(0x001 << 1)
#define IOCON_TRIS1_MODE(x)		(((x) & 0x001) << 1)
#define IOCON_TRIS0_MASK		(0x001 << 0)
#define IOCON_TRIS0_MODE(x)		(((x) & 0x001) << 0)

/* CAN_CTRL_REG_ECCCON; ECC control register */
#define ECCCON_PARITY_MASK		(0x07f << 8)
#define ECCCON_PARITY_MODE(x)	(((x) & 0x07f) << 8)
#define ECCCON_DEDIE_MASK		(0x001 << 2)
#define ECCCON_DEDIE_MODE(x)	(((x) & 0x001) << 2)
#define ECCCON_SECIE_MASK		(0x001 << 1)
#define ECCCON_SECIE_MODE(x)	(((x) & 0x001) << 1)
#define ECCCON_ECCEN_MASK		(0x001 << 0)
#define ECCCON_ECCEN_MODE(x)	(((x) & 0x001) << 0)

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

struct tx_obj_header_str {
	uint32_t dlc : 4;
	uint32_t ide : 1;
	uint32_t rtr : 1;
	uint32_t brs : 1;
	uint32_t fdf : 1;
	uint32_t esi : 1;
	uint32_t seq : 7;
	uint32_t reserved1 : 16;
	uint32_t sid : 11;
	uint32_t eid : 18;
	uint32_t sid11 : 1;
	uint32_t reserved2 : 2;
};

union tx_obj_header {
	struct tx_obj_header_str hdr_str;
	uint32_t word[2];
	uint8_t byte[8];
};

enum can_ctrl_op_modes {
	CAN_NORMAL_FD_MODE,
	CAN_SLEEP_MODE,
	CAN_INT_LOOPBACK_MODE,
	CAN_LISTEN_ONLY_MODE,
	CAN_CONFIGURATION_MODE,
	CAN_EXT_LOOPBACK_MODE,
	CAN_NORMAL_2_MODE,
	CAN_RESTRICTED_OP_MODE
};

enum can_ctrl_en_int {
	TXIF     = 0x0001,
	RXIF     = 0x0002,
	TBCIF    = 0x0004,
	MODIF    = 0x0008,
	TEFIF    = 0x0010,
	ECCIF    = 0x0020,
	SPICRCIF = 0x0040,
	TXATIF   = 0x0080,
	RXOVIF   = 0x0100,
	SERRIF   = 0x0200,
	CERRIF   = 0x0400,
	WAKIF    = 0x0800,
	IVMIF    = 0x1000
};

enum can_ctrl_fifo_int {
	TFNRFNIE = 0x01,
	TFHRFHIE = 0x02,
	TFERFFIE = 0x04,
	RXOVIE   = 0x08,
	TXATIE   = 0x10
};

enum can_ctrl_ssp_mode {
	SSP_MODE_OFF,
	SSP_MODE_MANUAL,
	SSP_MODE_AUTO
};

enum can_ctrl_data_bitrate {
	BITRATE_DBT_500K,
	BITRATE_DBT_833K,
	BITRATE_DBT_1M,
	BITRATE_DBT_1M5,
	BITRATE_DBT_2M,
	BITRATE_DBT_3M,
	BITRATE_DBT_4M,
	BITRATE_DBT_5M,
	BITRATE_DBT_6M7,
	BITRATE_DBT_8M,
	BITRATE_DBT_10M
};

enum can_ctrl_nominal_bitrate {
	BITRATE_NBT_125K,
	BITRATE_NBT_250K,
	BITRATE_NBT_500K,
	BITRATE_NBT_1M
};

enum can_ctrl_fifo_plsize {
	DATA_BYTES_8,
	DATA_BYTES_12,
	DATA_BYTES_16,
	DATA_BYTES_20,
	DATA_BYTES_24,
	DATA_BYTES_32,
	DATA_BYTES_48,
	DATA_BYTES_64
};

enum can_ctrl_dlc {
	CAN_DLC_0,
	CAN_DLC_1,
	CAN_DLC_2,
	CAN_DLC_3,
	CAN_DLC_4,
	CAN_DLC_5,
	CAN_DLC_6,
	CAN_DLC_7,
	CAN_DLC_8,
	CAN_DLC_12,
	CAN_DLC_16,
	CAN_DLC_20,
	CAN_DLC_24,
	CAN_DLC_32,
	CAN_DLC_48,
	CAN_DLC_64,
	CAN_DLC_TOO_SMALL
};

enum can_ctrl_txat {
	CAN_TXAT_DIS,
	CAN_TXAT_3,
	CAN_TXAT_INF = 3
};

struct can_ctrl_init_param {
	struct spi_init_param can_ctrl_spi_init;
	bool con_iso_crc_en;
	bool con_store_in_tef_en;
	bool con_txq_en;
	uint8_t tef_fifo_size; /* Number of messages in TEF FIFO*/
	bool tef_time_stamp_en;
	enum can_ctrl_fifo_plsize txq_plsize;
	uint8_t txq_fifo_size; /* Number of messages in TXQ FIFO */
	uint8_t txq_tx_priority; /* 0 is lowest; 0x1f is highest */

	uint8_t tx_fifo_nr;
	enum can_ctrl_fifo_plsize tx_fifo_plsize;
	uint8_t tx_fifo_size; /* Number of messages in FIFO */
	uint8_t tx_fifo_priority; /* 0 is lowest; 0x1f is highest */

	uint8_t rx_fifo_nr;
	enum can_ctrl_fifo_plsize rx_fifo_plsize;
	uint8_t rx_fifo_size; /* Number of messages in FIFO */
	bool rx_fifo_tsen;

	uint8_t rx_flt_nr;
	uint16_t rx_sid_addr;

	enum can_ctrl_nominal_bitrate can_nbt;
	enum can_ctrl_data_bitrate can_dbt;
	enum can_ctrl_ssp_mode ssp_mode;
};

struct can_ctrl_dev {
	struct spi_desc *can_ctrl_spi;

	enum can_ctrl_nominal_bitrate can_nbt;
	enum can_ctrl_data_bitrate can_dbt;
	enum can_ctrl_ssp_mode ssp_mode;

	enum can_ctrl_fifo_plsize txq_plsize;

	uint8_t tx_fifo_nr;
	enum can_ctrl_fifo_plsize tx_fifo_plsize;
	uint16_t tx_sid;

	uint8_t rx_fifo_nr;
	enum can_ctrl_fifo_plsize rx_fifo_plsize;
	uint16_t rx_sid;
	uint8_t rx_flt_nr;
};

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

#define can_ctrl_sfr_word_read	can_ctrl_ram_read
#define can_ctrl_sfr_word_write	can_ctrl_ram_write

/* Get default configuration for the CAN controller initialization structure. */
void can_ctrl_get_config(struct can_ctrl_init_param *init);

/* Read a byte from the CAN controller memory using the SPI interface. */
int32_t can_ctrl_sfr_byte_read(struct can_ctrl_dev *dev, uint16_t address,
			       uint8_t *data);

/* Write a byte in the CAN controller memory using the SPI interface. */
int32_t can_ctrl_sfr_byte_write(struct can_ctrl_dev *dev, uint16_t address,
				uint8_t data);

/* Write a 4-byte word in the CAN controller memory using the SPI interface. */
int32_t can_ctrl_ram_read(struct can_ctrl_dev *dev, uint16_t address,
			  uint32_t *data);

/* Write a 4-byte word in the CAN controller memory using the SPI interface. */
int32_t can_ctrl_ram_write(struct can_ctrl_dev *dev, uint16_t address,
			   uint32_t data);

/* Give reset command to the CAN controller using SPI interface. */
int32_t can_ctrl_reset(struct can_ctrl_dev *dev);

/* Give reset command to the CAN controller using SPI interface. */
int32_t can_ctrl_ecc_set(struct can_ctrl_dev *dev, bool enable);

/* Configure the bit times for the CAN controller. */
int32_t can_ctrl_bit_time_config(struct can_ctrl_dev *dev,
				 enum can_ctrl_nominal_bitrate nbt, enum can_ctrl_data_bitrate dbt,
				 enum can_ctrl_ssp_mode mode);

/* Clear interrupt flags for int the CAN controller. */
int32_t can_ctrl_clear_int(struct can_ctrl_dev *dev);

/* Request an operation mode to the CAN controller. */
int32_t can_ctrl_op_mode_req(struct can_ctrl_dev *dev,
			     enum can_ctrl_op_modes op_mode, bool *success);

/* See if a message has bee received by a FIFO. */
int32_t can_ctrl_is_fifo_rdy(struct can_ctrl_dev *dev, uint8_t fifo_nr,
			     bool *status);

/* Get data size in bytes based on the payload option of a message. */
uint8_t can_ctrl_message_size(struct can_ctrl_dev *dev,
			      enum can_ctrl_fifo_plsize pls);

/* Get received message from a specified FIFO. */
int32_t can_ctrl_get_rec_message(struct can_ctrl_dev *dev, uint8_t fifo_nr,
				 uint8_t *data);

/* Get DLC code from a given number of bytes. */
enum can_ctrl_dlc can_ctrl_size_to_dlc(uint8_t size);

/* Transmit a message through CAN. */
int32_t can_ctrl_message_transmit(struct can_ctrl_dev *dev, uint8_t fifo_nr,
				  uint8_t *data, union tx_obj_header header);

/* Set a new standard ID for received messages. */
void can_ctrl_set_tx_sid(struct can_ctrl_dev *dev, uint16_t new_sid);

/* Wait for message transmission. */
int32_t can_ctrl_wait_tx(struct can_ctrl_dev *dev, bool *scs);

/* Clear the wake-up interrupt flag. */
int32_t can_ctrl_clear_wake_flag(struct can_ctrl_dev *dev);

/* Wake-up routine for the CAN controller. */
int32_t can_ctrl_wake_up(struct can_ctrl_dev *dev);

/* Does a device setup including all registers needed to function. */
int32_t can_ctrl_setup_dev(struct can_ctrl_dev *dev);

/* Allocate memory for the device driver and initialize it. */
int32_t can_ctrl_setup(struct can_ctrl_dev **device,
		       struct can_ctrl_init_param *init_param);

/* Free memory allocated by the can_ctrl_setup() function. */
int32_t can_ctrl_remove(struct can_ctrl_dev *dev);

#endif /* CAN_OBJ_LAYER_H_ */

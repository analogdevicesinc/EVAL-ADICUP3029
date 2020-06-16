/***************************************************************************//**
 *   @file   can_obj_layer.c
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

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "can_obj_layer.h"
#include "error.h"
#include "delay.h"

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * Get default configuration for the CAN controller initialization structure.
 *
 * @param [out] init - Pointer to the initialization structure.
 *
 * @return void.
 */
void can_ctrl_get_config(struct can_ctrl_init_param *init)
{
	init->can_ctrl_spi_init.chip_select = 0xFF;
	init->can_ctrl_spi_init.extra = NULL;
	init->can_ctrl_spi_init.id = SPI_ARDUINO;
	init->can_ctrl_spi_init.max_speed_hz = 4000000;
	init->can_ctrl_spi_init.mode = SPI_MODE_0;
	init->can_ctrl_spi_init.type = ADICUP3029_SPI;
	init->can_dbt = BITRATE_DBT_2M;
	init->can_nbt = BITRATE_NBT_500K;
	init->con_iso_crc_en = true;
	init->con_store_in_tef_en = true;
	init->con_txq_en = true;
	init->rx_fifo_nr = 2;
	init->rx_fifo_plsize = DATA_BYTES_64;
	init->rx_fifo_size = 1;
	init->rx_fifo_tsen = true;
	init->rx_flt_nr = 0;
	init->rx_sid_addr = 0x300;
	init->ssp_mode = SSP_MODE_AUTO;
	init->tef_fifo_size = 10;
	init->tef_time_stamp_en = true;
	init->tx_fifo_nr = 1;
	init->tx_fifo_plsize = DATA_BYTES_64;
	init->tx_fifo_priority = 1;
	init->tx_fifo_size = 1;
	init->txq_fifo_size = 2;
	init->txq_plsize = DATA_BYTES_64;
	init->txq_tx_priority = 0;
}

/**
 * Read a byte from the CAN controller memory using the SPI interface.
 *
 * Only Special Function Registers (SFRs) can be read at byte level.
 *
 * @param [in]  dev     - Pointer to the handler structure.
 * @param [in]  address - The address of the byte to be read.
 * @param [out] data    - Pointer to the data container.
 *
 * @return 0 in case of success, error code in case of failure.
 */
int32_t can_ctrl_sfr_byte_read(struct can_ctrl_dev *dev, uint16_t address,
			       uint8_t *data)
{
	uint8_t spi_msg[] = {0, 0, 0};
	int32_t ret;

	spi_msg[0] |= CAN_CTRL_CMD_MODE(CAN_CTRL_CMD_READ);
	spi_msg[0] |= CAN_CTRL_ADDR_UP_NIBBLE_MODE(address);
	spi_msg[1] |= CAN_CTRL_ADDR_DW_BYTE_MODE(address);

	ret = spi_write_and_read(dev->can_ctrl_spi, spi_msg, 3);
	if(ret != SUCCESS)
		return ret;

	*data = spi_msg[2];

	return ret;
}

/**
 * Write a byte in the CAN controller memory using the SPI interface.
 *
 * Only Special Function Registers (SFRs) can be written at byte level.
 *
 * @param [in] dev     - Pointer to the handler structure.
 * @param [in] address - The address of the byte to be written.
 * @param [in] data    - New value of the byte.
 *
 * @return 0 in case of success, error code in case of failure.
 */
int32_t can_ctrl_sfr_byte_write(struct can_ctrl_dev *dev, uint16_t address,
				uint8_t data)
{
	uint8_t spi_msg[] = {0, 0, 0};

	spi_msg[0] = CAN_CTRL_CMD_MODE(CAN_CTRL_CMD_WRITE) |
		     CAN_CTRL_ADDR_UP_NIBBLE_MODE(address);
	spi_msg[1] |= CAN_CTRL_ADDR_DW_BYTE_MODE(address);
	spi_msg[2] |= data;

	return spi_write_and_read(dev->can_ctrl_spi, spi_msg, 3);
}

/**
 * Write a 4-byte word in the CAN controller memory using the SPI interface.
 *
 * Both SFRs and RAM memory can be read at word level.
 *
 * @param [in]  dev     - Pointer to the handler structure.
 * @param [in]  address - The address of the byte to be read.
 * @param [out] data    - Pointer to the data container.
 *
 * @return 0 in case of success, error code in case of failure.
 */
int32_t can_ctrl_ram_read(struct can_ctrl_dev *dev, uint16_t address,
			  uint32_t *data)
{
	uint8_t spi_msg[] = {0, 0, 0, 0, 0, 0};
	int32_t ret;

	spi_msg[0] = CAN_CTRL_CMD_MODE(CAN_CTRL_CMD_READ) |
		     CAN_CTRL_ADDR_UP_NIBBLE_MODE(address);
	spi_msg[1] |= CAN_CTRL_ADDR_DW_BYTE_MODE(address);

	ret = spi_write_and_read(dev->can_ctrl_spi, spi_msg, 6);
	if(ret != SUCCESS)
		return ret;

	*data =  ((spi_msg[2] << 0)  & 0x000000ff);
	*data |= ((spi_msg[3] << 8)  & 0x0000ff00);
	*data |= ((spi_msg[4] << 16) & 0x00ff0000);
	*data |= ((spi_msg[5] << 24) & 0xff000000);

	return ret;
}

/**
 * Write a 4-byte word in the CAN controller memory using the SPI interface.
 *
 * Both SFRs and RAM memory can be written at word level.
 *
 * @param [in] dev     - Pointer to the handler structure.
 * @param [in] address - The address of the byte to be written.
 * @param [in] data    - Data to be written.
 *
 * @return 0 in case of success, error code in case of failure.
 */
int32_t can_ctrl_ram_write(struct can_ctrl_dev *dev, uint16_t address,
			   uint32_t data)
{
	uint8_t spi_msg[] = {0, 0, 0, 0, 0, 0};

	spi_msg[0] |= CAN_CTRL_CMD_MODE(CAN_CTRL_CMD_WRITE);
	spi_msg[0] |= CAN_CTRL_ADDR_UP_NIBBLE_MODE(address);
	spi_msg[1] |= CAN_CTRL_ADDR_DW_BYTE_MODE(address);
	spi_msg[2] = (data & 0x000000ff) >> 0;
	spi_msg[3] = (data & 0x0000ff00) >> 8;
	spi_msg[4] = (data & 0x00ff0000) >> 16;
	spi_msg[5] = (data & 0xff000000) >> 24;

	return spi_write_and_read(dev->can_ctrl_spi, spi_msg, 6);
}

/**
 * Give reset command to the CAN controller using SPI interface.
 *
 * @param [in] dev - Pointer to the handler structure.
 *
 * @return 0 in case of success, error code in case of failure.
 */
int32_t can_ctrl_reset(struct can_ctrl_dev *dev)
{
	uint8_t spi_msg[] = {0, 0, 0};

	return spi_write_and_read(dev->can_ctrl_spi, spi_msg, 3);
}

/**
 * Give reset command to the CAN controller using SPI interface.
 *
 * @param [in] dev - Pointer to the handler structure.
 *
 * @return 0 in case of success, error code in case of failure.
 */
int32_t can_ctrl_ecc_set(struct can_ctrl_dev *dev, bool enable)
{
	int32_t ret;
	uint32_t temp;

	ret = can_ctrl_sfr_word_read(dev, CAN_CTRL_REG_ECCCON, &temp);
	if(ret != 0)
		return ret;
	temp &= ~ECCCON_ECCEN_MODE(WORD_MASK);
	temp |= ECCCON_ECCEN_MODE(enable);

	return can_ctrl_sfr_word_write(dev, CAN_CTRL_REG_ECCCON, temp);
}

/**
 * Configure the bit times for the CAN controller.
 *
 * The CAN controller can have 2 bit times: arbitration and data bit-times. As
 * such, there are 2 separate sets of registers that configure the bit times.
 *
 * @param [in] dev  - Pointer to the handler structure.
 * @param [in] nbt  - Nominal (arbitration) bit-time choice.
 * @param [in] dbt  - Data bit-time choice.
 * @param [in] mode - Second sampling mode.
 *
 * @return 0 in case of success, error code in case of failure.
 */
int32_t can_ctrl_bit_time_config(struct can_ctrl_dev *dev,
				 enum can_ctrl_nominal_bitrate nbt, enum can_ctrl_data_bitrate dbt,
				 enum can_ctrl_ssp_mode mode)
{
	uint8_t nominal_br_tab[][4] = {
		{0, 254, 63, 63}, /* 125K */
		{0, 126, 31, 31}, /* 250K */
		{0,  62, 15, 15}, /* 500K */
		{0,  30,  7,  7}  /* 1M */
	};
	uint8_t data_br_tab[][5] = {
		{1, 30, 7, 7, 62}, /* 500K */
		{1, 17, 3, 7, 36}, /* 833K */
		{0, 30, 7, 7, 31}, /* 1M   */
		{0, 19, 4, 7, 20}, /* 1M5  */
		{0, 14, 3, 3, 15}, /* 2M   */
		{0,  8, 2, 2,  9}, /* 3M   */
		{0,  6, 1, 1,  7}, /* 4M   */
		{0,  4, 1, 1,  5}, /* 5M   */
		{0,  2, 0, 0,  3}, /* 6M7  */
		{0,  2, 0, 0,  3}, /* 8M   */
		{0,  1, 0, 0,  2}  /* 10M  */
	};
	int32_t ret;
	uint32_t temp;

	/* Nominal bit rate */
	ret = can_ctrl_sfr_word_read(dev, CAN_CTRL_REG_NBTCFG, &temp);
	if(ret != 0)
		return ret;
	temp &= ~(NBTCFG_BRP_MODE(WORD_MASK) | NBTCFG_TSEG1_MODE(WORD_MASK) |
		  NBTCFG_TSEG2_MODE(WORD_MASK) |
		  NBTCFG_SJW_MODE(WORD_MASK));
	temp |= NBTCFG_BRP_MODE(nominal_br_tab[nbt][0]) |
		NBTCFG_TSEG1_MODE(nominal_br_tab[nbt][1]) |
		NBTCFG_TSEG2_MODE(nominal_br_tab[nbt][2]) |
		NBTCFG_SJW_MODE(nominal_br_tab[nbt][3]);
	ret = can_ctrl_sfr_word_write(dev, CAN_CTRL_REG_NBTCFG, temp);
	if(ret != 0)
		return ret;

	/* Data bit rate */
	ret = can_ctrl_sfr_word_read(dev, CAN_CTRL_REG_DBTCFG, &temp);
	if(ret != 0)
		return ret;
	temp &= ~DBTCFG_BRP_MODE(WORD_MASK);
	temp &= ~DBTCFG_TSEG1_MODE(WORD_MASK);
	temp &= ~DBTCFG_TSEG2_MODE(WORD_MASK);
	temp &= ~DBTCFG_SJW_MODE(WORD_MASK);
	temp |= DBTCFG_BRP_MODE(data_br_tab[nbt][0]);
	temp |= DBTCFG_TSEG1_MODE(data_br_tab[nbt][1]);
	temp |= DBTCFG_TSEG2_MODE(data_br_tab[nbt][2]);
	temp |= DBTCFG_SJW_MODE(data_br_tab[nbt][3]);
	ret = can_ctrl_sfr_word_write(dev, CAN_CTRL_REG_DBTCFG, temp);
	if(ret != 0)
		return ret;

	/* Transmitter delay compensation */
	ret = can_ctrl_sfr_word_read(dev, CAN_CTRL_REG_TDC, &temp);
	if(ret != 0)
		return ret;
	temp &= ~TDC_TDCMOD_MODE(WORD_MASK);
	temp &= ~TDC_TDCO_MODE(WORD_MASK);
	temp |= TDC_TDCMOD_MODE(mode);
	temp |= TDC_TDCO_MODE(data_br_tab[nbt][4]);

	return can_ctrl_sfr_word_write(dev, CAN_CTRL_REG_TDC, temp);
}

/**
 * Clear interrupt flags for int the CAN controller.
 *
 * Some interrupt flags need to be cleared for the controller to continue
 * switching to sleep mode from the FD normal mode.
 *
 * @param [in] dev - Pointer to the handler structure.
 *
 * @return 0 in case of success, error code in case of failure.
 */
int32_t can_ctrl_clear_int(struct can_ctrl_dev *dev)
{
	int32_t ret;
	uint32_t temp = 0;

	ret = can_ctrl_sfr_word_read(dev, CAN_CTRL_REG_INT, &temp);
	if(ret != 0)
		return ret;
	temp &= ~INT_IVMIF_MODE(WORD_MASK);
	temp &= ~INT_WAKIF_MODE(WORD_MASK);
	temp &= ~INT_CERRIF_MODE(WORD_MASK);
	temp &= ~INT_SERRIF_MODE(WORD_MASK);
	temp &= ~INT_MODIF_MODE(WORD_MASK);
	temp &= ~INT_TBCIF_MODE(WORD_MASK);

	return can_ctrl_sfr_word_write(dev, CAN_CTRL_REG_INT, temp);
}

/**
 * Request an operation mode to the CAN controller.
 *
 * @param [in]  dev     - Pointer to the handler structure.
 * @param [in]  op_mode - Mode requested.
 * @param [out] success - Pointer to a boolean value that is true if the mode
 *                       changed correctly and false if it did not.
 *
 * @return 0 in case of success, error code in case of failure.
 */
int32_t can_ctrl_op_mode_req(struct can_ctrl_dev *dev,
			     enum can_ctrl_op_modes op_mode, bool *success)
{
	int32_t ret;
	uint32_t temp, timeout = 1000;
	uint8_t sfr_byte;

	*success = true;

	if (op_mode == CAN_SLEEP_MODE) {
		ret = can_ctrl_sfr_byte_read(dev, (CAN_CTRL_REG_IOCON + 1), &sfr_byte);
		if(ret != 0)
			return ret;
		sfr_byte |= 1;
		ret = can_ctrl_sfr_byte_write(dev, (CAN_CTRL_REG_IOCON + 1), sfr_byte);
		if(ret != 0)
			return ret;
	}
	mdelay(1);

	ret = can_ctrl_sfr_word_read(dev, CAN_CTRL_REG_CON, &temp);
	if(ret != 0)
		return ret;
	temp &= ~CON_REQOP_MODE(WORD_MASK);
	temp |= CON_REQOP_MODE(op_mode);
	ret = can_ctrl_sfr_byte_write(dev, (CAN_CTRL_REG_CON + 3),
				      *(((uint8_t *)&temp) + 3));
	if(ret != 0)
		return ret;
	mdelay(4);

	if (op_mode == CAN_SLEEP_MODE)
		do {
			ret = can_ctrl_sfr_word_read(dev, CAN_CTRL_REG_OSC, &temp);
			if(ret != 0)
				return ret;
			timeout--;
		} while(((temp & OSC_OSCDIS_MASK) == 0) && (timeout));

	if(!timeout)
		*success = false;

	return can_ctrl_clear_int(dev);
}

/**
 * See if a message has bee received by a FIFO.
 *
 * @param [in]  dev     - Pointer to the handler structure.
 * @param [in]  fifo_nr - Number of the FIFO checked.
 * @param [out] status  - Pointer to a boolean value that is true if the FIFO
 *                        has received a message and false if it did not.
 *
 * @return 0 in case of success, error code in case of failure.
 */
int32_t can_ctrl_is_fifo_rdy(struct can_ctrl_dev *dev, uint8_t fifo_nr,
			     bool *status)
{
	int32_t ret;
	uint32_t temp;

	/* Nominal bit rate */
	ret = can_ctrl_sfr_word_read(dev, CAN_CTRL_REG_FIFOSTA(fifo_nr), &temp);
	if(ret != 0)
		return ret;

	if(FIFOSTA_TFNRFNIF_MASK & temp)
		*status = true;
	else
		*status = false;

	return ret;
}

/**
 * Get data size in bytes based on the payload option of a message.
 *
 * @param [in] dev - Pointer to the handler structure.
 * @param [in] pls - Payload size.
 *
 * @return number of data bytes.
 */
uint8_t can_ctrl_message_size(struct can_ctrl_dev *dev,
			      enum can_ctrl_fifo_plsize pls)
{
	switch(pls) {
	case DATA_BYTES_8:
		return 8;
	case DATA_BYTES_12:
		return 12;
	case DATA_BYTES_16:
		return 16;
	case DATA_BYTES_20:
		return 20;
	case DATA_BYTES_24:
		return 24;
	case DATA_BYTES_32:
		return 32;
	case DATA_BYTES_48:
		return 48;
	case DATA_BYTES_64:
		return 64;
	default:
		return 0;
	}
}

/**
 * Get data size in bytes based on the Data Length Code (DLC) option of a
 * message.
 *
 * @param [in] dlc - DLC code.
 *
 * @return number of data bytes.
 */
static uint8_t can_ctrl_dlc_to_size(enum can_ctrl_dlc dlc)
{
	switch(dlc) {
	case CAN_DLC_0:
	case CAN_DLC_1:
	case CAN_DLC_2:
	case CAN_DLC_3:
	case CAN_DLC_4:
	case CAN_DLC_5:
	case CAN_DLC_6:
	case CAN_DLC_7:
	case CAN_DLC_8:
		return dlc;
	case CAN_DLC_12:
		return 12;
	case CAN_DLC_16:
		return 16;
	case CAN_DLC_20:
		return 20;
	case CAN_DLC_24:
		return 24;
	case CAN_DLC_32:
		return 32;
	case CAN_DLC_48:
		return 48;
	case CAN_DLC_64:
		return 64;
	default:
		return 0;
	}
}

/**
 * Get DLC code from a given number of bytes.
 *
 * This function is used to calculate the minimum necessary DLC code for a
 * number of bytes.
 *
 * @param [in] size - Number of data bytes.
 *
 * @return DLC code.
 */
enum can_ctrl_dlc can_ctrl_size_to_dlc(uint8_t size)
{
	if(size > 64)
		return CAN_DLC_TOO_SMALL;
	if((size > 8) && (size <= 12))
		return CAN_DLC_12;
	if((size > 12) && (size <= 16))
		return CAN_DLC_16;
	if((size > 16) && (size <= 20))
		return CAN_DLC_20;
	if((size > 20) && (size <= 24))
		return CAN_DLC_24;
	if((size > 24) && (size <= 32))
		return CAN_DLC_32;
	if((size > 32) && (size <= 48))
		return CAN_DLC_48;
	if((size > 48) && (size <= 64))
		return CAN_DLC_64;

	return size;
}

/**
 * Get received message from a specified FIFO.
 *
 * This function checks if a CAN message has been received in the given FIFO and
 * parses the message to get the data payload form within.
 *
 * @param [in]  dev     - Pointer to the handler structure.
 * @param [in]  fifo_nr - Number of the FIFO checked.
 * @param [out] data    - Pointer to the data container.
 *
 * @return 0 in case of success, error code in case of failure.
 */
int32_t can_ctrl_get_rec_message(struct can_ctrl_dev *dev, uint8_t fifo_nr,
				 uint8_t *data)
{
	int32_t ret;
	uint32_t msg_addr, fifo_con;
	uint8_t msg_size;
	uint32_t *msg_buff;
	int8_t i;
	uint8_t payload_size;
	union tx_obj_header obj;

	ret = can_ctrl_sfr_word_read(dev, CAN_CTRL_REG_FIFOCON(fifo_nr), &fifo_con);
	if(ret != 0)
		return ret;

	if(fifo_con & FIFOCON_TXEN_MASK)
		return ret;

	ret = can_ctrl_sfr_word_read(dev, CAN_CTRL_REG_FIFOUA(fifo_nr), &msg_addr);
	if(ret != 0)
		return ret;
	msg_addr += CAN_CTRL_REG_RAM_ADDR;

	payload_size = can_ctrl_message_size(dev, dev->rx_fifo_plsize);
	msg_size = 8 + payload_size;
	if(fifo_con & FIFOCON_RXTSEN_MASK)
		msg_size += 4;
	if(msg_size % 4)
		msg_size = msg_size + 4 - (msg_size % 4);
	msg_buff = calloc(msg_size, sizeof(uint8_t));
	for(i = 0; i < (msg_size / 4); i++) {
		ret = can_ctrl_ram_read(dev, (msg_addr + i * 4), (msg_buff + i));
		if(ret != 0) {
			free(msg_buff);
			return ret;
		}
	}

	obj.word[0] = msg_buff[1];
	obj.word[1] = msg_buff[0];
	payload_size = can_ctrl_dlc_to_size(obj.hdr_str.dlc);

	if(fifo_con & FIFOCON_RXTSEN_MASK)
		memcpy(data, ((uint8_t *)msg_buff) + 12, payload_size);
	else
		memcpy(data, ((uint8_t *)msg_buff) + 8, payload_size);
	free(msg_buff);

	fifo_con &= ~FIFOCON_UINC_MODE(WORD_MASK);
	fifo_con |= FIFOCON_UINC_MODE(true);

	return can_ctrl_sfr_byte_write(dev, (CAN_CTRL_REG_FIFOCON(fifo_nr) + 1),
				       *(((uint8_t *)&fifo_con) + 1));
}

/**
 * Transmit a message through CAN.
 *
 * @param [in] dev     - Pointer to the handler structure.
 * @param [in] fifo_nr - Number of the FIFO that will contain the message until
 *                       transmission.
 * @param [in] data    - Pointer to the data payload of the message.
 * @param [in] header  - CAN message header structure. This structure contains
 *                       the actual header of the CAN message and some other
 *                       parameters like the priority.
 *
 * @return 0 in case of success, error code in case of failure.
 */
int32_t can_ctrl_message_transmit(struct can_ctrl_dev *dev, uint8_t fifo_nr,
				  uint8_t *data, union tx_obj_header header)
{
	int32_t ret;
	uint32_t fifo_con, msg_addr, *msg, temp_word;
	uint8_t msg_size;
	int8_t i;

	ret = can_ctrl_sfr_word_read(dev, CAN_CTRL_REG_FIFOCON(fifo_nr), &fifo_con);
	if(ret != 0)
		return ret;

	if(!(fifo_con & FIFOCON_TXEN_MASK))
		return ret;

	ret = can_ctrl_sfr_word_read(dev, CAN_CTRL_REG_FIFOUA(fifo_nr), &msg_addr);
	if(ret != 0)
		return ret;
	msg_addr += CAN_CTRL_REG_RAM_ADDR;

	msg_size = 8 + can_ctrl_message_size(dev, dev->tx_fifo_plsize);
	if(msg_size % 4)
		msg_size = msg_size + 4 - (msg_size % 4);

	msg = calloc((msg_size / 4), sizeof *msg);
	msg[0] = header.word[1];
	msg[1] = header.word[0];
	for(i = 0; i < (msg_size - 8); (i += 4)) {
		temp_word = data[i] << 0;
		temp_word |= data[i + 1] << 8;
		temp_word |= data[i + 2] << 16;
		temp_word |= data[i + 3] << 24;
		msg[(i / 4 + 2)] = temp_word;
	}

	for(i = 0; i < (msg_size / 4); i++) {
		ret = can_ctrl_ram_write(dev, (msg_addr + i * 4), msg[i]);
		if(ret != 0) {
			free(msg);
			return ret;
		}
	}

	fifo_con |= FIFOCON_UINC_MODE(true);
	fifo_con |= FIFOCON_TXREQ_MODE(true);
	ret = can_ctrl_sfr_word_write(dev, CAN_CTRL_REG_FIFOCON(fifo_nr),
				      fifo_con);

	free(msg);

	return ret;
}

/**
 * Set a new standard ID for received messages.
 *
 * @param [in] dev     - Pointer to the handler structure.
 * @param [in] new_sid - Value of the new SID (between 0x000 and 0x3FF).
 *
 * @return void
 */
void can_ctrl_set_tx_sid(struct can_ctrl_dev *dev, uint16_t new_sid)
{
	dev->tx_sid = new_sid;
}

/**
 * Wait for message transmission.
 *
 * This method block the program to wait for a message transmission. It waits
 * either until the message is acknowledged or 5 seconds.
 *
 * @param [in]  dev - Pointer to the handler structure.
 * @param [out] scs - Pointer to a boolean value that is true in case message
 *                    has been successfully acknowledged and false if the
 *                    message has not been acknowledged in 5 seconds.
 *
 * @return 0 in case of success, error code otherwise.
 */
int32_t can_ctrl_wait_tx(struct can_ctrl_dev *dev, bool *scs)
{
	int32_t ret;
	uint32_t data = 0;
	uint32_t timeout = 5000;

	*scs = true;

	do {
		ret = can_ctrl_sfr_word_read(dev, CAN_CTRL_REG_FIFOCON(dev->tx_fifo_nr),
					     &data);
		if(ret != SUCCESS)
			return ret;
		timeout--;
		mdelay(1);
	} while(((data & FIFOCON_TXREQ_MODE(WORD_MASK)) != 0) && timeout);

	if(!timeout)
		*scs = false;

	return ret;
}

/**
 * Initialize RAM memory with all 0xFFs.
 *
 * @param [in] dev - Pointer to the handler structure.
 *
 * @return 0 in case of success, error code otherwise.
 */
static int32_t can_ctrl_init_ram(struct can_ctrl_dev *dev)
{
	int32_t ret;
	uint16_t addr = CAN_CTRL_REG_RAM_ADDR;
	int32_t i;

	for(i = 0; i < CAN_CTRL_REG_RAM_SIZE; i += 4) {
		ret = can_ctrl_ram_write(dev, (addr + i), 0xffffffff);
		if(ret != SUCCESS)
			return ret;
	}

	return ret;
}

/**
 * Clear the wake-up interrupt flag.
 *
 * This function clears the wake-up interrupt flag to clear the interrupt it
 * gives.
 *
 * @param [in] dev - Pointer to the handler structure.
 *
 * @return 0 in case of success, error code otherwise.
 */
int32_t can_ctrl_clear_wake_flag(struct can_ctrl_dev *dev)
{
	int32_t ret;
	uint32_t data;

	ret = can_ctrl_sfr_word_read(dev, CAN_CTRL_REG_INT, &data);
	if(ret != 0)
		return ret;
	data &= ~INT_WAKIF_MODE(true);

	return can_ctrl_sfr_byte_write(dev, (CAN_CTRL_REG_INT + 1),
				       *(((uint8_t *)&data) + 1));
}

/**
 * Configure IOCON register of the CAN controller.
 *
 * This register is responsible for the function parameters of the GPIO/INT
 * present on the device.
 *
 * @param [in] dev - Pointer to the handler structure.
 *
 * @return 0 in case of success, error code otherwise.
 */
static int32_t can_ctrl_gpio_config(struct can_ctrl_dev *dev)
{
	int32_t ret;
	uint32_t temp;

	ret = can_ctrl_sfr_word_read(dev, CAN_CTRL_REG_IOCON, &temp);
	if(ret != 0)
		return ret;
	temp &= ~(IOCON_PM1_MODE(WORD_MASK) | IOCON_PM0_MODE(WORD_MASK) |
		  IOCON_XSTBYEN_MODE(WORD_MASK) |
		  IOCON_TRIS0_MODE(WORD_MASK));
	temp |= (IOCON_PM1_MODE(false) | IOCON_PM0_MODE(true) |
		 IOCON_XSTBYEN_MODE(false) | IOCON_TRIS0_MODE(false));

	return can_ctrl_sfr_word_write(dev, CAN_CTRL_REG_IOCON, temp);
}

/**
 * Wake-up routine for the CAN controller.
 *
 * The application needs to activate the system clock, wait for it to stabilize
 * (at least 4 ms), and put the controller in normal FD mode because it wakes up
 * in Configuration mode.
 *
 * @param [in] dev - Pointer to the handler structure.
 *
 * @return 0 in case of success, error code otherwise.
 */
int32_t can_ctrl_wake_up(struct can_ctrl_dev *dev)
{
	int32_t ret;
	uint32_t data = 0;
	uint8_t sfr_byte;
	bool scs;

	ret = can_ctrl_sfr_word_read(dev, CAN_CTRL_REG_OSC, &data);
	if(ret != 0)
		return ret;
	data &= ~OSC_OSCDIS_MODE(WORD_MASK);
	ret = can_ctrl_sfr_word_write(dev, CAN_CTRL_REG_OSC, data);
	if(ret != 0)
		return ret;
	mdelay(4);

	ret = can_ctrl_setup_dev(dev);
	if(ret != 0)
		return ret;

	ret = can_ctrl_op_mode_req(dev, CAN_NORMAL_FD_MODE, &scs);
	if(ret != 0)
		return ret;
	do {
		ret = can_ctrl_sfr_word_read(dev, CAN_CTRL_REG_CON, &data);
		if(ret != 0)
			return ret;
	} while((data & CON_OPMOD_MASK) != CON_OPMOD_MODE(CAN_NORMAL_FD_MODE));

	ret = can_ctrl_sfr_byte_read(dev, (CAN_CTRL_REG_IOCON + 1), &sfr_byte);
	if(ret != 0)
		return ret;
	sfr_byte &= 0xfe;

	ret = can_ctrl_sfr_byte_write(dev, (CAN_CTRL_REG_IOCON + 1), sfr_byte);
	if(ret != 0)
		return ret;

	mdelay(4);

	return ret;
}

/**
 * Configure CON register of the CAN controller.
 *
 * Responsible for different function parameters like TX Queue, wake-up filter,
 * etc.
 *
 * @param [in] dev        - Pointer to the handler structure.
 * @param [in] init_param - Pointer to the initialization structure.
 *
 * @return 0 in case of success, error code otherwise.
 */
static int32_t can_ctrl_con_reg_config(struct can_ctrl_dev *dev,
				       struct can_ctrl_init_param *init_param)
{
	int32_t ret;
	uint32_t temp;

	ret = can_ctrl_sfr_word_read(dev, CAN_CTRL_REG_CON, &temp);
	if(ret != 0)
		return ret;
	temp &= ~(CON_TXQEN_MODE(WORD_MASK) | CON_STEF_MODE(WORD_MASK) |
		  CON_ISOCRCEN_MODE(WORD_MASK) |
		  CON_WAKFIL_MODE(WORD_MASK));
	temp |= CON_TXQEN_MODE(init_param->con_txq_en) |
		CON_STEF_MODE(init_param->con_store_in_tef_en) |
		CON_ISOCRCEN_MODE(init_param->con_iso_crc_en) |
		CON_WAKFIL_MODE(true);

	return can_ctrl_sfr_word_write(dev, CAN_CTRL_REG_CON, temp);
}

/**
 * Configure TEFCON register of the CAN controller.
 *
 * Responsible with the Transmit Event FIFO.
 *
 * @param [in] dev        - Pointer to the handler structure.
 * @param [in] init_param - Pointer to the initialization structure.
 *
 * @return 0 in case of success, error code otherwise.
 */
static int32_t can_ctrl_tefcon_reg_config(struct can_ctrl_dev *dev,
		struct can_ctrl_init_param *init_param)
{
	int32_t ret;
	uint32_t temp;

	ret = can_ctrl_sfr_word_read(dev, CAN_CTRL_REG_TEFCON, &temp);
	if(ret != 0)
		return ret;
	temp &= ~TEFCON_FSIZE_MODE(WORD_MASK);
	temp &= ~TEFCON_TEFTSEN_MODE(WORD_MASK);
	temp |= TEFCON_FSIZE_MODE(init_param->tef_fifo_size - 1);
	temp |= TEFCON_TEFTSEN_MODE(init_param->tef_time_stamp_en);

	return can_ctrl_sfr_word_write(dev, CAN_CTRL_REG_TEFCON, temp);
}

/**
 * Configure TXQCON register of the CAN controller.
 *
 * Responsible with the Transmit Queue FIFO.
 *
 * @param [in] dev        - Pointer to the handler structure.
 * @param [in] init_param - Pointer to the initialization structure.
 *
 * @return 0 in case of success, error code otherwise.
 */
static int32_t can_ctrl_txqcon_reg_config(struct can_ctrl_dev *dev,
		struct can_ctrl_init_param *init_param)
{
	int32_t ret;
	uint32_t temp;

	ret = can_ctrl_sfr_word_read(dev, CAN_CTRL_REG_TXQCON, &temp);
	if(ret != 0)
		return ret;
	temp &= ~TXQCON_PLSIZE_MODE(WORD_MASK);
	temp &= ~TXQCON_FSIZE_MODE(WORD_MASK);
	temp &= ~TXQCON_TXPRI_MODE(WORD_MASK);
	temp |= TXQCON_PLSIZE_MODE(init_param->txq_plsize);
	temp |= TXQCON_FSIZE_MODE((init_param->txq_fifo_size - 1));
	temp |= TXQCON_TXPRI_MODE(init_param->txq_tx_priority);

	return can_ctrl_sfr_word_write(dev, CAN_CTRL_REG_TXQCON, temp);
}

/**
 * Configure a FIFOCON register of the CAN controller to be transmit FIFO.
 *
 * @param [in] dev        - Pointer to the handler structure.
 * @param [in] fifo_nr    - Number of the FIFO.
 * @param [in] init_param - Pointer to the initialization structure.
 *
 * @return 0 in case of success, error code otherwise.
 */
static int32_t can_ctrl_tx_fifo_config(struct can_ctrl_dev *dev,
				       uint8_t fifo_nr, struct can_ctrl_init_param *init_param)
{
	int32_t ret;
	uint32_t temp;

	ret = can_ctrl_sfr_word_read(dev, CAN_CTRL_REG_FIFOCON(fifo_nr), &temp);
	if(ret != 0)
		return ret;
	temp &= ~FIFOCON_PLSIZE_MODE(WORD_MASK);
	temp &= ~FIFOCON_FSIZE_MODE(WORD_MASK);
	temp &= ~FIFOCON_TXPRI_MODE(WORD_MASK);
	temp &= ~FIFOCON_TXEN_MODE(WORD_MASK);
	temp |= FIFOCON_PLSIZE_MODE(init_param->tx_fifo_plsize);
	temp |= FIFOCON_FSIZE_MODE((init_param->tx_fifo_size - 1));
	temp |= FIFOCON_TXPRI_MODE(init_param->tx_fifo_priority);
	temp |= FIFOCON_TXEN_MODE(true);

	return can_ctrl_sfr_word_write(dev, CAN_CTRL_REG_FIFOCON(fifo_nr), temp);
}

/**
 * Configure a FIFOCON register of the CAN controller to be receive FIFO.
 *
 * @param [in] dev        - Pointer to the handler structure.
 * @param [in] fifo_nr    - Number of the FIFO.
 * @param [in] init_param - Pointer to the initialization structure.
 *
 * @return 0 in case of success, error code otherwise.
 */
static int32_t can_ctrl_rx_fifo_config(struct can_ctrl_dev *dev,
				       uint8_t fifo_nr, struct can_ctrl_init_param *init_param)
{
	int32_t ret;
	uint32_t temp;

	ret = can_ctrl_sfr_word_read(dev, CAN_CTRL_REG_FIFOCON(fifo_nr), &temp);
	if(ret != 0)
		return ret;
	temp &= ~FIFOCON_PLSIZE_MODE(WORD_MASK);
	temp &= ~FIFOCON_FSIZE_MODE(WORD_MASK);
	temp &= ~FIFOCON_RXTSEN_MODE(WORD_MASK);
	temp |= FIFOCON_PLSIZE_MODE(init_param->rx_fifo_plsize);
	temp |= FIFOCON_FSIZE_MODE((init_param->rx_fifo_size - 1));
	temp |= FIFOCON_RXTSEN_MODE(init_param->rx_fifo_tsen);

	return can_ctrl_sfr_word_write(dev, CAN_CTRL_REG_FIFOCON(fifo_nr), temp);
}

/**
 * Configure a FLTCON register of the CAN controller.
 *
 * Configure the register to be appropriate for the intended receive FIFO.
 *
 * @param [in] dev        - Pointer to the handler structure.
 * @param [in] rx_flt_nr  - Number of the filter.
 * @param [in] init_param - Pointer to the initialization structure.
 *
 * @return 0 in case of success, error code otherwise.
 */
static int32_t can_ctrl_rx_flt_config(struct can_ctrl_dev *dev,
				      uint8_t rx_flt_nr, struct can_ctrl_init_param *init_param)
{
	int32_t ret;
	uint32_t temp;

	ret = can_ctrl_sfr_word_read(dev, CAN_CTRL_REG_FLTOBJ(rx_flt_nr), &temp);
	if(ret != 0)
		return ret;
	temp &= ~FLTOBJ_SID_MODE(WORD_MASK);
	temp |= FLTOBJ_SID_MODE(init_param->rx_sid_addr);

	ret = can_ctrl_sfr_word_write(dev, CAN_CTRL_REG_FLTOBJ(rx_flt_nr), temp);
	if(ret != 0)
		return ret;

	ret = can_ctrl_sfr_word_read(dev, CAN_CTRL_REG_MASK(rx_flt_nr), &temp);
	if(ret != 0)
		return ret;
	temp &= ~MASK_MSID_MODE(WORD_MASK);
	temp |= MASK_MSID_MODE(0x7FF);
	return can_ctrl_sfr_word_write(dev, CAN_CTRL_REG_MASK(rx_flt_nr), temp);
}

/**
 * Link a filter to a FIFO.
 *
 * @param [in] dev        - Pointer to the handler structure.
 * @param [in] rx_flt_nr  - Number of the filter.
 * @param [in] rx_fifo_nr - Number of the receive FIFO.
 * @param [in] enable     - If true enable the link and start receiving,
 *                          if false do not enable the link yet.
 *
 * @return 0 in case of success, error code otherwise.
 */
static int32_t can_ctrl_rx_flt_fifo_link(struct can_ctrl_dev *dev,
		uint8_t rx_flt_nr, uint8_t rx_fifo_nr, bool enable)
{
	int32_t ret;
	uint32_t temp;

	ret = can_ctrl_sfr_word_read(dev, CAN_CTRL_REG_FLTCON(rx_flt_nr / 4),
				     &temp);
	if(ret != 0)
		return ret;
	temp &= ~FLTCON_FLTEN_MODE(WORD_MASK, (rx_flt_nr % 4));
	temp &= ~FLTCON_FiBP_MODE(WORD_MASK, (rx_flt_nr % 4));
	temp |= FLTCON_FLTEN_MODE(enable, (rx_flt_nr % 4));
	temp |= FLTCON_FiBP_MODE(rx_fifo_nr, (rx_flt_nr % 4));

	return can_ctrl_sfr_word_write(dev, CAN_CTRL_REG_FLTCON(rx_flt_nr / 4),
				       temp);
}

/**
 * Setup which FIFO interrupts to mask and which to enable.
 *
 * @param [in] dev     - Pointer to the handler structure.
 * @param [in] fifo_nr - Number of the FIFO.
 * @param [in] flags   - The interrupts to be enabled.
 *
 * @return 0 in case of success, error code otherwise.
 */
static int32_t can_ctrl_setup_fifo_int(struct can_ctrl_dev *dev,
				       uint8_t fifo_nr, uint8_t flags)
{
	int32_t ret;
	uint32_t temp;

	ret = can_ctrl_sfr_word_read(dev, CAN_CTRL_REG_FIFOCON(fifo_nr), &temp);
	if(ret != 0)
		return ret;
	temp &= ~FIFOCON_TXATIE_MODE(WORD_MASK);
	temp &= ~FIFOCON_RXOVIE_MODE(WORD_MASK);
	temp &= ~FIFOCON_TFERFFIE_MODE(WORD_MASK);
	temp &= ~FIFOCON_TFHRFHIE_MODE(WORD_MASK);
	temp &= ~FIFOCON_TFNRFNIE_MODE(WORD_MASK);
	temp |= FIFOCON_TFNRFNIE_MODE(flags);
	flags >>= 1;
	temp |= FIFOCON_TFHRFHIE_MODE(flags);
	flags >>= 1;
	temp |= FIFOCON_TFERFFIE_MODE(flags);
	flags >>= 1;
	temp |= FIFOCON_RXOVIE_MODE(flags);
	flags >>= 1;
	temp |= FIFOCON_TXATIE_MODE(flags);

	return can_ctrl_sfr_word_write(dev, CAN_CTRL_REG_FIFOCON(fifo_nr), temp);
}

/**
 * Setup the INT register of the CAN controller.
 *
 * @param [in] dev   - Pointer to the handler structure.
 * @param [in] flags - The interrupts to be enabled.
 *
 * @return 0 in case of success, error code otherwise.
 */
static int32_t can_ctrl_setup_int(struct can_ctrl_dev *dev, uint16_t flags)
{
	int32_t ret;
	uint32_t temp;

	ret = can_ctrl_sfr_word_read(dev, CAN_CTRL_REG_INT, &temp);
	if(ret != 0)
		return ret;
	temp &= ~(INT_TXIE_MODE(WORD_MASK) | INT_RXIE_MODE(WORD_MASK) |
		  INT_TBCIE_MODE(WORD_MASK) | INT_MODIE_MODE(WORD_MASK) |
		  INT_TEFIE_MODE(WORD_MASK) | INT_ECCIE_MODE(WORD_MASK) |
		  INT_SPICRCIE_MODE(WORD_MASK) | INT_TXATIE_MODE(WORD_MASK) |
		  INT_RXOVIE_MODE(WORD_MASK) | INT_SERRIE_MODE(WORD_MASK) |
		  INT_CERRIE_MODE(WORD_MASK) | INT_WAKIE_MODE(WORD_MASK) |
		  INT_IVMIE_MODE(WORD_MASK));
	temp |= INT_TXIE_MODE(flags);
	flags >>= 1;
	temp |= INT_RXIE_MODE(flags);
	flags >>= 1;
	temp |= INT_TBCIE_MODE(flags);
	flags >>= 1;
	temp |= INT_MODIE_MODE(flags);
	flags >>= 1;
	temp |= INT_TEFIE_MODE(flags);
	flags >>= 1;
	temp |= INT_ECCIE_MODE(flags);
	flags >>= 1;
	temp |= INT_SPICRCIE_MODE(flags);
	flags >>= 1;
	temp |= INT_TXATIE_MODE(flags);
	flags >>= 1;
	temp |= INT_RXOVIE_MODE(flags);
	flags >>= 1;
	temp |= INT_SERRIE_MODE(flags);
	flags >>= 1;
	temp |= INT_CERRIE_MODE(flags);
	flags >>= 1;
	temp |= INT_WAKIE_MODE(flags);
	flags >>= 1;
	temp |= INT_IVMIE_MODE(flags);

	return can_ctrl_sfr_word_write(dev, CAN_CTRL_REG_INT, temp);
}

/**
 * Does a device setup including all registers needed to function.
 *
 * This is done to have a portable method that can be called any time after
 * allocating memory for the driver. This is most useful when the controller
 * wakes up after sleep mode because it looses most of the configuration.
 *
 * @param [in] dev - Pointer to the handler structure.
 *
 * @return 0 in case of success, error code otherwise.
 */
int32_t can_ctrl_setup_dev(struct can_ctrl_dev *dev)
{
	int32_t ret;
	struct can_ctrl_init_param init_param;

	can_ctrl_get_config(&init_param);

	ret = can_ctrl_ecc_set(dev, true);
	if(ret != SUCCESS)
		return ret;

	ret = can_ctrl_con_reg_config(dev, &init_param);
	if(ret != SUCCESS)
		return ret;

	ret = can_ctrl_tefcon_reg_config(dev, &init_param);
	if(ret != SUCCESS)
		return ret;

	ret = can_ctrl_txqcon_reg_config(dev, &init_param);
	if(ret != SUCCESS)
		return ret;

	ret = can_ctrl_tx_fifo_config(dev, init_param.tx_fifo_nr, &init_param);
	if(ret != SUCCESS)
		return ret;

	ret = can_ctrl_rx_fifo_config(dev, init_param.rx_fifo_nr, &init_param);
	if(ret != SUCCESS)
		return ret;

	ret = can_ctrl_rx_flt_config(dev, init_param.rx_flt_nr, &init_param);
	if(ret != SUCCESS)
		return ret;

	ret = can_ctrl_rx_flt_fifo_link(dev, init_param.rx_flt_nr,
					init_param.rx_fifo_nr, true);
	if(ret != SUCCESS)
		return ret;

	ret = can_ctrl_bit_time_config(dev, dev->can_nbt, dev->can_dbt,
				       dev->ssp_mode);
	if(ret != SUCCESS)
		return ret;

	ret = can_ctrl_gpio_config(dev);
	if(ret != SUCCESS)
		return ret;

	ret = can_ctrl_setup_fifo_int(dev, init_param.rx_fifo_nr, TFNRFNIE);
	if(ret != SUCCESS)
		return ret;

	return can_ctrl_setup_int(dev, WAKIF | RXIF);
}

/**
 * Allocate memory for the device driver and initialize it.
 *
 * @param [out] device    - Pointer to the device handler structure.
 * @param [in] init_param - Pointer to the device initialization structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t can_ctrl_setup(struct can_ctrl_dev **device,
		       struct can_ctrl_init_param *init_param)
{
	int32_t ret;
	struct can_ctrl_dev *dev;
	bool scs;

	dev = calloc(1, sizeof *dev);
	if(!dev)
		return FAILURE;

	dev->txq_plsize = init_param->txq_plsize;
	dev->tx_fifo_plsize = init_param->tx_fifo_plsize;
	dev->rx_fifo_plsize = init_param->rx_fifo_plsize;
	dev->rx_flt_nr = init_param->rx_flt_nr;
	dev->can_nbt = init_param->can_nbt;
	dev->can_dbt = init_param->can_dbt;
	dev->ssp_mode = init_param->ssp_mode;
	dev->rx_fifo_nr = init_param->rx_fifo_nr;
	dev->tx_fifo_nr = init_param->tx_fifo_nr;
	dev->tx_sid = 0x300;
	dev->rx_sid = init_param->rx_sid_addr;

	ret = spi_init(&dev->can_ctrl_spi, &init_param->can_ctrl_spi_init);
	if(ret != SUCCESS)
		goto error;

	ret = can_ctrl_reset(dev);
	if(ret != SUCCESS)
		goto error;

	mdelay(5);

	ret = can_ctrl_init_ram(dev);
	if(ret != SUCCESS)
		goto error;

	ret = can_ctrl_ecc_set(dev, true);
	if(ret != SUCCESS)
		goto error;

	ret = can_ctrl_con_reg_config(dev, init_param);
	if(ret != SUCCESS)
		goto error;

	ret = can_ctrl_tefcon_reg_config(dev, init_param);
	if(ret != SUCCESS)
		goto error;

	ret = can_ctrl_txqcon_reg_config(dev, init_param);
	if(ret != SUCCESS)
		goto error;

	ret = can_ctrl_tx_fifo_config(dev, init_param->tx_fifo_nr, init_param);
	if(ret != SUCCESS)
		goto error;

	ret = can_ctrl_rx_fifo_config(dev, init_param->rx_fifo_nr, init_param);
	if(ret != SUCCESS)
		goto error;

	ret = can_ctrl_rx_flt_config(dev, init_param->rx_flt_nr, init_param);
	if(ret != SUCCESS)
		goto error;

	ret = can_ctrl_rx_flt_fifo_link(dev, init_param->rx_flt_nr,
					init_param->rx_fifo_nr, true);
	if(ret != SUCCESS)
		goto error;

	ret = can_ctrl_bit_time_config(dev, dev->can_nbt, dev->can_dbt,
				       dev->ssp_mode);
	if(ret != SUCCESS)
		goto error;

	ret = can_ctrl_gpio_config(dev);
	if(ret != SUCCESS)
		goto error;

	ret = can_ctrl_setup_fifo_int(dev, init_param->rx_fifo_nr, TFNRFNIE);
	if(ret != SUCCESS)
		goto error;

	ret = can_ctrl_setup_int(dev, WAKIF | RXIF);
	if(ret != SUCCESS)
		goto error;

	ret = can_ctrl_op_mode_req(dev, CAN_SLEEP_MODE, &scs);
	if(ret != SUCCESS)
		goto error;

	*device = dev;

	return ret;
error:
	free(dev);

	return ret;
}

/**
 * Free memory allocated by the can_ctrl_setup() function.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t can_ctrl_remove(struct can_ctrl_dev *dev)
{
	int32_t ret;

	if(!dev)
		return FAILURE;

	ret = spi_remove(dev->can_ctrl_spi);
	if(ret != 0)
		return ret;

	free(dev);

	return ret;
}

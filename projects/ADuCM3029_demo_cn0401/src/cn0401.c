/***************************************************************************//**
 *   @file   cn0401.c
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

#include "cn0401.h"
#include <stdlib.h>
#include <string.h>
#include "power.h"

/******************************************************************************/
/************************** Variable Definitions ******************************/
/******************************************************************************/

/* Command vector */
cmd_func fnc_ptr[] = {
	(cmd_func)cn0401_help,
	(cmd_func)cn0401_can_send,
	(cmd_func)cn0401_can_set_tx_sid,
	(cmd_func)cn0401_can_get_rec,
	(cmd_func)cn0401_can_test_proc,
	NULL
};

/* Command call vector */
char *fnc_calls[] = {
	"help",
	"h",
	"can_transmit ",
	"ct ",
	"can_set_sid ",
	"css ",
	"can_get_rec",
	"cg",
	"can_test",
	"t",
	""
};

/* Command size vector */
uint8_t fnc_call_size[] = {5, 2, 13, 3, 12, 4, 12, 3, 9, 2, 1};

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * CAN receive callback.
 *
 * This function is called when the RX interrupt signal is asserted by the CAN
 * controller.
 *
 * @param [in] cb_param - Callback parameter passed by the application.
 * @param [in] event    - Trigger ID code.
 * @param [in] arg      - Callback parameter passed by the driver.
 *
 * @return void
 */
static void can_rec_callback(void *cb_param, uint32_t event, void *arg)
{
	((struct cn0401_dev *)cb_param)->rec_flag = true;
	NVIC_DisableIRQ(XINT_EVT0_IRQn);
}

/**
 * Provide default configuration for the application initialization handler.
 *
 * @param [out] init_param - Pointer to the initialization structure.
 *
 * @return void.
 */
void cn0401_get_config(struct cn0401_init_param *init_param)
{
	can_ctrl_get_config(&init_param->can_ctrl_init);

	init_param->cli_init.uart_init.baudrate = bd115200;
	init_param->cli_init.uart_init.bits_no = 8;
	init_param->cli_init.uart_init.has_callback = true;

	init_param->aux_gpio = 0x1B;
	init_param->silent_gpio = 0x09;
}

/**
 * Help command helper function. Display help function prompt.
 *
 * @param [in] dev	 	 	 - The device structure.
 * @param [in] short_command - True to display the long command prompt,
 *                             false to display the short command prompt.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0401_help_prompt(struct cn0401_dev *dev, bool short_command)
{
	int32_t ret;

	if (!short_command) {
		ret = usr_uart_write_string(dev->cli_handler->uart_device,
					    (uint8_t*)"\tCN0424 application.\n");
		if(ret < 0)
			return ret;
		usr_uart_write_string(dev->cli_handler->uart_device,
				      (uint8_t*)
				      "For commands with options as arguments typing the command and 'space' without arguments\n");
		if(ret < 0)
			return ret;
		usr_uart_write_string(dev->cli_handler->uart_device,
				      (uint8_t*)"will show the list of options.\n");
		if(ret < 0)
			return ret;
		return usr_uart_write_string(dev->cli_handler->uart_device,
					     (uint8_t*)"Available verbose commands.\n\n");
	} else {
		return usr_uart_write_string(dev->cli_handler->uart_device,
					     (uint8_t*)"\nAvailable short commands:\n\n");
	}
}

/**
 * Help command helper function.
 *
 * Display the communication commands.
 *
 * @param [in] dev	 	 	 - The device structure.
 * @param [in] short_command - True to display the long command prompt,
 *                             false to display the short command prompt.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0401_help_comm(struct cn0401_dev *dev, bool short_command)
{
	int32_t ret;

	if (!short_command) {
		ret = usr_uart_write_string(dev->cli_handler->uart_device,
					    (uint8_t*)" can_transmit <msg> - Transmit message through CAN.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->cli_handler->uart_device,
					    (uint8_t*)"                      <msg> = message to be sent.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->cli_handler->uart_device,
					    (uint8_t*)" can_set_sid <sid>  - Set standard ID for the CAN messages sent.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->cli_handler->uart_device,
					    (uint8_t*)
					    "                      <sid> = Standard ID in hexadecimal; between 0x000 and 0x3FF.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->cli_handler->uart_device,
					    (uint8_t*)"                      Example: can_set_sid 300\n");
		if(ret < 0)
			return ret;

		return usr_uart_write_string(dev->cli_handler->uart_device,
					     (uint8_t*)" can_get_rec        - Get received messages if any.\n");
	} else {
		ret = usr_uart_write_string(dev->cli_handler->uart_device,
					    (uint8_t*)" ct <msg>  - Transmit message through CAN.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->cli_handler->uart_device,
					    (uint8_t*)"             <msg> = message to be sent.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->cli_handler->uart_device,
					    (uint8_t*)" css <sid> - Set standard ID for the CAN messages sent.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->cli_handler->uart_device,
					    (uint8_t*)
					    "             <sid> = Standard ID in hexadecimal; between 0x000 and 0x3FF.\n");
		if(ret < 0)
			return ret;
		ret = usr_uart_write_string(dev->cli_handler->uart_device,
					    (uint8_t*)"             Example: css 300\n");
		if(ret < 0)
			return ret;
		return usr_uart_write_string(dev->cli_handler->uart_device,
					     (uint8_t*)" cg        - Get received messages if any.\n");
	}
}

/**
 * Display help options in the CLI.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0401_help(struct cn0401_dev *dev, uint8_t *arg)
{
	int32_t ret;

	ret = cn0401_help_prompt(dev, HELP_LONG_COMMAND);
	if(ret < 0)
		return ret;

	ret = cn0401_help_comm(dev, HELP_LONG_COMMAND);
	if(ret < 0)
		return ret;

	ret = usr_uart_write_string(dev->cli_handler->uart_device,
				    (uint8_t*)" can_test           - Run production test routine.\n");
	if(ret < 0)
		return ret;

	ret = cn0401_help_prompt(dev, HELP_SHORT_COMMAND);
	if(ret < 0)
		return ret;

	ret = cn0401_help_comm(dev, HELP_SHORT_COMMAND);
	if(ret < 0)
		return ret;

	return usr_uart_write_string(dev->cli_handler->uart_device,
				     (uint8_t*)" t         - Run production test routine.\n");
}

/**
 * Transmit a CAN message up to 64 characters.
 *
 * The method wakes up the CAN controller and transceiver and sends a CAN
 * message through the bus. After the message was acknowledged or after 5
 * seconds the application puts the controller and transceiver back to sleep and
 * exits, with an appropriate error message if the message was not acknowledged.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0401_can_send(struct cn0401_dev *dev, uint8_t *arg)
{
	union tx_obj_header header;
	enum can_ctrl_dlc msg_size;
	int32_t ret;
	bool scs;

	msg_size = can_ctrl_size_to_dlc(strlen((char *)arg));
	if(msg_size == CAN_DLC_TOO_SMALL)
		return usr_uart_write_string(dev->cli_handler->uart_device,
					     (uint8_t*)"Massage too large. Try splitting it.\n");

	header.word[0] = 0;
	header.word[1] = 0;

	header.hdr_str.sid = dev->can_ctrl_handler->tx_sid;
	header.hdr_str.brs = 1;
	header.hdr_str.fdf = 1;
	header.hdr_str.dlc = msg_size;
	header.hdr_str.seq = 1;

	ret = can_ctrl_wake_up(dev->can_ctrl_handler);
	if(ret != SUCCESS)
		return ret;

	ret = can_ctrl_message_transmit(dev->can_ctrl_handler,
					dev->can_ctrl_handler->tx_fifo_nr, arg, header);
	if(ret != SUCCESS)
		return ret;

	ret = can_ctrl_wait_tx(dev->can_ctrl_handler, &scs);
	if(ret != SUCCESS)
		return ret;

	if(!scs)
		usr_uart_write_string(dev->cli_handler->uart_device,
				      (uint8_t*)"Error! Message not received.\n");

	return can_ctrl_op_mode_req(dev->can_ctrl_handler, CAN_SLEEP_MODE, &scs);
}

/**
 * Set the standard ID that the messages sent use.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Pointer to the string that contains the ASCII
 *                   representation of the new SID in hexadecimal format.
 *                   (between 0x000 and 0x3FF)
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0401_can_set_tx_sid(struct cn0401_dev *dev, uint8_t *arg)
{
	uint16_t new_sid;

	new_sid = strtol((const char *)arg, NULL, 16);

	can_ctrl_set_tx_sid(dev->can_ctrl_handler, new_sid);

	return SUCCESS;
}

/**
 * Set the standard ID that the messages sent use.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Pointer to the string that contains the ASCII
 *                   representation of the new SID in hexadecimal format.
 *                   (between 0x000 and 0x3FF)
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0401_process_disp_rec(struct cn0401_dev *dev, uint8_t *msg)
{
	int32_t ret;

	ret = usr_uart_write_string(dev->cli_handler->uart_device,
				    (uint8_t *)"\nMessage received: \n");
	if(ret != SUCCESS)
		return ret;
	ret = usr_uart_write_string(dev->cli_handler->uart_device, msg);
	if(ret != SUCCESS)
		return ret;
	return usr_uart_write_string(dev->cli_handler->uart_device,
				     (uint8_t *)"\n\n>");
}

/**
 * The CAN receive routine.
 *
 * Called in the main process and as a command from the CLI.
 *
 * @param [in] dev - The device structure.
 * @param [in] rec - Pointer to a boolean variable: true if a message was
 *                   received; false if no message was received.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t cn0401_process_rec_routine(struct cn0401_dev *dev, bool *rec)
{
	uint8_t *can_rec_buff, rx_pl_size;
	int32_t ret;
	bool rec_dry;

	*rec = true;

	ret = can_ctrl_is_fifo_rdy(dev->can_ctrl_handler,
				   dev->can_ctrl_handler->rx_fifo_nr, &rec_dry);
	if((ret != SUCCESS) || (!rec_dry)) {
		*rec = false;
		return ret;
	}
	rx_pl_size = can_ctrl_message_size(dev->can_ctrl_handler,
					   dev->can_ctrl_handler->rx_fifo_plsize);
	can_rec_buff = calloc(rx_pl_size, sizeof *can_rec_buff);
	ret = can_ctrl_get_rec_message(dev->can_ctrl_handler,
				       dev->can_ctrl_handler->rx_fifo_nr, can_rec_buff);
	if(ret != SUCCESS)
		goto finish;
	ret = cn0401_process_disp_rec(dev, can_rec_buff);
	dev->rec_flag = false;
	NVIC_EnableIRQ(XINT_EVT0_IRQn);

finish:
	free(can_rec_buff);

	return ret;
}

/**
 * CLI command used to call the CAN receive routine.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0401_can_get_rec(struct cn0401_dev *dev, uint8_t *arg)
{
	bool rec;

	return cn0401_process_rec_routine(dev, &rec);
}

/**
 * Print the ASCII art 'PASS!!' message when a production test is passed.
 *
 * @param [in] dev - Pointer to the application structure.
 *
 * @return void
 */
static inline void cn0401_print_pass(struct cn0401_dev *dev)
{
	usr_uart_write_string(dev->cli_handler->uart_device,
			      (uint8_t *)"\n######     #     #####   #####  ### ###\n");
	usr_uart_write_string(dev->cli_handler->uart_device,
			      (uint8_t *)"#     #   # #   #     # #     # ### ###\n");
	usr_uart_write_string(dev->cli_handler->uart_device,
			      (uint8_t *)"#     #  #   #  #       #       ### ###\n");
	usr_uart_write_string(dev->cli_handler->uart_device,
			      (uint8_t *)"######  #     #  #####   #####   #   # \n");
	usr_uart_write_string(dev->cli_handler->uart_device,
			      (uint8_t *)"#       #######       #       # \n");
	usr_uart_write_string(dev->cli_handler->uart_device,
			      (uint8_t *)"#       #     # #     # #     # ### ### \n");
	usr_uart_write_string(dev->cli_handler->uart_device,
			      (uint8_t *)"#       #     #  #####   #####  ### ### \n\n\n");

}

/**
 * Print the ASCII art 'FAIL!!' message when a production test is failed.
 *
 * @param [in] dev - Pointer to the application structure.
 *
 * @return void
 */
static inline void cn0401_print_fail(struct cn0401_dev *dev)
{
	usr_uart_write_string(dev->cli_handler->uart_device,
			      (uint8_t *)"\n#######    #    ### #       ### ### \n");
	usr_uart_write_string(dev->cli_handler->uart_device,
			      (uint8_t *)"#         # #    #  #       ### ### \n");
	usr_uart_write_string(dev->cli_handler->uart_device,
			      (uint8_t *)"#        #   #   #  #       ### ###\n");
	usr_uart_write_string(dev->cli_handler->uart_device,
			      (uint8_t *)"#####   #     #  #  #        #   #  \n");
	usr_uart_write_string(dev->cli_handler->uart_device,
			      (uint8_t *)"#       #######  #  #               \n");
	usr_uart_write_string(dev->cli_handler->uart_device,
			      (uint8_t *)"#       #     #  #  #       ### ### \n");
	usr_uart_write_string(dev->cli_handler->uart_device,
			      (uint8_t *)"#       #     # ### ####### ### ### \n\n\n");
}

/**
 * Production test routine called in CLI.
 *
 * The production test routine first wakes up the CAN controller and the
 * transceiver and sets the controller in loopback mode. Then sends the test
 * data {11, 22, 33, 44, 55, 66, 77, 88}. The application then checks if the
 * message was received and if it is the correct message. All the conditions are
 * met the application flashes a platform LED 5 times then prints PASS!! in
 * ASCII art at the CLI. If the process fails no LED is blinked and the
 * application prints FAIL!!. Before the routine is exited, regardless of the
 * result, the CAN controller and transceiver are put back to sleep.
 *
 * @param [in] dev - The device structure.
 * @param [in] arg - Not used in this case. It exists to keep the function
 *                   prototype compatible with the other functions that can be
 *                   called from the CLI.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0401_can_test_proc(struct cn0401_dev *dev, uint8_t *arg)
{
	int32_t ret;
	union tx_obj_header header;
	uint8_t test_data[] = {11, 22, 33, 44, 55, 66, 77, 88};
	uint8_t control_data[] = {0, 0, 0, 0, 0, 0, 0, 0};
	bool rec_dry, pass = true, scs;
	uint32_t timeout = 1000, i;

	ret = can_ctrl_wake_up(dev->can_ctrl_handler);
	if(ret != SUCCESS)
		return ret;

	ret = can_ctrl_clear_wake_flag(dev->can_ctrl_handler);
	if(ret != SUCCESS)
		return ret;

	ret = can_ctrl_op_mode_req(dev->can_ctrl_handler, CAN_CONFIGURATION_MODE,
				   &scs);
	if(ret != SUCCESS)
		return ret;

	ret = can_ctrl_op_mode_req(dev->can_ctrl_handler, CAN_EXT_LOOPBACK_MODE,
				   &scs);
	if(ret != SUCCESS)
		return ret;
	if(!scs)
		goto finish;

	header.word[0] = 0;
	header.word[1] = 0;

	header.hdr_str.sid = dev->can_ctrl_handler->rx_sid;
	header.hdr_str.brs = 1;
	header.hdr_str.fdf = 1;
	header.hdr_str.dlc = 8;
	header.hdr_str.seq = 1;

	dev->rec_flag = false;

	ret = can_ctrl_message_transmit(dev->can_ctrl_handler,
					dev->can_ctrl_handler->tx_fifo_nr, test_data, header);
	if(ret != SUCCESS)
		return ret;

	NVIC_EnableIRQ(XINT_EVT0_IRQn);

	while((!dev->rec_flag) && (timeout != 0)) {
		mdelay(1);
		timeout--;
	}

	if(timeout == 0) {
		pass = false;
		cn0401_print_fail(dev);
		goto finish;
	}

	mdelay(1);
	ret = can_ctrl_is_fifo_rdy(dev->can_ctrl_handler,
				   dev->can_ctrl_handler->rx_fifo_nr, &rec_dry);
	if((ret != SUCCESS) || (!rec_dry))
		return ret;
	ret = can_ctrl_get_rec_message(dev->can_ctrl_handler,
				       dev->can_ctrl_handler->rx_fifo_nr, control_data);
	if(ret != SUCCESS)
		return ret;
	dev->rec_flag = false;

	for(i = 0; i < 8; i++)
		if(control_data[i] != test_data[i]) {
			pass = false;
			cn0401_print_fail(dev);
			goto finish;
		}

	for(i = 0; i < 5; i++) {
		ret = gpio_set_value(dev->aux_gpio, GPIO_LOW);
		if(ret != SUCCESS)
			return ret;
		mdelay(250);
		ret = gpio_set_value(dev->aux_gpio, GPIO_HIGH);
		if(ret != SUCCESS)
			return ret;
		mdelay(250);
	}
finish:
	ret = can_ctrl_op_mode_req(dev->can_ctrl_handler, CAN_CONFIGURATION_MODE,
				   &scs);
	if(ret != SUCCESS)
		return ret;

	ret = can_ctrl_op_mode_req(dev->can_ctrl_handler, CAN_SLEEP_MODE, &scs);
	if(ret != SUCCESS)
		return ret;

	if(pass)
		cn0401_print_pass(dev);

	NVIC_EnableIRQ(XINT_EVT0_IRQn);

	return ret;
}

/**
 * The CN0401 application main process.
 *
 * Runs the command line interpretor and receives and displays messages on the
 * CLI on wake up and receive interrupts.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0401_process(struct cn0401_dev *dev)
{
	int32_t ret;
	bool rec;

	ret = cli_process(dev->cli_handler);
	if(ret != SUCCESS)
		return ret;

	if((dev->state == CAN_SLEEP) && (dev->rec_flag)) {
		dev->state = CAN_FD_EN;
		ret = can_ctrl_clear_wake_flag(dev->can_ctrl_handler);
		if(ret != SUCCESS)
			return ret;
		ret = can_ctrl_wake_up(dev->can_ctrl_handler);
		if(ret != SUCCESS)
			return ret;
	}

	if(dev->state == CAN_FD_EN) {
		ret = cn0401_process_rec_routine(dev, &rec);
		if((ret != SUCCESS) && (!rec)) {
			NVIC_EnableIRQ(XINT_EVT0_IRQn);
			return ret;
		}
		ret = can_ctrl_op_mode_req(dev->can_ctrl_handler, CAN_SLEEP_MODE, &rec);
		if((ret != SUCCESS) && (!rec)) {
			NVIC_EnableIRQ(XINT_EVT0_IRQn);
			return ret;
		}
		dev->state = CAN_SLEEP;
		dev->rec_flag = false;
		NVIC_EnableIRQ(XINT_EVT0_IRQn);
	}

	return ret;
}

/**
 * The application initialization routine.
 *
 * @param [out] device    - Pointer to the application handler structure.
 * @param [in] init_param - Pointer to the application initialization structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0401_setup(struct cn0401_dev **device,
		     struct cn0401_init_param *init_param)
{
	int32_t ret;
	struct cn0401_dev *dev;
	uint8_t app_name[] = {'c', 'n', '0', '4', '0', '1'};
	struct gpio_desc *xint_gpio;

	dev = calloc(1, sizeof *dev);
	if(!dev)
		return -1;

	dev->rec_flag = false;
	dev->state = CAN_IDLE;

	ret = timer_start();
	if(ret != 0)
		goto error;

	ret = can_ctrl_setup(&dev->can_ctrl_handler, &init_param->can_ctrl_init);
	if(ret != SUCCESS)
		goto error;

	ret = cli_setup(&dev->cli_handler, &init_param->cli_init);
	if(ret != SUCCESS)
		goto error;
	cli_load_command_vector(dev->cli_handler, fnc_ptr);
	cli_load_command_calls(dev->cli_handler, (uint8_t **)fnc_calls);
	cli_load_command_sizes(dev->cli_handler, fnc_call_size);
	cli_load_descriptor_pointer(dev->cli_handler, dev);
	ret = cli_cmd_prompt(dev->cli_handler, app_name);
	if(ret != SUCCESS)
		goto error;

	ret = gpio_get(&xint_gpio, 15);
	if(ret != SUCCESS)
		goto error;
	ret = gpio_direction_input(xint_gpio);
	if(ret != SUCCESS)
		goto error;
	ret = gpio_set_pull_up(xint_gpio, false);
	if(ret != SUCCESS)
		goto error;
	ret = gpio_remove(xint_gpio);
	if(ret != SUCCESS)
		goto error;
	ret = intr_setup(&dev->intr_handler);
	if(ret != SUCCESS)
		goto error;
	ret = intr_register_callback(ADI_XINT_EVENT_INT0, can_rec_callback, dev);
	if(ret != SUCCESS)
		goto error;
	ret = intr_enable_irq(ADI_XINT_EVENT_INT0, ADI_XINT_IRQ_LOW_LEVEL);
	if(ret != SUCCESS)
		goto error;

	ret = gpio_get(&dev->aux_gpio, init_param->aux_gpio);
	if(ret != SUCCESS)
		goto error;
	ret = gpio_direction_output(dev->aux_gpio, GPIO_HIGH);
	if(ret != SUCCESS)
		goto error;
	ret = gpio_get(&dev->silent_gpio, init_param->silent_gpio);
	if(ret != SUCCESS)
		goto error;
	ret = gpio_direction_output(dev->silent_gpio, GPIO_LOW);
	if(ret != SUCCESS)
		goto error;

	dev->state = CAN_SLEEP;

	*device = dev;

	return ret;
error:
	free(dev);

	return ret;
}

/**
 * Free memory allocated by the cn0401_setup() function.
 *
 * @param [in] dev - The device structure.
 *
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0401_remove(struct cn0401_dev *dev)
{
	int32_t ret;

	if(!dev)
		return -1;

	ret = can_ctrl_remove(dev->can_ctrl_handler);
	if(ret != SUCCESS)
		return ret;

	ret = intr_remove(dev->intr_handler);
	if(ret != 0)
		return ret;

	ret = gpio_remove(dev->aux_gpio);
	if(ret != 0)
		return ret;

	return cli_remove(dev->cli_handler);
}

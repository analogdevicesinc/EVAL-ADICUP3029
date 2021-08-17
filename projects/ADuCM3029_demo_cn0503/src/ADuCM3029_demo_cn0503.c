/*****************************************************************************
 * ADuCM3029_demo_cn0503.c
 *****************************************************************************/

#include <sys/platform.h>
#include <stdint.h>
#include "cn0503.h"
#include "adi_initialize.h"
#include "error.h"
#include "uart_extra.h"
#ifdef IIO_SUPPORT
#include "iio_adpd410x.h"
#include "iio_app.h"
#include "util.h"
#endif

int main(int argc, char *argv[])
{
	int32_t ret;
	struct cn0503_dev *cn0503_dut;
	struct cn0503_init_param cn0503_initial;
	struct aducm_uart_init_param aducm_uart_ini;
	/**
	 * Initialize managed drivers and/or services that have been added to
	 * the project.
	 * @return zero on success
	 */
	adi_initComponents();

	cn0503_initial.cli_param.uart_init.extra = &aducm_uart_ini;

	cn0503_get_config(&cn0503_initial);

	ret = cn0503_init(&cn0503_dut, &cn0503_initial);
	if(ret != SUCCESS)
		return FAILURE;

#ifdef IIO_SUPPORT
	irq_ctrl_remove(cn0503_dut->irq_handler);
	uart_remove(cn0503_dut->cli_handler->uart_device);

	struct iio_app_device devices[] = {
		IIO_APP_DEVICE("adpd410x", cn0503_dut->adpd4100_handler,
			       &adpd410x_iio_descriptor,
			       NULL, NULL),
	};

	return iio_app_run(devices, ARRAY_SIZE(devices));
#else
	ret = cn0503_prompt(cn0503_dut);
	if(ret != SUCCESS)
		return FAILURE;

	while(1) {
		ret = cn0503_process(cn0503_dut);
		if(ret != SUCCESS)
			return FAILURE;
	}
#endif

	return SUCCESS;
}

#ifndef AD7799_IIO_H
#define AD7799_IIO_H

#include "iio_types.h"

ssize_t ad7799_iio_reg_read(void *device, char *buf, size_t len,
		const struct iio_ch_info *channel);
/** Store function pointer */
ssize_t ad7799_iio_reg_write(void *device, char *buf, size_t len,
		 const struct iio_ch_info *channel);
ssize_t ad7799_iio_channel_read(void *device, char *buf, size_t len,
		const struct iio_ch_info *channel);
ssize_t ad7799_iio_gain_read(void *device, char *buf, size_t len,
		const struct iio_ch_info *channel);
ssize_t ad7799_iio_gain_write(void *device, char *buf, size_t len,
		 const struct iio_ch_info *channel);
ssize_t ad7799_iio_read_channel(void *dev_instance, char *pbuf, size_t offset,
			     size_t bytes_count, uint32_t ch_mask);
ssize_t ad7799_iio_write_channel(void *dev_instance, char *buf, size_t offset,
			     size_t bytes_count, uint32_t ch_mask);


static struct iio_attribute ad7799_iio_channel_attr = {
	.name = "volts",
	.show = ad7799_iio_channel_read,
	.store = NULL
};

static struct iio_attribute *ad7799_iio_channel_attributes[] = {
	&ad7799_iio_channel_attr,
	NULL,
};


static struct iio_channel ad779_iio_channel_1 = {
	.name = "channel0",
	.scan_index = 0,
	.scan_type = {
		.sign = 'u',
		.realbits = 12,
		.storagebits = 24,
		.shift = 0,
		.is_big_endian = false
	},
	.attributes = ad7799_iio_channel_attributes,
	.ch_out = false,
};

static struct iio_channel ad779_iio_channel_2 = {
	.name = "channel1",
	.scan_index = 1,
	.scan_type = {
			.sign = 'u',
			.realbits = 12,
			.storagebits = 24,
			.shift = 0,
			.is_big_endian = false
	},
	.attributes = ad7799_iio_channel_attributes,
	.ch_out = false,
};

static struct iio_channel ad779_iio_channel_3 = {
	.name = "channel2",
	.scan_index = 2,
	.scan_type = {
			.sign = 'u',
			.realbits = 12,
			.storagebits = 24,
			.shift = 0,
			.is_big_endian = false
	},
	.attributes = ad7799_iio_channel_attributes,
	.ch_out = false,
};

static struct iio_channel *ad7799_iio_channels[] = {
	&ad779_iio_channel_1,
	&ad779_iio_channel_2,
	&ad779_iio_channel_3,
	NULL,
};

static struct iio_attribute ad7799_iio_reg_attr = {
	.name = "direct_reg_access",
	.show = ad7799_iio_reg_read,
	.store = ad7799_iio_reg_write,
};

static struct iio_attribute *ad7799_iio_debug_attributes[] = {
	&ad7799_iio_reg_attr,
	NULL,
};


static struct iio_attribute ad7799_iio_gain_attr = {
	.name = "gain",
	.show = ad7799_iio_gain_read,
	.store = ad7799_iio_gain_write,
};

static struct iio_attribute *ad7799_iio_attributes[] = {
	&ad7799_iio_gain_attr,
	NULL,
};
//extern struct iio_device ad7799_iio_descriptor ;
struct iio_device ad7799_iio_descriptor = {
		.num_ch = 3,
		.channels = ad7799_iio_channels,
		.attributes = ad7799_iio_attributes,
		.debug_attributes = ad7799_iio_debug_attributes,
		.buffer_attributes = NULL,
		.transfer_dev_to_mem = NULL,
		.transfer_mem_to_dev = NULL,
		.read_data = ad7799_iio_read_channel,
		.write_data = ad7799_iio_write_channel
};

#endif

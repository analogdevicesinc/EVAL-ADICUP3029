
#include "iio_types.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ad7799.h"
//#include "ad7799_iio.h"
#include "util.h"
#include "error.h"

#define NUM_CHANNELS 3

ssize_t ad7799_iio_reg_read(void *device, char *buf, size_t len,
		const struct iio_ch_info *channel)
{
	struct ad7799_dev *dev = (struct ad7799_dev *)device;

}
/** Store function pointer */
ssize_t ad7799_iio_reg_write(void *device, char *buf, size_t len,
		 const struct iio_ch_info *channel)
{
	struct ad7799_dev *dev = (struct ad7799_dev *)device;

	return len;
}

ssize_t ad7799_iio_channel_read(void *device, char *buf, size_t len,
		const struct iio_ch_info *channel)
{
	struct ad7799_dev *dev = (struct ad7799_dev *)device;
	uint32_t data;
	float ch;

	ad7799_get_channel(dev, channel->ch_num, &data);

	ch = (data * 4.096) / (0xFFFF);

	return snprintf(buf, len, "%.5f", ch);
}


ssize_t ad7799_iio_gain_read(void *device, char *buf, size_t len,
		const struct iio_ch_info *channel)
{
	struct ad7799_dev *dev = (struct ad7799_dev *)device;
	uint8_t gain;

	ad7799_get_gain(dev, &gain);

	return snprintf(buf, len, "%d", gain);
}

/** Store function pointer */
ssize_t ad7799_iio_gain_write(void *device, char *buf, size_t len,
		 const struct iio_ch_info *channel)
{
	struct ad7799_dev *dev = (struct ad7799_dev *)device;

	uint32_t gain = srt_to_uint32(buf);

	ad7799_set_gain(dev, (uint8_t)gain);

	return len;
}

ssize_t ad7799_iio_read_channel(void *dev_instance, char *pbuf, size_t offset,
			     size_t bytes_count, uint32_t ch_mask)
{
	struct ad7799_dev *dev = (struct ad7799_dev *)dev_instance;

	uint32_t i, j = 0, current_ch = 0;
	uint16_t *pbuf16;
	size_t samples;
	uint32_t data;

	if (!dev_instance)
		return FAILURE;

	if (!pbuf)
		return FAILURE;

	pbuf16 = (uint16_t*)pbuf;
	samples = (bytes_count * NUM_CHANNELS) / hweight8(
			  ch_mask);
	samples /= 2; /* because of uint16_t *pbuf16 = (uint16_t*)pbuf; */
	offset = (offset * NUM_CHANNELS) / hweight8(ch_mask);

	for (i = 0; i < samples; i++) {

		if (ch_mask & BIT(current_ch)) {
			ad7799_get_channel(dev, current_ch, &data);
			pbuf16[j] = data;//*(uint16_t*)(demo_device->ddr_base_addr +
						// (offset + i * 2) % demo_device->ddr_base_size);

			j++;
		}

		if (current_ch + 1 < NUM_CHANNELS)
			current_ch++;
		else
			current_ch = 0;
	}

	return bytes_count;
}

ssize_t ad7799_iio_write_channel(void *dev_instance, char *buf,
			   size_t offset,  size_t bytes_count, uint32_t ch_mask)
{
	struct ad7799_dev *dev = (struct ad7799_dev *)dev_instance;

	uint32_t index, addr;
	uint16_t *buf16;

	if (!dev_instance)
		return FAILURE;

	if (!buf)
		return FAILURE;

	buf16 = (uint16_t *)buf;

	//addr = demo_device->ddr_base_addr + offset;
//	for(index = 0; index < bytes_count; index += 2) {
//		uint32_t *local_addr = (uint32_t *)(addr +
//						    (index * 2) % demo_device->ddr_base_size);
//		*local_addr = (buf16[index + 1] << 16) | buf16[index];
//	}


	return bytes_count;
}

/*
 *
	ret = ad7799_read_channel(ad7799_device, AD7799_CH_AIN1P_AIN1M, &data_ch);

	ch = (data_ch * 4.096) / (0xFFFF);
	sprintf(ascii_buff, "%f", ch);

	ret = ad7799_read_channel(ad7799_device, AD7799_CH_AIN2P_AIN2M, &data_ch);

	ch = (data_ch * 4.096) / (0xFFFF);

	sprintf(ascii_buff, "%f", ch);

	ret = ad7799_read_channel(ad7799_device, AD7799_CH_AIN3P_AIN3M, &data_ch);

	ch = (data_ch * 4.096) / (0xFFFF);

	sprintf(ascii_buff, "%f", ch);
 */

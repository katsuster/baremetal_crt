/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/driver/gpio.h>
#include <bmetal/device.h>
#include <bmetal/file.h>
#include <bmetal/printk.h>
#include <bmetal/sys/stdio.h>
#include <bmetal/sys/string.h>
#include <bmetal/sys/ioctl.h>

ssize_t k_gpio_line_read(struct k_file_desc *desc, void *buf, size_t count)
{
	return 0;
}

ssize_t k_gpio_line_write(struct k_file_desc *desc, const void *buf, size_t count)
{
	return 0;
}

int k_gpio_line_ioctl(struct k_file_desc *desc, unsigned int cmd, unsigned long arg)
{
	switch (cmd) {
		case GPIO_V2_LINE_GET_VALUES_IOCTL:
			break;
		case GPIO_V2_LINE_SET_VALUES_IOCTL:
			break;
		default:
			break;
	}

	return 0;
}

int k_gpio_line_poll(struct k_file_desc *desc, struct k_poll_info *p)
{
	return 0;
}

int k_gpio_line_close(struct k_file_desc *desc)
{
	return 0;
}

//TODO: change to static, and register this ops to file system
const struct k_file_ops gpio_line_ops = {
	.read = k_gpio_line_read,
	.write = k_gpio_line_write,
	.ioctl = k_gpio_line_ioctl,
	.poll = k_gpio_line_poll,
	.close = k_gpio_line_close,
};

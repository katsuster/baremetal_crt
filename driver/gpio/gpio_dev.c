/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/driver/gpio.h>
#include <bmetal/device.h>
#include <bmetal/file.h>
#include <bmetal/printk.h>
#include <bmetal/sys/stdio.h>
#include <bmetal/sys/string.h>
#include <bmetal/sys/ioctl.h>

//TODO: tentative: can manage multiple gpio device
extern struct k_gpio_device *k_gpio_system;

int k_gpio_line_get_values(struct k_gpio_device *gpio, unsigned long arg)
{
	const struct k_gpio_driver *drv = k_gpio_get_drv(gpio);
	struct gpio_v2_line_values *p = (void *)(uintptr_t)arg;
	struct gpio_v2_line_values lineval;
	uint64_t m = 1;
	int ret = 0;

	if (!drv || !drv->ops || !drv->ops->get) {
		return -EINVAL;
	}
	if (!p) {
		return -EINVAL;
	}

	k_memset(&lineval, 0, sizeof(lineval));
	lineval.mask = p->mask;

	//TODO: can select GPIO lines
	for (int i = 0; i < 32; i++, m <<= 1) {
		int r;

		if ((lineval.mask & m) == 0) {
			continue;
		}

		r = drv->ops->get(gpio, i);
		if (r == -1) {
			ret = -EBADF;
		} else if (r == 1) {
			lineval.bits |= m;
		}
	}

	if (p) {
		k_memcpy(p, &lineval, sizeof(lineval));
	}

	return ret;
}

int k_gpio_line_set_values(struct k_gpio_device *gpio, unsigned long arg)
{
	const struct k_gpio_driver *drv = k_gpio_get_drv(gpio);
	const struct gpio_v2_line_values *p = (void *)(uintptr_t)arg;
	struct gpio_v2_line_values lineval;
	uint64_t m = 1;
	int ret = 0;

	if (!drv || !drv->ops || !drv->ops->set) {
		return -EINVAL;
	}
	if (!p) {
		return -EINVAL;
	}

	k_memcpy(&lineval, p, sizeof(lineval));

	//TODO: can select GPIO lines
	for (int i = 0; i < 32; i++, m <<= 1) {
		int r, v;

		if ((lineval.mask & m) == 0) {
			continue;
		}

		if ((lineval.bits & m) == 0) {
			v = 0;
		} else {
			v = 1;
		}

		r = drv->ops->set(gpio, i, v);
		if (r == -1) {
			ret = -EBADF;
		}
	}

	return ret;
}

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
	struct k_gpio_device *gpio;
	int ret = -ENOTTY;

	//TODO: tentative: can manage multiple gpio device
	if (!k_gpio_system) {
		k_pri_err("GPIO device is not found.\n");
		return -EBADF;
	}
	gpio = k_gpio_system;

	switch (cmd) {
	case GPIO_V2_LINE_GET_VALUES_IOCTL:
		ret = k_gpio_line_get_values(gpio, arg);
		break;
	case GPIO_V2_LINE_SET_VALUES_IOCTL:
		ret = k_gpio_line_set_values(gpio, arg);
		break;
	default:
		break;
	}

	return ret;
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

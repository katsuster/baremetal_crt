/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/driver/gpio.h>
#include <bmetal/device.h>
#include <bmetal/printk.h>
#include <bmetal/sys/stdio.h>
#include <bmetal/sys/string.h>

//TODO: tentative: can manage multiple gpio device
struct k_gpio_device *k_gpio_system = NULL;

int k_gpio_get_config(struct k_gpio_device *gpio, struct k_gpio_config *conf)
{
	const struct k_gpio_driver *drv = k_gpio_get_drv(gpio);
	int r;

	if (!gpio) {
		return -EINVAL;
	}

	if (drv && drv->ops && drv->ops->get_config) {
		r = drv->ops->get_config(gpio, conf);
		if (r) {
			k_dev_err(k_gpio_to_dev(gpio), "failed to get GPIO config.\n");
			return r;
		}
	}

	return 0;
}

int k_gpio_set_config(struct k_gpio_device *gpio, const struct k_gpio_config *conf)
{
	const struct k_gpio_driver *drv = k_gpio_get_drv(gpio);
	int r;

	if (!gpio) {
		return -EINVAL;
	}

	if (drv && drv->ops && drv->ops->set_config) {
		r = drv->ops->set_config(gpio, conf);
		if (r) {
			k_dev_err(k_gpio_to_dev(gpio), "failed to set GPIO config.\n");
			return r;
		}
	}

	return 0;
}

int k_gpio_read_default_config(struct k_gpio_device *gpio, struct k_gpio_config *conf)
{
	//struct k_device *dev = k_gpio_to_dev(gpio);

	k_memset(conf, 0, sizeof(*conf));

	return 0;
}

int k_gpio_add_device(struct k_gpio_device *gpio, struct k_bus *parent)
{
	int r;

	r = k_device_add(k_gpio_to_dev(gpio), parent);
	if (IS_ERROR(r)) {
		return r;
	}

	//TODO: can manage multiple gpio device
	if (!k_gpio_system) {
		k_gpio_system = gpio;
	}

	return 0;
}

int k_gpio_remove_device(struct k_gpio_device *gpio)
{
	return k_device_remove(k_gpio_to_dev(gpio));
}

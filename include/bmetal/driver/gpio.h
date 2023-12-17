/* SPDX-License-Identifier: Apache-2.0 */

#ifndef BAREMETAL_CRT_DRIVERS_GPIO_H_
#define BAREMETAL_CRT_DRIVERS_GPIO_H_

#include <stddef.h>
#include <stdint.h>

#include <bmetal/device.h>
#include <bmetal/sys/ioctl.h>

/* Same as Linux ioctl of GPIO */
struct gpio_v2_line_values {
	uint64_t bits;
	uint64_t mask;
};

#define GPIO_V2_LINE_GET_VALUES_IOCTL _IOWR(0xB4, 0x0E, struct gpio_v2_line_values)
#define GPIO_V2_LINE_SET_VALUES_IOCTL _IOWR(0xB4, 0x0F, struct gpio_v2_line_values)

struct k_gpio_config {

};

struct k_gpio_device;

struct k_gpio_driver_ops {
	int (*get)(struct k_gpio_device *gpio, int ch);
	int (*set)(struct k_gpio_device *gpio, int ch, int value);
	int (*get_config)(struct k_gpio_device *gpio, struct k_gpio_config *conf);
	int (*set_config)(struct k_gpio_device *gpio, const struct k_gpio_config *conf);
};

struct k_gpio_driver {
	struct k_device_driver base;

	const struct k_gpio_driver_ops *ops;
};

struct k_gpio_device {
	struct k_device base;

	int as_default;
};

struct k_gpio_priv_max {
	char dummy[88];
};
typedef struct k_gpio_priv_max    k_gpio_priv_t;
#define CHECK_PRIV_SIZE_GPIO(typ)    CHECK_PRIV_SIZE(typ, k_gpio_priv_t);

static inline const struct k_gpio_driver *k_gpio_get_drv(const struct k_gpio_device *gpio)
{
	if (!gpio) {
		return NULL;
	}

	return (const struct k_gpio_driver *)gpio->base.drv;
}

static inline struct k_device *k_gpio_to_dev(struct k_gpio_device *gpio)
{
	if (!gpio) {
		return NULL;
	}

	return &gpio->base;
}

static inline struct k_gpio_device *k_gpio_from_dev(struct k_device *dev)
{
	return (struct k_gpio_device *)dev;
}

static inline int k_gpio_add_driver(struct k_gpio_driver *drv)
{
	return k_driver_add(&drv->base.base);
}

static inline int k_gpio_remove_driver(struct k_gpio_driver *drv)
{
	return k_driver_remove(&drv->base.base);
}

#ifdef CONFIG_GPIO

int k_gpio_get_config(struct k_gpio_device *gpio, struct k_gpio_config *conf);
int k_gpio_set_config(struct k_gpio_device *gpio, const struct k_gpio_config *conf);
int k_gpio_set_default_console(struct k_gpio_device *gpio);
int k_gpio_read_default_config(struct k_gpio_device *gpio, struct k_gpio_config *conf);

int k_gpio_add_device(struct k_gpio_device *gpio, struct k_bus *parent);
int k_gpio_remove_device(struct k_gpio_device *gpio);

#else /* CONFIG_GPIO */

static inline int k_gpio_get_config(struct k_gpio_device *gpio, struct k_gpio_config *conf)
{
	return -ENOTSUP;
}

static inline int k_gpio_set_config(struct k_gpio_device *gpio, const struct k_gpio_config *conf)
{
	return -ENOTSUP;
}

static inline int k_gpio_set_default_console(struct k_gpio_device *gpio)
{
	return -ENOTSUP;
}

static inline int k_gpio_read_default_config(struct k_gpio_device *gpio, struct k_gpio_config *conf)
{
	return -ENOTSUP;
}


static inline int k_gpio_add_device(struct k_gpio_device *gpio, struct k_bus *parent)
{
	return -ENOTSUP;
}

static inline int k_gpio_remove_device(struct k_gpio_device *gpio)
{
	return -ENOTSUP;
}

#endif /* CONFIG_GPIO */

#endif /* BAREMETAL_CRT_DRIVERS_GPIO_H_ */

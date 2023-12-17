/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/driver/gpio.h>
#include <bmetal/init.h>
#include <bmetal/io.h>
#include <bmetal/printk.h>
#include <bmetal/driver/intc.h>
#include <bmetal/sys/errno.h>
#include <bmetal/sys/inttypes.h>

#define REG_DATA      0x000
#define REG_TRI       0x004
#define REG_GIER      0x11c
#define REG_IP_ISR    0x120
#define REG_IP_IER    0x128

#define OFFSET_CH     0x008

struct gpio_v2_priv {
	struct k_gpio_device *gpio;
	struct k_intc_device *intc;
	struct k_event_handler hnd_irq;
	int num_irq;
};
CHECK_PRIV_SIZE_GPIO(struct gpio_v2_priv);

static int gpio_v2_get(struct k_gpio_device *gpio, int ch)
{
	struct k_device *dev = k_gpio_to_dev(gpio);
	int ch_dev = ch / 32;
	int l = ch % 32;
	uintptr_t reg = REG_DATA + OFFSET_CH * ch_dev;
	uint32_t v;

	v = k_device_read32(dev, reg);

	return !!(v & BIT(l));
}

static int gpio_v2_set(struct k_gpio_device *gpio, int ch, int value)
{
	struct k_device *dev = k_gpio_to_dev(gpio);
	int ch_dev = ch / 32;
	int l = ch % 32;
	uintptr_t reg = REG_DATA + OFFSET_CH * ch_dev;
	uint32_t v;

	v = k_device_read32(dev, reg);
	if (value) {
		v |= BIT(l);
	} else {
		v &= ~BIT(l);
	}
	k_device_write32(dev, v, reg);

	return 0;
}

static int gpio_v2_enable_intr(struct k_gpio_device *gpio, int ch)
{
	struct k_device *dev = k_gpio_to_dev(gpio);
	int ch_dev = ch / 32;
	uint32_t v;

	v = k_device_read32(dev, REG_IP_IER);
	v |= BIT(ch_dev);
	k_device_write32(dev, v, REG_IP_IER);

	return 0;
}

static int gpio_v2_disable_intr(struct k_gpio_device *gpio, int ch)
{
	struct k_device *dev = k_gpio_to_dev(gpio);
	int ch_dev = ch / 32;
	uint32_t v;

	v = k_device_read32(dev, REG_IP_IER);
	v &= ~BIT(ch_dev);
	k_device_write32(dev, v, REG_IP_IER);

	return 0;
}

static int gpio_v2_intr(int event, struct k_event_handler *hnd)
{
	//struct gpio_v2_priv *priv = hnd->priv;
	//struct k_device *dev = k_gpio_to_dev(priv->gpio);

	//Clear interrupt
	//k_device_read32(dev, REG_STAT);

	return EVENT_HANDLED;
}

static int gpio_v2_add(struct k_device *dev)
{
	struct k_gpio_device *gpio = k_gpio_from_dev(dev);
	struct gpio_v2_priv *priv = dev->priv;
	int r;

	if (priv == NULL) {
		k_dev_err(dev, "priv is NULL\n");
		return -EINVAL;
	}

	priv->gpio = gpio;

	/* Register */
	r = k_io_mmap_device(NULL, dev);
	if (r) {
		return r;
	}

	/* Interrupt */
	r = k_intc_get_intc_from_config(dev, 0, &priv->intc, &priv->num_irq);
	if (r) {
		k_dev_warn(dev, "intc is not found, use polling.\n");
		priv->intc = NULL;
	}

	if (priv->intc) {
		priv->hnd_irq.func = gpio_v2_intr;
		priv->hnd_irq.priv = priv;

		r = k_intc_add_handler(priv->intc, priv->num_irq, &priv->hnd_irq);
		if (r) {
			return r;
		}

		//TODO: ch0 only
		r = gpio_v2_enable_intr(gpio, 0);
		if (r) {
			return r;
		}
	}

	return 0;
}

static int gpio_v2_remove(struct k_device *dev)
{
	struct k_gpio_device *gpio = k_gpio_from_dev(dev);
	struct gpio_v2_priv *priv = dev->priv;
	int r;

	if (priv->intc) {
		//TODO: ch0 only
		r = gpio_v2_disable_intr(gpio, 0);
		if (r) {
			return r;
		}

		r = k_intc_remove_handler(priv->intc, priv->num_irq, &priv->hnd_irq);
		if (r) {
			return r;
		}

		priv->hnd_irq.func = NULL;
		priv->hnd_irq.priv = NULL;

		priv->num_irq = 0;
		priv->intc = NULL;
	}

	/* TODO: to be implemented */
	return -ENOTSUP;
}

const static struct k_device_driver_ops v2_dev_ops = {
	.add = gpio_v2_add,
	.remove = gpio_v2_remove,
	.mmap = k_device_driver_mmap,
};

const static struct k_gpio_driver_ops v2_gpio_ops = {
	.get = gpio_v2_get,
	.set = gpio_v2_set,
};

static struct k_gpio_driver v2_drv = {
	.base = {
		.base = {
			.type_vendor = "xilinx",
			.type_device = "gpio_v2",
		},

		.ops = &v2_dev_ops,
	},

	.ops = &v2_gpio_ops,
};

static int gpio_v2_init(void)
{
	k_gpio_add_driver(&v2_drv);

	return 0;
}
define_init_func(gpio_v2_init);

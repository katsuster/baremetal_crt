/* SPDX-License-Identifier: Apache-2.0 */

#include <bmetal/driver/clk.h>
#include <bmetal/init.h>
#include <bmetal/device.h>
#include <bmetal/printk.h>
#include <bmetal/binding/clk/sifive/prci.h>
#include <bmetal/sys/errno.h>
#include <bmetal/sys/inttypes.h>

#define PRCI_HFXOSCCFG         0x00
#define PRCI_COREPLLCFG0       0x04
#define PRCI_DDRPLLCFG0        0x0c
#define PRCI_DDRPLLCFG1        0x10
#define PRCI_GEMGXLPLLCFG0     0x1c
#define PRCI_GEMGXLPLLCFG1     0x20
#define PRCI_CORECLKSEL        0x24
#define PRCI_DEVICERESETREG    0x28

#define PRCI_PLLCFG_DIVR_SHIFT     0
#define PRCI_PLLCFG_DIVR_MASK      (0x3f << PRCI_PLLCFG_DIVR_MASK)
#define PRCI_PLLCFG_DIVF_SHIFT     6
#define PRCI_PLLCFG_DIVF_MASK      (0x1ff << PRCI_PLLCFG_DIVF_MASK)
#define PRCI_PLLCFG_DIVQ_SHIFT     15
#define PRCI_PLLCFG_DIVQ_MASK      (0x7 << PRCI_PLLCFG_DIVQ_MASK)
#define PRCI_PLLCFG_RANGE_SHIFT    18
#define PRCI_PLLCFG_RANGE_MASK     (0x7 << PRCI_PLLCFG_RANGE_MASK)
#define PRCI_PLLCFG_FSE            BIT(25)
#define PRCI_PLLCFG_LOCK           BIT(31)

#define PRCI_PLLCFG_DIVR_MAX    64
#define PRCI_PLLCFG_DIVF_MAX    512
#define PRCI_PLLCFG_DIVQ_MAX    7

#define PRCI_PLLCFG1_CKE        BIT(24)

#define PRCI_CLKSEL_BYPASS      BIT(0)

/* For divr: frequency of reference */
#define PRCI_REF_MIN          (7 * MHZ)
#define PRCI_REF_MAX          PRCI_REF_RANGE7
#define PRCI_REF_RANGE1       (11 * MHZ)
#define PRCI_REF_RANGE2       (18 * MHZ)
#define PRCI_REF_RANGE3       (30 * MHZ)
#define PRCI_REF_RANGE4       (50 * MHZ)
#define PRCI_REF_RANGE5       (80 * MHZ)
#define PRCI_REF_RANGE6       (130 * MHZ)
#define PRCI_REF_RANGE7       (200 * MHZ)

/* For divf: frequency of VCO */
#define PRCI_VCO_MIN    (2400ULL * MHZ)
#define PRCI_VCO_MAX    (4800ULL * MHZ)

struct clk_prci_priv {
	struct k_clk_device *clk_in;
	int index_clk_in;
	uint64_t freq_in;
	uint64_t freq_ddrctrl;
	uint64_t freq_core;
	uint64_t freq_gemgxl;
};
CHECK_PRIV_SIZE_CLK(struct clk_prci_priv);

struct clk_prci_pllcfg {
	uint64_t target;

	int divr;
	uint64_t post_divr;
	int divf;
	uint64_t post_divf;
	int divq;

	uint64_t result;
	int divr_found;
	uint64_t post_divr_found;
	int divf_found;
	int divq_found;
	int range;
	int found;
};

struct clk_prci_pllcfg_off {
	int pllcfg0;
	int pllcfg1;
};

struct clk_prci_clksel_off {
	int clksel;
};

const static struct clk_prci_pllcfg_off pllcfg_offs[PRCI_INDEX_MAX] = {
	[PRCI_INDEX_DDRCTRLCLK] = {PRCI_DDRPLLCFG0, PRCI_DDRPLLCFG1},
	[PRCI_INDEX_TLCLK]      = {-1, -1},
	[PRCI_INDEX_CORECLK]    = {PRCI_COREPLLCFG0, -1},
	[PRCI_INDEX_GEMGXLCLK]  = {PRCI_GEMGXLPLLCFG0, PRCI_GEMGXLPLLCFG1},
};

const static struct clk_prci_clksel_off clksel_offs[PRCI_INDEX_MAX] = {
	[PRCI_INDEX_DDRCTRLCLK] = {-1},
	[PRCI_INDEX_TLCLK]      = {-1},
	[PRCI_INDEX_CORECLK]    = {PRCI_CORECLKSEL},
	[PRCI_INDEX_GEMGXLCLK]  = {-1},
};

static int clk_prci_enable(struct k_clk_device *clk, int index);
static int clk_prci_disable(struct k_clk_device *clk, int index);
static int clk_prci_get_freq(struct k_clk_device *clk, int index, uint64_t *freq);
static int clk_prci_set_freq(struct k_clk_device *clk, int index, uint64_t freq);

static int clk_prci_filter_range(struct k_clk_device *clk, uint64_t post_divr, int *range)
{
	struct k_device *dev = k_clk_to_dev(clk);
	int r;

	switch (post_divr) {
	case PRCI_REF_MIN ... PRCI_REF_RANGE1 - 1:
		r = 1;
		break;
	case PRCI_REF_RANGE1 ... PRCI_REF_RANGE2 - 1:
		r = 2;
		break;
	case PRCI_REF_RANGE2 ... PRCI_REF_RANGE3 - 1:
		r = 3;
		break;
	case PRCI_REF_RANGE3 ... PRCI_REF_RANGE4 - 1:
		r = 4;
		break;
	case PRCI_REF_RANGE4 ... PRCI_REF_RANGE5 - 1:
		r = 5;
		break;
	case PRCI_REF_RANGE5 ... PRCI_REF_RANGE6 - 1:
		r = 6;
		break;
	case PRCI_REF_RANGE6 ... PRCI_REF_RANGE7 - 1:
		r = 7;
		break;
	default:
		k_dev_err(dev, "post divr frequency out of range: %"PRId64"\n",
			post_divr);
		return -ERANGE;
	}
	if (range) {
		*range = r;
	}

	return 0;
}

static int clk_prci_find_divq(struct k_clk_device *clk, struct clk_prci_pllcfg *conf)
{
	uint64_t post_divq;
	int64_t diff, diff_found;

	for (int i = 0; i < PRCI_PLLCFG_DIVQ_MAX; i++) {
		post_divq = conf->post_divf >> i;
		diff = (int64_t)(conf->target - post_divq);
		diff_found = (int64_t)(conf->target - conf->result);
		if (NABS(diff) >= NABS(diff_found)) {
			continue;
		}

		conf->result = post_divq;
		conf->divr_found = conf->divr;
		conf->post_divr_found = conf->post_divr;
		conf->divf_found = conf->divf;
		conf->divq_found = i;
		conf->found = 1;
	}
	if (conf->found) {
		return 0;
	}

	return -EINVAL;
}

static int clk_prci_find_divf(struct k_clk_device *clk, struct clk_prci_pllcfg *conf)
{
	uint64_t post_divf;

	for (int i = 0; i < PRCI_PLLCFG_DIVF_MAX; i++) {
		post_divf = conf->post_divr * 2 * (i + 1);
		if (post_divf < PRCI_VCO_MIN || PRCI_VCO_MAX <= post_divf) {
			continue;
		}

		conf->divf = i;
		conf->post_divf = post_divf;
		clk_prci_find_divq(clk, conf);
	}
	if (conf->found) {
		return 0;
	}

	return -EINVAL;
}

static int clk_prci_find_divr(struct k_clk_device *clk, struct clk_prci_pllcfg *conf)
{
	struct clk_prci_priv *priv = k_clk_to_dev(clk)->priv;
	uint64_t post_divr;
	int r;

	conf->result = 0;
	conf->found = 0;

	for (int i = 0; i < PRCI_PLLCFG_DIVR_MAX; i++) {
		post_divr = priv->freq_in / (i + 1);
		if (post_divr < PRCI_REF_MIN || PRCI_REF_MAX <= post_divr) {
			continue;
		}

		conf->divr = i;
		conf->post_divr = post_divr;
		clk_prci_find_divf(clk, conf);
	}
	if (conf->found) {
		r = clk_prci_filter_range(clk, conf->post_divr_found, &conf->range);
		if (r != 0) {
			return -EINVAL;
		}

		return 0;
	}

	return -EINVAL;
}

static int clk_prci_update_pllcfg(struct k_clk_device *clk, uint64_t target, uint64_t *freq_found, int off)
{
	struct k_device *dev = k_clk_to_dev(clk);
	struct clk_prci_pllcfg conf = {0};
	uint32_t v;
	int r;

	conf.target = target;
	r = clk_prci_find_divr(clk, &conf);
	if (r != 0) {
		return r;
	}
	if (freq_found) {
		*freq_found = conf.result;
	}

	v = (conf.divr_found << PRCI_PLLCFG_DIVR_SHIFT) |
		(conf.divf_found << PRCI_PLLCFG_DIVF_SHIFT) |
		(conf.divq_found << PRCI_PLLCFG_DIVQ_SHIFT) |
		(conf.range << PRCI_PLLCFG_RANGE_SHIFT) |
		PRCI_PLLCFG_FSE;
	k_device_write32(dev, v, off);

	while ((k_device_read32(dev, off) & PRCI_PLLCFG_LOCK) == 0) {
		/* Busy wait for PLL lock*/
	}

	return 0;
}

static int clk_prci_update_cke(struct k_clk_device *clk, int enable, int off)
{
	struct k_device *dev = k_clk_to_dev(clk);
	uint32_t v;

	v = k_device_read32(dev, off);
	if (enable) {
		v |= PRCI_PLLCFG1_CKE;
	} else {
		v &= ~PRCI_PLLCFG1_CKE;
	}
	k_device_write32(dev, v, off);

	return 0;
}

static int clk_prci_update_clksel(struct k_clk_device *clk, int bypass, int off)
{
	struct k_device *dev = k_clk_to_dev(clk);
	uint32_t v;

	v = k_device_read32(dev, off);
	if (bypass) {
		v |= PRCI_CLKSEL_BYPASS;
	} else {
		v &= ~PRCI_CLKSEL_BYPASS;
	}
	k_device_write32(dev, v, off);

	return 0;
}

static int clk_prci_add(struct k_device *dev)
{
	struct clk_prci_priv *priv = dev->priv;
	struct k_clk_device *clk = k_clk_from_dev(dev);
	uint64_t freq;
	int r;

	if (priv == NULL) {
		k_dev_err(dev, "priv is NULL\n");
		return -EINVAL;
	}

	r = k_io_mmap_device(NULL, dev);
	if (r) {
		return r;
	}

	r = k_clk_get_clk_from_config(dev, 0, &priv->clk_in, &priv->index_clk_in);
	if (r) {
		return r;
	}

	r = k_clk_get_frequency(priv->clk_in, priv->index_clk_in, &priv->freq_in);
	if (r) {
		k_dev_err(dev, "clock freq is unknown.\n");
		return r;
	}

	r = k_clk_enable(priv->clk_in, priv->index_clk_in);
	if (r) {
		return r;
	}

	/* Set each PLL frequency */

	r = k_device_read_conf_u64(dev, "frequency", &freq, PRCI_INDEX_DDRCTRLCLK);
	if (r) {
		k_dev_err(dev, "config 'frequency' ddrctrlclk is not found.\n");
		return -EINVAL;
	}
	r = clk_prci_set_freq(clk, PRCI_INDEX_DDRCTRLCLK, freq);
	if (r) {
		return r;
	}

	r = k_device_read_conf_u64(dev, "frequency", &freq, PRCI_INDEX_CORECLK);
	if (r) {
		k_dev_err(dev, "config 'frequency' coreclk is not found.\n");
		return -EINVAL;
	}
	r = clk_prci_set_freq(clk, PRCI_INDEX_CORECLK, freq);
	if (r) {
		return r;
	}

	r = k_device_read_conf_u64(dev, "frequency", &freq, PRCI_INDEX_GEMGXLCLK);
	if (r) {
		k_dev_err(dev, "config 'frequency' gemgxlclk is not found.\n");
		return -EINVAL;
	}
	r = clk_prci_set_freq(clk, PRCI_INDEX_GEMGXLCLK, freq);
	if (r) {
		return r;
	}

	return 0;
}

static int clk_prci_remove(struct k_device *dev)
{
	return 0;
}

static int clk_prci_enable(struct k_clk_device *clk, int index)
{
	int off, r;

	if (index == PRCI_INDEX_TLCLK) {
		index = PRCI_INDEX_CORECLK;
	}

	off = pllcfg_offs[index].pllcfg1;
	if (off != -1) {
		r = clk_prci_update_cke(clk, 1, off);
		if (r) {
			return r;
		}
	}

	off = clksel_offs[index].clksel;
	if (off != -1) {
		r = clk_prci_update_clksel(clk, 0, off);
		if (r) {
			return r;
		}
	}

	return 0;
}

static int clk_prci_disable(struct k_clk_device *clk, int index)
{
	int off, r;

	if (index == PRCI_INDEX_TLCLK) {
		index = PRCI_INDEX_CORECLK;
	}

	off = clksel_offs[index].clksel;
	if (off != -1) {
		r = clk_prci_update_clksel(clk, 1, off);
		if (r) {
			return r;
		}
	}

	off = pllcfg_offs[index].pllcfg1;
	if (off != -1) {
		r = clk_prci_update_cke(clk, 0, off);
		if (r) {
			return r;
		}
	}

	return 0;
}

static int clk_prci_get_freq(struct k_clk_device *clk, int index, uint64_t *freq)
{
	struct clk_prci_priv *priv = k_clk_to_dev(clk)->priv;
	uint64_t f;

	switch (index) {
	case PRCI_INDEX_DDRCTRLCLK:
		f = priv->freq_ddrctrl;
		break;
	case PRCI_INDEX_TLCLK:
		f = priv->freq_core / 2;
		break;
	case PRCI_INDEX_CORECLK:
		f = priv->freq_core;
		break;
	case PRCI_INDEX_GEMGXLCLK:
		f = priv->freq_gemgxl;
		break;
	default:
		return -EINVAL;
	}
	if (freq) {
		*freq = f;
	}

	return 0;
}

static int clk_prci_set_freq(struct k_clk_device *clk, int index, uint64_t freq)
{
	struct k_device *dev = k_clk_to_dev(clk);
	struct clk_prci_priv *priv = dev->priv;
	uint64_t *freq_found;
	int r;

	switch (index) {
	case PRCI_INDEX_DDRCTRLCLK:
		freq_found = &priv->freq_ddrctrl;
		break;
	case PRCI_INDEX_TLCLK:
		k_dev_err(dev, "Cannot change TLCLK.\n");
		return -EINVAL;
	case PRCI_INDEX_CORECLK:
		freq_found = &priv->freq_core;
		break;
	case PRCI_INDEX_GEMGXLCLK:
		freq_found = &priv->freq_gemgxl;
		break;
	default:
		k_dev_err(dev, "Unknown index %d.\n", index);
		return -EINVAL;
	}

	r = clk_prci_update_pllcfg(clk, freq, freq_found, pllcfg_offs[index].pllcfg0);
	if (r) {
		return r;
	}

	return 0;
}

const static struct k_device_driver_ops clk_prci_dev_ops = {
	.add = clk_prci_add,
	.remove = clk_prci_remove,
	.mmap = k_device_driver_mmap,
};

const static struct k_clk_driver_ops clk_prci_clk_ops = {
	.enable = clk_prci_enable,
	.disable = clk_prci_disable,
	.get_freq = clk_prci_get_freq,
	.set_freq = clk_prci_set_freq,
};

static struct k_clk_driver clk_prci_drv = {
	.base = {
		.base = {
			.type_vendor = "sifive",
			.type_device = "prci0",
		},

		.ops = &clk_prci_dev_ops,
	},

	.ops = &clk_prci_clk_ops,
};

static int clk_prci_init(void)
{
	k_clk_add_driver(&clk_prci_drv);

	return 0;
}
define_init_func(clk_prci_init);

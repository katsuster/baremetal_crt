# SPDX-License-Identifier: Apache-2.0

prj_sources_ifdef(CONFIG_GPIO gpio.c gpio_dev.c)
prj_sources_ifdef(CONFIG_GPIO_XILINX_GPIO_V2 xilinx/gpio_v2.c)

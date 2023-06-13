/**
 * Copyright 2023 NXP
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>

LOG_MODULE_REGISTER(board_control, CONFIG_BOARD_MIMX93_EVK_A55_LOG_LEVEL);

#if !defined(CONFIG_BOARD_MIMX93_EVK_A55_EXP_SEL_NO_INIT)

#define MIMX93_EVK_A55_EXP_SEL_DEV DEVICE_DT_GET_OR_NULL(DT_NODELABEL(gpio_exp0))
#define MIMX93_EVK_A55_EXP_SEL_PIN (4U)
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios,
							      {0});

static int board_init_exp_sel(const struct device *dev)
{
    int rc = 0;
    const struct device *exp_dev = MIMX93_EVK_A55_EXP_SEL_DEV;
    const uint32_t exp_sel_pin = MIMX93_EVK_A55_EXP_SEL_PIN;
    uint32_t pin_state;

    printk("mux init func\n");
    if (exp_dev == NULL) {
        LOG_ERR("adp5585 expander not found, aborting\n");
        return -ENODEV;
    }
    else if (!device_is_ready(exp_dev)) {
		LOG_ERR("%s is not ready\n", exp_dev->name);
		return -ENODEV;
	}
    rc = gpio_pin_configure(exp_dev, exp_sel_pin, GPIO_OUTPUT);
	if (rc) {
		LOG_ERR("Error %d while configuring pin\n", rc);
		return rc;
	}


#if defined(CONFIG_BOARD_MIMX93_EVK_A55_EXP_SEL_MUX_A)
    LOG_INF("exp_sel mux A, prio %d\n", CONFIG_BOARD_MIMX93_EVK_A55_EXP_SEL_INIT_PRIO);
    pin_state = 0U;
#elif defined(CONFIG_BOARD_MIMX93_EVK_A55_EXP_SEL_MUX_B)
    LOG_INF("exp_sel mux B, prio %d\n", CONFIG_BOARD_MIMX93_EVK_A55_EXP_SEL_INIT_PRIO);
    pin_state = 1U;
#endif

    rc = gpio_pin_set_raw(exp_dev, exp_sel_pin, pin_state);
    if (rc) {
		LOG_ERR("Error %d while set pin to %d\n", rc, pin_state);
		return rc;
	}

	return 0;
}

SYS_INIT(board_init_exp_sel, POST_KERNEL, CONFIG_BOARD_MIMX93_EVK_A55_EXP_SEL_INIT_PRIO);

#endif // ! CONFIG_BOARD_MIMX93_EVK_A55_EXP_SEL_NO_INIT

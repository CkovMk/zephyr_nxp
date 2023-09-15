/*
 * Copyright 2019-2020 Peter Bigot Consulting, LLC
 * Copyright 2022 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT regulator_fixed_seq

#include <stdint.h>

#include <zephyr/kernel.h>
#include <zephyr/drivers/regulator.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(regulator_fixed_seq, CONFIG_REGULATOR_LOG_LEVEL);

struct regulator_fixed_seq_config {
	struct regulator_common_config common;
	uint32_t ngpios;
	const struct gpio_dt_spec * enable_gpios;
	const uint32_t * enable_seq;
	const uint32_t * enable_delay;
	const uint32_t * disable_seq;
	const uint32_t * disable_delay;
};

struct regulator_fixed_seq_data {
	struct regulator_common_data common;
};

static int regulator_fixed_seq_enable(const struct device *dev)
{
	const struct regulator_fixed_seq_config *cfg = dev->config;
	int ret;

	for(int i = 0; i < cfg->ngpios; ++i)
	{
		ret = gpio_pin_set_dt(&cfg->enable_gpios[cfg->enable_seq[i]], 1);
		if (ret < 0) {
			return ret;
		}
		if(cfg->enable_delay[i] != 0U)
		{
			k_sleep(K_USEC(cfg->enable_delay[i]));
		}
	}

	return 0;
}

static int regulator_fixed_seq_disable(const struct device *dev)
{
	const struct regulator_fixed_seq_config *cfg = dev->config;
	int ret;

	for(int i = 0; i < cfg->ngpios; ++i)
	{
		ret = gpio_pin_set_dt(&cfg->enable_gpios[cfg->disable_seq[i]], 0);
		if (ret < 0) {
			return ret;
		}
		if(cfg->disable_delay[i] != 0U)
		{
			k_sleep(K_USEC(cfg->disable_delay[i]));
		}
	}

	return 0;
}

static const struct regulator_driver_api regulator_fixed_seq_api = {
	.enable = regulator_fixed_seq_enable,
	.disable = regulator_fixed_seq_disable,
};

static int regulator_fixed_seq_init(const struct device *dev)
{
	const struct regulator_fixed_seq_config *cfg = dev->config;
	int ret;

	regulator_common_data_init(dev);

	for(int i = 0; i < cfg->ngpios; ++i)
	{
		if (!device_is_ready(cfg->enable_gpios[i].port)) {
			LOG_ERR("GPIO port: %s not ready", cfg->enable_gpios[i].port->name);
			return -ENODEV;
		}

		ret = gpio_pin_configure_dt(&cfg->enable_gpios[i], GPIO_OUTPUT_INACTIVE);
		if (ret < 0) {
			return ret;
		}
	}

	ret = regulator_common_init(dev, false);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

#define REGULATOR_FIXED_SEQ_DEFINE(inst)                                           \
	static struct regulator_fixed_seq_data data##inst;                         \
	struct gpio_dt_spec  config_enable_gpios_##inst[] = { \
		DT_FOREACH_PROP_ELEM_SEP(DT_DRV_INST(inst), enable_gpios, GPIO_DT_SPEC_GET_BY_IDX, (,)) \
	}; \
	static const uint32_t config_enable_seq_##inst[] = DT_INST_PROP(inst, enable_seq); \
	static const uint32_t config_enable_delay_##inst[] = DT_INST_PROP(inst, enable_delay); \
	static const uint32_t config_disable_seq_##inst[] = DT_INST_PROP(inst, disable_seq); \
	static const uint32_t config_disable_delay_##inst[] = DT_INST_PROP(inst, disable_delay); \
    BUILD_ASSERT(ARRAY_SIZE(config_enable_gpios_##inst) == ARRAY_SIZE(config_enable_seq_##inst),  \
		DEVICE_DT_NAME(DT_DRV_INST(inst))": number of enable_seq needs to match numer of gpio");   \
    BUILD_ASSERT(ARRAY_SIZE(config_enable_gpios_##inst) == ARRAY_SIZE(config_enable_delay_##inst),  \
		DEVICE_DT_NAME(DT_DRV_INST(inst))": number of enable_delay needs to match numer of gpio");   \
	BUILD_ASSERT(ARRAY_SIZE(config_enable_gpios_##inst) == ARRAY_SIZE(config_disable_seq_##inst),  \
		DEVICE_DT_NAME(DT_DRV_INST(inst))": number of disable_seq needs to match numer of gpio");   \
	BUILD_ASSERT(ARRAY_SIZE(config_enable_gpios_##inst) == ARRAY_SIZE(config_disable_delay_##inst),  \
		DEVICE_DT_NAME(DT_DRV_INST(inst))": number of disable_delay needs to match numer of gpio");   \
	static const struct regulator_fixed_seq_config config##inst = {            \
		.common = REGULATOR_DT_INST_COMMON_CONFIG_INIT(inst),          \
		.ngpios = DT_INST_PROP_LEN(inst, enable_gpios), \
		.enable_gpios = config_enable_gpios_##inst,           \
		.enable_seq = config_enable_seq_##inst, \
		.enable_delay = config_enable_delay_##inst, \
		.disable_seq = config_disable_seq_##inst, \
		.disable_delay = config_disable_delay_##inst, \
	};                                                \
                                                                               \
	DEVICE_DT_INST_DEFINE(inst, regulator_fixed_seq_init, NULL, &data##inst,   \
			      &config##inst, POST_KERNEL,                      \
			      CONFIG_REGULATOR_FIXED_SEQ_INIT_PRIORITY,            \
			      &regulator_fixed_seq_api);

DT_INST_FOREACH_STATUS_OKAY(REGULATOR_FIXED_SEQ_DEFINE)

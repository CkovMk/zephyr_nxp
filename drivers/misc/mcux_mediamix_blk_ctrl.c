/*
 * Copyright (c) 2023, NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT nxp_imx93_mediamix

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/clock_control.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(mediamix, CONFIG_MEDIAMIX_LOG_LEVEL);

#include <fsl_common.h>

struct mcux_mediamix_config {
	MEDIAMIX_BLK_CTRL_Type *base;
	const struct device *media_axi_clk_dev;
	clock_control_subsys_t media_axi_clk_subsys;
	const struct device *media_apb_clk_dev;
	clock_control_subsys_t media_apb_clk_subsys;
	const struct device *cam_pix_clk_dev;
	clock_control_subsys_t cam_pix_clk_subsys;
};

static void imx93_mediamix_set_qos_isi(MEDIAMIX_BLK_CTRL_Type *base)
{
	uint32_t reg = 0;

	reg |= MEDIAMIX_BLK_CTRL_ISI1_DEFAULT_QOS_V(0x3) |
	       MEDIAMIX_BLK_CTRL_ISI1_CFG_QOS_V(0x7) |
	       MEDIAMIX_BLK_CTRL_ISI1_DEFAULT_QOS_U(0x3) |
	       MEDIAMIX_BLK_CTRL_ISI1_CFG_QOS_U(0x7) |
	       MEDIAMIX_BLK_CTRL_ISI1_DEFAULT_QOS_Y_R(0x3) |
	       MEDIAMIX_BLK_CTRL_ISI1_CFG_QOS_Y_R(0x7) |
	       MEDIAMIX_BLK_CTRL_ISI1_DEFAULT_QOS_Y_W(0x3) |
	       MEDIAMIX_BLK_CTRL_ISI1_CFG_QOS_Y_W(0x7);
	base->BUS_CONTROL.ISI1 = reg;
}

static int mcux_mediamix_clock_init(const struct device *dev)
{
	const struct mcux_mediamix_config *config = dev->config;
	enum clock_control_status clk_status;
	struct _clock_root_config_t clk_config;
	uint32_t clk_freq;
	int ret;

	/* configure media_axi_clk */
	if (!device_is_ready(config->media_axi_clk_dev)) {
		LOG_ERR("media_axi clock control device not ready");
		return -ENODEV;
	}

	clk_config.clockOff = false;
	clk_config.mux = 2;
	clk_config.div = 2;

	clock_control_configure(config->media_axi_clk_dev, config->media_axi_clk_subsys, &clk_config);

	clk_status = clock_control_get_status(config->media_axi_clk_dev, config->media_axi_clk_subsys);
	if (clk_status != CLOCK_CONTROL_STATUS_ON) {
		if (clk_status == CLOCK_CONTROL_STATUS_OFF) {
			ret = clock_control_on(config->media_axi_clk_dev, config->media_axi_clk_subsys);
			if (ret) {
				LOG_ERR("media_axi clock can't be enabled");
				return ret;
			}
		}
		else
			return -EINVAL;
	}

	if (clock_control_get_rate(config->media_axi_clk_dev, config->media_axi_clk_subsys, &clk_freq)) {
		return -EINVAL;
	}
	LOG_DBG("media_axi clock frequency %d", clk_freq);

	/* configure media_apb_clk */
	if (!device_is_ready(config->media_apb_clk_dev)) {
		LOG_ERR("media_apb clock control device not ready");
		return -ENODEV;
	}

	clk_config.clockOff = false;
	clk_config.mux = 2;
	clk_config.div = 3;

	clock_control_configure(config->media_apb_clk_dev, config->media_apb_clk_subsys, &clk_config);

	clk_status = clock_control_get_status(config->media_apb_clk_dev, config->media_apb_clk_subsys);
	if (clk_status != CLOCK_CONTROL_STATUS_ON) {
		if (clk_status == CLOCK_CONTROL_STATUS_OFF) {
			ret = clock_control_on(config->media_apb_clk_dev, config->media_apb_clk_subsys);
			if (ret) {
				LOG_ERR("media_apb clock can't be enabled");
				return ret;
			}
		}
		else
			return -EINVAL;
	}

	if (clock_control_get_rate(config->media_apb_clk_dev, config->media_apb_clk_subsys, &clk_freq)) {
		return -EINVAL;
	}
	LOG_DBG("media_apb clock frequency %d", clk_freq);

	/* configure cam_pix_clk */
	if (!device_is_ready(config->cam_pix_clk_dev)) {
		LOG_ERR("cam_pix clock control device not ready");
		return -ENODEV;
	}

	clk_config.clockOff = false;
	clk_config.mux = 2;
	clk_config.div = 3;

	clock_control_configure(config->cam_pix_clk_dev, config->cam_pix_clk_subsys, &clk_config);

	clk_status = clock_control_get_status(config->cam_pix_clk_dev, config->cam_pix_clk_subsys);
	if (clk_status != CLOCK_CONTROL_STATUS_ON) {
		if (clk_status == CLOCK_CONTROL_STATUS_OFF) {
			ret = clock_control_on(config->cam_pix_clk_dev, config->cam_pix_clk_subsys);
			if (ret) {
				LOG_ERR("cam_pix clock can't be enabled");
				return ret;
			}
		}
		else
			return -EINVAL;
	}

	if (clock_control_get_rate(config->cam_pix_clk_dev, config->cam_pix_clk_subsys, &clk_freq)) {
		return -EINVAL;
	}
	LOG_DBG("cam_pix clock frequency %d", clk_freq);

	return 0;
}

static const struct mcux_mediamix_config mcux_mediamix_config_0 = {
	.base = (MEDIAMIX_BLK_CTRL_Type *)DT_INST_REG_ADDR(0),
	.media_axi_clk_dev = DEVICE_DT_GET(DT_INST_CLOCKS_CTLR_BY_IDX(0, 0)),
	.media_axi_clk_subsys = (clock_control_subsys_t)DT_INST_CLOCKS_CELL_BY_IDX(0, 0, name),
	.media_apb_clk_dev = DEVICE_DT_GET(DT_INST_CLOCKS_CTLR_BY_IDX(0, 1)),
	.media_apb_clk_subsys = (clock_control_subsys_t)DT_INST_CLOCKS_CELL_BY_IDX(0, 1, name),
	.cam_pix_clk_dev = DEVICE_DT_GET(DT_INST_CLOCKS_CTLR_BY_IDX(0, 2)),
	.cam_pix_clk_subsys = (clock_control_subsys_t)DT_INST_CLOCKS_CELL_BY_IDX(0, 2, name),
};

static int mcux_mediamix_init_0(const struct device *dev)
{
	const struct mcux_mediamix_config *config = dev->config;
	int ret;

	ret = mcux_mediamix_clock_init(dev);
	if (ret)
		return ret;

	imx93_mediamix_set_qos_isi(config->base);

	LOG_INF("%s init succeeded", dev->name);
	return 0;
}

DEVICE_DT_INST_DEFINE(0, mcux_mediamix_init_0, NULL,
		    NULL, &mcux_mediamix_config_0, POST_KERNEL,
		    CONFIG_KERNEL_INIT_PRIORITY_DEVICE, NULL);

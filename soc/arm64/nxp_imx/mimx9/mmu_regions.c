/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/arch/arm64/arm_mmu.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/util.h>

static const struct arm_mmu_region mmu_regions[] = {

	MMU_REGION_FLAT_ENTRY("GIC",
			      DT_REG_ADDR_BY_IDX(DT_NODELABEL(gic), 0),
			      DT_REG_SIZE_BY_IDX(DT_NODELABEL(gic), 0),
			      MT_DEVICE_nGnRnE | MT_P_RW_U_NA | MT_NS),

	MMU_REGION_FLAT_ENTRY("GIC",
			      DT_REG_ADDR_BY_IDX(DT_NODELABEL(gic), 1),
			      DT_REG_SIZE_BY_IDX(DT_NODELABEL(gic), 1),
			      MT_DEVICE_nGnRnE | MT_P_RW_U_NA | MT_NS),

	MMU_REGION_FLAT_ENTRY("CCM",
			      DT_REG_ADDR(DT_NODELABEL(ccm)),
			      DT_REG_SIZE(DT_NODELABEL(ccm)),
			      MT_DEVICE_nGnRnE | MT_P_RW_U_NA | MT_NS),

	MMU_REGION_FLAT_ENTRY("ANA_PLL",
			      DT_REG_ADDR(DT_NODELABEL(ana_pll)),
			      DT_REG_SIZE(DT_NODELABEL(ana_pll)),
			      MT_DEVICE_nGnRnE | MT_P_RW_U_NA | MT_NS),

	MMU_REGION_FLAT_ENTRY("UART1",
			      DT_REG_ADDR(DT_NODELABEL(lpuart1)),
			      DT_REG_SIZE(DT_NODELABEL(lpuart1)),
			      MT_DEVICE_nGnRnE | MT_P_RW_U_NA | MT_NS),

	MMU_REGION_FLAT_ENTRY("UART2",
			      DT_REG_ADDR(DT_NODELABEL(lpuart2)),
			      DT_REG_SIZE(DT_NODELABEL(lpuart2)),
			      MT_DEVICE_nGnRnE | MT_P_RW_U_NA | MT_NS),

	MMU_REGION_FLAT_ENTRY("IOMUXC",
			      DT_REG_ADDR(DT_NODELABEL(iomuxc)),
			      DT_REG_SIZE(DT_NODELABEL(iomuxc)),
			      MT_DEVICE_nGnRnE | MT_P_RW_U_NA | MT_NS),

	MMU_REGION_FLAT_ENTRY("GPIO1",
			      DT_REG_ADDR(DT_NODELABEL(gpio1)),
			      DT_REG_SIZE(DT_NODELABEL(gpio1)),
			      MT_DEVICE_nGnRnE | MT_P_RW_U_NA | MT_NS),

	MMU_REGION_FLAT_ENTRY("GPIO2",
			      DT_REG_ADDR(DT_NODELABEL(gpio2)),
			      DT_REG_SIZE(DT_NODELABEL(gpio2)),
			      MT_DEVICE_nGnRnE | MT_P_RW_U_NA | MT_NS),

	MMU_REGION_FLAT_ENTRY("GPIO3",
			      DT_REG_ADDR(DT_NODELABEL(gpio3)),
			      DT_REG_SIZE(DT_NODELABEL(gpio3)),
			      MT_DEVICE_nGnRnE | MT_P_RW_U_NA | MT_NS),

	MMU_REGION_FLAT_ENTRY("GPIO4",
			      DT_REG_ADDR(DT_NODELABEL(gpio4)),
			      DT_REG_SIZE(DT_NODELABEL(gpio4)),
			      MT_DEVICE_nGnRnE | MT_P_RW_U_NA | MT_NS),

	MMU_REGION_FLAT_ENTRY("I2C1",
			      DT_REG_ADDR(DT_NODELABEL(lpi2c1)),
			      DT_REG_SIZE(DT_NODELABEL(lpi2c1)),
			      MT_DEVICE_nGnRnE | MT_P_RW_U_NA | MT_NS),

	MMU_REGION_FLAT_ENTRY("I2C2",
			      DT_REG_ADDR(DT_NODELABEL(lpi2c2)),
			      DT_REG_SIZE(DT_NODELABEL(lpi2c2)),
			      MT_DEVICE_nGnRnE | MT_P_RW_U_NA | MT_NS),

	MMU_REGION_FLAT_ENTRY("I2C3",
			      DT_REG_ADDR(DT_NODELABEL(lpi2c3)),
			      DT_REG_SIZE(DT_NODELABEL(lpi2c3)),
			      MT_DEVICE_nGnRnE | MT_P_RW_U_NA | MT_NS),

	MMU_REGION_FLAT_ENTRY("I2C4",
			      DT_REG_ADDR(DT_NODELABEL(lpi2c4)),
			      DT_REG_SIZE(DT_NODELABEL(lpi2c4)),
			      MT_DEVICE_nGnRnE | MT_P_RW_U_NA | MT_NS),

	MMU_REGION_FLAT_ENTRY("I2C8",
			      DT_REG_ADDR(DT_NODELABEL(lpi2c8)),
			      DT_REG_SIZE(DT_NODELABEL(lpi2c8)),
			      MT_DEVICE_nGnRnE | MT_P_RW_U_NA | MT_NS),

	MMU_REGION_FLAT_ENTRY("SPI1",
			      DT_REG_ADDR(DT_NODELABEL(lpspi1)),
			      DT_REG_SIZE(DT_NODELABEL(lpspi1)),
			      MT_DEVICE_nGnRnE | MT_P_RW_U_NA | MT_NS),

	MMU_REGION_FLAT_ENTRY("SPI2",
			      DT_REG_ADDR(DT_NODELABEL(lpspi2)),
			      DT_REG_SIZE(DT_NODELABEL(lpspi2)),
			      MT_DEVICE_nGnRnE | MT_P_RW_U_NA | MT_NS),

	MMU_REGION_FLAT_ENTRY("SPI3",
			      DT_REG_ADDR(DT_NODELABEL(lpspi3)),
			      DT_REG_SIZE(DT_NODELABEL(lpspi3)),
			      MT_DEVICE_nGnRnE | MT_P_RW_U_NA | MT_NS),

	MMU_REGION_FLAT_ENTRY("SPI4",
			      DT_REG_ADDR(DT_NODELABEL(lpspi4)),
			      DT_REG_SIZE(DT_NODELABEL(lpspi4)),
			      MT_DEVICE_nGnRnE | MT_P_RW_U_NA | MT_NS),

	MMU_REGION_FLAT_ENTRY("CAN1",
			      DT_REG_ADDR(DT_NODELABEL(flexcan1)),
			      DT_REG_SIZE(DT_NODELABEL(flexcan1)),
			      MT_DEVICE_nGnRnE | MT_P_RW_U_NA | MT_NS),

	MMU_REGION_FLAT_ENTRY("CAN2",
			      DT_REG_ADDR(DT_NODELABEL(flexcan2)),
			      DT_REG_SIZE(DT_NODELABEL(flexcan2)),
			      MT_DEVICE_nGnRnE | MT_P_RW_U_NA | MT_NS),

	MMU_REGION_FLAT_ENTRY("MEDIAMIX",
			      DT_REG_ADDR(DT_NODELABEL(media_blk_ctrl)),
			      DT_REG_SIZE(DT_NODELABEL(media_blk_ctrl)),
			      MT_DEVICE_nGnRnE | MT_P_RW_U_NA | MT_NS),

	MMU_REGION_FLAT_ENTRY("ISI",
			      DT_REG_ADDR(DT_NODELABEL(isi)),
			      DT_REG_SIZE(DT_NODELABEL(isi)),
			      MT_DEVICE_nGnRnE | MT_P_RW_U_NA | MT_NS),
};

const struct arm_mmu_config mmu_config = {
	.num_regions = ARRAY_SIZE(mmu_regions),
	.mmu_regions = mmu_regions,
};

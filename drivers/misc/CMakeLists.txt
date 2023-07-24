# SPDX-License-Identifier: Apache-2.0

zephyr_library()
zephyr_library_sources_ifdef(CONFIG_MCUX_MEDIAMIX_BLK_CTRL mcux_mediamix_blk_ctrl.c)

add_subdirectory_ifdef(CONFIG_ARM_ETHOS_U ethos_u)
add_subdirectory_ifdef(CONFIG_FT800 ft8xx)
add_subdirectory_ifdef(CONFIG_GROVE_LCD_RGB grove_lcd_rgb)

# SPDX-License-Identifier: Apache-2.0

zephyr_library()

zephyr_include_directories(${ZEPHYR_BASE}/drivers)

zephyr_library_sources(mcux_flexio.c)

Zephyr project for i.MX93
=========================

This file describes how to build Zephyr project for i.MX93, which is support by NXP MPUSE.   


## Initialize the Zephyr project

According to: https://docs.zephyrproject.org/latest/develop/getting_started/index.html, initialize and configure the project.



## Apply the source code for i.MX93
Add these repos as new "remotes" for corresponding git directory.

```bash
cd ${zephyrproject}/zephyr
git remote add internal ssh://git@bitbucket.sw.nxp.com/mpucnse/zephyr.git
git fetch internal
git checkout -b imx93_zephyr internal/imx93_zephyr

west update
 
cd ${zephyrproject}/modules/hal/nxp
git remote add internal ssh://git@bitbucket.sw.nxp.com/mpucnse/zephyr_hal.git
git fetch internal
git checkout -b imx93_zephyr internal/imx93_zephyr
```

## Build the image
Here is an example for the Hello World application.
```bash
cd ${zephyrproject}/zephyr
west build -p always -b mimx93_evk_a55 samples/hello_world
```

Here is an example for the SMP application.
```bash
cd ${zephyrproject}/zephyr
west build -p always -b mimx93_evk_a55_smp samples/synchronization
```

Then find the image in ${zephyrproject}/zephyr/build/zephyr/zephyr.bin  
Rename zephyr.bin to u-boot.bin and copy it to imx-mkimage/iMX9


```bash
cd imx-mkimage
make SOC=iMX9 flash_singleboot_m33 # for single boot mode
```
Please refer to https://www.nxp.com/docs/en/user-guide/IMX_LINUX_USERS_GUIDE.pdf for more information.

## Flash the image
Use this command to flash image to SD card:
```bash
sudo dd if=flash.bin of=/dev/sdx bs=1k seek=32 && sync
```
Here, change the `/dev/sdx` to the actual device.


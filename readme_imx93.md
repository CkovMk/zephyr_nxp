Zephyr project for i.MX93
=========================

This file describes how to build Zephyr project for i.MX93, which is support by NXP MPUSE.   

## Initialize Zephyr project

According to: https://docs.zephyrproject.org/latest/develop/getting_started/index.html, initialize and configure the project.

## Get the Code for i.MX93

Add these repos as new "remotes" for corresponding git directory:

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

## Enable SPL Boot at Project Level

This step is optional.

ZephyrOS boots from OCRAM by default, and reylies on U-Boot to load the binary. By using SPL, we can simplify the boot process. To enable SPL boot at project level, here are the steps:

Create `app.overlay` under your project root path (if not already exist). Add the following code:

```
/delete-node/ &sram0;

/{
    chosen{
        zephyr,console = &lpuart1;
        zephyr,shell-uart = &lpuart1;
    };

    sram0: memory@80200000 {
        reg = <0x80200000 DT_SIZE_M(1792)>;
    };

};

&lpuart1 {
    status = "okay";
};
```

This code delete the old `sram0` node that contains OCRAM address space, and add new `sram0` node that contains DDR address space. It alse enables and reselects `lpuart1` as console port for zephyr, making it to reuse the SPL console.

Now this project is ready to boot with SPL.

## Build Projects

Here is an example for the `hello_world` application:

```bash
cd ${zephyrproject}/zephyr
west build -p always -b mimx93_evk_a55 samples/hello_world
```

Here is an example for building an SMP application:

```bash
cd ${zephyrproject}/zephyr
west build -p always -b mimx93_evk_a55_smp samples/synchronization
```

The build binary is `${zephyrproject}/zephyr/build/zephyr/zephyr.bin`

## Boot Zephyr

### Boot with U-Boot

For booting zephyr with U-Boot, refer to [NXP i.MX93 EVK - Zephyr Project Documentation](https://docs.zephyrproject.org/latest/boards/arm64/mimx93_evk/doc/index.html) for instructions.

### Boot with SPL

For booting zephyr with SPL, prepare `imx-mkimage` environment for i.MX 93. Copy `zephyr.bin`  to `imx-mkimage/iMX9/u-boot.bin` . Make the boot image:

```bash
cd imx-mkimage
make SOC=iMX9 flash_singleboot
```

> refer to https://www.nxp.com/docs/en/user-guide/IMX_LINUX_USERS_GUIDE.pdf for more information.

Flash image to SD card with `dd` :

```bash
sudo dd if=flash.bin of=/dev/sdx bs=1k seek=32 && sync
```

> Change the `/dev/sdx` to the actual device.
> 
> Note the 32K padding before the binary. The board would NOT boot without it.

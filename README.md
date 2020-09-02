# First aproach for CCS811 air quality sensor linux driver
Tested on ARM over I2C

## Device tree example
The following is an example for device tree changes to a beagle bone board.
arch/arm/boot/dts/am335x-bone-common.dtsi
```

&am33xx_pinmux {

...

       i2c1_pins: pinmux_i2c1_pins {
               pinctrl-single,pins = <
                       AM33XX_IOPAD(0x958, PIN_INPUT_PULLUP | MUX_MODE2)       /* spi0_d1.i2c1_sda */
                       AM33XX_IOPAD(0x95c, PIN_INPUT_PULLUP | MUX_MODE2)       /* spi0_cs0.i2c1_scl */
               >;
       };

 };
 
 &i2c1 {
       pinctrl-names = "default";
       pinctrl-0 = <&i2c1_pins>;
 
       status = "okay";
       clock-frequency = <100000>;
 
     /* CCS811 is at 0x5B or 0x5A */
       air0: air0@5A {
               compatible = "ams,ccs811";
               reg = <0x5A>;
     };

...
```
You will need to adapt yours acordingly.

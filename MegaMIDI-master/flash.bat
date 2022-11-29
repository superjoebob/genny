avrdude -c usbtiny -p usb1286 -B 0.5 -V -U flash:w:".pio\build\teensy2pp\firmware.hex":a

-U lfuse:w:0x5E:m -U hfuse:w:0xDF:m -U efuse:w:0xF3:m

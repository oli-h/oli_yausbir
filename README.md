# oli_yausbir
Plugin (i.e. driver) for "yaUsbIR V3 LIRC USB IR Empfänger/Sender/Einschalter"
At the moment this is just a quick'n'dirty "Eclipse CDT" project (i.e. no Makefile or so). Soon coming.

Just google for "yaUsbIR V3" and you quickly find info about this little InfraRed-Receiver/Transmitter/PowerSwitch-device

Use `xmode2 -U Debug/ -H oli_yausbir` 

## udev
Write this into `/lib/udev/rules.d/99-yausbir.rules` to give normal user access to the USB-Device.

```
SUBSYSTEM=="usb", ATTRS{idVendor}=="10c4", ATTRS{idProduct}=="876c", MODE="0666"
```
Simply reboot to take effect
 
# oli_yausbir - WORK IN PROGRESS
Plugin (i.e. driver) for "yaUsbIR V3 LIRC USB IR Empfänger/Sender/Einschalter"
At the moment this is just a quick'n'dirty "Eclipse CDT" project (i.e. no Makefile or so). Soon coming.

Just google for "yaUsbIR V3" and you quickly find info about this little InfraRed-Receiver/Transmitter/PowerSwitch-device

## Usage
`mode2 -U Debug/ -H oli_yausbir`

`xmode2 -U Debug/ -H oli_yausbir`

`irrecord -U Debug/ -H oli_yausbir`

etc...

Finally copy the compile/lin-result `oli_yausbir.so` to directory `/usr/.../lirc/.../plugins/` (Details follow).
Then you don't need to specify `-U ...` any more

## udev
Write this into `/lib/udev/rules.d/99-yausbir.rules` to give normal user access to the USB-Device.
Other wise you have to start all tools (`xmode2`, `irrecord`, etc) as root.

```
SUBSYSTEM=="usb", ATTRS{idVendor}=="10c4", ATTRS{idProduct}=="876c", MODE="0666"
```
Simply reboot to take effect
 

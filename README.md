# oli_yausbir - WORK IN PROGRESS
Plugin (i.e. driver) for "yaUsbIR V3 LIRC USB IR Empfänger/Sender/Einschalter"
At the moment this is just a quick'n'dirty "Eclipse CDT" project (i.e. no Makefile or so).

Just google for "yaUsbIR V3" and you quickly find info about this little InfraRed-Receiver/Transmitter/PowerSwitch-device

## Usage
- `mode2 -U Debug/ -H oli_yausbir`
- `xmode2 -U Debug/ -H oli_yausbir`
- `irrecord -U Debug/ -H oli_yausbir`
- etc...

Finally copy the compile/link-result `liboli_yausbir.so` to directory `/usr/lib/x86_64-linux-gnu/lirc/plugins/` (Details follow). Then you don't need to specify `-U ...` any more

## Sending control-commands
The yaUsbIr-Board understands several control-commands. This PlugIn-driver here supports sending such control-commands (Note: Sending 'any' IR-Codes with a real IR-Trasmitter-LED - which must be attached to the yaUsbIr-Board - is *not* tested at all)

First: Copy [yaUsbIR_V3_lircd.conf](./yaUsbIr-Doc/yausbirv3.003/yaUsbIR_V3_lircd.conf) to `/etc/lirc/lircd.conf.d/` first and (re)start `lircd` before you can send anything with `irsend`.

Then check if all works by switching the red LED (on yaUsbIr-Board) on and off:
- `irsend SEND_ONCE yaUsbIR_control C_IR 8 0 C_END` turn *off*
- `irsend SEND_ONCE yaUsbIR_control C_IR 8 1 C_END` turn *on*

The most relevant command then probably is:
- `irsend SEND_ONCE yaUsbIR_control C_IR 1 1 0 C_END` to start learning the "Power-IR-Code"

See [Anleitung yaUsbIR V3.4.pdf](./yaUsbir-Doc/Anleitung yaUsbIR V3.4.pdf) for details and a list of all commands.

## udev
Write this into `/lib/udev/rules.d/99-yausbir.rules` to give normal user access to the USB-Device.
Other wise you have to start all tools (`xmode2`, `irrecord`, etc) as root.

```
SUBSYSTEM=="usb", ATTRS{idVendor}=="10c4", ATTRS{idProduct}=="876c", MODE="0666"
```
Simply reboot to take effect

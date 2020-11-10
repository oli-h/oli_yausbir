# oli_yausbir
A LIRC-Plugin (i.e. driver) for "yaUsbIR V3 LIRC USB IR Empfänger/Sender/Einschalter"

Just google for "yaUsbIR V3" and you quickly find info about this little InfraRed-Receiver/Transmitter/PowerSwitch-device

## Notes
- Work in progress
- At the moment this is just a quick'n'dirty "Eclipse CDT" project (i.e. no Makefile or so).
- For the impatient: a precompiled lib is avaible in `bin/liboli_yausbir.so` (provided 'as is')
- Logging not yet proper. It just prints to stdout.

## Build and usage
Use *Eclipse IDE for C/C++ Developers* and invoke a build. Then use the well-known LIRC-Tools like this:
- `mode2 -U bin/ -H oli_yausbir`
- `xmode2 -U bin/ -H oli_yausbir`
- `irrecord -U bin/ -H oli_yausbir`
- etc...

To use *your* compiled version: replace `-U bin/` by `-U Debug/` or `-U Release/`

Finally copy the compile/link-result `liboli_yausbir.so` to directory `/usr/lib/x86_64-linux-gnu/lirc/plugins/` (Cross-check with property `plugindir` in *your* `/etc/lirc/lirc_options.conf`). Then you don't need to specify `-U ...` any more.

To avoid setting the driver explicit with `-H oli_yausbir` you can also set it globally in `/etc/lirc/lirc_options.conf`:

```
[lircd]
...
driver = oli_yausbir
...
```

## Sending control-commands
The yaUsbIr-Board understands several control-commands. This PlugIn-driver here supports sending such control-commands (Note: Sending 'any' IR-Codes with a real IR-Trasmitter-LED - which must be attached to the yaUsbIr-Board - is *not* yet supported)

You need to copy [yaUsbIR_V3_lircd.conf](./yaUsbIr-Doc/yausbirv3.003/yaUsbIR_V3_lircd.conf) to `/etc/lirc/lircd.conf.d/` and (re)start `lircd` first *before* you can can anything with `irsend`.

Check if "sending control-commands" work by switching the red LED (on yaUsbIr-Board) on and off:
- `irsend SEND_ONCE yaUsbIR_control C_IR 8 0 C_END` turn *off*
- `irsend SEND_ONCE yaUsbIR_control C_IR 8 1 C_END` turn *on*

The most relevant command for you then probably is:
- `irsend SEND_ONCE yaUsbIR_control C_IR 1 1 0 C_END` to start learning the "Power-IR-Code"

See [Anleitung_yaUsbIR V3.4.pdf](./yaUsbir-Doc/Anleitung_yaUsbIR V3.4.pdf) for details and a list of all commands.

## udev
Write this into `/lib/udev/rules.d/99-yausbir.rules` to give normal user access to the USB-Device.
Other wise you have to start all tools (`xmode2`, `irrecord`, etc) as root.

```
SUBSYSTEM=="usb", ATTRS{idVendor}=="10c4", ATTRS{idProduct}=="876c", MODE="0666"
```
Simply reboot to take effect

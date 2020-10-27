#include <stdio.h>
#include <lirc_driver.h>   /* The overall include, mandatory, often sufficient.  */
#include <lirc/lirc_log.h>
#include <libusb-1.0/libusb.h>
#include <pthread.h>

/**
 * The global driver data that drivers etc are accessing.
 * Set by hw_choose_driver().
 */
struct driver drv;

libusb_device *yaUsbIrdevice = NULL;
libusb_device_handle *yaUsbIrDeviceHandle = NULL;

pthread_t pollThread = 0;
int myPipe[2] = { -1, -1 };

//static const logchannel_t logchannel = LOG_DRIVER;

#define CMD_NONE       0x00
#define CMD_IRDATA     0x01
#define CMD_COMDATA    0x02
#define CMD_SETCOMBAUD 0x03
#define CMD_GETCOMBAUD 0x04
#define CMD_GETIOS     0x05
#define CMD_GETIO      0x06
#define CMD_SETIOS     0x07
#define CMD_SETIO      0x08
#define IRRX_NODATA    0x0000
#define IRRX_F_POLL    6000000 // 6MHz
#define IRRX_CMD       0x7500

static void *pollFromUsb(void *args) {
	printf("Oli: pollFromUsb started\n");
	unsigned char buf[64];
	int bytesReceived;
	while (1) {
//		printf("Oli: pollFromUsb\n");
		memset(buf, 0, 64 * sizeof(char));
		int ret = libusb_interrupt_transfer(yaUsbIrDeviceHandle, 0x81, buf, sizeof(buf), &bytesReceived, 10000);
		if (ret == LIBUSB_ERROR_TIMEOUT) {
			continue;
		}
		if (ret != 0) {
			printf("Oli error: libusb_interrupt_transfer (returned=%d %s)\n", ret, libusb_error_name(ret));
			break;
		}
		if (ret == 0 && buf[0] == CMD_IRDATA) {
			for (int i = 2; i < bytesReceived; i += 2) {
				lirc_t rcvdata = (((int) buf[i]) & 0x7F) << 8; // MSB
				rcvdata |= (int) buf[i + 1]; // LSB
				rcvdata *= (int) buf[1]; // us step
				if (rcvdata == IRRX_NODATA) {
					break;
				}
				if ((buf[i] & 0x80) == 0x80) {
					rcvdata |= PULSE_BIT;
				}
				write(myPipe[1], &rcvdata, sizeof(rcvdata));
			}
		}
	}
	pthread_exit(NULL);
	printf("Oli: pollFromUsb exit\n");
	return NULL;
}

static void cleanup() {
	if (yaUsbIrDeviceHandle) {
		printf("Oli: releasing/closing USB-Device 1\n");
		libusb_release_interface(yaUsbIrDeviceHandle, 0);
		printf("Oli: releasing/closing USB-Device 2\n");
		libusb_close(yaUsbIrDeviceHandle);
		printf("Oli: releasing/closing USB-Device 3\n");
		libusb_exit(NULL);
		printf("Oli: releasing/closing USB-Device 4\n");
		yaUsbIrDeviceHandle = NULL;
		printf("Oli: releasing/closing USB-Device done\n");
	}

}


/**
 *  Function called to do basic driver setup.
 *  @param device String describing what device driver should
 *      communicate with. Often (but not always) a /dev/... path.
 *  @return 0 if everything is fine, else positive error code.
 */
static int open_func(const char *device) {
	printf("Oli: open_func (with device=%s)\n", device);

	if (strcmp(device, "auto") == 0) {
		drv.device = "10c4:876c";
	}

//	send_buffer_init();

	return 0;
}

/**
 * Function called for initializing the driver and the hardware.
 * Zero return value indicates failure, all other return values success.
 */
static int init_func(void) {
	printf("Oli: init_func\n");

	cleanup();
	rec_buffer_init();
	send_buffer_init();

	int err = libusb_init(NULL);
	if (err != 0) {
		printf("Oli error: libusb_init returned=%d\n", err);
		return 0;
	}
//	{
//		// print libusb version etc
//		const struct libusb_version *usbv = libusb_get_version();
//		printf("Oli: libusb version %d.%d.%d.%d (%s)\n", usbv->major, usbv->minor, usbv->micro, usbv->nano, usbv->describe);
//	}

	yaUsbIrdevice = NULL;
	{
		libusb_device **list = NULL;
		struct libusb_device_descriptor desc;
		int count = libusb_get_device_list(NULL, &list);
		for (int i = 0; i < count; i++) {
			libusb_device *device = list[i];
			if (libusb_get_device_descriptor(device, &desc) == 0) {
				if (desc.idVendor == 0x10c4 && desc.idProduct == 0x876c) {
					printf("Oli: found yaUsbIr-USB-Device\n");
					yaUsbIrdevice = device;
					break;
				}
			}
		}
	}
	if (yaUsbIrdevice == NULL) {
		printf("Oli error: can't find any yaUsbIr-USB-Device\n");
		return 0;
	}

	int ret = libusb_open(yaUsbIrdevice, &yaUsbIrDeviceHandle);
	if (ret != 0) {
		printf("Oli error: can't open USB-Device (returned=%d %s)\n", ret, libusb_error_name(ret));
		return 0;
	}

	libusb_detach_kernel_driver(yaUsbIrDeviceHandle, 0);
	ret = libusb_claim_interface(yaUsbIrDeviceHandle, 0);
	if (ret != 0) {
		printf("Oli error: can't claim USB interface (returned=%d %s)\n", ret, libusb_error_name(ret));
		return 0;
	}

	pipe(myPipe);
	drv.fd = myPipe[0];

	pthread_create(&pollThread, NULL, pollFromUsb, (void *)NULL);

	return 1;
}

/**
 * Function called when transmitting/receiving stops. Zero return value
 *  indicates failure, all other return values success.
 */
static int deinit_func(void) {
	printf("Oli: deinit_func\n");
	cleanup();
	return 1;
}

/**
 * Send data to the remote.
 * @param remote The remote used to send.
 * @param code Code(s) to send, a single code or the head of a
 *             list of codes.
 */
static int send_func(struct ir_remote *remote, struct ir_ncode *code) {
	printf("Oli: send_func\n");
	return 1;
}

/**
 * Receive data from remote. Might close device on error conditions.
 * @param The remote to read from.
 * @return Formatted, statically allocated string with decoded
 *         data: "remote-name code-name code repetitions"
 */
static char* rec_func(struct ir_remote *remotes) {
//	printf("Oli: rec_func (remotes.name=%s)\n", remotes->name);
	if (!rec_buffer_clear()) {
		/* handle errors */
		return NULL;
	}
	return decode_all(remotes);
}

///**
// * TODO
// */
//static int decode_func(struct ir_remote *remote, struct decode_ctx_t *ctx) {
//	printf("Oli: decode_func\n");
//	return receive_decode(remote, ctx);
//}

/**
* Get length of next pulse/space from hardware.
* @param timeout Max time to wait (us).
* @return Length of pulse in lower 24 bits (us). PULSE_BIT
* is set to reflect if this is a pulse or space. 0
* indicates errors.
*/
static lirc_t readdata(lirc_t timeout) {
//	printf("Oli: readdata (with timeout=%d ms)\n", timeout);

	if (!waitfordata(timeout)) {
		return 0;
	}

	lirc_t res = 0;
	int n = read(drv.fd, &res, sizeof(res)); // read from fifo
	if (n != sizeof(res)) {
		res = 0;
	}
	return res;
}

/**< Hard closing, returns 0 on OK.*/
static int close_func(void) {
	printf("Oli: close_func\n");
	return 0;
}

const struct driver hw_oli_yausbir = {
	.device         = NULL,                // USB "vendor:product" in Hex
	.features       = LIRC_CAN_REC_MODE2,
	.send_mode      = 0,
	.rec_mode       = LIRC_MODE_MODE2,
	.code_length    = 0,
	.open_func      = open_func,
	.init_func      = init_func,
	.deinit_func    = deinit_func,
	.send_func      = send_func,
	.rec_func       = rec_func,
	.decode_func    = receive_decode,   // call LIRC's default implementation
	.drvctl_func    = default_drvctl,
	.readdata       = readdata,
	.close_func     = close_func,
	.name           = "oli_yausbir",
	.api_version    = 3,
	.driver_version = "0.0.0",
	.info           = "LIRC-Plugin-Driver for 'yaUsbIr V3' (for Lirc 0.10.0+)",
	.resolution     = 1,
	.device_hint    = "auto"
};

const struct driver* hardwares[] = { &hw_oli_yausbir, NULL };

#include <lirc_driver.h>   /* The overall include, mandatory, often sufficient.  */
#include <lirc/lirc_log.h>

/**
 * The global driver data that drivers etc are accessing.
 * Set by hw_choose_driver().
 */
struct driver drv;


/**
 *  Function called to do basic driver setup.
 *  @param device String describing what device driver should
 *      communicate with. Often (but not always) a /dev/... path.
 *  @return 0 if everything is fine, else positive error code.
 */
static int open_func(const char *device) {
	printf("Oli: open_func (with device=%s)\n", device);
	drv.device = "auto-detected";
//	send_buffer_init();
	return 0;
}

/**
 * Function called for initializing the driver and the hardware.
 * Zero return value indicates failure, all other return values success.
 */
static int init_func(void) {
	printf("Oli: init_func\n");

	FILE *dummy = fopen("/dev/zero", "r");
	drv.fd = fileno(dummy);
	return 1;
}

/**
 * Function called when transmitting/receiving stops. Zero return value
 *  indicates failure, all other return values success.
 */
static int deinit_func(void) {
	printf("Oli: init_func\n");
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
	printf("Oli: rec_func (remotes.name\n");
	if (!rec_buffer_clear()) {
		/* handle errors */
		return NULL;
	}
	return decode_all(remotes);
}

/**
 * TODO
 */
static int decode_func(struct ir_remote *remote, struct decode_ctx_t *ctx) {
	printf("Oli: decode_func\n");
	return receive_decode(remote, ctx);
}

/**
 * Generic driver control function with semantics as defined by driver
 * Returns 0 on success, else a positive error code.
 */
//static int drvctl_func(unsigned int cmd, void* arg) {
//	return 0;
//}


static int seq = 0;
static int pulseLength = 0;

/**
* Get length of next pulse/space from hardware.
* @param timeout Max time to wait (us).
* @return Length of pulse in lower 24 bits (us). PULSE_BIT
* is set to reflect if this is a pulse or space. 0
* indicates errors.
*/
static lirc_t readdata(lirc_t timeout) {
	printf("Oli: readdata (with timeout=%d ms)\n", timeout);
	seq++;
	pulseLength = 100 + (seq * 20);
	if (seq >= 10) {
		seq = 0;
		pulseLength = 1000000;
	}
	usleep(pulseLength);
	return pulseLength | ((seq & 1) ? PULSE_BIT : 0);
}

/**< Hard closing, returns 0 on OK.*/
static int close_func(void) {
	printf("Oli: close_func\n");
	return 0;
}

const struct driver hw_oli_yausbir = {
	.device         = NULL,
	.features       = LIRC_CAN_REC_MODE2,
	.send_mode      = 0,
	.rec_mode       = LIRC_MODE_MODE2,
	.code_length    = 0,
	.open_func      = open_func,
	.init_func      = init_func,
	.deinit_func    = deinit_func,
	.send_func      = send_func,
	.rec_func       = rec_func,
	.decode_func    = decode_func,
	.drvctl_func    = default_drvctl,
	.readdata       = readdata,
	.close_func     = close_func,
	.name           = "oli_yausbir",
	.api_version    = 3,
	.driver_version = "0.0.0",
	.info           = "Oli's approach for a LIRC-Driver for Lirc 0.10.0+",
	.resolution     = 1,
	.device_hint    = "auto"
};

const struct driver* hardwares[] = { &hw_oli_yausbir, NULL };

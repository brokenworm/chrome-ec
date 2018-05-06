/* Copyright 2018 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "atomic.h"
#include "clock_chip.h"
#include "console.h"
#include "ec_commands.h"
#include "fan_chip.h"
#include "gpio.h"
#include "hooks.h"
#include "host_command.h"
#include "mkbp_event.h"
#include "registers.h"
#include "task.h"
#include "timer.h"
#include "util.h"

#if !(DEBUG_CEC)
#define CPRINTF(...)
#define CPRINTS(...)
#else
#define CPRINTF(format, args...) cprintf(CC_CEC, format, ## args)
#define CPRINTS(format, args...) cprints(CC_CEC, format, ## args)
#endif

/* Time in us to timer clock ticks */
#define APB1_TICKS(t) ((t) * apb1_freq_div_10k / 100)
#if DEBUG_CEC
/* Timer clock ticks to us */
#define APB1_US(ticks) (100*(ticks)/apb1_freq_div_10k)
#endif

/* CEC broadcast address. Also the highest possible CEC address */
#define CEC_BROADCAST_ADDR 15

/*
 * The CEC specification requires at least one and a maximum of
 * five resends attempts
 */
#define CEC_MAX_RESENDS 5

/* Free time timing (us). */
#define NOMINAL_BIT_TIME APB1_TICKS(2400)
#define FREE_TIME_RS	(3 * (NOMINAL_BIT_TIME)) /* Resend */
#define FREE_TIME_NI	(5 * (NOMINAL_BIT_TIME)) /* New initiator */

/* Start bit timing (us) */
#define START_BIT_LOW		APB1_TICKS(3700)
#define START_BIT_MIN_LOW	APB1_TICKS(3500)
#define START_BIT_MAX_LOW	APB1_TICKS(3900)
#define START_BIT_HIGH		APB1_TICKS(800)
#define START_BIT_MIN_DURATION	APB1_TICKS(4300)
#define START_BIT_MAX_DURATION	APB1_TICKS(5700)

/* Data bit timing (us)  */
#define DATA_ZERO_LOW		APB1_TICKS(1500)
#define DATA_ZERO_MIN_LOW	APB1_TICKS(1300)
#define DATA_ZERO_MAX_LOW	APB1_TICKS(1700)
#define DATA_ZERO_HIGH		APB1_TICKS(900)
#define DATA_ZERO_MIN_DURATION	APB1_TICKS(2050)
#define DATA_ZERO_MAX_DURATION	APB1_TICKS(2750)

#define DATA_ONE_LOW		APB1_TICKS(600)
#define DATA_ONE_MIN_LOW	APB1_TICKS(400)
#define DATA_ONE_MAX_LOW	APB1_TICKS(800)
#define DATA_ONE_HIGH		APB1_TICKS(1800)
#define DATA_ONE_MIN_DURATION	APB1_TICKS(2050)
#define DATA_ONE_MAX_DURATION	APB1_TICKS(2750)

/* Time from low that it should be safe to sample an ACK */
#define NOMINAL_SAMPLE_TIME APB1_TICKS(1050)

#define DATA_TIME(type, data) ((data) ? (DATA_ONE_ ## type) : \
					(DATA_ZERO_ ## type))
#define DATA_HIGH(data) DATA_TIME(HIGH, data)
#define DATA_LOW(data) DATA_TIME(LOW, data)

/*
 * CEC state machine states. Each state typically takes action on entry and
 * timeouts. INITIATIOR states are used for sending, FOLLOWER states are used
 *  for receiving.
 */
enum cec_state {
	CEC_STATE_IDLE = 0,
	CEC_STATE_INITIATOR_FREE_TIME,
	CEC_STATE_INITIATOR_START_LOW,
	CEC_STATE_INITIATOR_START_HIGH,
	CEC_STATE_INITIATOR_HEADER_INIT_LOW,
	CEC_STATE_INITIATOR_HEADER_INIT_HIGH,
	CEC_STATE_INITIATOR_HEADER_DEST_LOW,
	CEC_STATE_INITIATOR_HEADER_DEST_HIGH,
	CEC_STATE_INITIATOR_DATA_LOW,
	CEC_STATE_INITIATOR_DATA_HIGH,
	CEC_STATE_INITIATOR_EOM_LOW,
	CEC_STATE_INITIATOR_EOM_HIGH,
	CEC_STATE_INITIATOR_ACK_LOW,
	CEC_STATE_INITIATOR_ACK_HIGH,
	CEC_STATE_INITIATOR_ACK_VERIFY,
};

/* CEC message during transfer */
struct cec_msg_transfer {
	/* The CEC message */
	uint8_t buf[MAX_CEC_MSG_LEN];
	/* Bit offset  */
	uint8_t bit;
	/* Byte offset */
	uint8_t byte;
};

/* Transfer buffer and states */
struct cec_tx {
	/* Outgoing message */
	struct cec_msg_transfer msgt;
	/* Message length */
	uint8_t len;
	/* Number of resends attempted in current send */
	uint8_t resends;
	/* Acknowledge received from sink? */
	uint8_t ack;
};

/* Single state for CEC. We are INITIATOR, FOLLOWER or IDLE */
static enum cec_state cec_state;

/* Parameters and buffer for initiator (sender) state */
static struct cec_tx cec_tx;

/* Events to send to AP */
static uint32_t cec_events;

/* APB1 frequency. Store divided by 10k to avoid some runtime divisions */
static uint32_t apb1_freq_div_10k;

static void send_mkbp_event(uint32_t event)
{
	atomic_or(&cec_events, event);
	mkbp_send_event(EC_MKBP_EVENT_CEC);
}

static void tmr_oneshot_start(int timeout)
{
	int mdl = NPCX_MFT_MODULE_1;

	NPCX_TCNT1(mdl) = timeout;
	SET_FIELD(NPCX_TCKC(mdl), NPCX_TCKC_C1CSEL_FIELD, 1);
}

static void tmr2_start(int timeout)
{
	int mdl = NPCX_MFT_MODULE_1;

	NPCX_TCNT2(mdl) = timeout;
	SET_FIELD(NPCX_TCKC(mdl), NPCX_TCKC_C2CSEL_FIELD, 1);
}

static void tmr2_stop(void)
{
	int mdl = NPCX_MFT_MODULE_1;

	SET_FIELD(NPCX_TCKC(mdl), NPCX_TCKC_C2CSEL_FIELD, 0);
}

static int msgt_get_bit(const struct cec_msg_transfer *msgt)
{
	if (msgt->byte >= MAX_CEC_MSG_LEN)
		return 0;

	return msgt->buf[msgt->byte] & (1 << (7 - msgt->bit));
}

static void msgt_inc_bit(struct cec_msg_transfer *msgt)
{
	if (++(msgt->bit) == 8) {
		if (msgt->byte >= MAX_CEC_MSG_LEN)
			return;
		msgt->bit = 0;
		msgt->byte++;
	}
}

static int msgt_is_eom(const struct cec_msg_transfer *msgt, int len)
{
	if (msgt->bit)
		return 0;
	return (msgt->byte == len);
}

void enter_state(enum cec_state new_state)
{
	int gpio = -1, timeout = -1;

	cec_state = new_state;
	switch (new_state) {
	case CEC_STATE_IDLE:
		cec_tx.msgt.bit = 0;
		cec_tx.msgt.byte = 0;
		break;
	case CEC_STATE_INITIATOR_FREE_TIME:
		gpio = 1;
		if (cec_tx.resends)
			timeout = FREE_TIME_RS;
		else
			timeout = FREE_TIME_NI;
		break;
	case CEC_STATE_INITIATOR_START_LOW:
		cec_tx.msgt.bit = 0;
		cec_tx.msgt.byte = 0;
		gpio = 0;
		timeout = START_BIT_LOW;
		break;
	case CEC_STATE_INITIATOR_START_HIGH:
		gpio = 1;
		timeout = START_BIT_HIGH;
		break;
	case CEC_STATE_INITIATOR_HEADER_INIT_LOW:
	case CEC_STATE_INITIATOR_HEADER_DEST_LOW:
	case CEC_STATE_INITIATOR_DATA_LOW:
		gpio = 0;
		timeout = DATA_LOW(msgt_get_bit(&cec_tx.msgt));
		break;
	case CEC_STATE_INITIATOR_HEADER_INIT_HIGH:
	case CEC_STATE_INITIATOR_HEADER_DEST_HIGH:
	case CEC_STATE_INITIATOR_DATA_HIGH:
		gpio = 1;
		timeout = DATA_HIGH(msgt_get_bit(&cec_tx.msgt));
		break;
	case CEC_STATE_INITIATOR_EOM_LOW:
		gpio = 0;
		timeout = DATA_LOW(msgt_is_eom(&cec_tx.msgt, cec_tx.len));
		break;
	case CEC_STATE_INITIATOR_EOM_HIGH:
		gpio = 1;
		timeout = DATA_HIGH(msgt_is_eom(&cec_tx.msgt, cec_tx.len));
		break;
	case CEC_STATE_INITIATOR_ACK_LOW:
		gpio = 0;
		timeout = DATA_LOW(1);
		break;
	case CEC_STATE_INITIATOR_ACK_HIGH:
		gpio = 1;
		/* Aim for the middle of the safe sample time */
		timeout = (DATA_ONE_LOW + DATA_ZERO_LOW)/2 - DATA_ONE_LOW;
		break;
	case CEC_STATE_INITIATOR_ACK_VERIFY:
		cec_tx.ack = !gpio_get_level(CEC_GPIO_OUT);
		if ((cec_tx.msgt.buf[0] & 0x0f) == CEC_BROADCAST_ADDR) {
			/*
			 * We are sending a broadcast. Any follower can
			 * can NAK a broadcast message the same way they
			 * would ACK a direct message
			 */
			cec_tx.ack = !cec_tx.ack;
		}
		/*
		 * We are at the safe sample time. Wait
		 * until the end of this bit
		 */
		timeout = NOMINAL_BIT_TIME - NOMINAL_SAMPLE_TIME;
		break;
	/* No default case, since all states must be handled explicitly */
	}

	if (gpio >= 0)
		gpio_set_level(CEC_GPIO_OUT, gpio);
	if (timeout >= 0)
		tmr_oneshot_start(timeout);
}

static void cec_event_timeout(void)
{
	switch (cec_state) {
	case CEC_STATE_IDLE:
		break;
	case CEC_STATE_INITIATOR_FREE_TIME:
		enter_state(CEC_STATE_INITIATOR_START_LOW);
		break;
	case CEC_STATE_INITIATOR_START_LOW:
		enter_state(CEC_STATE_INITIATOR_START_HIGH);
		break;
	case CEC_STATE_INITIATOR_START_HIGH:
		enter_state(CEC_STATE_INITIATOR_HEADER_INIT_LOW);
		break;
	case CEC_STATE_INITIATOR_HEADER_INIT_LOW:
		enter_state(CEC_STATE_INITIATOR_HEADER_INIT_HIGH);
		break;
	case CEC_STATE_INITIATOR_HEADER_INIT_HIGH:
		msgt_inc_bit(&cec_tx.msgt);
		if (cec_tx.msgt.bit == 4)
			enter_state(CEC_STATE_INITIATOR_HEADER_DEST_LOW);
		else
			enter_state(CEC_STATE_INITIATOR_HEADER_INIT_LOW);
		break;
	case CEC_STATE_INITIATOR_HEADER_DEST_LOW:
		enter_state(CEC_STATE_INITIATOR_HEADER_DEST_HIGH);
		break;
	case CEC_STATE_INITIATOR_HEADER_DEST_HIGH:
		msgt_inc_bit(&cec_tx.msgt);
		if (cec_tx.msgt.byte == 1)
			enter_state(CEC_STATE_INITIATOR_EOM_LOW);
		else
			enter_state(CEC_STATE_INITIATOR_HEADER_DEST_LOW);
		break;
	case CEC_STATE_INITIATOR_EOM_LOW:
		enter_state(CEC_STATE_INITIATOR_EOM_HIGH);
		break;
	case CEC_STATE_INITIATOR_EOM_HIGH:
		enter_state(CEC_STATE_INITIATOR_ACK_LOW);
		break;
	case CEC_STATE_INITIATOR_ACK_LOW:
		enter_state(CEC_STATE_INITIATOR_ACK_HIGH);
		break;
	case CEC_STATE_INITIATOR_ACK_HIGH:
		enter_state(CEC_STATE_INITIATOR_ACK_VERIFY);
		break;
	case CEC_STATE_INITIATOR_ACK_VERIFY:
		if (cec_tx.ack) {
			if (!msgt_is_eom(&cec_tx.msgt, cec_tx.len)) {
				/* More data in this frame */
				enter_state(CEC_STATE_INITIATOR_DATA_LOW);
			} else {
				/* Transfer completed successfully */
				cec_tx.len = 0;
				cec_tx.resends = 0;
				enter_state(CEC_STATE_IDLE);
				send_mkbp_event(EC_MKBP_CEC_SEND_OK);
			}
		} else {
			if (cec_tx.resends < CEC_MAX_RESENDS) {
				/* Resend */
				cec_tx.resends++;
				enter_state(CEC_STATE_INITIATOR_FREE_TIME);
			} else {
				/* Transfer failed */
				cec_tx.len = 0;
				cec_tx.resends = 0;
				enter_state(CEC_STATE_IDLE);
				send_mkbp_event(EC_MKBP_CEC_SEND_FAILED);
			}
		}
		break;
	case CEC_STATE_INITIATOR_DATA_LOW:
		enter_state(CEC_STATE_INITIATOR_DATA_HIGH);
		break;
	case CEC_STATE_INITIATOR_DATA_HIGH:
		msgt_inc_bit(&cec_tx.msgt);
		if (cec_tx.msgt.bit == 0)
			enter_state(CEC_STATE_INITIATOR_EOM_LOW);
		else
			enter_state(CEC_STATE_INITIATOR_DATA_LOW);
		break;
	}
}

static void cec_event_tx(void)
{
	enter_state(CEC_STATE_INITIATOR_FREE_TIME);
}

static void cec_isr(void)
{
	int mdl = NPCX_MFT_MODULE_1;
	uint8_t events;

	/* Retrieve events NPCX_TECTRL_TAXND */
	events = GET_FIELD(NPCX_TECTRL(mdl), FIELD(0, 4));

	/* Timer event for bit-flipping */
	if (events & (1 << NPCX_TECTRL_TCPND))
		cec_event_timeout();

	/* Oneshot timer, a transfer has been initiated from AP */
	if (events & (1 << NPCX_TECTRL_TDPND)) {
		tmr2_stop();
		cec_event_tx();
	}

	/* Clear handled events */
	SET_FIELD(NPCX_TECLR(mdl), FIELD(0, 4), events);
}
DECLARE_IRQ(NPCX_IRQ_MFT_1, cec_isr, 4);

static int cec_send(const uint8_t *msg, uint8_t len)
{
	int i;

	if (cec_tx.len != 0)
		return -1;

	cec_tx.len = len;

	CPRINTS("Send CEC:");
	for (i = 0; i < len && i < MAX_CEC_MSG_LEN; i++)
		CPRINTS(" 0x%02x", msg[i]);

	memcpy(cec_tx.msgt.buf, msg, len);

	/* Elevate to interrupt context */
	tmr2_start(0);

	return 0;
}

static int hc_cec_write(struct host_cmd_handler_args *args)
{
	const struct ec_params_cec_write *params = args->params;

	if (args->params_size == 0 || args->params_size > MAX_CEC_MSG_LEN)
		return EC_RES_INVALID_PARAM;

	if (cec_send(params->msg, args->params_size) != 0)
		return EC_RES_BUSY;

	return EC_RES_SUCCESS;
}
DECLARE_HOST_COMMAND(EC_CMD_CEC_WRITE_MSG, hc_cec_write, EC_VER_MASK(0));

static int cec_get_next_event(uint8_t *out)
{
	uint32_t event_out = atomic_read_clear(&cec_events);

	memcpy(out, &event_out, sizeof(event_out));

	return sizeof(event_out);
}
DECLARE_EVENT_SOURCE(EC_MKBP_EVENT_CEC, cec_get_next_event);

static void cec_init(void)
{
	int mdl = NPCX_MFT_MODULE_1;

	/* APB1 is the clock we base the timers on */
	apb1_freq_div_10k = clock_get_apb1_freq()/10000;

	/* Ensure Multi-Function timer is powered up. */
	CLEAR_BIT(NPCX_PWDWN_CTL(mdl), NPCX_PWDWN_CTL1_MFT1_PD);

	/* Mode 2 - Dual-input capture */
	SET_FIELD(NPCX_TMCTRL(mdl), NPCX_TMCTRL_MDSEL_FIELD, NPCX_MFT_MDSEL_2);

	/* Enable timer interrupts */
	SET_BIT(NPCX_TIEN(mdl), NPCX_TIEN_TCIEN);
	SET_BIT(NPCX_TIEN(mdl), NPCX_TIEN_TDIEN);

	/* Enable multifunction timer interrupt */
	task_enable_irq(NPCX_IRQ_MFT_1);

	CPRINTS("CEC initialized");
}
DECLARE_HOOK(HOOK_INIT, cec_init, HOOK_PRIO_LAST);

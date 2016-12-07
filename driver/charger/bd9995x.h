/* Copyright 2016 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * ROHM BD9995X battery charger driver.
 */

#ifndef __CROS_EC_BD9995X_H
#define __CROS_EC_BD9995X_H

#define BD9995X_ADDR		0x12 /* 7bit address 0001_001 */
#define I2C_ADDR_CHARGER	BD9995X_ADDR

#ifdef CONFIG_CHARGER_BD99955
#define BD9995X_CHARGER_NAME	"bd99955"
#define BD9995X_CHIP_ID		0x221
#elif defined(CONFIG_CHARGER_BD99956)
#define BD9995X_CHARGER_NAME	"bd99956"
#define BD9995X_CHIP_ID		0x331
#endif

/* BD9995X commands to change the command code map */
enum bd9995x_command {
	BD9995X_BAT_CHG_COMMAND,
	BD9995X_EXTENDED_COMMAND,
	BD9995X_DEBUG_COMMAND,
	BD9995X_INVALID_COMMAND
};

enum bd9995x_charge_port {
	BD9995X_CHARGE_PORT_VBUS,
	BD9995X_CHARGE_PORT_VCC,
	BD9995X_CHARGE_PORT_BOTH,
};

/* Charger parameters */
#define CHARGER_NAME	BD9995X_CHARGER_NAME
#define CHARGE_V_MAX	19200
#define CHARGE_V_MIN	3072
#define CHARGE_V_STEP	16
#define CHARGE_I_MAX	16320
#define CHARGE_I_MIN	128
#define CHARGE_I_OFF	0
#define CHARGE_I_STEP	64
#define INPUT_I_MAX	16352
#define INPUT_I_MIN	512
#define INPUT_I_STEP	32

/* Min. charge current w/ no battery to prevent collapse */
#define BD9995X_NO_BATTERY_CHARGE_I_MIN	512

/* VSYSREG settings */
#define BD9995X_DISCHARGE_VSYSREG	8960
#define BD9995X_CHARGE_VSYSREG		6144

/*
 * BC1.2 minimum voltage threshold.
 * BC1.2 charging port output voltage range is 4.75V to 5.25V,
 * BD9995X Anti-Collapse Threshold Voltage Accuracy is -100mV to +100mV,
 * and Delta of 50mV.
 */
#define BD9995X_BC12_MIN_VOLTAGE	4600

/* Battery Charger Commands */
#define BD9995X_CMD_CHG_CURRENT		0x14
#define BD9995X_CMD_CHG_VOLTAGE		0x15
#define BD9995X_CMD_IBUS_LIM_SET	0x3C
#define BD9995X_CMD_ICC_LIM_SET		0x3D
#define BD9995X_CMD_PROTECT_SET		0x3E
#define BD9995X_CMD_MAP_SET		0x3F

/* Extended commands */
#define BD9995X_CMD_CHGSTM_STATUS	0x00
#define BD9995X_CMD_VBAT_VSYS_STATUS	0x01
#define BD9995X_CMD_VBUS_VCC_STATUS	0x02
#define BD9995X_CMD_VBUS_VCC_STATUS_VCC_DETECT  (1 << 8)
#define BD9995X_CMD_VBUS_VCC_STATUS_VBUS_DETECT (1 << 0)

#define BD9995X_CMD_CHGOP_STATUS	0x03
#define BD9995X_CMD_CHGOP_STATUS_BATTEMP2	(1 << 10)
#define BD9995X_CMD_CHGOP_STATUS_BATTEMP1	(1 << 9)
#define BD9995X_CMD_CHGOP_STATUS_BATTEMP0	(1 << 8)
#define BD9995X_BATTTEMP_MASK			0x700
#define BD9995X_CMD_CHGOP_STATUS_BATTEMP_ROOMTEMP	0
#define BD9995X_CMD_CHGOP_STATUS_BATTEMP_HOT1		1
#define BD9995X_CMD_CHGOP_STATUS_BATTEMP_HOT2		2
#define BD9995X_CMD_CHGOP_STATUS_BATTEMP_HOT3		3
#define BD9995X_CMD_CHGOP_STATUS_BATTEMP_COLD1		4
#define BD9995X_CMD_CHGOP_STATUS_BATTEMP_COLD2		5
#define BD9995X_CMD_CHGOP_STATUS_BATTEMP_DISABLE	6
#define BD9995X_CMD_CHGOP_STATUS_BATTEMP_BATOPEN	7
#define BD9995X_CMD_CHGOP_STATUS_RBOOST_UV	(1 << 1)

#define BD9995X_CMD_WDT_STATUS		0x04
#define BD9995X_CMD_CUR_ILIM_VAL	0x05
#define BD9995X_CMD_SEL_ILIM_VAL	0x06
#define BD9995X_CMD_EXT_IBUS_LIM_SET	0x07
#define BD9995X_CMD_EXT_ICC_LIM_SET	0x08
#define BD9995X_CMD_IOTG_LIM_SET	0x09
#define BD9995X_CMD_VIN_CTRL_SET	0x0A
#define BD9995X_CMD_VIN_CTRL_SET_PP_BOTH_THRU  (1 << 11)
#define BD9995X_CMD_VIN_CTRL_SET_VBUS_PRIORITY (1 << 7)
#define BD9995X_CMD_VIN_CTRL_SET_VBUS_EN       (1 << 6)
#define BD9995X_CMD_VIN_CTRL_SET_VCC_EN        (1 << 5)

#define BD9995X_CMD_CHGOP_SET1		0x0B
#define BD9995X_CMD_CHGOP_SET1_DCP_2500_SEL    (1 << 15)
#define BD9995X_CMD_CHGOP_SET1_SDP_500_SEL     (1 << 14)
#define BD9995X_CMD_CHGOP_SET1_ILIM_AUTO_DISEN (1 << 13)
#define BD9995X_CMD_CHGOP_SET1_VCC_BC_DISEN    (1 << 11)
#define BD9995X_CMD_CHGOP_SET1_VBUS_BC_DISEN   (1 << 10)
#define BD9995X_CMD_CHGOP_SET1_SDP_CHG_TRIG_EN (1 << 9)
#define BD9995X_CMD_CHGOP_SET1_SDP_CHG_TRIG    (1 << 8)

#define BD9995X_CMD_CHGOP_SET2		0x0C
#define BD9995X_CMD_CHGOP_SET2_BATT_LEARN	 (1 << 8)
#define BD9995X_CMD_CHGOP_SET2_CHG_EN		 (1 << 7)
#define BD9995X_CMD_CHGOP_SET2_USB_SUS		 (1 << 6)
#define BD9995X_CMD_CHGOP_SET2_DCDC_CLK_SEL      (3 << 2)
#define BD9995X_CMD_CHGOP_SET2_DCDC_CLK_SEL_600  (0 << 2)
#define BD9995X_CMD_CHGOP_SET2_DCDC_CLK_SEL_857  (1 << 2)
#define BD9995X_CMD_CHGOP_SET2_DCDC_CLK_SEL_1000 (2 << 2)
#define BD9995X_CMD_CHGOP_SET2_DCDC_CLK_SEL_1200 (3 << 2)

#define BD9995X_CMD_VBUSCLPS_TH_SET	0x0D
#define BD9995X_CMD_VCCCLPS_TH_SET	0x0E
#define BD9995X_CMD_CHGWDT_SET		0x0F
#define BD9995X_CMD_BATTWDT_SET		0x10
#define BD9995X_CMD_VSYSREG_SET		0x11
#define BD9995X_CMD_VSYSVAL_THH_SET	0x12
#define BD9995X_CMD_VSYSVAL_THL_SET	0x13
#define BD9995X_CMD_ITRICH_SET		0x14

#define BD9995X_CMD_IPRECH_SET		0x15
#define BD9995X_IPRECH_MAX		1024

#define BD9995X_CMD_ICHG_SET		0x16
#define BD9995X_CMD_ITERM_SET		0x17
#define BD9995X_CMD_VPRECHG_TH_SET	0x18
#define BD9995X_CMD_VRBOOST_SET		0x19
#define BD9995X_CMD_VFASTCHG_REG_SET1	0x1A
#define BD9995X_CMD_VFASTCHG_REG_SET2	0x1B
#define BD9995X_CMD_VFASTCHG_REG_SET3	0x1C
#define BD9995X_CMD_VRECHG_SET		0x1D
#define BD9995X_CMD_VBATOVP_SET		0x1E
#define BD9995X_CMD_IBATSHORT_SET	0x1F
#define BD9995X_CMD_PROCHOT_CTRL_SET	0x20
#define BD9995X_CMD_PROCHOT_CTRL_SET_PROCHOT_EN4	(1 << 4)
#define BD9995X_CMD_PROCHOT_CTRL_SET_PROCHOT_EN3	(1 << 3)
#define BD9995X_CMD_PROCHOT_CTRL_SET_PROCHOT_EN2	(1 << 2)
#define BD9995X_CMD_PROCHOT_CTRL_SET_PROCHOT_EN1	(1 << 1)
#define BD9995X_CMD_PROCHOT_CTRL_SET_PROCHOT_EN0	(1 << 0)

#define BD9995X_CMD_PROCHOT_ICRIT_SET	0x21
#define BD9995X_CMD_PROCHOT_INORM_SET	0x22
#define BD9995X_CMD_PROCHOT_IDCHG_SET	0x23
#define BD9995X_CMD_PROCHOT_VSYS_SET	0x24
#define BD9995X_CMD_PMON_IOUT_CTRL_SET	0x25
#define BD9995X_CMD_PMON_IOUT_CTRL_SET_IMON_INSEL	(1 << 9)
#define BD9995X_CMD_PMON_IOUT_CTRL_SET_PMON_INSEL	(1 << 8)
#define BD9995X_CMD_PMON_IOUT_CTRL_SET_IOUT_OUT_EN	(1 << 7)
#define BD9995X_CMD_PMON_IOUT_CTRL_SET_IOUT_SOURCE_SEL	(1 << 6)
#define BD9995X_CMD_PMON_IOUT_CTRL_SET_IOUT_GAIN_SET_MASK	0x30
#define BD9995X_CMD_PMON_IOUT_CTRL_SET_IOUT_GAIN_SET_40V	0x03
#define BD9995X_CMD_PMON_IOUT_CTRL_SET_IOUT_GAIN_SET_20V	0x02
#define BD9995X_CMD_PMON_IOUT_CTRL_SET_IOUT_GAIN_SET_10V	0x01
#define BD9995X_CMD_PMON_IOUT_CTRL_SET_IOUT_GAIN_SET_05V	0x00
#define BD9995X_CMD_PMON_IOUT_CTRL_SET_PMON_OUT_EN	(1 << 3)
#define BD9995X_CMD_PMON_IOUT_CTRL_SET_PMON_GAIN_SET_MASK	0x07
#define BD9995X_CMD_PMON_IOUT_CTRL_SET_PMON_GAIN_SET_64UAW	0x06
#define BD9995X_CMD_PMON_IOUT_CTRL_SET_PMON_GAIN_SET_32UAW	0x05
#define BD9995X_CMD_PMON_IOUT_CTRL_SET_PMON_GAIN_SET_16UAW	0x04
#define BD9995X_CMD_PMON_IOUT_CTRL_SET_PMON_GAIN_SET_08UAW	0x03
#define BD9995X_CMD_PMON_IOUT_CTRL_SET_PMON_GAIN_SET_04UAW	0x02
#define BD9995X_CMD_PMON_IOUT_CTRL_SET_PMON_GAIN_SET_02UAW	0x01
#define BD9995X_CMD_PMON_IOUT_CTRL_SET_PMON_GAIN_SET_01UAW	0x00
#define BD9995X_PMON_IOUT_ADC_READ_COUNT	100

#define BD9995X_CMD_PMON_DACIN_VAL	0x26
#define BD9995X_CMD_IOUT_DACIN_VAL	0x27
#define BD9995X_CMD_VCC_UCD_SET		0x28
/* Bits for both VCC_UCD_SET and VBUS_UCD_SET regs */
#define BD9995X_CMD_UCD_SET_BCSRETRY           (1 << 12)
#define BD9995X_CMD_UCD_SET_USBDETEN           (1 << 7)
#define BD9995X_CMD_UCD_SET_USB_SW_EN          (1 << 1)

#define BD9995X_CMD_VCC_UCD_STATUS	0x29
/* Bits for both VCC_UCD_STATUS and VBUS_UCD_STATUS regs */
#define BD9995X_CMD_UCD_STATUS_DCDFAIL	(1 << 15)
#define BD9995X_CMD_UCD_STATUS_CHGPORT1	(1 << 13)
#define BD9995X_CMD_UCD_STATUS_CHGPORT0	(1 << 12)
#define BD9995X_CMD_UCD_STATUS_PUPDET	(1 << 11)
#define BD9995X_CMD_UCD_STATUS_CHGDET	(1 << 6)
#define BD9995X_TYPE_MASK		(BD9995X_CMD_UCD_STATUS_DCDFAIL | \
					 BD9995X_CMD_UCD_STATUS_CHGPORT1 | \
					 BD9995X_CMD_UCD_STATUS_CHGPORT0 | \
					 BD9995X_CMD_UCD_STATUS_PUPDET | \
					 BD9995X_CMD_UCD_STATUS_CHGDET)

/* BC1.2 chargers */
#define BD9995X_TYPE_CDP		(BD9995X_CMD_UCD_STATUS_CHGPORT1 | \
					 BD9995X_CMD_UCD_STATUS_CHGDET)
#define BD9995X_TYPE_DCP		(BD9995X_CMD_UCD_STATUS_CHGPORT1 | \
					 BD9995X_CMD_UCD_STATUS_CHGPORT0 | \
					 BD9995X_CMD_UCD_STATUS_CHGDET)
#define BD9995X_TYPE_SDP		(BD9995X_CMD_UCD_STATUS_CHGPORT0)
/* non-standard BC1.2 chargers */
#define BD9995X_TYPE_OTHER		(BD9995X_CMD_UCD_STATUS_DCDFAIL | \
					 BD9995X_CMD_UCD_STATUS_CHGPORT1 | \
					 BD9995X_CMD_UCD_STATUS_CHGPORT0 | \
					 BD9995X_CMD_UCD_STATUS_CHGDET)
#define BD9995X_TYPE_PUP_PORT		(BD9995X_CMD_UCD_STATUS_DCDFAIL | \
					 BD9995X_CMD_UCD_STATUS_CHGPORT0 | \
					 BD9995X_CMD_UCD_STATUS_PUPDET)
/* Open ports */
#define BD9995X_TYPE_OPEN_PORT		(BD9995X_CMD_UCD_STATUS_DCDFAIL | \
					 BD9995X_CMD_UCD_STATUS_CHGPORT0)
#define BD9995X_TYPE_VBUS_OPEN		0

#define BD9995X_CMD_VCC_IDD_STATUS	0x2A
#define BD9995X_CMD_VCC_UCD_FCTRL_SET	0x2B
#define BD9995X_CMD_VCC_UCD_FCTRL_EN	0x2C
#define BD9995X_CMD_VBUS_UCD_SET	0x30
#define BD9995X_CMD_VBUS_UCD_STATUS	0x31
#define BD9995X_CMD_VBUS_IDD_STATUS	0x32
#define BD9995X_CMD_VBUS_UCD_FCTRL_SET	0x33
#define BD9995X_CMD_VBUS_UCD_FCTRL_EN	0x34
#define BD9995X_CMD_CHIP_ID		0x38
#define BD9995X_CMD_CHIP_REV		0x39
#define BD9995X_CMD_IC_SET1		0x3A
#define BD9995X_CMD_IC_SET2		0x3B
#define BD9995X_CMD_SYSTEM_STATUS	0x3C
#define BD9995X_CMD_SYSTEM_STATUS_OTPLD_STATE	(1 << 1)
#define BD9995X_CMD_SYSTEM_STATUS_ALLRST_STATE	(1 << 0)

#define BD9995X_CMD_SYSTEM_CTRL_SET	0x3D
#define BD9995X_CMD_SYSTEM_CTRL_SET_OTPLD	(1 << 1)
#define BD9995X_CMD_SYSTEM_CTRL_SET_ALLRST	(1 << 0)

#define BD9995X_CMD_EXT_PROTECT_SET	0x3E
#define BD9995X_CMD_EXT_MAP_SET		0x3F
#define BD9995X_CMD_VM_CTRL_SET		0x40
#define BD9995X_CMD_VM_CTRL_SET_EXTIADPEN	(1 << 9)
#define BD9995X_CMD_THERM_WINDOW_SET1	0x41
#define BD9995X_CMD_THERM_WINDOW_SET2	0x42
#define BD9995X_CMD_THERM_WINDOW_SET3	0x43
#define BD9995X_CMD_THERM_WINDOW_SET4	0x44
#define BD9995X_CMD_THERM_WINDOW_SET5	0x45
#define BD9995X_CMD_IBATP_TH_SET	0x46
#define BD9995X_CMD_IBATM_TH_SET	0x47
#define BD9995X_CMD_VBAT_TH_SET		0x48
#define BD9995X_CMD_THERM_TH_SET	0x49
#define BD9995X_CMD_IACP_TH_SET		0x4A
#define BD9995X_CMD_VACP_TH_SET		0x4B

/* Enable discharge when VBUS falls below BD9995X_VBUS_DISCHARGE_TH */
#define BD9995X_VBUS_DISCHARGE_TH	3900
#define BD9995X_CMD_VBUS_TH_SET		0x4C
#define BD9995X_CMD_VCC_TH_SET		0x4D

#define BD9995X_CMD_VSYS_TH_SET		0x4E
#define BD9995X_CMD_EXTIADP_TH_SET	0x4F
#define BD9995X_CMD_IBATP_VAL		0x50
#define BD9995X_CMD_IBATP_AVE_VAL	0x51
#define BD9995X_CMD_IBATM_VAL		0x52
#define BD9995X_CMD_IBATM_AVE_VAL	0x53
#define BD9995X_CMD_VBAT_VAL		0x54
#define BD9995X_CMD_VBAT_AVE_VAL	0x55
#define BD9995X_CMD_THERM_VAL		0x56
#define BD9995X_CMD_VTH_VAL		0x57
#define BD9995X_CMD_IACP_VAL		0x58
#define BD9995X_CMD_IACP_AVE_VAL	0x59
#define BD9995X_CMD_VACP_VAL		0x5A
#define BD9995X_CMD_VACP_AVE_VAL	0x5B
#define BD9995X_CMD_VBUS_VAL		0x5C
#define BD9995X_CMD_VBUS_AVE_VAL	0x5D
#define BD9995X_CMD_VCC_VAL		0x5E
#define BD9995X_CMD_VCC_AVE_VAL		0x5F
#define BD9995X_CMD_VSYS_VAL		0x60
#define BD9995X_CMD_VSYS_AVE_VAL	0x61
#define BD9995X_CMD_EXTIADP_VAL		0x62
#define BD9995X_CMD_EXTIADP_AVE_VAL	0x63
#define BD9995X_CMD_VACPCLPS_TH_SET	0x64
#define BD9995X_CMD_INT0_SET		0x68
#define BD9995X_CMD_INT0_SET_INT2_EN	(1 << 2)
#define BD9995X_CMD_INT0_SET_INT1_EN	(1 << 1)
#define BD9995X_CMD_INT0_SET_INT0_EN	(1 << 0)

#define BD9995X_CMD_INT1_SET		0x69
/* Bits for both INT1 & INT2 reg */
#define BD9995X_CMD_INT_SET_TH_DET	(1 << 9)
#define BD9995X_CMD_INT_SET_TH_RES	(1 << 8)
#define BD9995X_CMD_INT_SET_DET		(1 << 1)
#define BD9995X_CMD_INT_SET_RES		(1 << 0)
#define BD9995X_CMD_INT_VBUS_DET	(BD9995X_CMD_INT_SET_RES | \
					 BD9995X_CMD_INT_SET_DET)
#define BD9995X_CMD_INT_VBUS_TH         (BD9995X_CMD_INT_SET_TH_RES | \
					 BD9995X_CMD_INT_SET_TH_DET)

#define BD9995X_CMD_INT2_SET		0x6A
#define BD9995X_CMD_INT3_SET		0x6B
#define BD9995X_CMD_INT4_SET		0x6C
#define BD9995X_CMD_INT5_SET		0x6D
#define BD9995X_CMD_INT6_SET		0x6E
#define BD9995X_CMD_INT7_SET		0x6F
#define BD9995X_CMD_INT0_STATUS		0x70
#define BD9995X_CMD_INT1_STATUS		0x71
/* Bits for both INT1_STATUS & INT2_STATUS reg */
#define BD9995X_CMD_INT_STATUS_DET	(1 << 1)
#define BD9995X_CMD_INT_STATUS_RES	(1 << 0)

#define BD9995X_CMD_INT2_STATUS		0x72
#define BD9995X_CMD_INT3_STATUS		0x73
#define BD9995X_CMD_INT4_STATUS		0x74
#define BD9995X_CMD_INT5_STATUS		0x75
#define BD9995X_CMD_INT6_STATUS		0x76
#define BD9995X_CMD_INT7_STATUS		0x77
#define BD9995X_CMD_REG0		0x78
#define BD9995X_CMD_REG1		0x79
#define BD9995X_CMD_OTPREG0		0x7A
#define BD9995X_CMD_OTPREG1		0x7B
#define BD9995X_CMD_SMBREG		0x7C
/* Normal functionality - power save mode disabled. */
#define BD9995X_PWR_SAVE_OFF		0
/* BGATE ON w/ PROCHOT# monitored only system voltage. */
#define BD9995X_PWR_SAVE_LOW		0x1
/* BGATE ON w/ PROCHOT# monitored only system voltage every 1ms. */
#define BD9995X_PWR_SAVE_MED		0x2
/* BGATE ON w/o PROCHOT# monitoring. */
#define BD9995X_PWR_SAVE_HIGH		0x5
/* BGATE OFF */
#define BD9995X_PWR_SAVE_MAX		0x6
#define BD9995X_CMD_DEBUG_MODE_SET	0x7F

/* Map PD port number to charge port number */
static inline enum bd9995x_charge_port bd9995x_pd_port_to_chg_port(int port)
{
#ifdef CONFIG_BD9995X_PRIMARY_CHARGE_PORT_VCC
	return port ? BD9995X_CHARGE_PORT_VBUS : BD9995X_CHARGE_PORT_VCC;
#else
	return port ? BD9995X_CHARGE_PORT_VCC : BD9995X_CHARGE_PORT_VBUS;
#endif
}

/*
 * Non-standard interface functions - bd9995x integrates additional
 * functionality not part of the standard charger interface.
 */

/* Is VBUS provided or external power present */
int bd9995x_is_vbus_provided(int port);
/* Select or deselect input port from {VCC, VBUS, VCC&VBUS}. */
int bd9995x_select_input_port(enum bd9995x_charge_port port, int select);
/* Get input current limit for BC1.2 suppliers */
int bd9995x_get_bc12_ilim(int charge_supplier);
/* Enable/Disable charging triggered by BC1.2 */
int bd9995x_bc12_enable_charging(enum bd9995x_charge_port port, int enable);
/* Interrupt handler for USB charger VBUS */
void bd9995x_vbus_interrupt(enum gpio_signal signal);
/* Read temperature measurement value (in Celsius) */
int bd9995x_get_battery_temp(int *temp_ptr);
/* Set power save mode */
void bd9995x_set_power_save_mode(int mode);
/* Get Battery Voltage Measurement Value */
int bd9995x_get_battery_voltage(void);

#ifdef CONFIG_CMD_I2C_STRESS_TEST_CHARGER
extern struct i2c_stress_test_dev bd9995x_i2c_stress_test_dev;
#endif

#endif /* __CROS_EC_BD9995X_H */

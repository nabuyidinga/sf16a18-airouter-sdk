/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *
 */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/tlv.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>

#include "ad82584f.h"

#define AD82584F_RATES (SNDRV_PCM_RATE_16000 | \
		SNDRV_PCM_RATE_32000 | \
		SNDRV_PCM_RATE_44100 | \
		SNDRV_PCM_RATE_48000 | \
		SNDRV_PCM_RATE_64000 | \
		SNDRV_PCM_RATE_88200 | \
		SNDRV_PCM_RATE_96000 | \
		SNDRV_PCM_RATE_176400 | \
		SNDRV_PCM_RATE_192000)

#define AD82584F_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | \
		SNDRV_PCM_FMTBIT_S24_LE | \
		SNDRV_PCM_FMTBIT_S32_LE)

static const DECLARE_TLV_DB_SCALE(mvol_tlv, -10300, 50, 1);
static const DECLARE_TLV_DB_SCALE(chvol_tlv, -10300, 50, 1);

static const struct snd_kcontrol_new ad82584f_snd_controls[] = {
	SOC_SINGLE_TLV("Master Volume", MVOL, 0,
			0xff, 1, mvol_tlv),
	SOC_SINGLE_TLV("Ch1 Volume", C1VOL, 0,
			0xff, 1, chvol_tlv),
	SOC_SINGLE_TLV("Ch2 Volume", C2VOL, 0,
			0xff, 1, chvol_tlv),
};

/* Power-up register defaults */
struct reg_default ad82584f_reg_defaults[AD82584F_REGISTER_COUNT] = {
	{0x00, 0x04},//##State_Control_1
	{0x01, 0x82},//##State_Control_2
	{0x02, 0x00},//##State_Control_3
	{0x03, 0x4f},//##Master_volume_control
	{0x04, 0x00},//##Channel_1_volume_control
	{0x05, 0x00},//##Channel_2_volume_control
	{0x06, 0x18},//##Channel_3_volume_control
	{0x07, 0x18},//##Channel_4_volume_control
	{0x08, 0x18},//##Channel_5_volume_control
	{0x09, 0x18},//##Channel_6_volume_control
	{0x0a, 0x10},//##Bass_Tone_Boost_and_Cut
	{0x0b, 0x10},//##treble_Tone_Boost_and_Cut
	{0x0c, 0x90},//##State_Control_4
	{0x0d, 0x00},//##Channel_1_configuration_registers
	{0x0e, 0x00},//##Channel_2_configuration_registers
	{0x0f, 0x00},//##Channel_3_configuration_registers
	{0x10, 0x00},//##Channel_4_configuration_registers
	{0x11, 0x00},//##Channel_5_configuration_registers
	{0x12, 0x00},//##Channel_6_configuration_registers
	{0x13, 0x00},//##Channel_7_configuration_registers
	{0x14, 0x00},//##Channel_8_configuration_registers
	{0x15, 0x6a},//##DRC1_limiter_attack/release_rate
	{0x16, 0x6a},//##DRC2_limiter_attack/release_rate
	{0x17, 0x6a},//##DRC3_limiter_attack/release_rate
	{0x18, 0x6a},//##DRC4_limiter_attack/release_rate
	{0x19, 0x06},//##Error_Delay
	{0x1a, 0x32},//##State_Control_5
	{0x1b, 0x00},//##HVUV_selection
	{0x1c, 0x00},//##State_Control_6
	{0x1d, 0x7f},//##Coefficient_RAM_Base_Address
	{0x1e, 0x00},//##Top_8-bits_of_coefficients_A1
	{0x1f, 0x00},//##Middle_8-bits_of_coefficients_A1
	{0x20, 0x00},//##Bottom_8-bits_of_coefficients_A1
	{0x21, 0x00},//##Top_8-bits_of_coefficients_A2
	{0x22, 0x00},//##Middle_8-bits_of_coefficients_A2
	{0x23, 0x00},//##Bottom_8-bits_of_coefficients_A2
	{0x24, 0x00},//##Top_8-bits_of_coefficients_B1
	{0x25, 0x00},//##Middle_8-bits_of_coefficients_B1
	{0x26, 0x00},//##Bottom_8-bits_of_coefficients_B1
	{0x27, 0x00},//##Top_8-bits_of_coefficients_B2
	{0x28, 0x00},//##Middle_8-bits_of_coefficients_B2
	{0x29, 0x00},//##Bottom_8-bits_of_coefficients_B2
	{0x2a, 0x40},//##Top_8-bits_of_coefficients_A0
	{0x2b, 0x00},//##Middle_8-bits_of_coefficients_A0
	{0x2c, 0x00},//##Bottom_8-bits_of_coefficients_A0
	{0x2d, 0x40},//##Coefficient_R/W_control
	{0x2e, 0x00},//##Protection_Enable/Disable
	{0x2f, 0x00},//##Memory_BIST_status
	{0x30, 0x00},//##Power_Stage_Status(Read_only)
	{0x31, 0x00},//##PWM_Output_Control
	{0x32, 0x00},//##Test_Mode_Control_Reg.
	{0x33, 0x6d},//##Qua-Ternary/Ternary_Switch_Level
	{0x34, 0x00},//##Volume_Fine_tune
	{0x35, 0x00},//##Volume_Fine_tune
	{0x36, 0x60},//##OC_bypass_&_GVDD_selection
	{0x37, 0x52},//##Device_ID_register
	{0x38, 0x00},//##RAM1_test_register_address
	{0x39, 0x00},//##Top_8-bits_of_RAM1_Data
	{0x3a, 0x00},//##Middle_8-bits_of_RAM1_Data
	{0x3b, 0x00},//##Bottom_8-bits_of_RAM1_Data
	{0x3c, 0x00},//##RAM1_test_r/w_control
	{0x3d, 0x00},//##RAM2_test_register_address
	{0x3e, 0x00},//##Top_8-bits_of_RAM2_Data
	{0x3f, 0x00},//##Middle_8-bits_of_RAM2_Data
	{0x40, 0x00},//##Bottom_8-bits_of_RAM2_Data
	{0x41, 0x00},//##RAM2_test_r/w_control
	{0x42, 0x00},//##Level_Meter_Clear
	{0x43, 0x00},//##Power_Meter_Clear
	{0x44, 0x20},//##TOP_of_C1_Level_Meter
	{0x45, 0x00},//##Middle_of_C1_Level_Meter
	{0x46, 0x00},//##Bottom_of_C1_Level_Meter
	{0x47, 0x20},//##TOP_of_C2_Level_Meter
	{0x48, 0x00},//##Middle_of_C2_Level_Meter
	{0x49, 0x00},//##Bottom_of_C2_Level_Meter
	{0x4a, 0x00},//##TOP_of_C3_Level_Meter
	{0x4b, 0x00},//##Middle_of_C3_Level_Meter
	{0x4c, 0x00},//##Bottom_of_C3_Level_Meter
	{0x4d, 0x00},//##TOP_of_C4_Level_Meter
	{0x4e, 0x00},//##Middle_of_C4_Level_Meter
	{0x4f, 0x00},//##Bottom_of_C4_Level_Meter
	{0x50, 0x00},//##TOP_of_C5_Level_Meter
	{0x51, 0x00},//##Middle_of_C5_Level_Meter
	{0x52, 0x00},//##Bottom_of_C5_Level_Meter
	{0x53, 0x00},//##TOP_of_C6_Level_Meter
	{0x54, 0x00},//##Middle_of_C6_Level_Meter
	{0x55, 0x00},//##Bottom_of_C6_Level_Meter
	{0x56, 0x00},//##TOP_of_C7_Level_Meter
	{0x57, 0x00},//##Middle_of_C7_Level_Meter
	{0x58, 0x00},//##Bottom_of_C7_Level_Meter
	{0x59, 0x00},//##TOP_of_C8_Level_Meter
	{0x5a, 0x00},//##Middle_of_C8_Level_Meter
	{0x5b, 0x00},//##Bottom_of_C8_Level_Meter
	{0x5c, 0x06},//##I2S_Data_Output_Selection_Register
	{0x5d, 0x00},//##Reserve
	{0x5e, 0x00},//##Reserve
	{0x5f, 0x00},//##Reserve
	{0x60, 0x00},//##Reserve
	{0x61, 0x00},//##Reserve
	{0x62, 0x00},//##Reserve
	{0x63, 0x00},//##Reserve
	{0x64, 0x00},//##Reserve
	{0x65, 0x00},//##Reserve
	{0x66, 0x00},//##Reserve
	{0x67, 0x00},//##Reserve
	{0x68, 0x00},//##Reserve
	{0x69, 0x00},//##Reserve
	{0x6a, 0x00},//##Reserve
	{0x6b, 0x00},//##Reserve
	{0x6c, 0x00},//##Reserve
	{0x6d, 0x00},//##Reserve
	{0x6e, 0x00},//##Reserve
	{0x6f, 0x00},//##Reserve
	{0x70, 0x00},//##Reserve
	{0x71, 0x00},//##Reserve
	{0x72, 0x00},//##Reserve
	{0x73, 0x00},//##Reserve
	{0x74, 0x00},//##Mono_Key_High_Byte
	{0x75, 0x00},//##Mono_Key_Low_Byte
	{0x76, 0x00},//##Boost_Control
	{0x77, 0x07},//##Hi-res_Item
	{0x78, 0x40},//##Test_Mode_register
	{0x79, 0x62},//##Boost_Strap_OV/UV_Selection
	{0x7a, 0x8c},//##OC_Selection_2
	{0x7b, 0x55},//##MBIST_User_Program_Top_Byte_Even
	{0x7c, 0x55},//##MBIST_User_Program_Middle_Byte_Even
	{0x7d, 0x55},//##MBIST_User_Program_Bottom_Byte_Even
	{0x7e, 0x55},//##MBIST_User_Program_Top_Byte_Odd
	{0x7f, 0x55},//##MBIST_User_Program_Middle_Byte_Odd
	{0x80, 0x55},//##MBIST_User_Program_Bottom_Byte_Odd
	{0x81, 0x00},//##ERROR_clear_register
	{0x82, 0x0c},//##Minimum_duty_test
	{0x83, 0x06},//##Reserve
	{0x84, 0xfa},//##Reserve
	{0x85, 0x2a},//##Reserve
};


static struct reg_default reg_init[] = {
	{0x03, 0x18},
	{0x04, 0x18},
	{0x05, 0x18},
	{0x0c, 0x98},
	{0x0d, 0x12},
	{0x0e, 0x12},
	{0x1a, 0x32},
	{0x1b, 0x81},
};

static int m_reg_tab[AD82584F_REGISTER_COUNT][2] = {
	{0x00, 0x04},//##State_Control_1
	{0x01, 0x81},//##State_Control_2
	{0x02, 0x00},//##State_Control_3
	{0x03, 0x14},//##Master_volume_control
	{0x04, 0x18},//##Channel_1_volume_control
	{0x05, 0x18},//##Channel_2_volume_control
	{0x06, 0x18},//##Channel_3_volume_control
	{0x07, 0x18},//##Channel_4_volume_control
	{0x08, 0x18},//##Channel_5_volume_control
	{0x09, 0x18},//##Channel_6_volume_control
	{0x0a, 0x10},//##Bass_Tone_Boost_and_Cut
	{0x0b, 0x10},//##treble_Tone_Boost_and_Cut
	{0x0c, 0x90},//##State_Control_4
	{0x0d, 0x00},//##Channel_1_configuration_registers
	{0x0e, 0x00},//##Channel_2_configuration_registers
	{0x0f, 0x00},//##Channel_3_configuration_registers
	{0x10, 0x00},//##Channel_4_configuration_registers
	{0x11, 0x00},//##Channel_5_configuration_registers
	{0x12, 0x00},//##Channel_6_configuration_registers
	{0x13, 0x00},//##Channel_7_configuration_registers
	{0x14, 0x00},//##Channel_8_configuration_registers
	{0x15, 0x6a},//##DRC1_limiter_attack/release_rate
	{0x16, 0x6a},//##DRC2_limiter_attack/release_rate
	{0x17, 0x6a},//##DRC3_limiter_attack/release_rate
	{0x18, 0x6a},//##DRC4_limiter_attack/release_rate
	{0x19, 0x06},//##Error_Delay
	{0x1a, 0x72},//##State_Control_5
	{0x1b, 0x00},//##HVUV_selection
	{0x1c, 0x00},//##State_Control_6
	{0x1d, 0x7f},//##Coefficient_RAM_Base_Address
	{0x1e, 0x00},//##Top_8-bits_of_coefficients_A1
	{0x1f, 0x00},//##Middle_8-bits_of_coefficients_A1
	{0x20, 0x00},//##Bottom_8-bits_of_coefficients_A1
	{0x21, 0x00},//##Top_8-bits_of_coefficients_A2
	{0x22, 0x00},//##Middle_8-bits_of_coefficients_A2
	{0x23, 0x00},//##Bottom_8-bits_of_coefficients_A2
	{0x24, 0x00},//##Top_8-bits_of_coefficients_B1
	{0x25, 0x00},//##Middle_8-bits_of_coefficients_B1
	{0x26, 0x00},//##Bottom_8-bits_of_coefficients_B1
	{0x27, 0x00},//##Top_8-bits_of_coefficients_B2
	{0x28, 0x00},//##Middle_8-bits_of_coefficients_B2
	{0x29, 0x00},//##Bottom_8-bits_of_coefficients_B2
	{0x2a, 0x40},//##Top_8-bits_of_coefficients_A0
	{0x2b, 0x00},//##Middle_8-bits_of_coefficients_A0
	{0x2c, 0x00},//##Bottom_8-bits_of_coefficients_A0
	{0x2d, 0x40},//##Coefficient_R/W_control
	{0x2e, 0x00},//##Protection_Enable/Disable
	{0x2f, 0x00},//##Memory_BIST_status
	{0x30, 0x00},//##Power_Stage_Status(Read_only)
	{0x31, 0x00},//##PWM_Output_Control
	{0x32, 0x00},//##Test_Mode_Control_Reg.
	{0x33, 0x6d},//##Qua-Ternary/Ternary_Switch_Level
	{0x34, 0x00},//##Volume_Fine_tune
	{0x35, 0x00},//##Volume_Fine_tune
	{0x36, 0x60},//##OC_bypass_&_GVDD_selection
	{0x37, 0x52},//##Device_ID_register
	{0x38, 0x00},//##RAM1_test_register_address
	{0x39, 0x00},//##Top_8-bits_of_RAM1_Data
	{0x3a, 0x00},//##Middle_8-bits_of_RAM1_Data
	{0x3b, 0x00},//##Bottom_8-bits_of_RAM1_Data
	{0x3c, 0x00},//##RAM1_test_r/w_control
	{0x3d, 0x00},//##RAM2_test_register_address
	{0x3e, 0x00},//##Top_8-bits_of_RAM2_Data
	{0x3f, 0x00},//##Middle_8-bits_of_RAM2_Data
	{0x40, 0x00},//##Bottom_8-bits_of_RAM2_Data
	{0x41, 0x00},//##RAM2_test_r/w_control
	{0x42, 0x00},//##Level_Meter_Clear
	{0x43, 0x00},//##Power_Meter_Clear
	{0x44, 0x15},//##TOP_of_C1_Level_Meter
	{0x45, 0xae},//##Middle_of_C1_Level_Meter
	{0x46, 0xdd},//##Bottom_of_C1_Level_Meter
	{0x47, 0x15},//##TOP_of_C2_Level_Meter
	{0x48, 0xae},//##Middle_of_C2_Level_Meter
	{0x49, 0xdd},//##Bottom_of_C2_Level_Meter
	{0x4a, 0x00},//##TOP_of_C3_Level_Meter
	{0x4b, 0x00},//##Middle_of_C3_Level_Meter
	{0x4c, 0x00},//##Bottom_of_C3_Level_Meter
	{0x4d, 0x00},//##TOP_of_C4_Level_Meter
	{0x4e, 0x00},//##Middle_of_C4_Level_Meter
	{0x4f, 0x00},//##Bottom_of_C4_Level_Meter
	{0x50, 0x00},//##TOP_of_C5_Level_Meter
	{0x51, 0x00},//##Middle_of_C5_Level_Meter
	{0x52, 0x00},//##Bottom_of_C5_Level_Meter
	{0x53, 0x00},//##TOP_of_C6_Level_Meter
	{0x54, 0x00},//##Middle_of_C6_Level_Meter
	{0x55, 0x00},//##Bottom_of_C6_Level_Meter
	{0x56, 0x00},//##TOP_of_C7_Level_Meter
	{0x57, 0x00},//##Middle_of_C7_Level_Meter
	{0x58, 0x00},//##Bottom_of_C7_Level_Meter
	{0x59, 0x00},//##TOP_of_C8_Level_Meter
	{0x5a, 0x00},//##Middle_of_C8_Level_Meter
	{0x5b, 0x00},//##Bottom_of_C8_Level_Meter
	{0x5c, 0x06},//##I2S_Data_Output_Selection_Register
	{0x5d, 0x00},//##Reserve
	{0x5e, 0x00},//##Reserve
	{0x5f, 0x00},//##Reserve
	{0x60, 0x00},//##Reserve
	{0x61, 0x00},//##Reserve
	{0x62, 0x00},//##Reserve
	{0x63, 0x00},//##Reserve
	{0x64, 0x00},//##Reserve
	{0x65, 0x00},//##Reserve
	{0x66, 0x00},//##Reserve
	{0x67, 0x00},//##Reserve
	{0x68, 0x00},//##Reserve
	{0x69, 0x00},//##Reserve
	{0x6a, 0x00},//##Reserve
	{0x6b, 0x00},//##Reserve
	{0x6c, 0x00},//##Reserve
	{0x6d, 0x00},//##Reserve
	{0x6e, 0x00},//##Reserve
	{0x6f, 0x00},//##Reserve
	{0x70, 0x00},//##Reserve
	{0x71, 0x00},//##Reserve
	{0x72, 0x00},//##Reserve
	{0x73, 0x00},//##Reserve
	{0x74, 0x30},//##Mono_Key_High_Byte
	{0x75, 0x06},//##Mono_Key_Low_Byte
	{0x76, 0x00},//##Boost_Control
	{0x77, 0x07},//##Hi-res_Item
	{0x78, 0x40},//##Test_Mode_register
	{0x79, 0x62},//##Boost_Strap_OV/UV_Selection
	{0x7a, 0x8c},//##OC_Selection_2
	{0x7b, 0x55},//##MBIST_User_Program_Top_Byte_Even
	{0x7c, 0x55},//##MBIST_User_Program_Middle_Byte_Even
	{0x7d, 0x55},//##MBIST_User_Program_Bottom_Byte_Even
	{0x7e, 0x55},//##MBIST_User_Program_Top_Byte_Odd
	{0x7f, 0x55},//##MBIST_User_Program_Middle_Byte_Odd
	{0x80, 0x55},//##MBIST_User_Program_Bottom_Byte_Odd
	{0x81, 0x00},//##ERROR_clear_register
	{0x82, 0x0c},//##Minimum_duty_test
	{0x83, 0x06},//##Reserve
	{0x84, 0xfe},//##Reserve
	{0x85, 0xca},//##Reserve
};

static int m_ram1_tab[][4] = {
	{0x00, 0x00, 0x00, 0x00},//##Channel_1_EQ1_A1 
	{0x01, 0x00, 0x00, 0x00},//##Channel_1_EQ1_A2 
	{0x02, 0x00, 0x00, 0x00},//##Channel_1_EQ1_B1 
	{0x03, 0x00, 0x00, 0x00},//##Channel_1_EQ1_B2 
	{0x04, 0x20, 0x00, 0x00},//##Channel_1_EQ1_A0 
	{0x05, 0x00, 0x00, 0x00},//##Channel_1_EQ2_A1 
	{0x06, 0x00, 0x00, 0x00},//##Channel_1_EQ2_A2 
	{0x07, 0x00, 0x00, 0x00},//##Channel_1_EQ2_B1 
	{0x08, 0x00, 0x00, 0x00},//##Channel_1_EQ2_B2 
	{0x09, 0x20, 0x00, 0x00},//##Channel_1_EQ2_A0 
	{0x0a, 0x00, 0x00, 0x00},//##Channel_1_EQ3_A1 
	{0x0b, 0x00, 0x00, 0x00},//##Channel_1_EQ3_A2 
	{0x0c, 0x00, 0x00, 0x00},//##Channel_1_EQ3_B1 
	{0x0d, 0x00, 0x00, 0x00},//##Channel_1_EQ3_B2 
	{0x0e, 0x20, 0x00, 0x00},//##Channel_1_EQ3_A0 
	{0x0f, 0x00, 0x00, 0x00},//##Channel_1_EQ4_A1 
	{0x10, 0x00, 0x00, 0x00},//##Channel_1_EQ4_A2 
	{0x11, 0x00, 0x00, 0x00},//##Channel_1_EQ4_B1 
	{0x12, 0x00, 0x00, 0x00},//##Channel_1_EQ4_B2 
	{0x13, 0x20, 0x00, 0x00},//##Channel_1_EQ4_A0 
	{0x14, 0x00, 0x00, 0x00},//##Channel_1_EQ5_A1 
	{0x15, 0x00, 0x00, 0x00},//##Channel_1_EQ5_A2 
	{0x16, 0x00, 0x00, 0x00},//##Channel_1_EQ5_B1 
	{0x17, 0x00, 0x00, 0x00},//##Channel_1_EQ5_B2 
	{0x18, 0x20, 0x00, 0x00},//##Channel_1_EQ5_A0 
	{0x19, 0x00, 0x00, 0x00},//##Channel_1_EQ6_A1 
	{0x1a, 0x00, 0x00, 0x00},//##Channel_1_EQ6_A2 
	{0x1b, 0x00, 0x00, 0x00},//##Channel_1_EQ6_B1 
	{0x1c, 0x00, 0x00, 0x00},//##Channel_1_EQ6_B2 
	{0x1d, 0x20, 0x00, 0x00},//##Channel_1_EQ6_A0 
	{0x1e, 0x00, 0x00, 0x00},//##Channel_1_EQ7_A1 
	{0x1f, 0x00, 0x00, 0x00},//##Channel_1_EQ7_A2 
	{0x20, 0x00, 0x00, 0x00},//##Channel_1_EQ7_B1 
	{0x21, 0x00, 0x00, 0x00},//##Channel_1_EQ7_B2 
	{0x22, 0x20, 0x00, 0x00},//##Channel_1_EQ7_A0 
	{0x23, 0x00, 0x00, 0x00},//##Channel_1_EQ8_A1 
	{0x24, 0x00, 0x00, 0x00},//##Channel_1_EQ8_A2 
	{0x25, 0x00, 0x00, 0x00},//##Channel_1_EQ8_B1 
	{0x26, 0x00, 0x00, 0x00},//##Channel_1_EQ8_B2 
	{0x27, 0x20, 0x00, 0x00},//##Channel_1_EQ8_A0 
	{0x28, 0x00, 0x00, 0x00},//##Channel_1_EQ9_A1 
	{0x29, 0x00, 0x00, 0x00},//##Channel_1_EQ9_A2 
	{0x2a, 0x00, 0x00, 0x00},//##Channel_1_EQ9_B1 
	{0x2b, 0x00, 0x00, 0x00},//##Channel_1_EQ9_B2 
	{0x2c, 0x20, 0x00, 0x00},//##Channel_1_EQ9_A0 
	{0x2d, 0x00, 0x00, 0x00},//##Channel_1_EQ10_A1 
	{0x2e, 0x00, 0x00, 0x00},//##Channel_1_EQ10_A2 
	{0x2f, 0x00, 0x00, 0x00},//##Channel_1_EQ10_B1 
	{0x30, 0x00, 0x00, 0x00},//##Channel_1_EQ10_B2 
	{0x31, 0x20, 0x00, 0x00},//##Channel_1_EQ10_A0 
	{0x32, 0x00, 0x00, 0x00},//##Channel_1_EQ11_A1 
	{0x33, 0x00, 0x00, 0x00},//##Channel_1_EQ11_A2 
	{0x34, 0x00, 0x00, 0x00},//##Channel_1_EQ11_B1 
	{0x35, 0x00, 0x00, 0x00},//##Channel_1_EQ11_B2 
	{0x36, 0x20, 0x00, 0x00},//##Channel_1_EQ11_A0 
	{0x37, 0x00, 0x00, 0x00},//##Channel_1_EQ12_A1 
	{0x38, 0x00, 0x00, 0x00},//##Channel_1_EQ12_A2 
	{0x39, 0x00, 0x00, 0x00},//##Channel_1_EQ12_B1 
	{0x3a, 0x00, 0x00, 0x00},//##Channel_1_EQ12_B2 
	{0x3b, 0x20, 0x00, 0x00},//##Channel_1_EQ12_A0 
	{0x3c, 0x00, 0x00, 0x00},//##Channel_1_EQ13_A1 
	{0x3d, 0x00, 0x00, 0x00},//##Channel_1_EQ13_A2 
	{0x3e, 0x00, 0x00, 0x00},//##Channel_1_EQ13_B1 
	{0x3f, 0x00, 0x00, 0x00},//##Channel_1_EQ13_B2 
	{0x40, 0x20, 0x00, 0x00},//##Channel_1_EQ13_A0 
	{0x41, 0x00, 0x00, 0x00},//##Channel_1_EQ14_A1 
	{0x42, 0x00, 0x00, 0x00},//##Channel_1_EQ14_A2 
	{0x43, 0x00, 0x00, 0x00},//##Channel_1_EQ14_B1 
	{0x44, 0x00, 0x00, 0x00},//##Channel_1_EQ14_B2 
	{0x45, 0x20, 0x00, 0x00},//##Channel_1_EQ14_A0 
	{0x46, 0x00, 0x00, 0x00},//##Channel_1_EQ15_A1 
	{0x47, 0x00, 0x00, 0x00},//##Channel_1_EQ15_A2 
	{0x48, 0x00, 0x00, 0x00},//##Channel_1_EQ15_B1 
	{0x49, 0x00, 0x00, 0x00},//##Channel_1_EQ15_B2 
	{0x4a, 0x20, 0x00, 0x00},//##Channel_1_EQ15_A0 
	{0x4b, 0x40, 0x00, 0x00},//##Channel_1_Mixer1 
	{0x4c, 0x40, 0x00, 0x00},//##Channel_1_Mixer2 
	{0x4d, 0x7f, 0xff, 0xff},//##Channel_1_Prescale 
	{0x4e, 0x7f, 0xff, 0xff},//##Channel_1_Postscale 
	{0x4f, 0xc7, 0xb6, 0x91},//##A0_of_L_channel_SRS_HPF 
	{0x50, 0x38, 0x49, 0x6e},//##A1_of_L_channel_SRS_HPF 
	{0x51, 0x0c, 0x46, 0xf8},//##B1_of_L_channel_SRS_HPF 
	{0x52, 0x0e, 0x81, 0xb9},//##A0_of_L_channel_SRS_LPF 
	{0x53, 0xf2, 0x2c, 0x12},//##A1_of_L_channel_SRS_LPF 
	{0x54, 0x0f, 0xca, 0xbb},//##B1_of_L_channel_SRS_LPF 
	{0x55, 0x20, 0x00, 0x00},//##CH1.2_Power_Clipping 
	{0x56, 0x20, 0x00, 0x00},//##CCH1.2_DRC1_Attack_threshold 
	{0x57, 0x08, 0x00, 0x00},//##CH1.2_DRC1_Release_threshold 
	{0x58, 0x20, 0x00, 0x00},//##CH3.4_DRC2_Attack_threshold 
	{0x59, 0x08, 0x00, 0x00},//##CH3.4_DRC2_Release_threshold 
	{0x5a, 0x20, 0x00, 0x00},//##CH5.6_DRC3_Attack_threshold 
	{0x5b, 0x08, 0x00, 0x00},//##CH5.6_DRC3_Release_threshold 
	{0x5c, 0x20, 0x00, 0x00},//##CH7.8_DRC4_Attack_threshold 
	{0x5d, 0x08, 0x00, 0x00},//##CH7.8_DRC4_Release_threshold 
	{0x5e, 0x00, 0x00, 0x1a},//##Noise_Gate_Attack_Level 
	{0x5f, 0x00, 0x00, 0x53},//##Noise_Gate_Release_Level 
	{0x60, 0x00, 0x80, 0x00},//##DRC1_Energy_Coefficient 
	{0x61, 0x00, 0x20, 0x00},//##DRC2_Energy_Coefficient 
	{0x62, 0x00, 0x80, 0x00},//##DRC3_Energy_Coefficient 
	{0x63, 0x00, 0x80, 0x00},//##DRC4_Energy_Coefficient 
	{0x64, 0x00, 0x00, 0x00},//DRC1_Power_Meter
	{0x65, 0x00, 0x00, 0x00},//DRC3_Power_Mete
	{0x66, 0x00, 0x00, 0x00},//DRC5_Power_Meter
	{0x67, 0x00, 0x00, 0x00},//DRC7_Power_Meter
	{0x68, 0x00, 0x00, 0x00},//##Channel_1_DEQ1_A1 
	{0x69, 0x00, 0x00, 0x00},//##Channel_1_DEQ1_A2
	{0x6a, 0x00, 0x00, 0x00},//##Channel_1_DEQ1_B1
	{0x6b, 0x00, 0x00, 0x00},//##Channel_1_DEQ1_B2 
	{0x6c, 0x20, 0x00, 0x00},//##Channel_1_DEQ1_A0
	{0x6d, 0x00, 0x00, 0x00},//##Channel_1_DEQ2_A1 
	{0x6e, 0x00, 0x00, 0x00},//##Channel_1_DEQ2_A2 
	{0x6f, 0x00, 0x00, 0x00},//##Channel_1_DEQ2_B1 
	{0x70, 0x00, 0x00, 0x00},//##Channel_1_DEQ2_B2 
	{0x71, 0x20, 0x00, 0x00},//##Channel_1_DEQ2_A0 
	{0x72, 0x00, 0x00, 0x00},//##Channel_1_DEQ3_A1 
	{0x73, 0x00, 0x00, 0x00},//##Channel_1_DEQ3_A2 
	{0x74, 0x00, 0x00, 0x00},//##Channel_1_DEQ3_B1 
	{0x75, 0x00, 0x00, 0x00},//##Channel_1_DEQ3_B2 
	{0x76, 0x20, 0x00, 0x00},//##Channel_1_DEQ3_A0 
	{0x77, 0x00, 0x00, 0x00},//##Channel_1_DEQ4_A1 
	{0x78, 0x00, 0x00, 0x00},//##Channel_1_DEQ4_A2 
	{0x79, 0x00, 0x00, 0x00},//##Channel_1_DEQ4_B1 
	{0x7a, 0x00, 0x00, 0x00},//##Channel_1_DEQ4_B2 
	{0x7b, 0x20, 0x00, 0x00},//##Channel_1_DEQ4_A0 
	{0x7c, 0x00, 0x00, 0x00},//##Reserve
	{0x7d, 0x00, 0x00, 0x00},//##Reserve
	{0x7e, 0x00, 0x00, 0x00},//##Reserve
	{0x7f, 0x00, 0x00, 0x00},//##Reserve
};

static int m_ram2_tab[][4] = {
	{0x00, 0x00, 0x00, 0x00},//##Channel_2_EQ1_A1 
	{0x01, 0x00, 0x00, 0x00},//##Channel_2_EQ1_A2 
	{0x02, 0x00, 0x00, 0x00},//##Channel_2_EQ1_B1 
	{0x03, 0x00, 0x00, 0x00},//##Channel_2_EQ1_B2 
	{0x04, 0x20, 0x00, 0x00},//##Channel_2_EQ1_A0 
	{0x05, 0x00, 0x00, 0x00},//##Channel_2_EQ2_A1 
	{0x06, 0x00, 0x00, 0x00},//##Channel_2_EQ2_A2 
	{0x07, 0x00, 0x00, 0x00},//##Channel_2_EQ2_B1 
	{0x08, 0x00, 0x00, 0x00},//##Channel_2_EQ2_B2 
	{0x09, 0x20, 0x00, 0x00},//##Channel_2_EQ2_A0 
	{0x0a, 0x00, 0x00, 0x00},//##Channel_2_EQ3_A1 
	{0x0b, 0x00, 0x00, 0x00},//##Channel_2_EQ3_A2 
	{0x0c, 0x00, 0x00, 0x00},//##Channel_2_EQ3_B1 
	{0x0d, 0x00, 0x00, 0x00},//##Channel_2_EQ3_B2 
	{0x0e, 0x20, 0x00, 0x00},//##Channel_2_EQ3_A0 
	{0x0f, 0x00, 0x00, 0x00},//##Channel_2_EQ4_A1 
	{0x10, 0x00, 0x00, 0x00},//##Channel_2_EQ4_A2 
	{0x11, 0x00, 0x00, 0x00},//##Channel_2_EQ4_B1 
	{0x12, 0x00, 0x00, 0x00},//##Channel_2_EQ4_B2 
	{0x13, 0x20, 0x00, 0x00},//##Channel_2_EQ4_A0 
	{0x14, 0x00, 0x00, 0x00},//##Channel_2_EQ5_A1 
	{0x15, 0x00, 0x00, 0x00},//##Channel_2_EQ5_A2 
	{0x16, 0x00, 0x00, 0x00},//##Channel_2_EQ5_B1 
	{0x17, 0x00, 0x00, 0x00},//##Channel_2_EQ5_B2 
	{0x18, 0x20, 0x00, 0x00},//##Channel_2_EQ5_A0 
	{0x19, 0x00, 0x00, 0x00},//##Channel_2_EQ6_A1 
	{0x1a, 0x00, 0x00, 0x00},//##Channel_2_EQ6_A2 
	{0x1b, 0x00, 0x00, 0x00},//##Channel_2_EQ6_B1 
	{0x1c, 0x00, 0x00, 0x00},//##Channel_2_EQ6_B2 
	{0x1d, 0x20, 0x00, 0x00},//##Channel_2_EQ6_A0 
	{0x1e, 0x00, 0x00, 0x00},//##Channel_2_EQ7_A1 
	{0x1f, 0x00, 0x00, 0x00},//##Channel_2_EQ7_A2 
	{0x20, 0x00, 0x00, 0x00},//##Channel_2_EQ7_B1 
	{0x21, 0x00, 0x00, 0x00},//##Channel_2_EQ7_B2 
	{0x22, 0x20, 0x00, 0x00},//##Channel_2_EQ7_A0 
	{0x23, 0x00, 0x00, 0x00},//##Channel_2_EQ8_A1 
	{0x24, 0x00, 0x00, 0x00},//##Channel_2_EQ8_A2 
	{0x25, 0x00, 0x00, 0x00},//##Channel_2_EQ8_B1 
	{0x26, 0x00, 0x00, 0x00},//##Channel_2_EQ8_B2 
	{0x27, 0x20, 0x00, 0x00},//##Channel_2_EQ8_A0 
	{0x28, 0x00, 0x00, 0x00},//##Channel_2_EQ9_A1 
	{0x29, 0x00, 0x00, 0x00},//##Channel_2_EQ9_A2 
	{0x2a, 0x00, 0x00, 0x00},//##Channel_2_EQ9_B1 
	{0x2b, 0x00, 0x00, 0x00},//##Channel_2_EQ9_B2 
	{0x2c, 0x20, 0x00, 0x00},//##Channel_2_EQ9_A0 
	{0x2d, 0x00, 0x00, 0x00},//##Channel_2_EQ10_A1 
	{0x2e, 0x00, 0x00, 0x00},//##Channel_2_EQ10_A2 
	{0x2f, 0x00, 0x00, 0x00},//##Channel_2_EQ10_B1 
	{0x30, 0x00, 0x00, 0x00},//##Channel_2_EQ10_B2 
	{0x31, 0x20, 0x00, 0x00},//##Channel_2_EQ10_A0 
	{0x32, 0x00, 0x00, 0x00},//##Channel_2_EQ11_A1 
	{0x33, 0x00, 0x00, 0x00},//##Channel_2_EQ11_A2 
	{0x34, 0x00, 0x00, 0x00},//##Channel_2_EQ11_B1 
	{0x35, 0x00, 0x00, 0x00},//##Channel_2_EQ11_B2 
	{0x36, 0x20, 0x00, 0x00},//##Channel_2_EQ11_A0 
	{0x37, 0x00, 0x00, 0x00},//##Channel_2_EQ12_A1 
	{0x38, 0x00, 0x00, 0x00},//##Channel_2_EQ12_A2 
	{0x39, 0x00, 0x00, 0x00},//##Channel_2_EQ12_B1 
	{0x3a, 0x00, 0x00, 0x00},//##Channel_2_EQ12_B2 
	{0x3b, 0x20, 0x00, 0x00},//##Channel_2_EQ12_A0 
	{0x3c, 0x00, 0x00, 0x00},//##Channel_2_EQ13_A1 
	{0x3d, 0x00, 0x00, 0x00},//##Channel_2_EQ13_A2 
	{0x3e, 0x00, 0x00, 0x00},//##Channel_2_EQ13_B1 
	{0x3f, 0x00, 0x00, 0x00},//##Channel_2_EQ13_B2 
	{0x40, 0x20, 0x00, 0x00},//##Channel_2_EQ13_A0 
	{0x41, 0x00, 0x00, 0x00},//##Channel_2_EQ14_A1 
	{0x42, 0x00, 0x00, 0x00},//##Channel_2_EQ14_A2 
	{0x43, 0x00, 0x00, 0x00},//##Channel_2_EQ14_B1 
	{0x44, 0x00, 0x00, 0x00},//##Channel_2_EQ14_B2 
	{0x45, 0x20, 0x00, 0x00},//##Channel_2_EQ14_A0 
	{0x46, 0x00, 0x00, 0x00},//##Channel_2_EQ15_A1 
	{0x47, 0x00, 0x00, 0x00},//##Channel_2_EQ15_A2 
	{0x48, 0x00, 0x00, 0x00},//##Channel_2_EQ15_B1 
	{0x49, 0x00, 0x00, 0x00},//##Channel_2_EQ15_B2 
	{0x4a, 0x20, 0x00, 0x00},//##Channel_2_EQ15_A0 
	{0x4b, 0x40, 0x00, 0x00},//##Channel_2_Mixer1 
	{0x4c, 0x40, 0x00, 0x00},//##Channel_2_Mixer2 
	{0x4d, 0x7f, 0xff, 0xff},//##Channel_2_Prescale 
	{0x4e, 0x7f, 0xff, 0xff},//##Channel_2_Postscale 
	{0x4f, 0xc7, 0xb6, 0x91},//##A0_of_R_channel_SRS_HPF 
	{0x50, 0x38, 0x49, 0x6e},//##A1_of_R_channel_SRS_HPF 
	{0x51, 0x0c, 0x46, 0xf8},//##B1_of_R_channel_SRS_HPF 
	{0x52, 0x0e, 0x81, 0xb9},//##A0_of_R_channel_SRS_LPF 
	{0x53, 0xf2, 0x2c, 0x12},//##A1_of_R_channel_SRS_LPF 
	{0x54, 0x0f, 0xca, 0xbb},//##B1_of_R_channel_SRS_LPF 
	{0x55, 0x00, 0x00, 0x00},//##Reserve
	{0x56, 0x00, 0x00, 0x00},//##Reserve
	{0x57, 0x00, 0x00, 0x00},//##Reserve
	{0x58, 0x00, 0x00, 0x00},//##Reserve
	{0x59, 0x00, 0x00, 0x00},//##Reserve
	{0x5a, 0x00, 0x00, 0x00},//##Reserve
	{0x5b, 0x00, 0x00, 0x00},//##Reserve
	{0x5c, 0x00, 0x00, 0x00},//##Reserve
	{0x5d, 0x00, 0x00, 0x00},//##Reserve
	{0x5e, 0x00, 0x00, 0x00},//##Reserve
	{0x5f, 0x00, 0x00, 0x00},//##Reserve
	{0x60, 0x00, 0x00, 0x00},//##Reserve
	{0x61, 0x00, 0x00, 0x00},//##Reserve
	{0x62, 0x00, 0x00, 0x00},//##Reserve
	{0x63, 0x00, 0x00, 0x00},//##Reserve
	{0x64, 0x00, 0x00, 0x00},//DRC2_Power_Meter
	{0x65, 0x00, 0x00, 0x00},//DRC4_Power_Mete
	{0x66, 0x00, 0x00, 0x00},//DRC6_Power_Meter
	{0x67, 0x00, 0x00, 0x00},//DRC8_Power_Meter
	{0x68, 0x00, 0x00, 0x00},//##Channel_2_DEQ1_A1 
	{0x69, 0x00, 0x00, 0x00},//##Channel_2_DEQ1_A2
	{0x6a, 0x00, 0x00, 0x00},//##Channel_2_DEQ1_B1
	{0x6b, 0x00, 0x00, 0x00},//##Channel_2_DEQ1_B2 
	{0x6c, 0x20, 0x00, 0x00},//##Channel_2_DEQ1_A0
	{0x6d, 0x00, 0x00, 0x00},//##Channel_2_DEQ2_A1 
	{0x6e, 0x00, 0x00, 0x00},//##Channel_2_DEQ2_A2 
	{0x6f, 0x00, 0x00, 0x00},//##Channel_2_DEQ2_B1 
	{0x70, 0x00, 0x00, 0x00},//##Channel_2_DEQ2_B2 
	{0x71, 0x20, 0x00, 0x00},//##Channel_2_DEQ2_A0 
	{0x72, 0x00, 0x00, 0x00},//##Channel_2_DEQ3_A1 
	{0x73, 0x00, 0x00, 0x00},//##Channel_2_DEQ3_A2 
	{0x74, 0x00, 0x00, 0x00},//##Channel_2_DEQ3_B1 
	{0x75, 0x00, 0x00, 0x00},//##Channel_2_DEQ3_B2 
	{0x76, 0x20, 0x00, 0x00},//##Channel_2_DEQ3_A0 
	{0x77, 0x00, 0x00, 0x00},//##Channel_2_DEQ4_A1 
	{0x78, 0x00, 0x00, 0x00},//##Channel_2_DEQ4_A2 
	{0x79, 0x00, 0x00, 0x00},//##Channel_2_DEQ4_B1 
	{0x7a, 0x00, 0x00, 0x00},//##Channel_2_DEQ4_B2 
	{0x7b, 0x20, 0x00, 0x00},//##Channel_2_DEQ4_A0 
	{0x7c, 0x00, 0x00, 0x00},//##Reserve
	{0x7d, 0x00, 0x00, 0x00},//##Reserve
	{0x7e, 0x00, 0x00, 0x00},//##Reserve
	{0x7f, 0x00, 0x00, 0x00},//##Reserve
};
/* codec private data */
struct ad82584f_priv {
	struct regmap *regmap;
	struct snd_soc_codec *codec;
	struct ad82584f_platform_data *pdata;
	struct delayed_work work;
	int gpio;
};

static int ad82584f_set_dai_sysclk(struct snd_soc_dai *codec_dai,
		int clk_id, unsigned int freq, int dir)
{
	return 0;
}

static int ad82584f_set_dai_fmt(struct snd_soc_dai *codec_dai, unsigned int fmt)
{
	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
		case SND_SOC_DAIFMT_CBS_CFS:
			break;
		default:
			return 0;
	}

	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
		case SND_SOC_DAIFMT_I2S:
		case SND_SOC_DAIFMT_RIGHT_J:
		case SND_SOC_DAIFMT_LEFT_J:
			break;
		default:
			return 0;
	}

	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
		case SND_SOC_DAIFMT_NB_NF:
			break;
		case SND_SOC_DAIFMT_NB_IF:
			break;
		default:
			return 0;
	}

	return 0;
}

static int ad82584f_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params,
		struct snd_soc_dai *dai)
{
	unsigned int rate;

	rate = params_rate(params);
	pr_debug("rate: %u\n", rate);

	switch (params_format(params)) {
		case SNDRV_PCM_FORMAT_S24_LE:
		case SNDRV_PCM_FORMAT_S24_BE:
			pr_debug("24bit\n");
			/* fall through */
		case SNDRV_PCM_FORMAT_S32_LE:
		case SNDRV_PCM_FORMAT_S20_3LE:
		case SNDRV_PCM_FORMAT_S20_3BE:
			pr_debug("20bit\n");

			break;
		case SNDRV_PCM_FORMAT_S16_LE:
		case SNDRV_PCM_FORMAT_S16_BE:
			pr_debug("16bit\n");

			break;
		default:
			return -EINVAL;
	}

	return 0;
}

static int ad82584f_set_bias_level(struct snd_soc_codec *codec,
		enum snd_soc_bias_level level)
{
	pr_debug("level = %d\n", level);

	switch (level) {
		case SND_SOC_BIAS_ON:
			break;

		case SND_SOC_BIAS_PREPARE:
			/* Full power on */
			break;

		case SND_SOC_BIAS_STANDBY:
			break;

		case SND_SOC_BIAS_OFF:
			/* The chip runs through the power down sequence for us. */
			break;
	}
	codec->component.dapm.bias_level = level;

	return 0;
}

static int ad82584f_trigger(struct snd_pcm_substream *substream, int cmd,
		struct snd_soc_dai *dai)
{
	struct snd_soc_codec *codec = dai->codec;
	struct ad82584f_priv *ad82584f = snd_soc_codec_get_drvdata(codec);
	//according to the ad82584f document, the pd gpio must pull up after the bclk has generated,
	// and pull down before the bclk missing.
	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		schedule_delayed_work(&ad82584f->work, msecs_to_jiffies(1));
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		gpio_direction_output(ad82584f->gpio, 0);
		break;
	}
	return 0;
}

static const struct snd_soc_dai_ops ad82584f_dai_ops = {
	.hw_params = ad82584f_hw_params,
	.set_sysclk = ad82584f_set_dai_sysclk,
	.set_fmt = ad82584f_set_dai_fmt,
	.trigger = ad82584f_trigger,
};

static struct snd_soc_dai_driver ad82584f_dai = {
	.name = "ad82584f",
	.playback = {
		.stream_name = "ESMT EQ DAC",
		.channels_min = 2,
		.channels_max = 8,
		.rates = AD82584F_RATES,
		.formats = AD82584F_FORMATS,
	},
	.ops = &ad82584f_dai_ops,
};


static int ad82584f_set_eq_drc(struct snd_soc_codec *codec)
{
	u8 i;

	for (i = 0; i < ARRAY_SIZE(m_ram1_tab); i++) {
		snd_soc_write(codec, CFADDR, m_ram1_tab[i][0]);
		snd_soc_write(codec, A1CF1, m_ram1_tab[i][1]);
		snd_soc_write(codec, A1CF2, m_ram1_tab[i][2]);
		snd_soc_write(codec, A1CF3, m_ram1_tab[i][3]);
		snd_soc_write(codec, CFUD, 0x01);
	}
	for (i = 0; i < ARRAY_SIZE(m_ram2_tab); i++) {
		snd_soc_write(codec, CFADDR, m_ram2_tab[i][0]);
		snd_soc_write(codec, A1CF1, m_ram2_tab[i][1]);
		snd_soc_write(codec, A1CF2, m_ram2_tab[i][2]);
		snd_soc_write(codec, A1CF3, m_ram2_tab[i][3]);
		snd_soc_write(codec, CFUD, 0x41);
	}
	return 0;
}


static int ad82584f_reg_init(struct snd_soc_codec *codec)
{
	int i = 0;
	for (i = 0; i < AD82584F_REGISTER_COUNT; i++) {
		snd_soc_write(codec, m_reg_tab[i][0], m_reg_tab[i][1]);
	};
}


static void ad82584f_pa_enable_work(struct work_struct *work)
{
	struct ad82584f_priv *ad82584f = container_of(work, struct ad82584f_priv, work.work);
	gpio_direction_output(ad82584f->gpio, 1);
}

static int ad82584f_init(struct snd_soc_codec *codec)
{

	struct ad82584f_priv *ad82584f;
	dev_info(codec->dev, "ad82584f_init!\n");
	ad82584f = (struct ad82584f_priv *)snd_soc_codec_get_drvdata(codec);
	INIT_DELAYED_WORK(&ad82584f->work, ad82584f_pa_enable_work);

	udelay(250 * 1000);
	ad82584f_reg_init(codec);

	/*unmute,default power-on is mute.*/
	ad82584f_set_eq_drc(codec);
	snd_soc_write(codec, 0x02, 0x00);

	return 0;
}

static int ad82584f_probe(struct snd_soc_codec *codec)
{
	ad82584f_init(codec);
	return 0;
}

static int ad82584f_remove(struct snd_soc_codec *codec)
{
	struct ad82584f_priv *ad82584f;
	ad82584f = (struct ad82584f_priv *)snd_soc_codec_get_drvdata(codec);
	flush_delayed_work(&ad82584f->work);
	return 0;
}

#ifdef CONFIG_PM
static int ad82584f_suspend(struct snd_soc_codec *codec)
{
	dev_info(codec->dev, "ad82584f_suspend!\n");

	return 0;
}

static int ad82584f_resume(struct snd_soc_codec *codec)
{
	dev_info(codec->dev, "ad82584f_resume!\n");

	return 0;
}
#else
#define ad82584f_suspend NULL
#define ad82584f_resume NULL
#endif


static const struct snd_soc_dapm_widget ad82584f_dapm_widgets[] = {
	SND_SOC_DAPM_DAC("DAC", "ESMT EQ DAC", SND_SOC_NOPM, 0, 0),
};

static const struct snd_soc_codec_driver soc_codec_dev_ad82584f = {
	.probe = ad82584f_probe,
	.remove = ad82584f_remove,
	.suspend = ad82584f_suspend,
	.resume = ad82584f_resume,
	.set_bias_level = ad82584f_set_bias_level,
	.component_driver = {
		.controls = ad82584f_snd_controls,
		.num_controls = ARRAY_SIZE(ad82584f_snd_controls),
		.dapm_widgets = ad82584f_dapm_widgets,
		.num_dapm_widgets = ARRAY_SIZE(ad82584f_dapm_widgets),
	}
};

static const struct regmap_config ad82584f_regmap = {
	.reg_bits = 8,
	.val_bits = 8,

	.max_register = AD82584F_REGISTER_COUNT,
	.reg_defaults = ad82584f_reg_defaults,
	.num_reg_defaults = ARRAY_SIZE(ad82584f_reg_defaults),
	.cache_type = REGCACHE_NONE,
};

static int ad82584f_i2c_probe(struct i2c_client *i2c,
		const struct i2c_device_id *id)
{
	struct ad82584f_priv *ad82584f;
	int ret;
	struct device *dev;
	pr_err("%s \n", __func__);
	ad82584f = devm_kzalloc(&i2c->dev, sizeof(struct ad82584f_priv),
			GFP_KERNEL);
	if (!ad82584f)
		return -ENOMEM;

	ad82584f->regmap = devm_regmap_init_i2c(i2c, &ad82584f_regmap);
	if (IS_ERR(ad82584f->regmap)) {
		ret = PTR_ERR(ad82584f->regmap);
		dev_err(&i2c->dev, "Failed to allocate register map: %d\n",
				ret);
		return ret;
	}

	i2c_set_clientdata(i2c, ad82584f);
	dev = &i2c->dev;
	if (dev) {
		ad82584f->gpio = of_get_named_gpio(dev->of_node, "enable-gpio", 0);
		if(!gpio_is_valid(ad82584f->gpio)) {
			dev_err(&i2c->dev, "Can not find pa control gpio\n");
			ret = -EINVAL;
			return ret;
		}
		ret = gpio_request(ad82584f->gpio, "audio-pa-gpio");
		if (ret) {
			dev_err(&i2c->dev, "Can't request audio pa gpio %d\n", ad82584f->gpio);
			return ret;
		}
	}

	ret = snd_soc_register_codec(&i2c->dev, &soc_codec_dev_ad82584f,
			&ad82584f_dai, 1);
	if (ret != 0)
		dev_err(&i2c->dev, "Failed to register codec (%d)\n", ret);

	return ret;
}

static int ad82584f_i2c_remove(struct i2c_client *client)
{
	struct ad82584f_priv *ad82584f;
	ad82584f = (struct ad82584f_priv *)i2c_get_clientdata(client);
	if (ad82584f->gpio)
		gpio_free(ad82584f->gpio);
	snd_soc_unregister_codec(&client->dev);

	return 0;
}

static const struct i2c_device_id ad82584f_i2c_id[] = {
	{ " ad82584f", 0 },
	{}
};
MODULE_DEVICE_TABLE(i2c, ad82584f_i2c_id);

static const struct of_device_id ad82584f_of_id[] = {
	{ .compatible = "ESMT, ad82584f", },
	{ /* senitel */ }
};
MODULE_DEVICE_TABLE(of, ad82584f_of_id);

static struct i2c_driver ad82584f_i2c_driver = {
	.class = I2C_CLASS_HWMON,
	.driver = {
		.name = "ad82584f",
		.of_match_table = ad82584f_of_id,
		.owner = THIS_MODULE,
	},
	.probe = ad82584f_i2c_probe,
	.remove = ad82584f_i2c_remove,
	.id_table = ad82584f_i2c_id,
};

module_i2c_driver(ad82584f_i2c_driver);
MODULE_DESCRIPTION("ASoC ad82584f driver");
MODULE_AUTHOR("siflower.com.cn");
MODULE_LICENSE("GPL");
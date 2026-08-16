/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2024 - 2024 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Allwinner SoCs hdmirx driver.
 *
 * Copyright (C) 2024 Allwinner.
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/kfifo.h>
#include "../aw_hdmirx_define.h"
#include "aw_hdmirx_core_drv.h"

struct aw_hdmirx_core_s *g_aw_core;
u8 debug_log_current_port_id;
bool SigmaHDR10enabled = true;

const char *ColorFormatStrings[] = {
	"YUV420_888",
	"YUV420_1088",
	"YUV420_101010",
	"YUV420_121212",

	"YUV422_888",
	"YUV422_1088",
	"YUV422_101010",
	"YUV422_121212",

	"YUV444_888",
	"YUV444_101010",
	"YUV444_121212",

	"RGB_888",
	"RGB_101010",
	"RGB_121212",

	"YUV420_101010_P010_Low",
	"YUV420_101010_P010_High",

	"DETN_ALPHA_1_BIT",
	"Max",
};

const char *SourceIdStrings[] = {
	"kSourceId_Dummy",
	"kSourceId_VideoDec",
	"kSourceId_Image",
	"kSourceId_HDMI_1",
	"kSourceId_HDMI_2",
	"kSourceId_HDMI_3",
	"kSourceId_HDMI_4",
	"kSourceId_CVBS_1",
	"kSourceId_CVBS_2",
	"kSourceId_CVBS_3",
	"kSourceId_ATV",
	"kSourceId_Max",
};

const char *AudioTypeStrings[] = {
	"UNKOWN",
	"L-PCM",
	"DSD",
	"DST",
	"HBR",
};

const char *getModeDescription(int mode)
{
	switch (mode) {
	case 0: return "NoSignal";
	case AI_SIGNAL_MODE_1080P: return "1080P";
	case AI_SIGNAL_MODE_UNKNOW: return "UnknownMode";
	case AI_SIGNAL_MODE_NO_SIGNAL: return "NoSignal";
	case AI_SIGNAL_MODE_480I: return "480I";
	case AI_SIGNAL_MODE_480IX2: return "480IX2";
	case AI_SIGNAL_MODE_480P: return "480P";
	case AI_SIGNAL_MODE_480PX2: return "480PX2";
	case AI_SIGNAL_MODE_576I: return "576I";
	case AI_SIGNAL_MODE_576IX2: return "576IX2";
	case AI_SIGNAL_MODE_576P: return "576P";
	case AI_SIGNAL_MODE_576PX2: return "576PX2";
	case AI_SIGNAL_MODE_720P: return "720P";
	case AI_SIGNAL_MODE_1080I: return "1080I";
	case AI_SIGNAL_MODE_1440P2560: return "1440P2560";
	case AI_SIGNAL_MODE_2160P1920: return "2160P1920";
	case AI_SIGNAL_MODE_2160P3840: return "2160P3840";
	case AI_SIGNAL_MODE_2160P4096: return "2160P4096";
	case AI_SIGNAL_MODE_640_350: return "640_350";
	case AI_SIGNAL_MODE_640_400: return "640_400";
	case AI_SIGNAL_MODE_640_480: return "640_480";
	case AI_SIGNAL_MODE_720_400: return "720_400";
	case AI_SIGNAL_MODE_720_576: return "720_576";
	case AI_SIGNAL_MODE_800_600: return "800_600";
	case AI_SIGNAL_MODE_832_624: return "832_624";
	case AI_SIGNAL_MODE_848_480: return "848_480";
	case AI_SIGNAL_MODE_852_480: return "852_480";
	case AI_SIGNAL_MODE_960_600: return "960_600";
	case AI_SIGNAL_MODE_1024_768: return "1024_768";
	case AI_SIGNAL_MODE_1152_864: return "1152_864";
	case AI_SIGNAL_MODE_1152_870: return "1152_870";
	case AI_SIGNAL_MODE_1152_900: return "1152_900";
	case AI_SIGNAL_MODE_1280_720: return "1280_720";
	case AI_SIGNAL_MODE_1280_768: return "1280_768";
	case AI_SIGNAL_MODE_1280_800: return "1280_800";
	case AI_SIGNAL_MODE_1280_960: return "1280_960";
	case AI_SIGNAL_MODE_1280_1024: return "1280_1024";
	case AI_SIGNAL_MODE_1360_765: return "1360_765";
	case AI_SIGNAL_MODE_1360_768: return "1360_768";
	case AI_SIGNAL_MODE_1600_1200: return "1600_1200";
	case AI_SIGNAL_MODE_1920_1080: return "1920_1080";
	case AI_SIGNAL_MODE_1920_1200: return "1920_1200";
	case AI_SIGNAL_MODE_1365_1024: return "1365_1024";
	case AI_SIGNAL_MODE_1366_768: return "1366_768";
	case AI_SIGNAL_MODE_1440_900: return "1440_900";
	case AI_SIGNAL_MODE_1400_1050: return "1400_1050";
	case AI_SIGNAL_MODE_1680_1050: return "1680_1050";
	case AI_SIGNAL_MODE_1792_1344: return "1792_1344";
	case AI_SIGNAL_MODE_1856_1392: return "1856_1392";
	case AI_SIGNAL_MODE_2560_1440: return "2560_1440";
	case AI_SIGNAL_MODE_3840_2160: return "3840_2160";
	case AI_SIGNAL_MODE_4096_2160: return "4096_2160";
	case AI_SIGNAL_MODE_720_480I: return "720_480I";
	case AI_SIGNAL_MODE_720_240P: return "720_240P";
	case AI_SIGNAL_MODE_720_288P: return "720_288P";
	case AI_SIGNAL_MODE_640_480P: return "640_480P";
	case AI_SIGNAL_MODE_240PX2: return "240PX2";
	case AI_SIGNAL_MODE_288PX2: return "288PX2";
	default: return "Unknown Mode";
	}
}

unsigned int getFrameRate(int frameRateMode)
{
	switch (frameRateMode) {
	case AI_SIGNAL_FRAMERATE_60: return 60000;
	case AI_SIGNAL_FRAMERATE_23_97: return 23970;
	case AI_SIGNAL_FRAMERATE_24: return 24000;
	case AI_SIGNAL_FRAMERATE_25: return 25000;
	case AI_SIGNAL_FRAMERATE_29_97: return 29970;
	case AI_SIGNAL_FRAMERATE_30: return 30000;
	case AI_SIGNAL_FRAMERATE_43: return 43000;
	case AI_SIGNAL_FRAMERATE_47_952: return 47952;
	case AI_SIGNAL_FRAMERATE_48: return 48000;
	case AI_SIGNAL_FRAMERATE_50: return 50000;
	case AI_SIGNAL_FRAMERATE_56: return 56000;
	case AI_SIGNAL_FRAMERATE_59_934: return 59934;
	case AI_SIGNAL_FRAMERATE_59_94: return 59940;
	case AI_SIGNAL_FRAMERATE_67: return 67000;
	case AI_SIGNAL_FRAMERATE_70: return 70000;
	case AI_SIGNAL_FRAMERATE_72: return 72000;
	case AI_SIGNAL_FRAMERATE_75: return 75000;
	case AI_SIGNAL_FRAMERATE_82: return 82000;
	case AI_SIGNAL_FRAMERATE_85: return 85000;
	case AI_SIGNAL_FRAMERATE_90: return 90000;
	case AI_SIGNAL_FRAMERATE_100: return 100000;
	case AI_SIGNAL_FRAMERATE_119_88: return 119880;
	case AI_SIGNAL_FRAMERATE_120: return 120000;
	case AG_SIGNAL_FRAMERATE_ALLEXCEPT120: return 120000;
	case AG_SIGNAL_FRAMERATE_PC: return 30000;
	case AG_SIGNAL_FRAMERATE_VIDEO: return 30000;
	case AI_SIGNAL_FRAMERATE_8: return 8000;
	case AI_SIGNAL_FRAMERATE_12: return 12000;
	case AI_SIGNAL_FRAMERATE_15: return 15000;
	case AI_SIGNAL_FRAMERATE_16: return 16000;
	case AG_SIGNAL_FRAMERATE_LOWFRAMERATE: return 1000;
	case AG_SIGNAL_FRAMERATE_HIGHFRAMERATE: return 200000;
	case AG_SIGNAL_FRAMERATE_FRAMERATEOVER30HZ: return 30000;
	default: return 0;
	}
}

const char *getColorSpaceDescription(int colorSpaceMode)
{
	switch (colorSpaceMode) {
	case 0: return "BT709";
	case AI_SIGNAL_COLORSPACE_BT709: return "BT709";
	case AI_SIGNAL_COLORSPACE_BT601: return "BT601";
	case AI_SIGNAL_COLORSPACE_BT2020_NCLYCC: return "BT2020_NCLYCC";
	case AI_SIGNAL_COLORSPACE_BT2020_CLYCC: return "BT2020_CLYCC";
	case AI_SIGNAL_COLORSPACE_XVYCC: return "XVYCC";
	case AI_SIGNAL_COLORSPACE_RGB: return "RGB";
	case AG_SIGNAL_COLORSPACE_ALL_SIGNAL_CS: return "ALL_SIGNAL_CS";
	default: return "Unknown Color Space";
	}
}

U32 DataMap_SourceIdToTfd(TSourceId source_id)
{
	U32 uc;

	switch (source_id) {
	case kSourceId_HDMI_1:
		uc = AI_SIGNAL_CHANNEL_HDMI1;
		break;
	case kSourceId_HDMI_2:
		uc = AI_SIGNAL_CHANNEL_HDMI2;
		break;
	case kSourceId_HDMI_3:
		uc = AI_SIGNAL_CHANNEL_HDMI3;
		break;
	default:
		uc = 0xf;
		break;
	}
	return uc;
}

U8 HDMIChannelMapToPort(U32 dwChannel)
{
	U8 uc;

	switch (dwChannel) {
	case AI_SIGNAL_CHANNEL_HDMI1:
		uc = HDMI_PORT1;
		break;
	case AI_SIGNAL_CHANNEL_HDMI2:
		uc = HDMI_PORT2;
		break;
	case AI_SIGNAL_CHANNEL_HDMI3:
		uc = HDMI_PORT3;
		break;
	default:
		uc = 0xf;
		break;
	}
	return uc;
}

ssize_t aw_core_edid_parse(char *buf)
{
	ssize_t n = 0;
	U8 edid_data[EDID_DATA_LEN] = {0};
	struct edid_info_s edid_info;
	HDMIRx_Edid_Getedid(edid_data);

	memset(&edid_info, 0, sizeof(struct edid_info_s));
	HDMIRx_Edid_Dumpedid(edid_data, buf, &n);
	HDMIRx_Edid_Parse_Block0(edid_data, &edid_info, buf, &n);
	HDMIRx_Edid_Parse_Cea_Block(&(edid_data[128]), &edid_info,  buf, &n);
	HDMIRx_Edid_Parse_Print(&edid_info, buf, &n);

	return n;
}

ssize_t aw_core_dump_SignalInfo(char *buf)
{
	TSignalInfo *psignal_info;
	ssize_t n = 0;

	psignal_info = HDMIRx_DisplayModuleCtx_GetSignalInfo();

	n += sprintf(buf + n, "video\n");
	n += sprintf(buf + n, "    source_id              = %s\n", SourceIdStrings[psignal_info->source_id]);
	n += sprintf(buf + n, "    signal_id              = %s\n", getModeDescription(psignal_info->signal_id));
	n += sprintf(buf + n, "    frame_rate             = %d\n", getFrameRate(psignal_info->frame_rate));
	n += sprintf(buf + n, "    b_interlace            = %d\n", psignal_info->b_interlace);
	n += sprintf(buf + n, "    color_format           = %s\n", ColorFormatStrings[psignal_info->color_format]);
	n += sprintf(buf + n, "    color_space            = %s\n", getColorSpaceDescription(psignal_info->color_space));
	n += sprintf(buf + n, "    resolution.h_size      = %d\n", psignal_info->ext.hdmi.timing.h_active);
	n += sprintf(buf + n, "    resolution.v_size      = %d\n", psignal_info->ext.hdmi.timing.v_active);
	n += sprintf(buf + n, "    hdr_mode               = %d\n", psignal_info->hdr_scheme);
	n += sprintf(buf + n, "    b_full_range           = %d\n", psignal_info->ext.hdmi.b_full_range);
	n += sprintf(buf + n, "    b_dvi_mode             = %d\n", psignal_info->ext.hdmi.b_dvi_mode);
	n += sprintf(buf + n, "    tmds_clock             = %d\n", psignal_info->ext.hdmi.tmds_clock);
	n += sprintf(buf + n, "    pixel_repetition       = %d\n", psignal_info->ext.hdmi.prmode);
	n += sprintf(buf + n, "    video_mute             = %d\n", g_aw_core->pDisplayModuleCtx->videomute);
	n += sprintf(buf + n, "audio\n");
	n += sprintf(buf + n, "    cts                    = %d\n", g_aw_core->pActivePort ?
														g_aw_core->pActivePort->PortCtx.audioCtx.ctx.CTS : 0);
	n += sprintf(buf + n, "    N                      = %d\n", g_aw_core->pActivePort ?
														g_aw_core->pActivePort->PortCtx.audioCtx.ctx.N : 0);
	n += sprintf(buf + n, "    audio_fs               = %d\n", g_aw_core->pActivePort ?
														g_aw_core->pActivePort->PortCtx.audioCtx.ctx.fs : 0);
	n += sprintf(buf + n, "    audio_PLLfs            = %d\n", g_aw_core->pActivePort ?
														g_aw_core->pActivePort->PortCtx.audioCtx.ctx.audioPLLfs : 0);
	n += sprintf(buf + n, "    audio_type             = %s\n", g_aw_core->pActivePort ?
														AudioTypeStrings[g_aw_core->pActivePort->PortCtx.audioCtx.ctx.audioType >> 4] : AudioTypeStrings[0]);
	n += sprintf(buf + n, "    fifo_errcnt            = %d\n", g_aw_core->pActivePort ?
														g_aw_core->pActivePort->PortCtx.audioCtx.ctx.fifoErrCnt : 0);
	n += sprintf(buf + n, "    audio_pkterrcnt        = %d\n", g_aw_core->pActivePort ?
														g_aw_core->pActivePort->PortCtx.audioCtx.ctx.audioPktErrCnt : 0);
	n += sprintf(buf + n, "    audio_mute             = %d\n", g_aw_core->pDisplayModuleCtx->audiomute);

	return n;
}

ssize_t aw_core_dump_status(char *buf)
{
	return THDMIRx_Port_Dump_Status(buf);
}

ssize_t aw_core_dump_hdcp14(char *buf)
{
	THdcp14Info hdcp14_info;
	ssize_t n = 0;
	memset(&hdcp14_info, 0, sizeof(THdcp14Info));
	HDMIRx_Port_GetHDCP14Status(&hdcp14_info);

	n += sprintf(buf + n, "aksv                   = 0x%x 0x%x 0x%x 0x%x 0x%x\n", \
				hdcp14_info.aksv[0], hdcp14_info.aksv[1], hdcp14_info.aksv[2], \
				hdcp14_info.aksv[3], hdcp14_info.aksv[4]);
	n += sprintf(buf + n, "bksv                   = 0x%x 0x%x 0x%x 0x%x 0x%x\n", \
				hdcp14_info.bksv[0], hdcp14_info.bksv[1], hdcp14_info.bksv[2], \
				hdcp14_info.bksv[3], hdcp14_info.bksv[4]);
	n += sprintf(buf + n, "keyload                = %d\n", hdcp14_info.hdcpkey);
	n += sprintf(buf + n, "authenticate           = %d\n", hdcp14_info.auth);

	return n;
}

void aw_core_SetPortMap(TSourceId source_id, U8 soc_port_id)
{
	hdmirx_inf("%s: source_id %d soc_port_id %d\n", __func__, source_id, soc_port_id);
	U8 i;
	HDMIPortMap_t *pMap = HdmiRx_GetPortMap();
	for (i = 0; i < HDMI_PORT_NUM; i++) {
		if (pMap[i].display_source_id == source_id) {
			hdmirx_inf("%s: Change Port(%d) Map, from %d to %d\n", __func__, \
			pMap[i].hdmi_port_id, pMap[i].mcu_port_id, soc_port_id);
			pMap[i].mcu_port_id = (MCUPortID_e)soc_port_id;
			break;
		}
	}

#if (HDMIRX_PORT_NEEDREMAP)
	HDMIRx_Port_SetRemap(true);
#endif
}

void aw_core_SetUpdateEdid(U8 *edid_table, U8 edid_version)
{
	HDMIRx_Edid_SetUpdateEdid(edid_table, edid_version);
}

void aw_core_SetHPDTimeInterval(U32 val)
{
	HDMIRx_Port_SetHPDTimeInterval(val);
}

void aw_core_SetARCEnable(bool bOn)
{
	HDMIRx_DataPath_SetARCEnable(bOn);
}

void aw_core_SetEdidVersion(U8 setting_version)
{
	HDMIRx_Edid_SetEdidVersion(setting_version);
}

void aw_core_SetPullHotplug(U8 port_id, U8 type)
{
	HDMIRx_Edid_SetPullHotplug(port_id, type);
}

void aw_core_Get5vState(U32 *pb5vstate)
{
	HdmiRx_Edid_Get_5VState(pb5vstate);
}

void aw_core_SetBlueScreen(bool bON)
{
	return HDMIRx_DisplayModuleCtx_SetBlueScreen(bON);
}

// switch port, active<-->bg
void aw_core_SwitchPort(struct THDMIRx_Port *pPort, bool isActive)
{
	if (!pPort)
		return;
	hdmirx_inf("%s: port id=%d isActive=%d tick=%d\n", __func__, pPort->PortCtx.PortID, isActive, HDMIRx_GetCurrentTick());
	if (isActive) {
		// bg--->active
		HDMIRx_Port_SetIsActivePort(pPort, true);
		HDMIRx_DataPath_Connect_DDC_Path(pPort);
		HDMIRx_DataPath_Connect_Link_Path(pPort);
		HDMIRx_DataPath_Connect_VideoOut_Path(pPort);
		HDMIRx_DataPath_Connect_AudioOut_Path(pPort);
	} else {
		// active-->bg
		HDMIRx_Port_SetIsActivePort(pPort, false);
		HDMIRx_DataPath_Disconnect_Link_Path(pPort);
		HDMIRx_DataPath_Disconnect_VideoOut_Path(pPort);
	}
}

void aw_core_SetActivePort(U8 PortID)
{
	U32 i;
	hdmirx_inf("%s: SetActivePort %d tick=%d\n", __func__, PortID, HDMIRx_GetCurrentTick());
	// switch port
	if (g_aw_core->pActivePort) {
		hdmirx_inf("%s: pActivePort is ture!!\n", __func__);
		if (g_aw_core->pActivePort->PortCtx.PortID != PortID) {
			aw_core_SwitchPort(g_aw_core->pActivePort, false);   //Cut away the original hdmi channel
			// find active port from BG
			for (i = 0; i < HDMI_PORT_NUM; i++) {
				struct THDMIRx_Port *pPort = g_aw_core->pPortArray[i];
				if (pPort->PortCtx.PortID == PortID) {
					g_aw_core->pActivePort = pPort;
					break;
				}
			}
			aw_core_SwitchPort(g_aw_core->pActivePort, true);  //Cut to current channel
		} else
			aw_core_SwitchPort(g_aw_core->pActivePort, true);  // from other source to active source

	} else {
		// find active port from BG
		//first select hdmi source
		hdmirx_inf("%s: pActivePort init!!\n", __func__);
		for (i = 0; i < HDMI_PORT_NUM; i++) {
			struct THDMIRx_Port *pPort = g_aw_core->pPortArray[i];
			hdmirx_inf("%s: pPort->PortCtx.PortID = %d PortID = %d\n", __func__, pPort->PortCtx.PortID, PortID);
			if (pPort->PortCtx.PortID == PortID) {
				g_aw_core->pActivePort = pPort;
				break;
			}
		}
		aw_core_SwitchPort(g_aw_core->pActivePort, true);
	}
}

#if IS_ENABLED(CONFIG_AW_VIDEO_SUNXI_HDMIRX_CEC)
int aw_hdmirx_core_set_cecbase(void)
{
	HdmiRx_Cec_Hardware_set_regbase(g_aw_core->reg_edid_top);
	return 0;
}

int aw_core_Cec_Enable(void)
{
	HdmiRx_Cec_Hardware_assist_ctrl();
	udelay(200);
	HdmiRx_Cec_Hardware_enable();
	return 0;
}

int aw_core_Cec_Disable(void)
{
	HdmiRx_Cec_Hardware_disable();
	udelay(200);

	return 0;
}

s32 aw_core_Cec_Receive(unsigned char *msg, unsigned *size)
{
	return HdmiRx_Cec_Hardware_receive_frame(msg, size);
}

s32 aw_core_Cec_Send(unsigned char *msg, unsigned size, unsigned frame_type)
{
	u32 dw_frame_type = 0;

	switch (frame_type) {
	case AW_HDMI_CEC_FRAME_TYPE_RETRY:
		dw_frame_type = CEC_CTRL_FRAME_TYP_RETRY;
		break;
	case AW_HDMI_CEC_FRAME_TYPE_NORMAL:
	default:
		dw_frame_type = CEC_CTRL_FRAME_TYP_NORMAL;
		break;
	case AW_HDMI_CEC_FRAME_TYPE_IMMED:
		dw_frame_type = CEC_CTRL_FRAME_TYP_IMMED;
		break;
	}
	return HdmiRx_Cec_Hardware_send_frame(msg, size, dw_frame_type);
}

int aw_core_Cec_Set_Logical_Addr(unsigned int addr)
{
	return HdmiRx_Cec_Hardware_set_logical_addr(addr);
}

int aw_core_Cec_interrupt_get_state(void)
{
	u32 dw_state = HdmiRx_Cec_Hardware_interrupt_get_state();
	u32 state = 0;

	if (dw_state & IH_CEC_STAT0_WAKEUP_MASK)
		state |= AW_HDMI_CEC_STAT_WAKEUP;
	if (dw_state & IH_CEC_STAT0_DONE_MASK)
		state |= AW_HDMI_CEC_STAT_DONE;
	if (dw_state & IH_CEC_STAT0_EOM_MASK)
		state |= AW_HDMI_CEC_STAT_EOM;
	if (dw_state & IH_CEC_STAT0_NACK_MASK)
		state |= AW_HDMI_CEC_STAT_NACK;
	if (dw_state & IH_CEC_STAT0_ARB_LOST_MASK)
		state |= AW_HDMI_CEC_STAT_ARBLOST;
	if (dw_state & IH_CEC_STAT0_ERROR_INITIATOR_MASK)
		state |= AW_HDMI_CEC_STAT_ERROR_INIT;
	if (dw_state & IH_CEC_STAT0_ERROR_FOLLOW_MASK)
		state |= AW_HDMI_CEC_STAT_ERROR_FOLL;

	return state;
}

void aw_core_Cec_interrupt_clear_state(unsigned state)
{
	u32 dw_state = 0;

	if (state & AW_HDMI_CEC_STAT_WAKEUP)
		dw_state |= IH_CEC_STAT0_WAKEUP_MASK;
	if (state & AW_HDMI_CEC_STAT_DONE)
		dw_state |= IH_CEC_STAT0_DONE_MASK;
	if (state & AW_HDMI_CEC_STAT_EOM)
		dw_state |= IH_CEC_STAT0_EOM_MASK;
	if (state & AW_HDMI_CEC_STAT_NACK)
		dw_state |= IH_CEC_STAT0_NACK_MASK;
	if (state & AW_HDMI_CEC_STAT_ARBLOST)
		dw_state |= IH_CEC_STAT0_ARB_LOST_MASK;
	if (state & AW_HDMI_CEC_STAT_ERROR_INIT)
		dw_state |= IH_CEC_STAT0_ERROR_INITIATOR_MASK;
	if (state & AW_HDMI_CEC_STAT_ERROR_FOLL)
		dw_state |= IH_CEC_STAT0_ERROR_FOLLOW_MASK;

	HdmiRx_Cec_Hardware_interrupt_clear_state(dw_state);
}

#endif

bool aw_core_Enable(TSourceId source_id)
{
	hdmirx_inf("%s: source_id %d HDMIRx_Enable\n", __func__, source_id);
	HRESULT ret = HDMIRx_DisplayModuleCtx_Enable(_OUTPUT_MP_, source_id);

	if (SUCCEEDED(ret)) {
		return true;
	}

	return false;
}

bool aw_core_AfterEnable(TSourceId source_id)
{
	hdmirx_inf("%s: source_id %d AfterEnable\n", __func__, source_id);
	U32 channel_id = DataMap_SourceIdToTfd(source_id);
	aw_core_SetActivePort(HDMIChannelMapToPort(channel_id));
	CHECK_POINTER(g_aw_core->pActivePort, E_FAIL);
	HDMIRx_DisplayModuleCtx_AfterEnable();
	if (g_aw_core->pActivePort->PortCtx.is5V) {
		aw_core_SetBlueScreen(true);
	} else {
		HDMIRx_DisplayModuleCtx_SetSignal(AI_SIGNAL_MODE_NO_SIGNAL, &g_aw_core->pActivePort->PortCtx);
		return true;
	}

	return true;
}

bool aw_core_Disable(void)
{
	hdmirx_inf("%s: HDMIRx_Disable\n", __func__);
	HRESULT ret = HDMIRx_DisplayModuleCtx_Disable(_OUTPUT_MP_);

	// for power saving, disable clk
	HdmiRx_MVTOP_ICGEnable(0, false);

	if (SUCCEEDED(ret)) {
		return true;
	}

	return false;
}


static int aw_core_ScanTask(void *parg)
{
	hdmirx_inf("throop HDMIRx_ScanTask\n");

	U8 i;
	while (1) {
		if (kthread_should_stop())
			break;

		for (i = 0; i < SizeOfArray(g_aw_core->pPortArray); i++) {
			HDMIRx_Port_EventTask(i + 1);
		}

		msleep(10);
	}
	return 0;
}

int aw_core_scan_thread_init(void)
{
	g_aw_core->hdmirx_scan_task = kthread_create(aw_core_ScanTask, (void *)0, "hdmirx scan");
	if (IS_ERR(g_aw_core->hdmirx_scan_task)) {
		hdmirx_err("%s: Unable to start kernel thread %s.\n",
				__func__, "hdmirx scan");
		g_aw_core->hdmirx_scan_task = NULL;
		return -1;
	}
	wake_up_process(g_aw_core->hdmirx_scan_task);
	return 0;
}

void aw_core_scan_thread_exit(void)
{
	if (g_aw_core->hdmirx_scan_task) {
		kthread_stop(g_aw_core->hdmirx_scan_task);
		g_aw_core->hdmirx_scan_task = NULL;
	}
}

static int aw_core_hdcp_task(void *parg)
{
	hdmirx_inf("throop HDMIRx_HdcpTask\n");

	while (1) {
		if (kthread_should_stop())
			break;

		if (g_aw_core->pActivePort) {
			debug_log_current_port_id = g_aw_core->pActivePort->PortCtx.PortID;
			//hdmirx_inf("1debug_log_current_port_id = %d\n", debug_log_current_port_id);
			HDMIRx_Port_HDCPTask(debug_log_current_port_id);
		}
		msleep(1);
	}
	return 0;
}

int aw_core_hdcp_thread_init(void)
{
	hdmirx_inf("%s: start.\n", __func__);

	g_aw_core->hdmirx_hdcp_task = kthread_create(aw_core_hdcp_task, (void *)0, "hdmirx hdcp");
	if (IS_ERR(g_aw_core->hdmirx_hdcp_task)) {
		hdmirx_err("%s: Unable to start kernel thread %s.\n",
				__func__, "hdmirx hdcp");
		g_aw_core->hdmirx_hdcp_task = NULL;
		return -1;
	}
	wake_up_process(g_aw_core->hdmirx_hdcp_task);
	return 0;
}

void aw_core_hdcp_thread_exit(void)
{
	if (g_aw_core->hdmirx_hdcp_task) {
		kthread_stop(g_aw_core->hdmirx_hdcp_task);
		g_aw_core->hdmirx_hdcp_task = NULL;
	}
}

static int aw_core_StateMachineTask(void *parg)
{
	hdmirx_inf("throop HDMIRx_StateMachineTask\n");

	while (1) {
		if (kthread_should_stop())
			break;

		if (g_aw_core->pActivePort) {
			debug_log_current_port_id = g_aw_core->pActivePort->PortCtx.PortID;
			//hdmirx_inf("1debug_log_current_port_id = %d\n", debug_log_current_port_id);
			HDMIRx_Port_StateMachineTask(debug_log_current_port_id);
		}
		msleep(10);
	}
	return 0;
}

int aw_core_statemachine_thread_init(void)
{
	hdmirx_inf("%s: start.\n", __func__);
	g_aw_core->hdmirx_statemachine_task = kthread_create(aw_core_StateMachineTask, (void *)0, "hdmirx statemachine");
	if (IS_ERR(g_aw_core->hdmirx_statemachine_task)) {
		hdmirx_err("%s: Unable to start kernel thread %s.\n",
				__func__, "hdmirx statemachine");
		g_aw_core->hdmirx_statemachine_task = NULL;
		return -1;
	}
	wake_up_process(g_aw_core->hdmirx_statemachine_task);
	return 0;
}

void aw_core_statemachine_thread_exit(void)
{
	if (g_aw_core->hdmirx_statemachine_task) {
		kthread_stop(g_aw_core->hdmirx_statemachine_task);
		g_aw_core->hdmirx_statemachine_task = NULL;
	}
}

int aw_hdmirx_core_init_thread(void)
{
	if (aw_core_statemachine_thread_init() != 0) {
		hdmirx_err("%s: hdmirx statemachine thread init failed!!!\n", __func__);
		return -1;
	}

	if (aw_core_hdcp_thread_init() != 0) {
		hdmirx_err("%s: hdmirx hdcp thread init failed!!!\n", __func__);
		return -1;
	}

	if (aw_core_scan_thread_init() != 0) {
		hdmirx_err("%s: hdmirx scan thread init failed!!!\n", __func__);
		return -1;
	}

	return 0;
}

void aw_hdmirx_core_set_regbase(void)
{
	HdmiRx_set_regbase(g_aw_core->reg_base_vs);
	HdmiRx_Audio_set_regbase(g_aw_core->reg_base_vs);
	HdmiRx_Video_set_regbase(g_aw_core->reg_base_vs);
	HdmiRx_Edid_Hardware_set_regbase(g_aw_core->reg_edid_top);

	HdmiRx_Edid_Hardware_set_top_cfg();
	HdmiRx_Edid_Hardware_set_5v_cfg();
	HdmiRx_set_vincap_enable();
}

int aw_hdmirx_core_init(struct aw_hdmirx_core_s *core)
{
	U32 i;
	int ret = 0;
	g_aw_core = core;
	hdmirx_inf("%s: start.\n", __func__);

	aw_hdmirx_core_set_regbase();

	g_aw_core->pActivePort = NULL;

	g_aw_core->pDisplayModuleCtx = kzalloc(sizeof(struct THDMIRx_DisplayModuleCtx), GFP_KERNEL);
	if (!g_aw_core->pDisplayModuleCtx) {
		hdmirx_err("%s: could not allocated pDisplayModuleCtx!\n", __func__);
		return -1;
	}

	g_aw_core->pDataPath = kzalloc(sizeof(struct THDMIRx_DataPath), GFP_KERNEL);
	if (!g_aw_core->pDataPath) {
		hdmirx_err("%s: could not allocated pDataPath!\n", __func__);
		return -1;
	}

	memset(g_aw_core->pPortArray, 0, sizeof(struct THDMIRx_Port *) * HDMI_PORT_NUM);
	THDMIRx_DisplayModuleCtxinit(g_aw_core->pDisplayModuleCtx);
	THDMIRx_DataPathinit(g_aw_core->pDataPath);
	THDMIRx_Edidinit();

	for (i = 0; i < HDMI_PORT_NUM; i++) {
		g_aw_core->pPortArray[i] = kzalloc(sizeof(struct THDMIRx_Port), GFP_KERNEL);
		if (!g_aw_core->pPortArray[i]) {
			hdmirx_err("%s: could not allocated pPortArray!\n", __func__);
			return -1;
		}
		THDMIRx_Portinit(i, g_aw_core->pPortArray[i], g_aw_core->pDisplayModuleCtx);
		HDMIRx_DataPath_SetPortArray(g_aw_core->pPortArray[i]);
	}

	ret = aw_hdmirx_core_init_thread();
	if (ret != 0) {
		hdmirx_err("%s: aw hdmi initial thread failed!!!\n", __func__);
		return -1;
	}

	return 0;
}

void aw_hdmirx_core_exit(struct aw_hdmirx_core_s *g_aw_core)
{
	int i;
	kfree(g_aw_core->pDisplayModuleCtx);
	for (i = 0; i < HDMI_PORT_NUM; i++) {
		if (g_aw_core->pPortArray[i]->pHDMIRxEvent.pEventQueue) {
			kfifo_free(g_aw_core->pPortArray[i]->pHDMIRxEvent.pEventQueue);
			kfree(g_aw_core->pPortArray[i]->pHDMIRxEvent.pEventQueue);
			hdmirx_inf("kfifo_free\n");
		}

		if (g_aw_core->pPortArray[i]) {
			kfree(g_aw_core->pPortArray[i]);
			hdmirx_inf("kfree pPortArray\n");
		}

		HDMIRx_Edid_DeinitHpdtimer(i);
	}
	kfree(g_aw_core->pDataPath);
	kfree(g_aw_core);
}

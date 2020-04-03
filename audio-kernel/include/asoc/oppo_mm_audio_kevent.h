/***************************************************************
** Copyright (C),  2019,  OPPO Mobile Comm Corp.,  Ltd
** VENDOR_EDIT
** File : oppo_mm_audio_kevent.h
** Description : MM audio kevent data
** Version : 1.0
** Date : 2019/02/03
** Author : Jianfeng.Qiu@PSW.MM.AudioDriver.Stability
**
** ------------------------------- Revision History: -----------
**  <author>        <data>        <version >        <desc>
**  Jianfeng.Qiu    2019/02/03     1.0             create this file
******************************************************************/

#ifdef CONFIG_OPPO_KEVENT_UPLOAD

#ifndef _OPPO_MM_AUDIO_KEVENT_
#define _OPPO_MM_AUDIO_KEVENT_

enum OPPO_MM_AUDIO_EVENT_ID {
	OPPO_MM_AUDIO_EVENT_ID_ADSP_RESET = 801,
	OPPO_MM_AUDIO_EVENT_ID_ADSP_FW_FAIL = 802,
	OPPO_MM_AUDIO_EVENT_ID_CLK_FAIL = 803,
	OPPO_MM_AUDIO_EVENT_ID_DBMDX_CARD_FAIL = 804,
	OPPO_MM_AUDIO_EVENT_ID_DBMDX_SPI_FAIL = 805,
	OPPO_MM_AUDIO_EVENT_ID_DBMDX_CHIP_ID_FAIL = 806,
	OPPO_MM_AUDIO_EVENT_ID_DBMDX_FW_VER_FAIL = 807,
	OPPO_MM_AUDIO_EVENT_ID_DBMDX_DETECTION_IRQ_FAIL = 808,
	OPPO_MM_AUDIO_EVENT_ID_MAX = 1000,
};

int upload_mm_audio_kevent_data(unsigned char *payload);

#endif /* _OPPO_MM_AUDIO_KEVENT_ */

#endif /* CONFIG_OPPO_KEVENT_UPLOAD */

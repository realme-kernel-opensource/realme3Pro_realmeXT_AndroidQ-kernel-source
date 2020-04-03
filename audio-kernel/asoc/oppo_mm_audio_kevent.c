/***************************************************************
** Copyright (C),  2019,  OPPO Mobile Comm Corp.,  Ltd
** VENDOR_EDIT
** File : oppo_mm_audio_kevent.c
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

#include <linux/err.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/oppo_kevent.h>

#include <asoc/oppo_mm_audio_kevent.h>

static DEFINE_MUTEX(mm_audio_kevent_lock);

int upload_mm_audio_kevent_data(unsigned char *payload)
{
	struct kernel_packet_info *user_msg_info;
	char log_tag[32] = "psw_multimedia";
	char event_id_audio[20] = "20181205";
	void *buffer = NULL;
	int len, size;

	mutex_lock(&mm_audio_kevent_lock);

	len = strlen(payload);

	size = sizeof(struct kernel_packet_info) + len + 1;
	pr_info("%s: kevent_send_to_user:size=%d\n", __func__, size);

	buffer = kmalloc(size, GFP_KERNEL);
	if (!buffer) {
		pr_err("%s: Allocation failed\n", __func__);
		mutex_unlock(&mm_audio_kevent_lock);
		return -ENOMEM;
	}
	memset(buffer, 0, size);
	user_msg_info = (struct kernel_packet_info *)buffer;
	user_msg_info->type = 1;

	memcpy(user_msg_info->log_tag, log_tag, strlen(log_tag) + 1);
	memcpy(user_msg_info->event_id, event_id_audio, strlen(event_id_audio) + 1);

	user_msg_info->payload_length = len + 1;
	memcpy(user_msg_info->payload, payload, user_msg_info->payload_length);

	kevent_send_to_user(user_msg_info);
	msleep(20);
	kfree(buffer);

	mutex_unlock(&mm_audio_kevent_lock);

	return 0;
}
EXPORT_SYMBOL(upload_mm_audio_kevent_data);

#endif /* CONFIG_OPPO_KEVENT_UPLOAD */
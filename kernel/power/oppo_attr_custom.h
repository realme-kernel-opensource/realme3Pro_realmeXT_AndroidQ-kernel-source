/******************************************************************
** Copyright (C), 2019-2030, OPPO Mobile Comm Corp., Ltd.
** VENDOR_EDIT
** File: - oppo_attr_custom.h
** Description: header file for oppo power attr.
** Version: 1.0
** Date : 2019/07/09
** Author: Cong.Dai@PSW.BSP.TP
**
** --------------------------- Revision History: ---------------------
* <version>	<date>		<author>              		<desc>
* Revision 1.0      2019/07/09       Cong.Dai@PSW.BSP.TP   	Created
*******************************************************************/
#ifndef _OPPO_ATTR_CUSTOM_H_
#define _OPPO_ATTR_CUSTOM_H_

char pon_reason[128];
static ssize_t pon_reason_show(struct kobject *kobj,
            struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%s", pon_reason);
}

static ssize_t pon_reason_store(struct kobject *kobj,
            struct kobj_attribute *attr,
            const char *buf, size_t n)
{
    return -EINVAL;
}
power_attr(pon_reason);


char poff_reason[128];
static ssize_t poff_reason_show(struct kobject *kobj,
    struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%s", poff_reason);
}

static ssize_t poff_reason_store(struct kobject *kobj,
            struct kobj_attribute *attr,
            const char *buf, size_t n)
{
    return -EINVAL;
}
power_attr(poff_reason);


extern char pwron_event[];
static ssize_t startup_mode_show(struct kobject *kobj, struct kobj_attribute *attr,
                 char *buf)
{
    return sprintf(buf, "%s", pwron_event);
}

static ssize_t startup_mode_store(struct kobject *kobj, struct kobj_attribute *attr,
               const char *buf, size_t n)
{
    return 0;
}
power_attr(startup_mode);


extern char boot_mode[];
static ssize_t app_boot_show(struct kobject *kobj, struct kobj_attribute *attr,
                 char *buf)
{
    return sprintf(buf, "%s", boot_mode);
}

static ssize_t app_boot_store(struct kobject *kobj, struct kobj_attribute *attr,
               const char *buf, size_t n)
{
    return 0;
}
power_attr(app_boot);

#endif  /*_OPPO_ATTR_CUSTOM_H_*/
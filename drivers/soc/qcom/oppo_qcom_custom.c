/**********************************************************************************
* Copyright (c)  2008-2016  Guangdong OPPO Mobile Comm Corp., Ltd
* VENDOR_EDIT
* Description: Provide some oppo custom function
* Version   : 1.0
* Date      : 2019-07-27
* Author    : daicong@psw.bsp.tp
* ------------------------------ Revision History: --------------------------------
* <version>       <date>        <author>          	<desc>
* Revision 1.0    2016-06-22    daicong@psw.bsp.tp	Created file
***********************************************************************************/

#include <asm/barrier.h>
#include <linux/printk.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>
#include <soc/qcom/smem.h>

#define MAX_SSR_REASON_LEN	256U


#ifdef CONFIG_RECORD_MDMRST
extern void mdmreason_set(char * buf);
#endif
int oppo_log_modem_sfr(void)
{
	u32 size;
	int rc = -1;
	char *smem_reason, reason[MAX_SSR_REASON_LEN];

	smem_reason = smem_get_entry_no_rlock(SMEM_SSR_REASON_MSS0, &size, 0,
							SMEM_ANY_HOST_FLAG);
	if (!smem_reason || !size) {
		pr_err("modem subsystem failure reason: (unknown, smem_get_entry_no_rlock failed).\n");
		return rc;
	}
	if (!smem_reason[0]) {
		pr_err("modem subsystem failure reason: (unknown, empty string found).\n");
		return rc;
	}
	strlcpy(reason, smem_reason, min(size, MAX_SSR_REASON_LEN));
#ifdef CONFIG_RECORD_MDMRST
	mdmreason_set(reason);
#endif
	pr_err("modem subsystem failure reason: %s.\n", reason);


	smem_reason[0] = '\0';
	wmb();
	return rc;
}
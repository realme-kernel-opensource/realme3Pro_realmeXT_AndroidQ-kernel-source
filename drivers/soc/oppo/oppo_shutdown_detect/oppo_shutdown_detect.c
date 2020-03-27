/**********************************************************************************
* Copyright (c)  2008-2015  Guangdong OPPO Mobile Comm Corp., Ltd
* VENDOR_EDIT
* Description:     shutdown_detect Monitor  Kernel Driver
*
* Version   : 1.0
* Date       : 2010-01-05
* Author     : wen.luo@PSW.Kernel.Stability
* ------------------------------ Revision History: --------------------------------
* <version>           <date>                <author>                            <desc>
* Revision 1.0        2010-01-05       wen.luo@PSW.Kernel.Stability       Created for shutdown_detect Monitor
***********************************************************************************/

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/sched.h>
#include <linux/sched/rt.h>
#include <linux/interrupt.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/mount.h>
#include <linux/kdev_t.h>
#include <linux/major.h>
#include <linux/reboot.h>
#include <linux/sysrq.h>
#include <linux/kbd_kern.h>
#include <linux/proc_fs.h>
#include <linux/nmi.h>
#include <linux/quotaops.h>
#include <linux/perf_event.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/suspend.h>
#include <linux/writeback.h>
#include <linux/swap.h>
#include <linux/spinlock.h>
#include <linux/vt_kern.h>
#include <linux/workqueue.h>
#include <linux/hrtimer.h>
#include <linux/delay.h>
#include <linux/oom.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/uaccess.h>
#include <linux/moduleparam.h>
#include <linux/jiffies.h>
#include <linux/syscalls.h>
#include <linux/of.h>
#include <linux/rcupdate.h>
#include <linux/kthread.h>

#include <asm/ptrace.h>
#include <asm/irq_regs.h>

#include <linux/sysrq.h>
#include <linux/clk.h>

#define SEQ_printf(m, x...)	    \
	do {			    \
		if (m)		    \
			seq_printf(m, x);	\
		else		    \
			pr_debug(x);	    \
	} while (0)

#define SS_DELAY_TIME_120S	120
#define INIT_DELAY_TIME_90S	90
#define SYSCALL_DELAY_TIME_60S	60
#define KERNEL_DELAY_TIME_30S	30

static struct hrtimer shutdown_detect_timer;
static unsigned int shutdown_phase;
static bool shutdown_detect_started = false;
static bool is_shutdows = false;
static struct task_struct *shutdown_task = NULL;

static int shutdown_kthread(void *data){
	kernel_power_off();
	return 0;
}

static ssize_t shutdown_detect_trigger(struct file *filp, const char *ubuf, size_t cnt, loff_t *data)
{
	char buf[64];
	long val;
	int ret;

	if (cnt >= sizeof(buf))
		return -EINVAL;

	if (copy_from_user(&buf, ubuf, cnt))
		return -EFAULT;

	buf[cnt] = 0;

	ret = kstrtoul(buf, 10, (unsigned long *)&val);

	if (ret < 0)
		return ret;

	if (val > 4) {
		is_shutdows = true;
		val = val - 4;
	}
	switch (val) {
	case 0:
		if (shutdown_detect_started) {
			shutdown_detect_started = false;
			hrtimer_cancel(&shutdown_detect_timer);
			shutdown_phase = 0;
		}
		pr_err("shutdown_detect: abort shutdown detect\n");
		break;
	case 2:
		if (!shutdown_detect_started) {
			shutdown_detect_started = true;
			hrtimer_start(&shutdown_detect_timer, ktime_set(SYSCALL_DELAY_TIME_60S, 0), HRTIMER_MODE_REL);
		}
		shutdown_phase = shutdown_phase|(1U << 1);
		pr_err("shutdown_detect: shutdown  current phase systemcall\n");
		break;
	case 3:
		if (!shutdown_detect_started) {
			shutdown_detect_started = true;
			hrtimer_start(&shutdown_detect_timer, ktime_set(INIT_DELAY_TIME_90S, 0), HRTIMER_MODE_REL);
		}
		shutdown_phase = shutdown_phase|(1U << 2);
		pr_err("shutdown_detect: shutdown  current phase init\n");
		break;
	case 4:
		if (!shutdown_detect_started) {
			shutdown_detect_started = true;
			hrtimer_start(&shutdown_detect_timer, ktime_set(SS_DELAY_TIME_120S, 0), HRTIMER_MODE_REL);
		}
		shutdown_phase = shutdown_phase|(1U << 3);
		pr_err("shutdown_detect: shutdown  current phase systemserver\n");
		break;
	default:
		break;
	}
	if(!shutdown_task && is_shutdows) {
		shutdown_task = kthread_create(shutdown_kthread, NULL,"shutdown_kthread");
		if (IS_ERR(shutdown_task)) {
			pr_err("create shutdown thread fail, will BUG()\n");
			msleep(60*1000);
			BUG();
		}
	}
	return cnt;
}

static int shutdown_detect_show(struct seq_file *m, void *v)
{
	SEQ_printf(m, "=== shutdown_detect controller ===\n");
	SEQ_printf(m, "0:   shutdown_detect abort\n");
	SEQ_printf(m, "2:   shutdown_detect systemcall reboot phase\n");
	SEQ_printf(m, "3:   shutdown_detect init reboot phase\n");
	SEQ_printf(m, "4:   shutdown_detect system server reboot phase\n");
	SEQ_printf(m, "=== shutdown_detect controller ===\n\n");
	SEQ_printf(m, "shutdown_detect: shutdown phase: %u\n", shutdown_phase);
	return 0;
}

static int shutdown_detect_open(struct inode *inode, struct file *file)
{
	return single_open(file, shutdown_detect_show, inode->i_private);
}

static const struct file_operations shutdown_detect_fops = {
	.open		= shutdown_detect_open,
	.write		= shutdown_detect_trigger,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static enum hrtimer_restart shutdown_detect_func(struct hrtimer *timer)
{
	pr_err("shutdown_detect:%s call sysrq show block and cpu thread. BUG\n", __func__);
	handle_sysrq('w');
	handle_sysrq('l');
	pr_err("shutdown_detect:%s shutdown_detect status:%u. \n", __func__, shutdown_phase);
//#ifdef CONFIG_OPPO_REALEASE_BUILD
	if(is_shutdows){
		pr_err("shutdown_detect: shutdown or reboot? shutdown\n");
		//kernel_power_off();
		if(shutdown_task) {
			wake_up_process(shutdown_task);
		}
	}else{
		pr_err("shutdown_detect: shutdown or reboot? reboot\n");
		BUG();
	}
//#endif
	return HRTIMER_NORESTART;;
}

static int __init init_shutdown_detect_ctrl(void)
{
	struct proc_dir_entry *pe;
	pr_err("shutdown_detect:register shutdown_detect interface\n");
	pe = proc_create("shutdown_detect", 0664, NULL, &shutdown_detect_fops);
	if (!pe) {
		pr_err("shutdown_detect:Failed to register shutdown_detect interface\n");
		return -ENOMEM;
	}
	hrtimer_init(&shutdown_detect_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	shutdown_detect_timer.function = shutdown_detect_func;
	//shutdown_detect_task = kthread_run(shutdown_detect_thread, NULL, "shutdown_detect_thread");
	return 0;
}

device_initcall(init_shutdown_detect_ctrl);


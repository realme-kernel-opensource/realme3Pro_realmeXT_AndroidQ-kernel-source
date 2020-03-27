#include <linux/fs.h>
#include <linux/init.h>
#include <linux/pid_namespace.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/sched/loadavg.h>
#include <linux/seq_file.h>
#include <linux/seqlock.h>
#include <linux/time.h>

static int color_em_show(struct seq_file *m, void *v)
{
	struct task_struct *p;

	rcu_read_lock();
	for_each_process(p) {
		if ((p->flags & PF_KTHREAD) == 0) {
	        seq_printf(m, "%d#_#%s;", task_pid_nr(p), p->comm);
		}
	}
	seq_printf(m, "\n");
	rcu_read_unlock();
	return 0;
}

static int color_em_open(struct inode *inode, struct file *file)
{
	return single_open(file, color_em_show, NULL);
}

static const struct file_operations color_em_fops = {
	.open		= color_em_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int __init color_em_init(void)
{
	proc_create("color_em", 0, NULL, &color_em_fops);
	return 0;
}
fs_initcall(color_em_init);

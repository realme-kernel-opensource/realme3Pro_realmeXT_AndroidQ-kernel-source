#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/seqlock.h>
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/ctype.h>
#include <linux/file.h>
#include <linux/string.h>
#include <linux/psi.h>
#include <asm/uaccess.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/list_sort.h>
#include <linux/oom.h>
#include <linux/mm.h>
#include <linux/err.h>
#include <linux/gfp.h>
#include <linux/swap.h>
#include <linux/timex.h>
#include <linux/jiffies.h>
#include <linux/cpuset.h>
#include <linux/export.h>
#include <linux/notifier.h>
#include <linux/memcontrol.h>
#include <linux/mempolicy.h>
#include <linux/security.h>
#include <linux/ptrace.h>
#include <linux/freezer.h>
#include <linux/ftrace.h>
#include <linux/ratelimit.h>
#include <linux/kthread.h>
#include <linux/init.h>
#include <linux/mmu_notifier.h>
#include <asm/tlb.h>
#include <linux/psi.h>
#include <linux/oom.h>
#include "olmk.h"

#define PSI_THRESHOLD_MAX_LEN 128
static DEFINE_MUTEX(trigger_mutex);
static void	 *trigger_ptr;


static char psi_thresholds[][PSI_THRESHOLD_MAX_LEN] = {
	{ "some 100000 1000000"},   /* 100ms out of 1s for partial stall */
	{ "full 70000  1000000"},     /* 70ms out of 1s for complete stall */
};
struct work_struct	psi_lmk_work;

struct task_points {
    char comm[TASK_COMM_LEN];
    int pid;
    int adj;
    int tasksize;
    int tasksize_swap;
    unsigned long points;
    struct list_head list;
};
static struct task_points tskp_all[150];

/*
 * [0]:100->0, total_ram * 2 / ([val] * 5)
 */
static unsigned int tasksize_mb_to_adj[10] = {2000, 300, 300, 300, 300, 200, 200, 160, 160, 160};
static int adj_index = 10;

int cache_app_gap =200;


extern int psi_mem_notifier_register(struct notifier_block *nb);
static int psi_level_adj_size = 3;
int psi_level;
enum psi_mem_level {
	PSI_MEM_LEVEL_LOW = 0,
	PSI_MEM_LEVEL_MEDIUM ,
	PSI_MEM_LEVEL_CRITICAL,
	PSI_MEM_LEVEL_COUNT
};
static unsigned int psi_level_adj[PSI_MEM_LEVEL_COUNT] = {
	OOM_SCORE_ADJ_MAX + 1,
	800,
	250,
};
module_param_array_named(psi_level_adj, psi_level_adj, uint, &psi_level_adj_size,
			 S_IRUGO | S_IWUSR);

int psi_adjust_minadj(short *min_score_adj)
{

    *min_score_adj = psi_level_adj[psi_level];

    return 0;
}
EXPORT_SYMBOL_GPL(psi_adjust_minadj);

int points_adjust_minadj(short *min_score_adj)
{
    if (*min_score_adj < OOM_SCORE_ADJ_MAX + 1) {
        if (*min_score_adj >= CACHED_APP_MIN_ADJ)
            *min_score_adj = SERVICE_B_ADJ;  

        if (*min_score_adj <= BACKUP_APP_ADJ)
            *min_score_adj = FOREGROUND_APP_ADJ;
    }

	return 0;

}
EXPORT_SYMBOL_GPL(points_adjust_minadj);

static bool MemAvailable_enough(void)
{
	int other_free, other_file;

	other_file = global_node_page_state(NR_FILE_PAGES) -
			global_node_page_state(NR_SHMEM) -
			total_swapcache_pages();
	other_free = global_page_state(NR_FREE_PAGES);


	return true;
}
static int lmk_psi_mem_monitor_notifier(struct notifier_block *nb,
				   unsigned long action, void *data)
{
	struct psi_trigger *trigger =( struct psi_trigger *)data;


    if (MemAvailable_enough())
		return 0;
	psi_level = (trigger->state == PSI_MEM_SOME )? PSI_MEM_LEVEL_MEDIUM : PSI_MEM_LEVEL_CRITICAL;


	schedule_work(&psi_lmk_work);

	return 0;
}

static struct notifier_block psi_mem_monitor_nb = {
	.notifier_call = lmk_psi_mem_monitor_notifier,
};
static void psi_lmk_run_work(struct work_struct *work)
{
}

static ssize_t psi_monitor_write( const char *buf,
				 size_t nbytes, enum psi_res res)
{
	size_t buf_size;
	char buffer[PSI_THRESHOLD_MAX_LEN];
	struct psi_trigger *new;
	printk(KERN_ERR "===>PSIDEBUG:psi_monitor_write\n");
	if (static_branch_likely(&psi_disabled))
		return -EOPNOTSUPP;

	buf_size = min(nbytes, (sizeof(buffer) - 1));
	memcpy(buffer, buf, buf_size);
	buffer[buf_size - 1] = '\0';

	new = psi_trigger_create(&psi_system, buffer, nbytes, res);
	if (IS_ERR(new))
		return PTR_ERR(new);

	mutex_lock(&trigger_mutex);
	psi_trigger_replace(&trigger_ptr, new);
	mutex_unlock(&trigger_mutex);
	/*add refcount for kernel psi poll*/
	kref_get(&new->refcount);

	return nbytes;
}


static int olmk_psi_monitor_init(void)
{
    int i, rc;
  
    for (i = 0; i < sizeof(psi_thresholds)/sizeof(psi_thresholds[0]); i++) {
        rc = psi_monitor_write(psi_thresholds[i], strlen(psi_thresholds[i]) + 1, PSI_MEM);
        if(rc  <= 0)
            pr_err("psi_monitor_write failed\n");
    }
    return rc;
}
unsigned long lmk_points(struct task_struct *p)
{
    long points;
    long adj, tasksize, taskpages, i;

    adj = (long)p->signal->oom_score_adj;
    
    if (adj <= 0 || in_vfork(p) ||
        test_bit(MMF_OOM_SKIP, &p->mm->flags)) {
		return 0;
    }

    points = adj;
    if (adj  >= CACHED_APP_MIN_ADJ)
        points += cache_app_gap * (adj - CACHED_APP_MIN_ADJ + 1);

    taskpages = get_mm_rss(p->mm) + (get_mm_counter(p->mm, MM_SWAPENTS) * zram_comp_ratio() / 100) +
		atomic_long_read(&p->mm->nr_ptes) + mm_nr_pmds(p->mm);
    tasksize = taskpages / 256;   // MB

    for (i = adj / 100; i < adj_index; i++) {
        if (tasksize >= tasksize_mb_to_adj[i]) {
            points += 100;
            tasksize -= tasksize_mb_to_adj[i];

            if (tasksize > 0 && i == (adj_index - 1)) {
                points += (tasksize / tasksize_mb_to_adj[i]) * 100 +
                    (tasksize % tasksize_mb_to_adj[i] * 100) / tasksize_mb_to_adj[i];
            }
        } else {
            points += (tasksize * 100) / tasksize_mb_to_adj[i];
            break;
        }
    }
    
    if (has_capability_noaudit(p, CAP_SYS_ADMIN))
		points -= (points * 3) / 100;

    return points > 0 ? points : 1;
}

static int list_do_fork_count_cmp(void *priv, struct list_head *a, struct list_head *b)
{
    struct task_points *tskp1 = NULL, *tskp2 = NULL;

    tskp1 = list_entry(a, struct task_points, list);
    tskp2 = list_entry(b, struct task_points, list);

    if (tskp1->points < tskp2->points) {
        return 1;
    }

    if (tskp1->points > tskp2->points) {
        return -1;
    }

    return 0;
}

static int task_points_show(struct seq_file *m, void *p)
{
    struct task_points *tskp = NULL, *tskp_new;
	struct task_struct *ptsk, *tsk;
    struct list_head tskp_head_list;
    int oom_score_adj, tasksize, tasksize_swap, count = 0;
    unsigned long points;

    INIT_LIST_HEAD(&(tskp_head_list));
    seq_printf(m, "-------------------------------------------------------\n");
    seq_printf(m, "task_comm\t\tpid\t\ttasksize\t\ttasksize+swap\t\tadj\t\tpoints\n");
	rcu_read_lock();
	for_each_process(tsk) {

		ptsk = find_lock_task_mm(tsk);
		if (!ptsk)
			continue;

		oom_score_adj = ptsk->signal->oom_score_adj;
        tasksize = get_mm_rss(ptsk->mm) + atomic_long_read(&ptsk->mm->nr_ptes) + mm_nr_pmds(ptsk->mm);
        tasksize_swap = get_mm_rss(ptsk->mm) + (get_mm_counter(ptsk->mm, MM_SWAPENTS) * zram_comp_ratio() / 100) +
                            atomic_long_read(&ptsk->mm->nr_ptes) + mm_nr_pmds(ptsk->mm);
        points = lmk_points(ptsk);

        if (points) {
            tskp_new = &tskp_all[count];
            memset(tskp_new, 0, sizeof(struct task_points));

            snprintf(tskp_new->comm, TASK_COMM_LEN, "%s", (char *)ptsk->comm);
            tskp_new->adj = oom_score_adj;
            tskp_new->pid = ptsk->pid;
            tskp_new->tasksize = tasksize/256;
            tskp_new->tasksize_swap = tasksize_swap/256;
            tskp_new->points = points;

            list_add(&tskp_new->list, &(tskp_head_list));
            count++;
        }

        task_unlock(ptsk);
    }

	rcu_read_unlock();
    list_sort(NULL,  &(tskp_head_list), list_do_fork_count_cmp);

    list_for_each_entry(tskp, &(tskp_head_list), list) {
        seq_printf(m, "%-24s%-24d%-24d%-24d%-24d%-24ld\n", tskp->comm, tskp->pid,
                tskp->tasksize, tskp->tasksize_swap, tskp->adj, tskp->points);
    }
    seq_printf(m, "-------------------------------------------------------\n");

    seq_printf(m, "zram_comp_ratio = %lu\n", zram_comp_ratio());
    return 0;
}


static int task_points_open(struct inode *inode, struct file *file)
{
    return single_open(file, task_points_show, NULL);
}

static const struct file_operations proc_task_points_fops = {
    .open       = task_points_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int __init olmk_init(void)
{
    struct proc_dir_entry *pentry;
    int ret = 0;

    ret = olmk_psi_monitor_init();
	
    psi_mem_notifier_register(&psi_mem_monitor_nb);
	INIT_WORK(&psi_lmk_work, psi_lmk_run_work);
    if (ret < 0) {
        pr_err("olmk_psi_monitor_init failed\n");
        return -1;
    }

	pentry = proc_create("task_points", S_IRWXUGO, NULL, &proc_task_points_fops);
	if(!pentry) {
		pr_err("create  proc ask_points failed.\n");
		return -1;
	}
    return 0;
}
device_initcall(olmk_init);

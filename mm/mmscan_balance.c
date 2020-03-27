#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/semaphore.h>
#include <linux/rwsem.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/cpu.h>
#include <linux/jiffies.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/vmstat.h>
#include <linux/kernel_stat.h>
#include <linux/tick.h>
#include <linux/version.h>
#include <linux/cpumask.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel_stat.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/irqnr.h>
#include <linux/cputime.h>
#include <linux/delay.h>
#include <linux/mmscan_balance.h>

#define CPU_LOADING_CRITICAL (90)
#define CPU_LOADING_HEAVY (80)

struct cpu_load_stat {
        u64 t_user;
        u64 t_system;
        u64 t_idle;
        u64 t_iowait;
        u64 t_irq;
        u64 t_softirq;
};

extern int kswapd_threads_current;
struct kswapd_cpu_account kc_account;

int cpu_loading;
static bool calc_in;
static unsigned long kswap_get_last_jiffies;
struct work_struct mmscan_balance_wt;

#ifdef arch_idle_time

static cputime64_t get_idle_time(int cpu)
{
	cputime64_t idle;

	idle = kcpustat_cpu(cpu).cpustat[CPUTIME_IDLE];
	if (cpu_online(cpu) && !nr_iowait_cpu(cpu))
		idle += arch_idle_time(cpu);
	return idle;
}

static cputime64_t get_iowait_time(int cpu)
{
	cputime64_t iowait;

	iowait = kcpustat_cpu(cpu).cpustat[CPUTIME_IOWAIT];
	if (cpu_online(cpu) && nr_iowait_cpu(cpu))
		iowait += arch_idle_time(cpu);
	return iowait;
}

#else

static u64 get_idle_time(int cpu)
{
	u64 idle, idle_time = -1ULL;

#ifdef CONFIG_MEDIATEK_SOLUTION
	idle_time = get_cpu_idle_time_us_wo_cpuoffline(cpu, NULL);
#else
	if (cpu_online(cpu))
		idle_time = get_cpu_idle_time_us(cpu, NULL);
#endif

	if (idle_time == -1ULL)
		/* !NO_HZ or cpu offline so we can rely on cpustat.idle */
		idle = kcpustat_cpu(cpu).cpustat[CPUTIME_IDLE];
	else
		idle = usecs_to_cputime64(idle_time);

	return idle;
}

static u64 get_iowait_time(int cpu)
{
	u64 iowait, iowait_time = -1ULL;

#ifdef CONFIG_MEDIATEK_SOLUTION
	iowait_time = get_cpu_iowait_time_us_wo_cpuoffline(cpu, NULL);
#else
	if (cpu_online(cpu))
		iowait_time = get_cpu_iowait_time_us(cpu, NULL);
#endif

	if (iowait_time == -1ULL)
		/* !NO_HZ or cpu offline so we can rely on cpustat.iowait */
		iowait = kcpustat_cpu(cpu).cpustat[CPUTIME_IOWAIT];
	else
		iowait = usecs_to_cputime64(iowait_time);

	return iowait;
}
#endif

void get_cur_cpuload(struct cpu_load_stat *cpu_load)
{
	int i;

	for_each_online_cpu(i) {
		cpu_load->t_user    += kcpustat_cpu(i).cpustat[CPUTIME_USER];
		cpu_load->t_system  += kcpustat_cpu(i).cpustat[CPUTIME_SYSTEM];
		cpu_load->t_idle    += get_idle_time(i);
		cpu_load->t_iowait  += get_iowait_time(i);
		cpu_load->t_irq     += kcpustat_cpu(i).cpustat[CPUTIME_IRQ];
		cpu_load->t_softirq += kcpustat_cpu(i).cpustat[CPUTIME_SOFTIRQ];
	}
}

int calc_cpu_loading(struct cpu_load_stat cpu_load, struct cpu_load_stat cpu_load_last)
{
    clock_t ct_user, ct_system, ct_idle, ct_iowait, ct_irq, ct_softirq, load, sum = 0;
    
    ct_user = cputime64_to_clock_t(cpu_load.t_user) - cputime64_to_clock_t(cpu_load_last.t_user);
    ct_system = cputime64_to_clock_t(cpu_load.t_system) - cputime64_to_clock_t(cpu_load_last.t_system);
    ct_idle = cputime64_to_clock_t(cpu_load.t_idle) - cputime64_to_clock_t(cpu_load_last.t_idle);
    ct_iowait = cputime64_to_clock_t(cpu_load.t_iowait) - cputime64_to_clock_t(cpu_load_last.t_iowait);
    ct_irq = cputime64_to_clock_t(cpu_load.t_irq) - cputime64_to_clock_t(cpu_load_last.t_irq);
    ct_softirq = cputime64_to_clock_t(cpu_load.t_softirq) - cputime64_to_clock_t(cpu_load_last.t_softirq);

	sum = ct_user + ct_system + ct_idle + ct_iowait + ct_irq + ct_softirq;
        load = ct_user + ct_system + ct_iowait + ct_irq + ct_softirq;

	if (sum == 0)
		return -1;

	return 100 * load / sum;
}

void _update_kswapd_load(void)
{
    if (jiffies_to_msecs(jiffies - kswap_get_last_jiffies) < 1000)
        return ;

    if (calc_in)
        return ;

    calc_in = true;
    schedule_work(&mmscan_balance_wt);
}
EXPORT_SYMBOL_GPL(_update_kswapd_load);

static void mmscan_balance_run_work(struct work_struct *work)
{
    int i = 0, nid = 0;
	pg_data_t *pgdat = NULL;
    unsigned long long kswap_exec_sum[kswapd_threads_current], kswap_exec_sum_last[kswapd_threads_current];
    unsigned int kswap_load_last;
	struct cpu_load_stat cpu_load_last = { 0, 0, 0, 0, 0, 0};
    struct cpu_load_stat cpu_load = { 0, 0, 0, 0, 0, 0};

    kswap_get_last_jiffies = jiffies;

    for_each_node_state(nid, N_MEMORY) {
        pgdat = NODE_DATA(nid);
       
        for (i = 0; i < kswapd_threads_current; i++) {

            if (pgdat->kswapd)
                continue;

            kswap_exec_sum_last[i] = pgdat->kswapd->se.sum_exec_runtime;
        }
    }
    get_cur_cpuload(&cpu_load_last); 
  
    msleep(1000);
    
    get_cur_cpuload(&cpu_load);
    cpu_loading = calc_cpu_loading(cpu_load, cpu_load_last);
    
    for_each_node_state(nid, N_MEMORY) {
        pgdat = NODE_DATA(nid);
       
        for (i = 0; i < kswapd_threads_current; i++) {

            if (pgdat->kswapd)
                continue;

            kswap_exec_sum[i] = pgdat->kswapd->se.sum_exec_runtime;
	        kswap_load_last = 100 * (kswap_exec_sum[i] - kswap_exec_sum_last[i]) / 1000000000;

            if ((cpu_loading > CPU_LOADING_HEAVY) && (kswap_load_last > CPU_LOADING_HEAVY)) {
                if (cpu_loading > CPU_LOADING_CRITICAL && kswap_load_last > CPU_LOADING_CRITICAL)
                    kc_account.critical_count += 1;
                else
                     kc_account.heavy_count += 1;
                printk(KERN_EMERG"cpu_loading: %d, kswapd_load[%d]:%d\n", cpu_loading, i, kswap_load_last);
            }
        }
    }
    calc_in = false;
}

static int __init lowmem_init(void)
{ 
    INIT_WORK(&mmscan_balance_wt, mmscan_balance_run_work);
    return 0;
}
device_initcall(lowmem_init);

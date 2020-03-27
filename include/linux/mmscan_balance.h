#ifndef MMSCAN_BALANCE_H
#define MMSCAN_BALANCE_H

struct kswapd_cpu_account {
    int heavy_count;
    int critical_count;
};

extern int cpu_loading;
extern void _update_kswapd_load(void);
extern struct kswapd_cpu_account kc_account;
extern void _update_kswapd_load(void);
#endif

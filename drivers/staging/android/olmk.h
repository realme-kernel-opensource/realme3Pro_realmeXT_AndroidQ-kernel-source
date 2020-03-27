#ifndef OLMK_H
#define OLMK_H

#include <linux/psi_types.h>

#define CACHED_APP_MAX_ADJ (906)
#define CACHED_APP_MIN_ADJ (900)
#define SERVICE_B_ADJ (800)
#define PREVIOUS_APP_ADJ (700)
#define BACKUP_APP_ADJ (300)
#define PERCEPTIBLE_APP_ADJ (200)
#define VISIBLE_APP_ADJ (100)
#define FOREGROUND_APP_ADJ (0)

extern struct psi_group psi_system;
extern int psi_adjust_minadj(short *min_score_adj);
extern unsigned long lmk_points(struct task_struct *p);
extern int points_adjust_minadj(short *min_score_adj);
extern unsigned long zram_comp_ratio(void);
extern int psi_level;
extern int enable_psi_lmk;
extern unsigned int almk_totalram_ratio;
extern u32 lowmem_debug_level;
#endif

/**************************************************************
* Copyright (c) 201X- 201X OPPO Mobile Comm Corp., Ltd.
*
* File : Op_bootprof.c
* Description: Source file for bootlog.
* Storage the boot log on proc.
* Version : 1.0
* Date : 2018-10-18
* Author : bright.zhang@oppo.com
* ---------------- Revision History: --------------------------
* <version> <date> < author > <desc>
****************************************************************/

/*=============================================================================

                            INCLUDE FILES FOR MODULE

=============================================================================*/
#ifdef VENDOR_EDIT
//Liang.Zhang@PSW.TECH.BOOTUP, 2018/10/19, Add for get bootup log
#ifdef HANG_OPPO_ALL

#include <linux/kernel.h>
#include <linux/fcntl.h>

#include <linux/unistd.h>
#include <linux/syscalls.h>
#include <linux/types.h>
#include <linux/stat.h>

#include <linux/fs.h>
#include <linux/errno.h>
#include <asm/errno.h>
#include <linux/string.h>
#include <linux/unistd.h>

#include "op_panic_recovery.h"

#ifndef CAP_DAC_OVERRIDE
#define CAP_DAC_OVERRIDE 1
#endif

#ifndef CAP_DAC_READ_SEARCH
#define CAP_DAC_READ_SEARCH  2
#endif

int isFirstWrite(struct pon_struct *pon_info)
{
    if(strcmp(pon_info->magic, "KernelRecovery") == 0)
    {
        pr_err("KernelRecovery flag not first write\n");
        return 0;
    }
    return 1;
}

int creds_change_dac(void)
{
/* need to add cred for Kernel RW file in SELINUX */
    struct cred *new;
    int rc = 0;

    pr_info("creds_change_dac enter!\n");

    new = prepare_creds();
    if (!new) {
            pr_err("opmonitor_boot_kthread init err!\n");
            rc = -1;
            return rc;
    }

    cap_raise(new->cap_effective, CAP_DAC_OVERRIDE);
    cap_raise(new->cap_effective, CAP_DAC_READ_SEARCH);

    rc = commit_creds(new);

    return rc;
}
int creds_change_id(void)
{
/* need to add cred for Kernel RW file in SELINUX */
    struct cred *new;
    int rc = 0;

    pr_info("creds_change_id enter!\n");

    new = prepare_creds();
    if (!new) {
            pr_err("opmonitor_boot_kthread init err!\n");
            rc = -1;
            return rc;
    }

    new->fsuid = new->euid = KUIDT_INIT(1000);

    rc = commit_creds(new);

    return rc;
}


int set_pon_info(struct pon_struct *pon_info, int fatal_error)
{
    int first_write = 0;
    int i = 0;
    int cur_pos = 0;
    int PonState = 0;

    if(fatal_error)
    {
        PonState = fatal_error;
    }

    first_write = isFirstWrite(pon_info);

    strcpy(pon_info->magic, "KernelRecovery");

    pr_info("set_pon_info first_write %d fatal_error %x \n", first_write, fatal_error);
    pr_info("pon_info.write_count %d \n", pon_info->write_count);

    if(first_write)
    {
        pon_info->write_count = 0;
        pon_info->pon_state[0] = PonState;
        for(i=1; i<RECOVERY_SUM;i++)
        {
            pon_info->pon_state[i]= PON_STATE_DEFAULT;
        }

    } else {
        cur_pos = (pon_info->write_count) % RECOVERY_SUM;
        if((cur_pos<0) || (cur_pos >= RECOVERY_SUM))
        {
            cur_pos=0;
        }
        pon_info->pon_state[cur_pos] = PonState;
    }

    pon_info->write_count = 1 + pon_info->write_count;
    pr_info("set_pon_info pon_info.magic %s pon_info.write_count %d \n", pon_info->magic, pon_info->write_count);

    pr_info("set_pon_info done \n");

    return 1;
}

static int read_from_reserve1(struct pon_struct *pon_info)
{
    struct file *opfile;
    struct file *devfile;
    ssize_t size;
    loff_t offsize;
    char data_info[64] = {'\0'};
    mm_segment_t old_fs;
    int ufs_type = 0;

    creds_change_dac();

    devfile = filp_open("/proc/devinfo/emmc", O_RDONLY, 0440);
    if (IS_ERR(devfile))
    {
        devfile = filp_open("/proc/devinfo/ufs", O_RDONLY, 0440);
        if(IS_ERR(devfile))
        {
            pr_err("read open /proc/devinfo/emmc /proc/devinfo/ufs error: (%ld)\n",PTR_ERR(devfile));
            return -1;
        } else {
            ufs_type = 1;
            offsize = OPPO_UFS_PARTITION_BOOTLOG_OFFSET + OPPO_UFS_PARTITION_SPEC_FLAG_OFFSET;
            filp_close(devfile,NULL);
        }
    } else {
        offsize = OPPO_EMMC_PARTITION_BOOTLOG_OFFSET + OPPO_EMMC_PARTITION_SPEC_FLAG_OFFSET;
        filp_close(devfile,NULL);
    }

    if(ufs_type) {
        opfile = filp_open(OPPO_PARTITION_OPPORESERVE1_UFS, O_RDONLY, 0440);
    } else {
        opfile = filp_open(OPPO_PARTITION_OPPORESERVE1_EMMC, O_RDONLY, 0440);
    }
    if (IS_ERR(opfile)) {
        pr_err("read open OPPO_PARTITION_OPPORESERVE_1 error: %ld\n",PTR_ERR(opfile));
        opfile = filp_open(OPPO_PARTITION_OPPORESERVE1_LINK, O_RDONLY, 0440);
        if (IS_ERR(opfile)) {
            pr_err("open OPPO_PARTITION_OPPORESERVE1_LINK error: %ld\n",PTR_ERR(opfile));
            opfile = filp_open(OPPO_PARTITION_OPPORESERVE1_LINK2, O_RDONLY, 0440);
            if (IS_ERR(opfile)) {
                pr_err("open OPPO_PARTITION_OPPORESERVE1_LINK2 error: %ld\n",PTR_ERR(opfile));
                return -1;
            }
        }
    }

    old_fs = get_fs();
    set_fs(KERNEL_DS);

    size = vfs_read(opfile, data_info, SIZE_OF_PON_STRUCT, &offsize);
    if (size < 0) {
        pr_err("read data_info %s size %ld", data_info, size);
        set_fs(old_fs);
        return -1;
    }
    set_fs(old_fs);
    filp_close(opfile,NULL);

    memcpy(pon_info, data_info, SIZE_OF_PON_STRUCT);

    pr_info("read_from_reserve1 pon_info.magic %s pon_info.write_count %d \n", pon_info->magic, pon_info->write_count);

    pr_info("read_from_reserve1 done\n");

    return 0;
}

int write_to_reserve1(struct pon_struct *pon_info, int fatal_error)
{
    struct file *opfile;
    struct file *devfile;
    ssize_t size;
    loff_t offsize;
    char data_info[64] = {'\0'};
    mm_segment_t old_fs;
    int rc;
    int ufs_type = 0;

    creds_change_dac();

    devfile = filp_open("/proc/devinfo/emmc", O_RDONLY, 0440);
    if (IS_ERR(devfile))
    {
        devfile = filp_open("/proc/devinfo/ufs", O_RDONLY, 0440);
        if(IS_ERR(devfile))
        {
            pr_err("open /proc/devinfo/emmc /proc/devinfo/ufs error: (%ld)\n",PTR_ERR(devfile));
            return -1;
        } else {
            ufs_type = 1;
            offsize = OPPO_UFS_PARTITION_BOOTLOG_OFFSET + OPPO_UFS_PARTITION_SPEC_FLAG_OFFSET;
            filp_close(devfile,NULL);
        }
    } else {
        pr_err("/proc/devinfo/emmc \n");
        offsize = OPPO_EMMC_PARTITION_BOOTLOG_OFFSET + OPPO_EMMC_PARTITION_SPEC_FLAG_OFFSET;
        filp_close(devfile,NULL);
    }

    set_pon_info(pon_info, fatal_error);

    memcpy(data_info, pon_info, SIZE_OF_PON_STRUCT);

    if(ufs_type) {
        opfile = filp_open(OPPO_PARTITION_OPPORESERVE1_UFS, O_RDWR, 0600);
    } else {
        opfile = filp_open(OPPO_PARTITION_OPPORESERVE1_EMMC, O_RDWR, 0600);
    }
    if (IS_ERR(opfile)) {
        pr_err("open OPPO_PARTITION_OPPORESERVE_1 error: %ld\n",PTR_ERR(opfile));
        opfile = filp_open(OPPO_PARTITION_OPPORESERVE1_LINK, O_RDWR, 0600);
        if (IS_ERR(opfile)) {
            pr_err("open OPPO_PARTITION_OPPORESERVE1_LINK error: %ld\n",PTR_ERR(opfile));
            opfile = filp_open(OPPO_PARTITION_OPPORESERVE1_LINK2, O_RDWR, 0600);
            if (IS_ERR(opfile)) {
                pr_err("open OPPO_PARTITION_OPPORESERVE1_LINK2 error: %ld\n",PTR_ERR(opfile));
                return -1;
            }
        }
    }

    old_fs = get_fs();
    set_fs(KERNEL_DS);

    pr_info("data_info %x %x %x %x pon_state %x %x %x %x write_count:%x\n", data_info[0], data_info[1],data_info[2],data_info[3],data_info[16],data_info[20],data_info[24],data_info[28],data_info[32]);
    pr_info("SIZE_OF_PON_STRUCT %ld SIZE_OF_PON_STRUCT %ld\n",sizeof(struct pon_struct),SIZE_OF_PON_STRUCT);

    size = vfs_write(opfile, data_info, SIZE_OF_PON_STRUCT, &offsize);

    if (size < 0) {
         pr_err("vfs_write data_info %s size %ld \n", data_info, size);
         set_fs(old_fs);
         filp_close(opfile,NULL);
         return -1;
    }
    rc = vfs_fsync(opfile, 1);
    if (rc)
        pr_err("sync returns %d\n", rc);

    set_fs(old_fs);
    filp_close(opfile,NULL);
    pr_info("write_to_reserve1 done \n");

    return 0;
}

int need_recovery(struct pon_struct *pon_info)
{
    int pon_state_sum = 0;
    int i = 0;

    if(!hang_oppo_main_on || hang_oppo_recovery_method != RESTART_AND_RECOVERY) {
        return 0;
    }

    memset(pon_info, 0, SIZE_OF_PON_STRUCT);

    read_from_reserve1(pon_info);

    for(i=0; i < RECOVERY_SUM; i++)
    {
        pon_state_sum += pon_info->pon_state[i];
    }

    pr_info("pon_reason_sum:0x%x pon_reason[0]:0x%x pon_reason[1]:0x%x\n", pon_state_sum, pon_info->pon_state[0], pon_info->pon_state[1]);

    if(pon_state_sum == (RECOVERY_FLAG * RECOVERY_SUM))
    {
        pr_err("fatal error 3 times, need recovery\n");
        return 1;
    }

    return 0;
}

#endif  //HANG_OPPO_ALL
#endif


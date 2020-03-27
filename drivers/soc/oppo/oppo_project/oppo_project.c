
#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <soc/qcom/smem.h>
#include <soc/oppo/oppo_project.h>
/*#include <mach/oppo_reserve3.h>*/
#include <linux/io.h>
#include <linux/syscalls.h>
#include <linux/string.h>

/*****************************************************/
static struct proc_dir_entry *oppoVersion = NULL;
static ProjectInfoCDTType *format = NULL;

static const char* nfc_feature = "nfc_feature";
static const char* feature_src = "/vendor/etc/nfc/com.oppo.nfc_feature.xml";

void init_project_version(void)
{
        unsigned int len = (sizeof(ProjectInfoCDTType) + 3)&(~0x3);

        format = (ProjectInfoCDTType *)smem_alloc(SMEM_PROJECT, len, 0, 0);
        if (format == ERR_PTR(-EPROBE_DEFER)) {
                format = NULL;
        }
}


unsigned int get_project(void)
{
        if (format == NULL) {
                init_project_version();
        }
        return format->nproject;
}

#ifdef VENDOR_EDIT
//Nan.Zhong@PSW.MM.AudioDriver.SmartPA, 2019/06/11, Add for export get_project
EXPORT_SYMBOL(get_project);
#endif /* VENDOR_EDIT */

unsigned int is_project(OPPO_PROJECT project)
{
        return (get_project() == project?1:0);
}
#ifdef VENDOR_EDIT
/* Hui.Fan@PSW.BSP.OPPOFeature.Hypnus, 2017-7-17, export is_project */
EXPORT_SYMBOL(is_project);
#endif /* VENDOR_EDIT */

unsigned char get_PCB_Version(void)
{
        if (format == NULL) {
                init_project_version();
        }
        return format->npcbversion;
}
#ifdef VENDOR_EDIT
/* Jianfeng.Qiu@PSW.MM.AudioDriver.HeadsetDet, 2018/07/09,
 * Add for export get_PCB_Version
 */
EXPORT_SYMBOL(get_PCB_Version);
#endif /* VENDOR_EDIT */

unsigned char get_Modem_Version(void)
{
        if (format == NULL) {
                init_project_version();
        }
        return format->nmodem;
}

unsigned char get_Operator_Version(void)
{
        if (format == NULL) {
                init_project_version();
        }
        return  format->noperator;
}


unsigned char get_Oppo_Boot_Mode(void)
{
        if (format == NULL) {
                init_project_version();
        }
        return  format->noppobootmode;
}

#ifdef VENDOR_EDIT
/*Fei.Mo@BSP.Bootloader.Bootflows, 2019/03/07, Add for diff manifest*/
#define MANIFEST_LEN 2
static const char* manifest = "manifest";
static const char* manifest_src[MANIFEST_LEN] = {
	"/vendor/etc/vintf/manifest_dsds.xml",
	"/vendor/etc/vintf/manifest_ssss.xml"
};

 static int __init update_manifest(void)
{
	mm_segment_t fs;
	char * substr = strstr(boot_command_line, "simcardnum.doublesim=");

	if (NULL == substr) {
		return 0;
	}
	substr += strlen("simcardnum.doublesim=");

	pr_err("update_manifest, project [%d] substr:[%c]", get_project(), substr[0]);

	fs = get_fs();
	set_fs(KERNEL_DS);

	if (oppoVersion) {
		if (substr[0] == '0') {
			proc_symlink(manifest,oppoVersion,manifest_src[1]);		//single sim
		} else {
			proc_symlink(manifest,oppoVersion,manifest_src[0]);		//double sim
		}
	}
	set_fs(fs);

	return 0;
}
late_initcall(update_manifest);
#endif

#ifdef VENDOR_EDIT
static int __init update_feature(void)
{
	mm_segment_t fs;
	fs = get_fs();
    pr_err("update_feature, Operator Version [%d], Project Name [%d]", get_Operator_Version(), get_project());
    set_fs(KERNEL_DS);
    if (oppoVersion) {
            if ((get_project() == 19651) &&
                ((get_Operator_Version() == 11 || get_Operator_Version() == 34))) {
			proc_symlink(nfc_feature, oppoVersion, feature_src);
		}
	}
	set_fs(fs);
	return 0;
}
late_initcall(update_feature);
#endif

/*this module just init for creat files to show which version*/
static ssize_t prjVersion_read_proc(struct file *file, char __user *buf,
                size_t count, loff_t *off)
{
        char page[256] = {0};
        int len = 0;
        len = sprintf(page, "%d", get_project());

        if (len > *off) {
                len -= *off;
        }
        else
                len = 0;
        if (copy_to_user(buf, page, (len < count ? len : count))) {
                return -EFAULT;
        }
        *off += len < count ? len : count;
        return (len < count ? len : count);
}

struct file_operations prjVersion_proc_fops = {
        .read = prjVersion_read_proc,
        .write = NULL,
};


static ssize_t pcbVersion_read_proc(struct file *file, char __user *buf,
                size_t count, loff_t *off)
{
        char page[256] = {0};
        int len = 0;

        len = sprintf(page, "%d", get_PCB_Version());

        if (len > *off) {
                len -= *off;
        }
        else
                len = 0;

        if (copy_to_user(buf, page, (len < count ? len : count))) {
                return -EFAULT;
        }
        *off += len < count ? len : count;
        return (len < count ? len : count);
}

struct file_operations pcbVersion_proc_fops = {
        .read = pcbVersion_read_proc,
};


static ssize_t operatorName_read_proc(struct file *file, char __user *buf,
                size_t count, loff_t *off)
{
        char page[256] = {0};
        int len = 0;

        len = sprintf(page, "%d", get_Operator_Version());

        if (len > *off) {
                len -= *off;
        }
        else
                len = 0;

        if (copy_to_user(buf, page, (len < count ? len : count))) {
                return -EFAULT;
        }
        *off += len < count ? len : count;
        return (len < count ? len : count);
}

struct file_operations operatorName_proc_fops = {
        .read = operatorName_read_proc,
};


static ssize_t modemType_read_proc(struct file *file, char __user *buf,
                size_t count, loff_t *off)
{
        char page[256] = {0};
        int len = 0;

        len = sprintf(page, "%d", get_Modem_Version());

        if (len > *off) {
                len -= *off;
        }
        else
                len = 0;

        if (copy_to_user(buf, page, (len < count ? len : count))) {
                return -EFAULT;
        }
        *off += len < count ? len : count;
        return (len < count ? len : count);
}

struct file_operations modemType_proc_fops = {
        .read = modemType_read_proc,
};


static ssize_t oppoBootmode_read_proc(struct file *file, char __user *buf,
                size_t count, loff_t *off)
{
        char page[256] = {0};
        int len = 0;

        len = sprintf(page, "%d", get_Oppo_Boot_Mode());

        if (len > *off) {
                len -= *off;
        }
        else
                len = 0;

        if (copy_to_user(buf, page, (len < count ? len : count))) {
                return -EFAULT;
        }
        *off += len < count ? len : count;
        return (len < count ? len : count);
}

struct file_operations oppoBootmode_proc_fops = {
        .read = oppoBootmode_read_proc,
};

#ifdef VENDOR_EDIT
/*Ziqing.Guo@BSP.Fingerprint.Secure 2017/03/28 Add for displaying secure boot switch*/
#define OEM_SEC_BOOT_REG 0x780350 /*sdm660
*/
static ssize_t secureType_read_proc(struct file *file, char __user *buf,
                size_t count, loff_t *off)
{
        char page[256] = {0};
        int len = 0;
        void __iomem *oem_config_base;
        uint32_t secure_oem_config = 0;

        oem_config_base = ioremap(OEM_SEC_BOOT_REG, 4);
        secure_oem_config = __raw_readl(oem_config_base);
        iounmap(oem_config_base);
        printk(KERN_EMERG "lycan test secure_oem_config 0x%x\n", secure_oem_config);
        len = sprintf(page, "%d", secure_oem_config);

        if (len > *off) {
                len -= *off;
        }
        else
                len = 0;

        if (copy_to_user(buf, page, (len < count ? len : count))) {
                return -EFAULT;
        }
        *off += len < count ? len : count;
        return (len < count ? len : count);
}
#endif /* VENDOR_EDIT */

#ifdef VENDOR_EDIT
/*Ziqing.Guo@BSP.Fingerprint.Secure 2017/04/16 Add for distinguishing secureboot stage*/
#define OEM_SEC_ENABLE_ANTIROLLBACK_REG 0x78019c /*sdm660
*/
static ssize_t secureStage_read_proc(struct file *file, char __user *buf,
                size_t count, loff_t *off)
{
        char page[256] = {0};
        int len = 0;
        void __iomem *oem_config_base;
        uint32_t secure_oem_config = 0;

        oem_config_base = ioremap(OEM_SEC_ENABLE_ANTIROLLBACK_REG, 4);
        secure_oem_config = __raw_readl(oem_config_base);
        iounmap(oem_config_base);
        printk(KERN_EMERG "lycan test secureStage_oem_config 0x%x\n", secure_oem_config);
        len = sprintf(page, "%d", secure_oem_config);

        if (len > *off) {
                len -= *off;
        }
        else
                len = 0;

        if (copy_to_user(buf, page, (len < count ? len : count))) {
                return -EFAULT;
        }
        *off += len < count ? len : count;
        return (len < count ? len : count);
}
#endif /* VENDOR_EDIT */

struct file_operations secureType_proc_fops = {
        .read = secureType_read_proc,
};

#ifdef VENDOR_EDIT
/*Ziqing.Guo@BSP.Fingerprint.Secure 2017/04/16 Add for distinguishing secureboot stage*/
struct file_operations secureStage_proc_fops = {
        .read = secureStage_read_proc,
};
#endif /* VENDOR_EDIT */

#define QFPROM_RAW_SERIAL_NUM 0x00786134 /*different at each platform, please ref boot_images\core\systemdrivers\hwio\scripts\xxx\hwioreg.per
*/

static unsigned int g_serial_id = 0x00; /*maybe can use for debug
*/

static ssize_t serialID_read_proc(struct file *file, char __user *buf,
                size_t count, loff_t *off)
{
        char page[256] = {0};
        int len = 0;
        len = sprintf(page, "0x%x", g_serial_id);

        if (len > *off) {
                len -= *off;
        }
        else
                len = 0;

        if (copy_to_user(buf, page, (len < count ? len : count))) {
                return -EFAULT;
        }
        *off += len < count ? len : count;
        return (len < count ? len : count);
}


struct file_operations serialID_proc_fops = {
        .read = serialID_read_proc,
};
/*for get which ldo ocp*/
void print_ocp(void)
{
        int i = 0;

        if (format == NULL) {
                init_project_version();
        }
        printk("ocp:");
        for (i = 0;i < OCPCOUNTMAX;i++) {
                printk(" %d", format->npmicocp[i]);
        }
        printk("\n");
}

static int __init ocplog_init(void)
{
        print_ocp();
        return 0;
}
late_initcall(ocplog_init);
static ssize_t ocplog_read_proc(struct file *file, char __user *buf,
                size_t count, loff_t *off)
{
        char page[256] = {0};
        int len = 0;
        int i = 0;

        if (format == NULL) {
                init_project_version();
        }
        len += sprintf(&page[len], "ocp:");
        for (i = 0;i < OCPCOUNTMAX;i++) {
                len += sprintf(&page[len], " %d", format->npmicocp[i]);
        }
        len += sprintf(&page[len], "\n");

        if (len > *off) {
                len -= *off;
        }
        else
                len = 0;

        if (copy_to_user(buf, page, (len < count ? len : count))) {
                return -EFAULT;
        }
        *off += len < count ? len : count;
        return (len < count ? len : count);
}


struct file_operations ocp_proc_fops = {
        .read = ocplog_read_proc,
};

/*#ifdef VENDOR_EDIT*/
/* Cong.Dai@psw.bsp.tp 2019/06/27 add for sw_ver start*/
/*sw_version = 0x00,  release
 *sw_version = 0x01,  aging
 *sw_version = 0x02,  cta
 *sw_version = 0x03,  performance
 *sw_version = 0x04,  preversion
 *sw_version = 0x05,  allnetcmcctest
 *sw_version = 0x06,  allnetcmccfield
 *sw_version = 0x07,  allnetcutest
 *sw_version = 0x08,  allnetcufield
 *sw_version = 0x09,  allnetcttest
 *sw_version = 0x0A,  allnetctfield
*/
static int oppo_eng_version = OPPO_ENG_VERSION_NOT_INIT;
static int oppo_confidential = true;
int get_eng_version(void)
{
    int i = 0, eng_len = 3;
    char *substr = NULL;

    if (oppo_eng_version != OPPO_ENG_VERSION_NOT_INIT)
        return oppo_eng_version;

    if (strstr(boot_command_line, "is_confidential=0"))
        oppo_confidential = false;

    oppo_eng_version = RELEASE;
    substr = strstr(boot_command_line, "eng_version=");
    if (!substr) {      //if cmdline does't cover the version, we use normal version as default version
        printk(KERN_EMERG "cmdline does't have the sw_version %s \n", __func__);
        return oppo_eng_version;
    }

    substr += strlen("eng_version=");
    for (i = 0; substr[i] != ' ' && i < eng_len && substr[i] != '\0'; i++) {
        if ((substr[i] >= '0') && (substr[i] <= '9')) {
            oppo_eng_version = oppo_eng_version * 10 + substr[i] - '0';
        } else {
            oppo_eng_version = RELEASE;
            break;
        }
    }

    return oppo_eng_version;
}
EXPORT_SYMBOL(get_eng_version);

bool is_confidential(void)
{
    if (oppo_eng_version != OPPO_ENG_VERSION_NOT_INIT)
        return oppo_confidential;

    get_eng_version();

    return oppo_confidential;
}
EXPORT_SYMBOL(is_confidential);

bool oppo_daily_build(void)
{
#if defined(CONFIG_OPPO_BUILD_USER)
    return false;
#endif

    get_eng_version();
    if ((ALL_NET_CMCC_TEST == oppo_eng_version) || (ALL_NET_CMCC_FIELD == oppo_eng_version) ||
        (ALL_NET_CU_TEST == oppo_eng_version) || (ALL_NET_CU_FIELD == oppo_eng_version) ||
        (ALL_NET_CT_TEST == oppo_eng_version) || (ALL_NET_CT_FIELD == oppo_eng_version))
        return false;

    return true;
}
EXPORT_SYMBOL(oppo_daily_build);

static ssize_t eng_version_read_proc(struct file *file, char __user *buf,
                size_t count, loff_t *off)
{
        int ret = 0;
        char page[64] = {0};

        get_eng_version();
        snprintf(page, 63, "%d", oppo_eng_version);
        ret = simple_read_from_buffer(buf, count, off, page, strlen(page));

        return ret;
}

struct file_operations eng_version_proc_fops = {
        .read = eng_version_read_proc,
        .open  = simple_open,
        .owner = THIS_MODULE,
};

static ssize_t confidential_read_proc(struct file *file, char __user *buf,
                size_t count, loff_t *off)
{
        int ret = 0;
        char page[64] = {0};

        snprintf(page, 63, "%d", is_confidential());
        ret = simple_read_from_buffer(buf, count, off, page, strlen(page));

        return ret;
}

struct file_operations confidential_proc_fops = {
        .read = confidential_read_proc,
        .open  = simple_open,
        .owner = THIS_MODULE,
};
/*#endif VENDOR_EDIT*/

/*RPMB_KEY_PROVISIONED 24bit 0x780178 in the Anti-rollback*/
/*This register must get from xxx_qfprom_programming_reference_guide.xlsm*/
#define RPMB_KEY_PROVISIONED 0x00780178

static unsigned int rpmbenable = 0;

int rpmb_is_enable(void)
{
        static unsigned int rpmbtmp = 0;
        void __iomem *rpmb_addr = NULL;

        if (rpmbenable) {
                return rpmbenable;
        }
        rpmb_addr = ioremap(RPMB_KEY_PROVISIONED , 4);
        if (rpmb_addr) {
                rpmbtmp = __raw_readl(rpmb_addr);
                iounmap(rpmb_addr);
                rpmbenable = (rpmbtmp >> 24) & 0x01;
                /*printk(KERN_EMERG "rpmb 0x%x\n", rpmbenable);
*/
        } else {
                rpmbenable = 0;
        }

        return rpmbenable;
}

EXPORT_SYMBOL(rpmb_is_enable);
static int __init oppo_project_init(void)
{
        int ret = 0;
        struct proc_dir_entry *pentry;
        void __iomem *serial_id_addr = NULL;

        serial_id_addr = ioremap(QFPROM_RAW_SERIAL_NUM , 4);
        if (serial_id_addr) {
                g_serial_id = __raw_readl(serial_id_addr);
                iounmap(serial_id_addr);
                printk(KERN_EMERG "serialID 0x%x\n", g_serial_id);
        } else
        {
                g_serial_id = 0xffffffff;
        }

        #ifdef VENDOR_EDIT
        // Cong.Dai@psw.bsp.tp, 2019.06.15, add for init engineering version
        get_eng_version();
        #endif /* VENDOR_EDIT */

        oppoVersion =  proc_mkdir("oppoVersion", NULL);
        if (!oppoVersion) {
                pr_err("can't create oppoVersion proc\n");
                goto ERROR_INIT_VERSION;
        }
        pentry = proc_create("prjName", S_IRUGO, oppoVersion, &prjVersion_proc_fops);
        if (!pentry) {
                pr_err("create prjVersion proc failed.\n");
                goto ERROR_INIT_VERSION;
        }
        pentry = proc_create("pcbVersion", S_IRUGO, oppoVersion, &pcbVersion_proc_fops);
        if (!pentry) {
                pr_err("create pcbVersion proc failed.\n");
                goto ERROR_INIT_VERSION;
        }
        pentry = proc_create("operatorName", S_IRUGO, oppoVersion, &operatorName_proc_fops);
        if (!pentry) {
                pr_err("create operatorName proc failed.\n");
                goto ERROR_INIT_VERSION;
        }
        pentry = proc_create("modemType", S_IRUGO, oppoVersion, &modemType_proc_fops);
        if (!pentry) {
                pr_err("create modemType proc failed.\n");
                goto ERROR_INIT_VERSION;
        }
        pentry = proc_create("oppoBootmode", S_IRUGO, oppoVersion, &oppoBootmode_proc_fops);
        if (!pentry) {
                pr_err("create oppoBootmode proc failed.\n");
                goto ERROR_INIT_VERSION;
        }
        pentry = proc_create("secureType", S_IRUGO, oppoVersion, &secureType_proc_fops);
        if (!pentry) {
                pr_err("create secureType proc failed.\n");
                goto ERROR_INIT_VERSION;
        }

#ifdef VENDOR_EDIT
/*Ziqing.Guo@BSP.Fingerprint.Secure 2017/04/16 Add for distinguishing secureboot stage*/
        pentry = proc_create("secureStage", S_IRUGO, oppoVersion, &secureStage_proc_fops);
        if (!pentry) {
                pr_err("create secureStage proc failed.\n");
                goto ERROR_INIT_VERSION;
        }
#endif /* VENDOR_EDIT */
        pentry = proc_create("serialID", S_IRUGO, oppoVersion, &serialID_proc_fops);
        if (!pentry) {
                pr_err("create serialID proc failed.\n");
                goto ERROR_INIT_VERSION;
        }

        pentry = proc_create("ocp", S_IRUGO, oppoVersion, &ocp_proc_fops);
        if (!pentry) {
                pr_err("create serialID proc failed.\n");
                goto ERROR_INIT_VERSION;
        }

#ifdef VENDOR_EDIT
/* Cong.Dai@psw.bsp.tp, 2019.06.15, add for init engineering version file*/
        pentry = proc_create("engVersion", S_IRUGO, oppoVersion, &eng_version_proc_fops);
        if (!pentry) {
                pr_err("create engVersion proc failed.\n");
                goto ERROR_INIT_VERSION;
        }
        pentry = proc_create("isConfidential", S_IRUGO, oppoVersion, &confidential_proc_fops);
        if (!pentry) {
                pr_err("create isConfidential proc failed.\n");
                goto ERROR_INIT_VERSION;
        }
#endif /* VENDOR_EDIT */
        return ret;
ERROR_INIT_VERSION:
                remove_proc_entry("oppoVersion", NULL);
                return -ENOENT;
}
arch_initcall(oppo_project_init);

MODULE_DESCRIPTION("OPPO project version");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Joshua <gyx@oppo.com>");

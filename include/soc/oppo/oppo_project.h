/* 
 *
 * yixue.ge add for oppo project
 *
 *
 */
#ifndef _OPPO_PROJECT_H_
#define _OPPO_PROJECT_H_

enum{
        HW_VERSION__UNKNOWN,
        HW_VERSION__10,   /*1452mV*/
        HW_VERSION__11,   /*1636 mV*/
        HW_VERSION__12,   /*1224 mV*/
        HW_VERSION__13,   /*900 mV*/
        HW_VERSION__14,   /*720 mV*/
        HW_VERSION__15,
        HW_VERSION__16,
        HW_VERSION__17,
};


enum OPPO_MODEM {
        RF_VERSION__UNKNOWN = 0x00,
        RF_VERSION__11      = 0x01,
        RF_VERSION__12      = 0x02,
        RF_VERSION__13      = 0x03,
        RF_VERSION__14      = 0x04,
        RF_VERSION__15      = 0x05,
        RF_VERSION__16      = 0x06,
        RF_VERSION__17      = 0x07,
        RF_VERSION__18      = 0x08,
        RF_VERSION__19      = 0x09,
        RF_VERSION__1A      = 0x0A,
};


#define GET_PCB_VERSION() (get_PCB_Version())
#define GET_PCB_VERSION_STRING() (get_PCB_Version_String())

#define GET_MODEM_VERSION() (get_Modem_Version())
#define GET_OPERATOR_VERSION() (get_Operator_Version())



enum OPPO_PROJECT {
        OPPO_UNKOWN = 0,
        OPPO_18041 = 18041,
        OPPO_18081 = 18081,
        OPPO_18085 = 18085,
        OPPO_18181 = 18181,
        OPPO_18097 = 18097,
        OPPO_18621 = 18621,
        OPPO_18099 = 18099,
        OPPO_18383 = 18383,
        OPPO_19651 = 19651,
        OPPO_19691 = 19691,
};

enum OPPO_OPERATOR {
        OPERATOR_UNKOWN                     = 0x00,
        OPERATOR_OPEN_MARKET                = 0x01,
        OPERATOR_CHINA_MOBILE               = 0x02,
        OPERATOR_CHINA_UNICOM               = 0x03,
        OPERATOR_CHINA_TELECOM              = 0x04,
        OPERATOR_FOREIGN                    = 0x05,
/*#ifdef VENDOR_EDIT*/
/*TongJing.Shi@EXP.DataComm.Phone, 2014.04.18, Add for exp just 3G no 4G*/
        OPERATOR_FOREIGN_WCDMA              = 0x06,   /*qifeng.liu 2014.08.07 FOR MAC*/
        OPERATOR_FOREIGN_RESERVED           = 0x07,    /*shaoming 2014/10/04 add for 14085's dual sim version*/
        OPERATOR_ALL_CHINA_CARRIER          = 0x08,    /*instead of TELECOM CARRIER because of history Tong.han@Bsp.Group.Tp add for all china carrier phone, 2015/03/23*/
        OPERATOR_ALL_CHINA_CARRIER_MOBILE   = 0x09,    /*rendong.shi@Bsp.Group.Tp add for all china carrier MOBILE phone, 2016/01/07*/
        OPERATOR_ALL_CHINA_CARRIER_UNICOM   = 0x0A,    /*rendong.shi@Bsp.Group.Tp add for all china carrier UNICOM  phone, 2016/01/07*/
        OPERATOR_FOREIGN_EUROPE             = 0x0B,    //wanghao@Bsp.Group.Tp add for foreign europe  phone, 2018/07/14
        OPERATOR_RM_FOREIGN     = 30,//linzhenming add for realme foreign  18623
        OPERATOR_RM_FOREIGN_INDIA   = 31,//linzhenming add for realme foreign  india 18621
        OPERATOR_RM_FOREIGN_VIETNAM_128GB            = 32,//linzhenming add for realme foreign  vietnam 18625 
        OPERATOR_RM_FOREIGN_VIETNAM_64GB            = 33,//linzhenming add for realme foreign  vietnam 18627
		OPERATOR_RM_FOREIGN_EUROPE            = 34,//linzhenming add for realme foreign  EUROPE 18633
};

typedef enum OPPO_PROJECT OPPO_PROJECT;
#define OCPCOUNTMAX 4
typedef struct
{
        unsigned int                  nproject;
        unsigned char                 nmodem;
        unsigned char                 noperator;
        unsigned char                 npcbversion;
        unsigned char                 noppobootmode;
        unsigned char                 npmicocp[OCPCOUNTMAX];
} ProjectInfoCDTType;

/*#ifdef VENDOR_EDIT*/
/*Cong.Dai@psw.bsp.tp, 2019.06.27, Add for sw_version*/
#define OPPO_ENG_VERSION_NOT_INIT      -1
enum OPPO_ENG_VERSION {
    RELEASE                 = 0x00,
    AGING                   = 0x01,
    CTA                     = 0x02,
    PERFORMANCE             = 0x03,
    PREVERSION              = 0x04,
    ALL_NET_CMCC_TEST       = 0x05,
    ALL_NET_CMCC_FIELD      = 0x06,
    ALL_NET_CU_TEST         = 0x07,
    ALL_NET_CU_FIELD        = 0x08,
    ALL_NET_CT_TEST         = 0x09,
    ALL_NET_CT_FIELD        = 0x0A,
};
/*#endif VENDOR_EDIT*/

#ifdef CONFIG_OPPO_COMMON_SOFT
void init_project_version(void);
unsigned int get_project(void);
unsigned int is_project(OPPO_PROJECT project);
unsigned char get_PCB_Version(void);
unsigned char get_Modem_Version(void);
unsigned char get_Operator_Version(void);
int get_eng_version(void);
bool is_confidential(void);
bool oppo_daily_build(void);
#else
unsigned int init_project_version(void) { return 0;}
unsigned int get_project(void) { return 0;}
unsigned int is_project(OPPO_PROJECT project) { return 0;}
unsigned char get_PCB_Version(void) { return 0;}
unsigned char get_Modem_Version(void) { return 0;}
unsigned char get_Operator_Version(void) { return 0;}
int get_eng_version(void) { return 0;}
bool is_confidential(void) { return true;}
bool oppo_daily_build(void) { return false;}
#endif
#endif  /*_OPPO_PROJECT_H_*/

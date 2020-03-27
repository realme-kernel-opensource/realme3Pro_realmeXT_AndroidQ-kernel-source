/***************************************************
 * File:touch.c
 * VENDOR_EDIT
 * Copyright (c)  2008- 2030  Oppo Mobile communication Corp.ltd.
 * Description:
 *             tp dev
 * Version:1.0:
 * Date created:2016/09/02
 * Author: hao.wang@Bsp.Driver
 * TAG: BSP.TP.Init
*/

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/serio.h>
#include "oppo_touchscreen/tp_devices.h"
#include "oppo_touchscreen/touchpanel_common.h"
#include <soc/oppo/oppo_project.h>
#include <linux/regulator/consumer.h>
#include <linux/of_gpio.h>
#include "touch.h"
#define MAX_LIMIT_DATA_LENGTH         100
int g_tp_dev_vendor = TP_UNKNOWN;
char *g_tp_chip_name;
static bool is_tp_type_got_in_match = false;    /*indicate whether the tp type is got in the process of ic match*/
#define MAX_LIMIT_DATA_LENGTH         100

#define NT36672_NF_CHIP_NAME "NT_NF36672"
#define HX83112A_NF_CHIP_NAME "HX_NF83112A"
#define TD4320_NF_CHIP_NAME "TD4320_NF"

/*if can not compile success, please update vendor/oppo_touchsreen*/
struct tp_dev_name tp_dev_names[] = {
     {TP_OFILM, "OFILM"},
     {TP_BIEL, "BIEL"},
     {TP_TRULY, "TRULY"},
     {TP_BOE, "BOE"},
     {TP_G2Y, "G2Y"},
     {TP_TPK, "TPK"},
     {TP_JDI, "JDI"},
     {TP_TIANMA, "TIANMA"},
     {TP_SAMSUNG, "SAMSUNG"},
     {TP_DSJM, "DSJM"},
     {TP_BOE_B8, "BOEB8"},
     {TP_UNKNOWN, "UNKNOWN"},
};

#define GET_TP_DEV_NAME(tp_type) ((tp_dev_names[tp_type].type == (tp_type))?tp_dev_names[tp_type].name:"UNMATCH")

bool __init tp_judge_ic_match(char * tp_ic_name)
{
	pr_err("[TP] tp_ic_name = %s \n", tp_ic_name);
	//pr_err("[TP] boot_command_line = %s \n", boot_command_line);

	switch(get_project()) {
	case 18621:
	case 19691:
		if(strstr(tp_ic_name, "himax,hx83112a_nf")){
			pr_err("[TP]Project use himax \n");
			is_tp_type_got_in_match = true;

			g_tp_dev_vendor = TP_DSJM;

			g_tp_chip_name = kzalloc(sizeof(HX83112A_NF_CHIP_NAME), GFP_KERNEL);
			g_tp_chip_name = HX83112A_NF_CHIP_NAME;
			return true;
		}else{
			pr_err("[TP] Project not himax cannot support\n");
			return false;
		}
	case 19651:
	case 18041:
		if(strstr(tp_ic_name, "synaptics-s3706")){
			pr_err("[TP] Project Name use synaptics\n");
			is_tp_type_got_in_match = true;
			return true;
		}else{
			pr_err("[TP] Project not synaptics cannot support\n");
			return false;
		}
	default:
		pr_err("Invalid project\n");
		break;
	}
	pr_err("Lcd module not found\n");
	return false;
}

int tp_util_get_vendor(struct hw_resource *hw_res, struct panel_info *panel_data)
{
	char* vendor;
	int prj_id = 0;

	panel_data->test_limit_name = kzalloc(MAX_LIMIT_DATA_LENGTH, GFP_KERNEL);
	if (panel_data->test_limit_name == NULL) {
		pr_err("[TP]panel_data.test_limit_name kzalloc error\n");
	}

	if (is_project(OPPO_18081) || is_project(OPPO_18085) || is_project(OPPO_18181)) {
		panel_data->tp_type = TP_SAMSUNG;
		prj_id = OPPO_18081;
	}else if (is_project(OPPO_18097) || is_project(OPPO_18099)) {
		panel_data->tp_type = TP_SAMSUNG;
		prj_id = OPPO_18097;
	} else if (is_project(OPPO_18041)) {
		panel_data->tp_type = TP_SAMSUNG;
		if (get_PCB_Version() == HW_VERSION__10) {
			pr_info("18041 T0 use 18097 TP FW\n");
			prj_id = OPPO_18097;
		}else{
			prj_id = OPPO_18041;
		}
	}else if (is_project(OPPO_18383)) {
		panel_data->tp_type = TP_SAMSUNG;
		prj_id = OPPO_18383;
	}else if(is_project(OPPO_18621) || is_project(OPPO_19691)){	
#ifdef CONFIG_TOUCHPANEL_MULTI_NOFLASH
	if (g_tp_chip_name != NULL) {
			panel_data->chip_name = g_tp_chip_name;
	}
#endif
		panel_data->tp_type = TP_DSJM;
		if(is_project(OPPO_18621))
			prj_id = OPPO_18621;
		else if(is_project(OPPO_19691))
			prj_id = OPPO_19691;
    }else if (is_project(OPPO_19651)) {
		panel_data->tp_type = TP_SAMSUNG;
		prj_id = OPPO_19651;
	}
	if (panel_data->tp_type == TP_UNKNOWN) {
		pr_err("[TP]%s type is unknown\n", __func__);
		return 0;
    }

    vendor = GET_TP_DEV_NAME(panel_data->tp_type);

    strcpy(panel_data->manufacture_info.manufacture, vendor);
    snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
            "tp/%d/FW_%s_%s.img",
            prj_id, panel_data->chip_name, vendor);

    if (panel_data->test_limit_name) {
        snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
            "tp/%d/LIMIT_%s_%s.img",
            prj_id, panel_data->chip_name, vendor);
    }
    panel_data->manufacture_info.fw_path = panel_data->fw_name;

    pr_info("[TP]vendor:%s fw:%s limit:%s\n",
        vendor,
        panel_data->fw_name,
        panel_data->test_limit_name==NULL?"NO Limit":panel_data->test_limit_name);
		
	 switch(get_project()) {
	    case OPPO_18621:
	        panel_data->firmware_headfile.firmware_data = FW_18621_HX83112A_NF_DSJM;
	        panel_data->firmware_headfile.firmware_size = sizeof(FW_18621_HX83112A_NF_DSJM);
	        break;

	    default:
	        panel_data->firmware_headfile.firmware_data = NULL;
	        panel_data->firmware_headfile.firmware_size = 0;
    }
    return 0;
}

int reconfig_power_control(struct touchpanel_data *ts)
{
    int ret = 0;
    if ((is_project(OPPO_18097) || is_project(OPPO_18099))) {
        if (get_PCB_Version() == HW_VERSION__10) {
            if (gpio_is_valid(ts->hw_res.reset_gpio)) {
                gpio_free(ts->hw_res.reset_gpio);
            } else {
                pr_info("ts->reset-gpio not specified\n");
            }

            ts->hw_res.reset_gpio = of_get_named_gpio(ts->dev->of_node, "reset_t0-gpio", 0);
            if (gpio_is_valid(ts->hw_res.reset_gpio)) {
                ret = gpio_request(ts->hw_res.reset_gpio, "reset-gpio");
                if (ret) {
                    pr_info("unable to request gpio [%d]\n", ts->hw_res.reset_gpio);
                } else {
                    pr_info("ts->reset-gpio not specified\n");
                }
            }
        } else {
            if (!IS_ERR_OR_NULL(ts->hw_res.vdd_2v8)) {
                //ret = regulator_disable(ts->hw_res.vdd_2v8);
                regulator_put(ts->hw_res.vdd_2v8);
                ts->hw_res.vdd_2v8 = NULL;
            }
        }
    }
    if (is_project(OPPO_18041)) {
        if (get_PCB_Version() == HW_VERSION__10) {
            ts->hw_res.TX_NUM = 16;
            ts->hw_res.RX_NUM = 33;
            pr_info("18041 T0 use TX,RX=[%d],[%d]\n", ts->hw_res.TX_NUM, ts->hw_res.RX_NUM);
        }
    }
	if (is_project(OPPO_19651)) {
		ts->hw_res.TX_NUM = 16;
		ts->hw_res.RX_NUM = 33;
		pr_info("19651 T0 use TX,RX=[%d],[%d]\n", ts->hw_res.TX_NUM, ts->hw_res.RX_NUM);
	}
    return ret;
}

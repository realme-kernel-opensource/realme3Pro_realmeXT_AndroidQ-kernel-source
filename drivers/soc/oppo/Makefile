obj-$(CONFIG_OPPO_COMMON_SOFT)	+=  oppo_project/
obj-$(CONFIG_OPPO_DEVICE_IFNO)	+=  oppo_devinfo/
obj-$(CONFIG_OPPO_DEBUG_ASSERT)	+=  oppo_trace/
obj-$(CONFIG_RECORD_MDMRST)     +=  oppo_mdmrst/

#ifdef VENDOR_EDIT
#//long.gao@PSW.SH.HYPNUS, 2019/06/04, Add for hypnus
obj-$(CONFIG_OPPO_HYPNUS)     +=  oppo_hypnus/
#endif /* VENDOR_EDIT */

#ifdef VENDOR_EDIT
#//Chao.zeng@PSW.BSP.Sensor, 2019/08/20, Add for wise light
obj-y     +=  oppo_als_cali_value/
#endif /* VENDOR_EDIT */

#ifdef VENDOR_EDIT
#//wenbin.liu@PSW.Kernel.MM, 2018/05/23, Add for Healthinfo monitor
obj-y     +=  oppo_healthinfo/
#endif /* VENDOR_EDIT */

#ifdef VENDOR_EDIT
#wen.luo@PSW.Kernel.MM, 2018/05/23, Add for Healthinfo monitor
obj-$(CONFIG_OPPO_SHUTDOWN_DETECT)	+=  oppo_shutdown_detect/
#endif /* VENDOR_EDIT */

#Kun.Hu@TECH.BSP.Stability.PHOENIX_PROJECT 2019/06/15, Add for PHOENIX_PROJECT
obj-$(CONFIG_OPPO_PHOENIX)     +=  oppo_phoenix/

#ifdefine VENDOR_EDIT
#GaoTing.Gan@PSW.MultiMedia.MediaServer, 2019/03/07, Add for kevent feedback
obj-y +=  oppo_kevent_feedback/
#endif /* VENDOR_EDIT */
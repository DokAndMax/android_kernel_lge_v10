/*****************************************************************************
 *
 * Filename:
 * ---------
 *    charging_hw_rt9536.c
 *
 * Project:
 * --------
 *   ALPS_Software
 *
 * Description:
 * ------------
 *   This file implements the interface between BMT and ADC scheduler.
 *
 * Author:
 * -------
 *  Oscar Liu
 *
 *============================================================================
  * $Revision:   1.0  $
 * $Modtime:   11 Aug 2005 10:28:16  $
 * $Log:   //mtkvs01/vmdata/Maui_sw/archives/mcu/hal/peripheral/inc/bmt_chr_setting.h-arc  $
 *             HISTORY
 * Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#include <mach/charging.h>
#include <mach/upmu_common.h>
#include <mach/mt_gpio.h>
#include <cust_gpio_usage.h>
#include <mach/upmu_hw.h>
#include <linux/xlog.h>
#include <linux/delay.h>
#include <mach/mt_sleep.h>
#include <mach/mt_boot.h>
#include <mach/system.h>
#include <cust_charging.h>
//david
//#include <cust_gpio_usage.h>
#include "rt9536.h"


 // ============================================================ //
 //define
 // ============================================================ //
#define STATUS_OK	0
#define STATUS_UNSUPPORTED	-1
#define GETARRAYNUM(array) (sizeof(array)/sizeof(array[0]))


 // ============================================================ //
 //global variable
 // ============================================================ //
//<2015/1/20-davidda, [V10][Feature][Common][BSP][]enable rt9536
#if 0 //david
int gpio_number     = GPIO_EINT_CHG_STAT_PIN; 
int gpio_mode       = GPIO_EINT_CHG_STAT_PIN_M_EINT;
#endif
//>2015/1/20-davidda

int gpio_off_dir  = GPIO_DIR_OUT;
int gpio_off_out  = GPIO_OUT_ONE;
int gpio_on_dir   = GPIO_DIR_OUT;
int gpio_on_out   = GPIO_OUT_ZERO;

static CHARGER_TYPE g_charger_type = CHARGER_UNKNOWN;
kal_bool charging_type_det_done = KAL_TRUE;

const kal_uint32 VBAT_CV_VTH[]=
{
	BATTERY_VOLT_03_500000_V,   BATTERY_VOLT_03_520000_V,	BATTERY_VOLT_03_540000_V,   BATTERY_VOLT_03_560000_V,
	BATTERY_VOLT_03_580000_V,   BATTERY_VOLT_03_600000_V,	BATTERY_VOLT_03_620000_V,   BATTERY_VOLT_03_640000_V,
	BATTERY_VOLT_03_660000_V,	BATTERY_VOLT_03_680000_V,	BATTERY_VOLT_03_700000_V,	BATTERY_VOLT_03_720000_V,
	BATTERY_VOLT_03_740000_V,	BATTERY_VOLT_03_760000_V,	BATTERY_VOLT_03_780000_V,	BATTERY_VOLT_03_800000_V,
	BATTERY_VOLT_03_820000_V,	BATTERY_VOLT_03_840000_V,	BATTERY_VOLT_03_860000_V,	BATTERY_VOLT_03_880000_V,
	BATTERY_VOLT_03_900000_V,	BATTERY_VOLT_03_920000_V,	BATTERY_VOLT_03_940000_V,	BATTERY_VOLT_03_960000_V,
	BATTERY_VOLT_03_980000_V,	BATTERY_VOLT_04_000000_V,	BATTERY_VOLT_04_020000_V,	BATTERY_VOLT_04_040000_V,
	BATTERY_VOLT_04_060000_V,	BATTERY_VOLT_04_080000_V,	BATTERY_VOLT_04_100000_V,	BATTERY_VOLT_04_120000_V,
	BATTERY_VOLT_04_140000_V,   BATTERY_VOLT_04_160000_V,	BATTERY_VOLT_04_180000_V,   BATTERY_VOLT_04_200000_V,
	BATTERY_VOLT_04_220000_V,   BATTERY_VOLT_04_240000_V,	BATTERY_VOLT_04_260000_V,   BATTERY_VOLT_04_280000_V,
	BATTERY_VOLT_04_300000_V,   BATTERY_VOLT_04_320000_V,	BATTERY_VOLT_04_340000_V,   BATTERY_VOLT_04_360000_V,	
	BATTERY_VOLT_04_380000_V,   BATTERY_VOLT_04_400000_V,	BATTERY_VOLT_04_420000_V,   BATTERY_VOLT_04_440000_V	
	
};

const kal_uint32 CS_VTH[]=
{
	CHARGE_CURRENT_550_00_MA,   CHARGE_CURRENT_650_00_MA,	CHARGE_CURRENT_750_00_MA, CHARGE_CURRENT_850_00_MA,
	CHARGE_CURRENT_950_00_MA,   CHARGE_CURRENT_1050_00_MA,	CHARGE_CURRENT_1150_00_MA, CHARGE_CURRENT_1250_00_MA
}; 

 const kal_uint32 INPUT_CS_VTH[]=
 {
	 CHARGE_CURRENT_100_00_MA,	 CHARGE_CURRENT_500_00_MA,	 CHARGE_CURRENT_800_00_MA, CHARGE_CURRENT_MAX
 }; 

 const kal_uint32 VCDT_HV_VTH[]=
 {
	  BATTERY_VOLT_04_200000_V, BATTERY_VOLT_04_250000_V,	  BATTERY_VOLT_04_300000_V,   BATTERY_VOLT_04_350000_V,
	  BATTERY_VOLT_04_400000_V, BATTERY_VOLT_04_450000_V,	  BATTERY_VOLT_04_500000_V,   BATTERY_VOLT_04_550000_V,
	  BATTERY_VOLT_04_600000_V, BATTERY_VOLT_06_000000_V,	  BATTERY_VOLT_06_500000_V,   BATTERY_VOLT_07_000000_V,
	  BATTERY_VOLT_07_500000_V, BATTERY_VOLT_08_500000_V,	  BATTERY_VOLT_09_500000_V,   BATTERY_VOLT_10_500000_V		  
 };

 kal_uint32 g_charging_current = 0;
 static kal_uint32 charging_error = false;

 // ============================================================ //
 // function prototype
 // ============================================================ //
 
 
 // ============================================================ //
 //extern variable
 // ============================================================ //
 
 // ============================================================ //
 //extern function
 // ============================================================ //
 extern kal_uint32 upmu_get_reg_value(kal_uint32 reg);
 extern bool mt_usb_is_device(void);
 extern void Charger_Detect_Init(void);
 extern void Charger_Detect_Release(void);
 extern void mt_power_off(void);
 extern bool get_usb_current_unlimited(void);
 
 // ============================================================ //
 kal_uint32 charging_value_to_parameter(const kal_uint32 *parameter, const kal_uint32 array_size, const kal_uint32 val)
{
	if (val < array_size)
	{
		return parameter[val];
	}
	else
	{
		battery_xlog_printk(BAT_LOG_CRTI, "Can't find the parameter \r\n");	
		return parameter[0];
	}
}

 
 kal_uint32 charging_parameter_to_value(const kal_uint32 *parameter, const kal_uint32 array_size, const kal_uint32 val)
{
	kal_uint32 i;

	for(i=0;i<array_size;i++)
	{
		if (val == *(parameter + i))
		{
				return i;
		}
	}

    battery_xlog_printk(BAT_LOG_CRTI, "NO register value match \r\n");
	//TODO: ASSERT(0);	// not find the value
	return 0;
}


 static kal_uint32 bmt_find_closest_level(const kal_uint32 *pList,kal_uint32 number,kal_uint32 level)
 {
	 kal_uint32 i;
	 kal_uint32 max_value_in_last_element;
 
	 if(pList[0] < pList[1])
		 max_value_in_last_element = KAL_TRUE;
	 else
		 max_value_in_last_element = KAL_FALSE;
 
	 if(max_value_in_last_element == KAL_TRUE)
	 {
		 for(i = (number-1); i != 0; i--)	 //max value in the last element
		 {
			 if(pList[i] <= level)
			 {
				 return pList[i];
			 }	  
		 }

 		 battery_xlog_printk(BAT_LOG_CRTI, "Can't find closest level, small value first \r\n");
		 return pList[0];
		 //return CHARGE_CURRENT_0_00_MA;
	 }
	 else
	 {
		 for(i = 0; i< number; i++)  // max value in the first element
		 {
			 if(pList[i] <= level)
			 {
				 return pList[i];
			 }	  
		 }

		 battery_xlog_printk(BAT_LOG_CRTI, "Can't find closest level, large value first \r\n"); 	 
		 return pList[number -1];
  		 //return CHARGE_CURRENT_0_00_MA;
	 }
 }


 static void hw_bc11_dump_register(void)
 {
	kal_uint32 reg_val = 0;
	kal_uint32 reg_num = CHR_CON18;
	kal_uint32 i = 0;

	for(i=reg_num ; i<=CHR_CON19 ; i+=2)
	{
		reg_val = upmu_get_reg_value(i);
		battery_xlog_printk(BAT_LOG_FULL, "Chr Reg[0x%x]=0x%x \r\n", i, reg_val);
	}
 }
	

 static void hw_bc11_init(void)
 {
 	 msleep(300);
	 Charger_Detect_Init();
		 
	 //RG_BC11_BIAS_EN=1	
	 upmu_set_rg_bc11_bias_en(0x1);
	 //RG_BC11_VSRC_EN[1:0]=00
	 upmu_set_rg_bc11_vsrc_en(0x0);
	 //RG_BC11_VREF_VTH = [1:0]=00
	 upmu_set_rg_bc11_vref_vth(0x0);
	 //RG_BC11_CMP_EN[1.0] = 00
	 upmu_set_rg_bc11_cmp_en(0x0);
	 //RG_BC11_IPU_EN[1.0] = 00
	 upmu_set_rg_bc11_ipu_en(0x0);
	 //RG_BC11_IPD_EN[1.0] = 00
	 upmu_set_rg_bc11_ipd_en(0x0);
	   //BC11_RST=1
	 upmu_set_rg_bc11_rst(0x1);
	 //BC11_BB_CTRL=1
	 upmu_set_rg_bc11_bb_ctrl(0x1);
 
 	 //msleep(10);
 	 mdelay(50);

	if(Enable_BATDRV_LOG == BAT_LOG_FULL)
    {
    	battery_xlog_printk(BAT_LOG_FULL, "hw_bc11_init() \r\n");
		hw_bc11_dump_register();
    }	
	 
 }
 
 
 static U32 hw_bc11_DCD(void)
 {
	 U32 wChargerAvail = 0;
 
	  //RG_BC11_IPU_EN[1.0] = 10
	 upmu_set_rg_bc11_ipu_en(0x2);
	   //RG_BC11_IPD_EN[1.0] = 01
	 upmu_set_rg_bc11_ipd_en(0x1);
	  //RG_BC11_VREF_VTH = [1:0]=01
	 upmu_set_rg_bc11_vref_vth(0x1);
	  //RG_BC11_CMP_EN[1.0] = 10
	 upmu_set_rg_bc11_cmp_en(0x2);
 
	 //msleep(20);
	 mdelay(80);

 	 wChargerAvail = upmu_get_rgs_bc11_cmp_out();

	if(Enable_BATDRV_LOG == BAT_LOG_FULL)
    {
    	battery_xlog_printk(BAT_LOG_FULL, "hw_bc11_DCD() \r\n");
		hw_bc11_dump_register();
    }
	 
	 //RG_BC11_IPU_EN[1.0] = 00
	 upmu_set_rg_bc11_ipu_en(0x0);
	  //RG_BC11_IPD_EN[1.0] = 00
	 upmu_set_rg_bc11_ipd_en(0x0);
	  //RG_BC11_CMP_EN[1.0] = 00
	 upmu_set_rg_bc11_cmp_en(0x0);
	  //RG_BC11_VREF_VTH = [1:0]=00
	 upmu_set_rg_bc11_vref_vth(0x0);
 
	 return wChargerAvail;
 }
 
 
 static U32 hw_bc11_stepA1(void)
 {
	 U32 wChargerAvail = 0;
	  
	  //RG_BC11_IPU_EN[1.0] = 10
	 upmu_set_rg_bc11_ipu_en(0x2);
	   //RG_BC11_VREF_VTH = [1:0]=10
	 upmu_set_rg_bc11_vref_vth(0x2);
	  //RG_BC11_CMP_EN[1.0] = 10
	 upmu_set_rg_bc11_cmp_en(0x2);
 
	 //msleep(80);
	 mdelay(80);
 
     wChargerAvail = upmu_get_rgs_bc11_cmp_out();

	if(Enable_BATDRV_LOG == BAT_LOG_FULL)
    {
    	battery_xlog_printk(BAT_LOG_FULL, "hw_bc11_stepA1() \r\n");
		hw_bc11_dump_register();
    }
 
	 //RG_BC11_IPU_EN[1.0] = 00
	 upmu_set_rg_bc11_ipu_en(0x0);
	  //RG_BC11_CMP_EN[1.0] = 00
	 upmu_set_rg_bc11_cmp_en(0x0);
 
	 return  wChargerAvail;
 }
 
 
 static U32 hw_bc11_stepB1(void)
 {
	 U32 wChargerAvail = 0;
	  
	  //RG_BC11_IPU_EN[1.0] = 01
	 upmu_set_rg_bc11_ipu_en(0x1);
	  //RG_BC11_VREF_VTH = [1:0]=10
	 upmu_set_rg_bc11_vref_vth(0x2);
	  //RG_BC11_CMP_EN[1.0] = 01
	 upmu_set_rg_bc11_cmp_en(0x1);
 
	 //msleep(80);
	 mdelay(80);
 
    wChargerAvail = upmu_get_rgs_bc11_cmp_out();

	if(Enable_BATDRV_LOG == BAT_LOG_FULL)
    {
    	battery_xlog_printk(BAT_LOG_FULL, "hw_bc11_stepB1() \r\n");
		hw_bc11_dump_register();
    }
 
	 //RG_BC11_IPU_EN[1.0] = 00
	 upmu_set_rg_bc11_ipu_en(0x0);
	  //RG_BC11_CMP_EN[1.0] = 00
	 upmu_set_rg_bc11_cmp_en(0x0);
	   //RG_BC11_VREF_VTH = [1:0]=00
	 upmu_set_rg_bc11_vref_vth(0x0);
 
	 return  wChargerAvail;
 }
 
 
 static U32 hw_bc11_stepC1(void)
 {
	 U32 wChargerAvail = 0;
	  
	  //RG_BC11_IPU_EN[1.0] = 01
	 upmu_set_rg_bc11_ipu_en(0x1);
	   //RG_BC11_VREF_VTH = [1:0]=10
	 upmu_set_rg_bc11_vref_vth(0x2);
	  //RG_BC11_CMP_EN[1.0] = 01
	 upmu_set_rg_bc11_cmp_en(0x1);
 
	 //msleep(80);
	 mdelay(80);
 
     wChargerAvail = upmu_get_rgs_bc11_cmp_out();

	if(Enable_BATDRV_LOG == BAT_LOG_FULL)
    {
    	battery_xlog_printk(BAT_LOG_FULL, "hw_bc11_stepC1() \r\n");
		hw_bc11_dump_register();
    }
 
	 //RG_BC11_IPU_EN[1.0] = 00
	 upmu_set_rg_bc11_ipu_en(0x0);
	  //RG_BC11_CMP_EN[1.0] = 00
	 upmu_set_rg_bc11_cmp_en(0x0);
	   //RG_BC11_VREF_VTH = [1:0]=00
     upmu_set_rg_bc11_vref_vth(0x0);
 
	 return  wChargerAvail;
 }
 
 
 static U32 hw_bc11_stepA2(void)
 {
	 U32 wChargerAvail = 0;
	  
	 //RG_BC11_VSRC_EN[1.0] = 10 
	 upmu_set_rg_bc11_vsrc_en(0x2);
	 //RG_BC11_IPD_EN[1:0] = 01
	 upmu_set_rg_bc11_ipd_en(0x1);
	 //RG_BC11_VREF_VTH = [1:0]=00
	 upmu_set_rg_bc11_vref_vth(0x0);
	 //RG_BC11_CMP_EN[1.0] = 01
	 upmu_set_rg_bc11_cmp_en(0x1);
 
	 //msleep(80);
	 mdelay(80);
	 
     wChargerAvail = upmu_get_rgs_bc11_cmp_out();

	if(Enable_BATDRV_LOG == BAT_LOG_FULL)
    {
    	battery_xlog_printk(BAT_LOG_FULL, "hw_bc11_stepA2() \r\n");
		hw_bc11_dump_register();
    }
 
	 //RG_BC11_VSRC_EN[1:0]=00
	 upmu_set_rg_bc11_vsrc_en(0x0);
	 //RG_BC11_IPD_EN[1.0] = 00
	 upmu_set_rg_bc11_ipd_en(0x0);
	 //RG_BC11_CMP_EN[1.0] = 00
	 upmu_set_rg_bc11_cmp_en(0x0);
 
	 return  wChargerAvail;
 }
 
 
 static U32 hw_bc11_stepB2(void)
 {
	 U32 wChargerAvail = 0;
 
	//RG_BC11_IPU_EN[1:0]=10
	upmu_set_rg_bc11_ipu_en(0x2);
	//RG_BC11_VREF_VTH = [1:0]=10
	upmu_set_rg_bc11_vref_vth(0x1);
	//RG_BC11_CMP_EN[1.0] = 01
	upmu_set_rg_bc11_cmp_en(0x1);
 
	//msleep(80);
	mdelay(80);
 
    wChargerAvail = upmu_get_rgs_bc11_cmp_out();

	if(Enable_BATDRV_LOG == BAT_LOG_FULL)
    {
    	battery_xlog_printk(BAT_LOG_FULL, "hw_bc11_stepB2() \r\n");
		hw_bc11_dump_register();
    }
 
	//RG_BC11_IPU_EN[1.0] = 00
	upmu_set_rg_bc11_ipu_en(0x0);
	//RG_BC11_CMP_EN[1.0] = 00
	upmu_set_rg_bc11_cmp_en(0x0);
	//RG_BC11_VREF_VTH = [1:0]=00
	upmu_set_rg_bc11_vref_vth(0x0);
 
	 return  wChargerAvail;
 }
 
 
 static void hw_bc11_done(void)
 {
 	 //RG_BC11_VSRC_EN[1:0]=00
	 upmu_set_rg_bc11_vsrc_en(0x0);
	 //RG_BC11_VREF_VTH = [1:0]=0
	 upmu_set_rg_bc11_vref_vth(0x0);
	 //RG_BC11_CMP_EN[1.0] = 00
	 upmu_set_rg_bc11_cmp_en(0x0);
	 //RG_BC11_IPU_EN[1.0] = 00
	 upmu_set_rg_bc11_ipu_en(0x0);
	 //RG_BC11_IPD_EN[1.0] = 00
	 upmu_set_rg_bc11_ipd_en(0x0);
	 //RG_BC11_BIAS_EN=0
	 upmu_set_rg_bc11_bias_en(0x0); 
 
	 Charger_Detect_Release();

	if(Enable_BATDRV_LOG == BAT_LOG_FULL)
    {
    	battery_xlog_printk(BAT_LOG_FULL, "hw_bc11_done() \r\n");
		hw_bc11_dump_register();
    }
    
 }


 static kal_uint32 charging_hw_init(void *data)
 {
 	kal_uint32 status = STATUS_OK;	
	
	upmu_set_rg_usbdl_rst(1);		//force leave USBDL mode
	        
 	return status;
 }


 static kal_uint32 charging_dump_register(void *data)
 {
 	kal_uint32 status = STATUS_OK;
   	
	return status;
 }	


 static kal_uint32 charging_enable(void *data)
 {
 	kal_uint32 status = STATUS_OK;
	kal_uint32 enable = *(kal_uint32*)(data);

	if(KAL_TRUE == enable)
	{
        rt9536_charging_enable(g_charging_current, enable);
	}
	else
	{
        g_charging_current = 0;
        rt9536_charging_enable(g_charging_current, enable);
	}
    
    battery_xlog_printk(BAT_LOG_CRTI, "[%s][charger_rt9536] charger enable = %d, g_charging_current = %d \r\n", __func__, enable, g_charging_current);
        
	return status;
 }


 static kal_uint32 charging_set_cv_voltage(void *data)
 {
 	kal_uint32 status = STATUS_OK;
	//kal_uint32 cv_value = *(kal_uint32 *)(data);

    /* TO DO */
    battery_xlog_printk(BAT_LOG_FULL, "[%s][charger_rt9536] NO Action! \r\n", __func__);
    
	return status;
 } 	


 static kal_uint32 charging_get_current(void *data)
 {
	kal_uint32 status = STATUS_OK;

    /* TO DO */
	
    battery_xlog_printk(BAT_LOG_CRTI, "[%s][charger_rt9536] g_charging_current = %d! \r\n", __func__, g_charging_current);
    
	return status;
 }  
  


 static kal_uint32 charging_set_current(void *data)
 {
 	kal_uint32 status = STATUS_OK;
	kal_uint32 current_value = *(kal_uint32 *)data;
//<2015/1/26-davidda, [V10][Feature][Common][BSP][]Battery OTP test
#if 0	
    if( (current_value == AC_CHARGER_CURRENT) || (current_value == USB_CHARGER_CURRENT) )
    {
        g_charging_current = current_value;
    }
    else
    {
        g_charging_current = USB_CHARGER_CURRENT;
    }
#else
        g_charging_current = current_value;
#endif
//>2015/1/26-davidda
        
    battery_xlog_printk(BAT_LOG_CRTI, "[%s][charger_rt9536] current_value = %d,     g_charging_current = %d \r\n", __func__, current_value, g_charging_current);
        
	return status;
 } 	
 

 static kal_uint32 charging_set_input_current(void *data)
 {
 	kal_uint32 status = STATUS_OK;
	kal_uint32 current_value = *(kal_uint32 *)data;

    battery_xlog_printk(BAT_LOG_CRTI, "[%s][charger_rt9536] input_current_value = %d \r\n", __func__, current_value);
    
	return status;
 } 	


 static kal_uint32 charging_get_charging_status(void *data)
 {
 	kal_uint32 status = STATUS_OK;

    *(kal_uint32 *)data = rt9536_check_eoc_status();
    
    battery_xlog_printk(BAT_LOG_CRTI, "[%s][charger_rt9536] EOC_status = %d \r\n", __func__, *(kal_uint32 *)data);
   
    return status;
}     


 static kal_uint32 charging_reset_watch_dog_timer(void *data)
 {
 	kal_uint32 status = STATUS_OK;

    /* TO DO */
    battery_xlog_printk(BAT_LOG_FULL, "[%s][charger_rt9536] NO Action! \r\n", __func__);
    
	return status;
 }
 
 
 static kal_uint32 charging_set_hv_threshold(void *data)
 {
    kal_uint32 status = STATUS_OK;

    kal_uint32 set_hv_voltage;
    kal_uint32 array_size;
    kal_uint16 register_value;
    kal_uint32 voltage = *(kal_uint32*)(data);

    array_size = GETARRAYNUM(VCDT_HV_VTH);
    set_hv_voltage = bmt_find_closest_level(VCDT_HV_VTH, array_size, voltage);
    register_value = charging_parameter_to_value(VCDT_HV_VTH, array_size ,set_hv_voltage);
    upmu_set_rg_vcdt_hv_vth(register_value);

    return status;
 }
 
 
  static kal_uint32 charging_get_hv_status(void *data)
  {
	   kal_uint32 status = STATUS_OK;
 
	   *(kal_bool*)(data) = upmu_get_rgs_vcdt_hv_det();

       
	   return status;
  }


 static kal_uint32 charging_get_battery_status(void *data)
 {
	   kal_uint32 status = STATUS_OK;
 
 	   upmu_set_baton_tdet_en(1);	
	   upmu_set_rg_baton_en(1);
	   *(kal_bool*)(data) = upmu_get_rgs_baton_undet();
	   
	   return status;
 }
//<2015/7/20-davidda, [V20][Feature][Common][BSP][]decrease the power consumption
extern void wake_up_bat(void);
kal_uint32  ext_charger_status=0;

 static kal_uint32 charging_get_charger_det_status(void *data)
 {
	   kal_uint32 status = STATUS_OK;
	   *(kal_bool*)(data) = upmu_get_rgs_chrdet();
          battery_xlog_printk(BAT_LOG_CRTI, " charger status detect from PMU  is  %d!\r\n", *(kal_bool*)(data));

      //<2015/6/19-davidda, [v10][Feature][Common][BSP][]detect charger status
      if( upmu_get_rgs_chrdet() == 0 )
      {
         if(rt9536_check_charger_status()==0)
         {
           *(kal_bool*)(data)=0;
	       charging_ic_deactive();
	       // could use this method, but not make sure if impact sleep current. 
           if(g_charger_type!=CHARGER_UNKNOWN) 
	 	       wake_up_bat();
	       g_charger_type = CHARGER_UNKNOWN;  
	  ext_charger_status = 0;
         }
         else
	 {
	       *(kal_bool*)(data)=1;
	 }
      }
      else
      {
      	 *(kal_bool*)(data)=1;
         ext_charger_status = 1;
      }
       //>2015/6/19-davidda
	   return status;
}
//>2015/7/10-davidda


 kal_bool charging_type_detection_done(void)
 {
	 return charging_type_det_done;
 } 	

 static kal_uint32 charging_get_charger_type(void *data)
 {
	 kal_uint32 status = STATUS_OK;
#if defined(CONFIG_POWER_EXT)
	 *(CHARGER_TYPE*)(data) = STANDARD_HOST;
#else

    if(g_charger_type!=CHARGER_UNKNOWN && g_charger_type!=WIRELESS_CHARGER)
    {
        *(CHARGER_TYPE*)(data) = g_charger_type;
        battery_xlog_printk(BAT_LOG_CRTI, "return %d!\r\n", g_charger_type);
        return status;
    }
    
	charging_type_det_done = KAL_FALSE;

	/********* Step initial  ***************/		 
	hw_bc11_init();
 
	/********* Step DCD ***************/  
	if(1 == hw_bc11_DCD())
	{
		 /********* Step A1 ***************/
		 if(1 == hw_bc11_stepA1())
		 {
			 /********* Step B1 ***************/
			 if(1 == hw_bc11_stepB1())
			 {
				 *(CHARGER_TYPE*)(data) = NONSTANDARD_CHARGER;
				 battery_xlog_printk(BAT_LOG_CRTI, "step B1 : Non STANDARD CHARGER!\r\n");
			 }	 
			 else
			 {
				 *(CHARGER_TYPE*)(data) = APPLE_2_1A_CHARGER;
				 battery_xlog_printk(BAT_LOG_CRTI, "step B1 : Apple 2.1A CHARGER!\r\n");
			 }	 
		 }
		 else
		 {
			 /********* Step C1 ***************/
			 if(1 == hw_bc11_stepC1())
			 {
				 *(CHARGER_TYPE*)(data) = APPLE_1_0A_CHARGER;
				 battery_xlog_printk(BAT_LOG_CRTI, "step C1 : Apple 1A CHARGER!\r\n");
			 }	 
			 else
			 {
				 *(CHARGER_TYPE*)(data) = APPLE_0_5A_CHARGER;
				 battery_xlog_printk(BAT_LOG_CRTI, "step C1 : Apple 0.5A CHARGER!\r\n");			 
			 }	 
		 }
 
	}
	else
	{
		 /********* Step A2 ***************/
		 if(1 == hw_bc11_stepA2())
		 {
			 /********* Step B2 ***************/
			 if(1 == hw_bc11_stepB2())
			 {
				 *(CHARGER_TYPE*)(data) = STANDARD_CHARGER;
				 battery_xlog_printk(BAT_LOG_CRTI, "step B2 : STANDARD CHARGER!\r\n");
			 }
			 else
			 {
				 *(CHARGER_TYPE*)(data) = CHARGING_HOST;
				 battery_xlog_printk(BAT_LOG_CRTI, "step B2 :  Charging Host!\r\n");
			 }
		 }
		 else
		 {
			 *(CHARGER_TYPE*)(data) = STANDARD_HOST;
			 battery_xlog_printk(BAT_LOG_CRTI, "step A2 : Standard USB Host!\r\n");
		 }
 
	}
 
	 /********* Finally setting *******************************/
	 hw_bc11_done();

 	charging_type_det_done = KAL_TRUE;

    g_charger_type = *(CHARGER_TYPE*)(data);    
#endif
	 return status;
}

static kal_uint32 charging_get_is_pcm_timer_trigger(void *data)
{
    kal_uint32 status = STATUS_OK;

    if(slp_get_wake_reason() == WR_PCM_TIMER)
        *(kal_bool*)(data) = KAL_TRUE;
    else
        *(kal_bool*)(data) = KAL_FALSE;

    battery_xlog_printk(BAT_LOG_CRTI, "slp_get_wake_reason=%d\n", slp_get_wake_reason());
       
    return status;
}

static kal_uint32 charging_set_platform_reset(void *data)
{
    kal_uint32 status = STATUS_OK;

    battery_xlog_printk(BAT_LOG_CRTI, "charging_set_platform_reset\n");
 
    arch_reset(0,NULL);
        
    return status;
}

static kal_uint32 charging_get_platfrom_boot_mode(void *data)
{
    kal_uint32 status = STATUS_OK;
  
    *(kal_uint32*)(data) = get_boot_mode();

    battery_xlog_printk(BAT_LOG_CRTI, "get_boot_mode=%d\n", get_boot_mode());
         
    return status;
}

static kal_uint32 charging_set_power_off(void *data)
{
    kal_uint32 status = STATUS_OK;

    battery_xlog_printk(BAT_LOG_CRTI, "charging_set_power_off\n");
    //<2015/1/30-davidda, [V10][Feature][Common][BSP][]Power on the phone without battery
    //mt_power_off();
    //>2015/1/30-davidda

    return status;
}

static kal_uint32 charging_get_power_source(void *data)
{
//<2015/1/30-davidda, [V10][Feature][Common][BSP][]Power on the phone without battery
#if defined(CONFIG_MTK_POWER_EXT_DETECT)
	 	kal_uint32 status = STATUS_OK;

 	//if (MT_BOARD_PHONE == mt_get_board_type())
//		*(kal_bool *)data = KAL_FALSE;
	//else
 	 	*(kal_bool *)data = KAL_TRUE;

#else
	kal_uint32 status = STATUS_UNSUPPORTED;
	return status;
#endif
//>2015/1/30-davidda
}

static kal_uint32 charging_get_csdac_full_flag(void *data)
{
	return STATUS_UNSUPPORTED;	
}

static kal_uint32 charging_set_ta_current_pattern(void *data)
{
	return STATUS_UNSUPPORTED;	
}

static kal_uint32 charging_get_error_state(void)
{
	return charging_error;
}

static kal_uint32 charging_set_error_state(void *data)
{
	kal_uint32 status = STATUS_OK;
	charging_error = *(kal_uint32*)(data);
	
	return status;
}

 static kal_uint32 (* const charging_func[CHARGING_CMD_NUMBER])(void *data)=
 {
 	  charging_hw_init
	,charging_dump_register  	
	,charging_enable
	,charging_set_cv_voltage
	,charging_get_current
	,charging_set_current
	,charging_set_input_current
	,charging_get_charging_status
	,charging_reset_watch_dog_timer
	,charging_set_hv_threshold
	,charging_get_hv_status
	,charging_get_battery_status
	,charging_get_charger_det_status
	,charging_get_charger_type
	,charging_get_is_pcm_timer_trigger
	,charging_set_platform_reset
	,charging_get_platfrom_boot_mode
	,charging_set_power_off
	,charging_get_power_source
	,charging_get_csdac_full_flag
	,charging_set_ta_current_pattern
	,charging_set_error_state
 };

 
 /*
 * FUNCTION
 *		Internal_chr_control_handler
 *
 * DESCRIPTION															 
 *		 This function is called to set the charger hw
 *
 * CALLS  
 *
 * PARAMETERS
 *		None
 *	 
 * RETURNS
 *		
 *
 * GLOBALS AFFECTED
 *	   None
 */
 kal_int32 chr_control_interface(CHARGING_CTRL_CMD cmd, void *data)
 {
	 kal_int32 status;
     
	 if(cmd < CHARGING_CMD_NUMBER)
	 {
		 status = charging_func[cmd](data);
	 }
	 else
	 {
		 return STATUS_UNSUPPORTED;
	 }
 
	 return status;
 }



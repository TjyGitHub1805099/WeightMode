/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "hal_gpio.h"
#include "app_led_ctrl.h"
#include "app_hx711_ctrl.h"
#include "app_main_task.h"
#include "app_key_ctrl.h"
#include "app_sdwe_ctrl.h"
#include "app_modbus_rtu_ctrl.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
//sys main task status
enumMainTaskCtrlType mainTaskStatus = MainTask_IDLE;
static UINT32 g_sys_ms_tick = 0 ;
gSystemParaType gSystemPara = gSystemParaDefault;

/*******************************************************************************
 * Functions
 ******************************************************************************/
//==sys main function
void app_main_task( void )
{
	UINT8 hx711DataUpgrade = 0 ;
	static UINT8 removeWeight = TRUE;
	//KEY sample and filter
	key_MainFunction();

	//HX711 sanple and calculate avgSampleValue and weight
	hx711DataUpgrade = hx711_MainFunction();

	//update LED and SDWE BLACK color
	//useWeightUpdateLedAndSdweColor(hx711DataUpgrade);
	//useWeightCompareOutColor(hx711DataUpgrade);

	//useWeightUpdataOutColor(hx711DataUpgrade);
	//useWeightUpdataOutColor_3030(hx711DataUpgrade);

	//test(hx711DataUpgrade,g_sys_ms_tick);
	
#if(COLOR_ALT_20210328_DEFINE)
	useWeightUpdataOutColor(hx711DataUpgrade);
#endif	
	
	
#if(COLOR_ALT_20210414_DEFINE)
	useWeightUpdataOutColor_20210414(hx711DataUpgrade);
#endif

#if(COLOR_ALT_20210606_DEFINE)
	useWeightUpdataOutColor_20210606(hx711DataUpgrade);
#endif

	//LED control
	#if LED_CTRL_TEST//test
		LedSysTest(g_sys_ms_tick);
		led_MainFunction(led_test_flag);
		led_test_flag=0;
	#else
		led_MainFunction(hx711DataUpgrade);
	#endif
	
	//SDWE RX/TX deal
	sdwe_MainFunction(hx711DataUpgrade);

	//bus comm
	ModbusRtu_MainFunction();
	
	//after power up 3 seconds clear all weight
	if((TRUE == removeWeight)&&(g_sys_ms_tick >= 3000))
	{
		removeWeight = FALSE;
		hx711_setAllRemoveWeight();
	}
	
	//sys tick add
	g_sys_ms_tick++;
	
}






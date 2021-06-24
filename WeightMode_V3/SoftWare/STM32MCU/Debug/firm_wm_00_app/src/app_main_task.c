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
#include "app_t5l_ctrl.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
//sys main task status
UINT32 g_sys_ms_tick = 0 ;

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

	//LED control test
	#if LED_CTRL_TEST
		LedSysTest(g_sys_ms_tick);
	#endif

	//T5L Screen Voice Pritf test
	#if T5L_VOICE_PRITF_TEST
		T5L_VoicePritfTest(g_sys_ms_tick);
	#endif

	#if T5L_WEIGHT_COLOR_TEST
		sdwe_MainFunctionTest();
	#endif

	//led contrl mainfunction
	led_MainFunction();
	
	//T5L contrl mainfunction
	sreenT5L_MainFunction();

	//data comm contrl mainfunction
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



#include "hal_gpio.h"
#include "app_led_ctrl.h"
#include "app_hx711_ctrl.h"
#include "app_main_task.h"
#include "app_key_ctrl.h"
#include "app_sdwe_ctrl.h"

//sys main task status
enumMainTaskCtrlType mainTaskStatus = MainTask_IDLE;
static UINT32 g_sys_ms_tick = 0 ;

//test
UINT8 KEY_COMBIN = 0XF0;

//sys main function
void app_main_task( void )
{
	UINT8 hx711DataUpgrade = 0 ;
	//KEY sample and filter
	key_MainFunction();
	
	//HX711 sanple and calculate weight
	hx711DataUpgrade = hx711_MainFunction();

	//update LED and SDWE BLACK color
	useWeightUpdateLedAndSdweColor(hx711DataUpgrade);
	
	//LED control
	led_MainFunction(hx711DataUpgrade);
	
	//SDWE RX/TX deal
	sdwe_MainFunction(hx711DataUpgrade);
	
	//
	g_sys_ms_tick++;
}


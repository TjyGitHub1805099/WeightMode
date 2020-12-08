#include "hal_gpio.h"
#include "app_led_ctrl.h"
#include "app_hx711_ctrl.h"
#include "app_main_task.h"
#include "app_key_ctrl.h"
#include "app_sdwe_ctrl.h"

//sys main task status
enumMainTaskCtrlType mainTaskStatus = MainTask_IDLE;
static UINT32 g_sys_ms_tick = 0 ;
static UINT32 g_sys_sdwe_tick = 0 ;

//test
UINT8 KEY_COMBIN = 0XF0;

//sys main function
void app_main_task( void )
{
	UINT8 i = 0 ,hx711DataUpgrade = 0 ;
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

	
	switch(mainTaskStatus)
	{
		case MainTask_IDLE:
			mainTaskStatus = MainTask_HX711_CTRL;
			//
			switch(KEY_COMBIN)
			{
				case 0xF0:
					if((SYS_KEY_VALUED == key_FilterGet(SYS_KEY_1))&&(SYS_KEY_INVALUED == key_FilterGet(SYS_KEY_2)))
					{
						KEY_COMBIN += 0x1;
						mainTaskStatus = MainTask_SYS_HX711_TEST;
					}
				break;
				default:
					if((SYS_KEY_INVALUED == key_FilterGet(SYS_KEY_1))&&(SYS_KEY_INVALUED == key_FilterGet(SYS_KEY_2)))
					{
						KEY_COMBIN = 0XF0;
					}
				break;
			}
		break;
		//==============================================	
		case MainTask_HX711_CTRL:
			//hx711_MainFunction();
			mainTaskStatus = MainTask_LED_CTRL;
		break;
		case MainTask_LED_CTRL:
			//useWeightUpdateLedColor();
			//led_MainFunction();
			mainTaskStatus = MainTask_SDWE_CTRL;
		break;
		case MainTask_SDWE_CTRL:
			//sdwe_MainCalFunction();
			if(0 == ((g_sys_sdwe_tick++)%25))//25*4=100ms
			{
				//sdwe_MainFunction();
			}
			mainTaskStatus = MainTask_IDLE;
		break;
		//==============================================
		case MainTask_SYS_LED_TEST:
			//LedSysTest(g_sys_ms_tick);
			//led_MainFunction();
			if((SYS_KEY_VALUED == key_FilterGet(SYS_KEY_1))&&(SYS_KEY_VALUED == key_FilterGet(SYS_KEY_2)))
			{
				led_init();
				//return idle
				mainTaskStatus = MainTask_IDLE;
			}
		break;
		case MainTask_SYS_HX711_TEST:
			for(i=0;i<HX711_CHANEL_NUM;i++)
			{
				//sampleCalcKB(i,0,0);
			}
			mainTaskStatus = MainTask_IDLE;
		break;
		case MainTask_SYS_SDWE_TEST:
			if((g_sys_ms_tick%125)==0)
			{
	 			sdwe_test();
			}
			if((SYS_KEY_VALUED == key_FilterGet(SYS_KEY_1))&&(SYS_KEY_VALUED == key_FilterGet(SYS_KEY_2)))
			{
				//return idle
				mainTaskStatus = MainTask_IDLE;
			}
		break;
		default :
			mainTaskStatus = MainTask_IDLE;
		break;
	}
	//
	g_sys_ms_tick++;
}


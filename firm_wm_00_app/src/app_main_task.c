#include "hal_gpio.h"
#include "app_led_ctrl.h"
#include "app_hx711_ctrl.h"
#include "app_main_task.h"
#include "app_key_ctrl.h"
#include "app_sdwe_ctrl.h"

enumMainTaskCtrlType mainTaskStatus = MainTask_IDLE;

void LedSysTest(UINT32 ms_tick)
{
	static UINT16 l_led_test_cycle = 1000;
	static enumLedColorType color = LED_COLOR_REG;
	enumLedSeqType seq = LED_SEQ_1;
	
	//every 1s change color:reg->yellow->blue->green
	if(0 == (ms_tick%l_led_test_cycle))
	{
		for(seq = LED_SEQ_1 ; seq < LED_SEQ_NUM ; seq++)
		{
			LedDataSet(seq,color);
		}
		
		color++;
		if( color >= LED_COLOR_NUM )
		{
			color = LED_COLOR_REG;
		}
	}
}

static UINT32 g_sys_ms_tick = 0 ;
static UINT32 g_sys_sdwe_tick = 0 ;
void app_main_task( void )
{
	key_MainFunction();
	switch(mainTaskStatus)
	{
		case MainTask_IDLE:
			if((SYS_KEY_VALUED == key_FilterGet(SYS_KEY_1))&&(SYS_KEY_INVALUED == key_FilterGet(SYS_KEY_2)))
			{
				mainTaskStatus = MainTask_SYS_LED_TEST;
			}
			else if((SYS_KEY_INVALUED == key_FilterGet(SYS_KEY_1))&&(SYS_KEY_VALUED == key_FilterGet(SYS_KEY_2)))
			{
				mainTaskStatus = MainTask_SYS_SDWE_TEST;
			}
			else
			{
				mainTaskStatus = MainTask_HX711_CTRL;
			}
		break;
		//==============================================	
		case MainTask_HX711_CTRL:
			hx711_MainFunction();
			mainTaskStatus = MainTask_LED_CTRL;
		break;
		case MainTask_LED_CTRL:
			useWeightUpdateLedColor();
			led_MainFunction();
			mainTaskStatus = MainTask_SDWE_CTRL;
		break;
		case MainTask_SDWE_CTRL:
			if(0 == ((g_sys_sdwe_tick++)%25))//25*4=100ms
			{
				sdwe_MainFunction();
			}
			mainTaskStatus = MainTask_IDLE;
		break;
		//==============================================
		case MainTask_SYS_LED_TEST:
			LedSysTest(g_sys_ms_tick);
			led_MainFunction();
			if((SYS_KEY_VALUED == key_FilterGet(SYS_KEY_1))&&(SYS_KEY_VALUED == key_FilterGet(SYS_KEY_2)))
			{
				led_init();
				//return idle
				mainTaskStatus = MainTask_IDLE;
			}
		break;
		case MainTask_SYS_HX711_TEST:
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

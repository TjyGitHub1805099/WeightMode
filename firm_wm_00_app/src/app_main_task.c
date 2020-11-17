#include "app_led_ctrl.h"
#include "app_hx711_ctrl.h"
#include "app_main_task.h"
#include "hal_gpio.h"

enumMainTaskCtrlType mainTaskStatus = MainTask_LED_CTRL;

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

void app_main_task( void )
{
	
	switch(mainTaskStatus)
	{
		case 	MainTask_LED_CTRL:
					LedCtrlModeCrtl();
					mainTaskStatus = MainTask_HX711_SAMPLE_CTRL;
					if(1 == hal_di_get(SYS_KEY_1))
					{
						mainTaskStatus = MainTask_SYS_LED_TEST;
					}

		break;
		case MainTask_HX711_SAMPLE_CTRL:
					hx711_DataSampleCtrl();
					mainTaskStatus = MainTask_LED_CTRL;
		break;
		case MainTask_SYS_LED_TEST:
				 LedSysTest(g_sys_ms_tick);
				 LedCtrlModeCrtl();
					if(1 == hal_di_get(SYS_KEY_2))
					{
						LedCtrlModeInit();
						mainTaskStatus = MainTask_LED_CTRL;
					}		
		break;
		default :
					mainTaskStatus = MainTask_LED_CTRL;
		break;
	}
	//
	g_sys_ms_tick++;
}

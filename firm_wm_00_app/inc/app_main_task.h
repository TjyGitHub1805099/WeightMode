#ifndef __APP_MAIN_TASK_H__
#define __APP_MAIN_TASK_H__

//main task status
typedef enum MainTaskCtrlType
{
	MainTask_IDLE=0,		  /**< LED���� */

	MainTask_LED_CTRL,		  /**< LED���� */
	  MainTask_HX711_SAMPLE_CTRL,	/**< HX711���� */
	  MainTask_SYS_LED_TEST,		  /**< ϵͳLED���Կ��� */
		MainTask_SYS_SDWE_TEST,			/**< ϵͳLED���Կ��� */
    MainTask_MAIN_TASK_CTRL
}enumMainTaskCtrlType;

#endif

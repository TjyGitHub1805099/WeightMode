#ifndef __APP_MAIN_TASK_H__
#define __APP_MAIN_TASK_H__

//main task status
typedef enum MainTaskCtrlType
{
	  MainTask_LED_CTRL=0,      	/**< LED控制 */
	  MainTask_HX711_SAMPLE_CTRL,	/**< HX711控制 */
	  MainTask_SYS_LED_TEST,		  /**< 系统LED测试控制 */
    MainTask_MAIN_TASK_CTRL
}enumMainTaskCtrlType;

#endif

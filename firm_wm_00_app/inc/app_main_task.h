#ifndef __APP_MAIN_TASK_H__
#define __APP_MAIN_TASK_H__

//main task status
typedef enum MainTaskCtrlType
{
	MainTask_IDLE=0,		  /**< LED控制 */

	MainTask_LED_CTRL,		  /**< LED控制 */
	  MainTask_HX711_SAMPLE_CTRL,	/**< HX711控制 */
	  MainTask_SYS_LED_TEST,		  /**< 系统LED测试控制 */
		MainTask_SYS_SDWE_TEST,			/**< 系统LED测试控制 */
    MainTask_MAIN_TASK_CTRL
}enumMainTaskCtrlType;

#endif

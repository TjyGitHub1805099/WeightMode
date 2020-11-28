#ifndef __APP_MAIN_TASK_H__
#define __APP_MAIN_TASK_H__

#define TRUE 1
#define FALSE 1


//main task status
typedef enum MainTaskCtrlType
{
	MainTask_IDLE=0,		/**< 空闲 */
	MainTask_HX711_CTRL,	/**< HX711控制 */
	MainTask_LED_CTRL,		/**< LED控制 */
	MainTask_SDWE_CTRL,		/**< SDWE控制 */
	MainTask_SYS_LED_TEST,	/**< 系统LED测试控制 */
	MainTask_SYS_HX711_TEST,/**< 系统HX711测试控制 */
	MainTask_SYS_SDWE_TEST,	/**< 系统SDWE测试控制 */
	MainTask_MAIN_TASK_CTRL
}enumMainTaskCtrlType;

#endif

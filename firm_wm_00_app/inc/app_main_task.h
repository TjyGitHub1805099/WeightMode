#ifndef __APP_MAIN_TASK_H__
#define __APP_MAIN_TASK_H__

#include "typedefine.h"
#include "app_led_ctrl.h"

#define TRUE 1
#define FALSE 0


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


#define SYS_COLOR_GROUP_NUM		(4)
#define SYS_COLOR_USED_FLAG		(0X123)
typedef struct
{
	//store in flash
	INT32	uint;/**< 单位 */
	INT32	minWeight;/**< 最小量程 */
	INT32	maxWeight;/**< 最大量程 */
	float	errRange;/**< 误差范围 */
	INT32	isCascade;/**< 是否级联 */
	INT32	isLedIndicate;/**< 是否LED指示 */
	INT32	userColorSet[SYS_COLOR_GROUP_NUM];/**< 配平色1~4 */
	float	zeroRange;/**< 零点范围 */

	//sys used flag
	UINT16				userColorUsed[SYS_COLOR_GROUP_NUM];/**< chanel_a<<8 + chanel_b*/
} gSystemParaType;

#define gSystemParaDefault {\
0, \
0, \
5000, \
(float)(2.0), \
FALSE, \
TRUE, \
{LED_COLOR_GREEN, LED_COLOR_NONE, LED_COLOR_NONE, LED_COLOR_NONE },\
(float)(5.0), \
{FALSE, FALSE, FALSE, FALSE },\
}


#define STM32MCU_LOCKED		(0X10CD)
#define STM32MCU_UNLOCKED	(0X20CD)
extern gSystemParaType gSystemPara;
extern INT32 g_passWordId;
extern INT32 g_passWordStore;
extern INT32 g_sysLocked;
extern UINT8 STM32CheckPassWord(INT32 passwordIn);

#endif

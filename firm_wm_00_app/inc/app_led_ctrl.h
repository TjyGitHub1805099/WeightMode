#ifndef __APP_LED_CTRL_H__
#define __APP_LED_CTRL_H__

#include "typedefine.h"

#define LED_CTRL_TEST	(0)//0:close test 1:open test
#define LED_CTRL_DATA_LEN 3

//main task status
typedef enum LedSeqType
{
	LED_SEQ_1 = 0,    /**< LED 1控制 */
	LED_SEQ_2 ,       /**< LED 2控制 */
	LED_SEQ_3 ,       /**< LED 3控制 */
	LED_SEQ_4 ,       /**< LED 4控制 */
	LED_SEQ_5 ,       /**< LED 5控制 */
	LED_SEQ_6 ,       /**< LED 6控制 */
	LED_SEQ_NUM
}enumLedSeqType;

typedef enum LedColorType
{
	LED_COLOR_NONE = 0,		/**< LED 无 控制 */
	LED_COLOR_REG ,	/**< LED 红 控制 */
	LED_COLOR_GREEN,	/**< LED 绿 控制 */
	LED_COLOR_BLUE, 	/**< LED 蓝 控制 */
	LED_COLOR_WHITE,	/**< LED 白 控制 */
	LED_COLOR_NUM,
	LED_COLOR_LOCK=0X80,
}enumLedColorType;


void led_init(void);
void led_MainFunction(UINT8 hx711DataUpgrade);
UINT8 LedDataSet(enumLedSeqType seq , enumLedColorType color);
void useWeightUpdateLedAndSdweColor(UINT8 hx711DataUpgrade);
void useWeightCompareOutColor(UINT8 hx711DataUpgrade);
extern void useWeightUpdataOutColor(UINT8 hx711DataUpgrade);
void LedSysTest(UINT32 ms_tick);
extern UINT8 led_test_flag ;
#endif


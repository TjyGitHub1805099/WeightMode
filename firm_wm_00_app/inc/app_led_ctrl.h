#ifndef __APP_LED_CTRL_H__
#define __APP_LED_CTRL_H__

#include "typedefine.h"

#define LED_CTRL_DATA_LEN 4

//main task status
typedef enum LedSeqType
{
	  LED_SEQ_1 = 0,    /**< LED 1���� */
	  LED_SEQ_2 ,       /**< LED 2���� */
	  LED_SEQ_3 ,       /**< LED 3���� */
	  LED_SEQ_4 ,       /**< LED 4���� */
	  LED_SEQ_5 ,       /**< LED 5���� */
	  LED_SEQ_6 ,       /**< LED 6���� */
	  LED_SEQ_7 ,       /**< LED 7���� */
	  LED_SEQ_8 ,       /**< LED 8���� */
    LED_SEQ_NUM
}enumLedSeqType;

typedef enum LedColorType
{
	  LED_COLOR_REG = 0,  /**< LED �� ���� */
	  LED_COLOR_WHITE,    /**< LED �� ���� */
	  LED_COLOR_BLUE,    	/**< LED �� ���� */
	  LED_COLOR_GREEN,    /**< LED �� ���� */
    LED_COLOR_NUM
}enumLedColorType;


void LedCtrlModeInit(void);
void LedCtrlModeCrtl(void);
UINT8 LedDataSet(enumLedSeqType seq , enumLedColorType color);

#endif

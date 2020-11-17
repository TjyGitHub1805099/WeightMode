#ifndef __APP_HX711_CTRL_H__
#define __APP_HX711_CTRL_H__

#include "typedefine.h"

//sample NUM

#define HX711_CHANEL_NUM (8)
#define HX711_DATA_SAMPLE_TYPE (25)//
#define HX711_DATA_SAMPLE_WIDE (10)//15us

#define HX711_DATA_SAMPLE_NUM (10)

#define HX711_DEFAULT_DATA	(0X1FFFFFF)
#define HX711_NEGATIVE_DATA	(0X1000000)
#define HX711_MAX_WAIT_TIME (200)//200ms


#define WM_LINEAR_K	(float)(0.0011)
#define WM_LINEAR_B	(float)(268.80)

//main task status
typedef enum HX711ChanelType
{
	  HX711Chanel_1 = 0,    /**< HX711  1���� */
	  HX711Chanel_2 ,       /**< HX711  2���� */
	  HX711Chanel_3 ,       /**< HX711  3���� */
	  HX711Chanel_4 ,       /**< HX711  4���� */
	  HX711Chanel_5 ,       /**< HX711  5���� */
	  HX711Chanel_6 ,       /**< HX711  6���� */
	  HX711Chanel_7 ,       /**< HX711  7���� */
	  HX711Chanel_8 ,       /**< HX711  8���� */
    HX711Chanel_NUM
}enumHX711ChanelType;

//main task status
typedef enum HX711CtrlType
{
	  HX711_CTRL_INIT = 0,   			/**< HX711  ��ʼ������ */
	  HX711_CTRL_POWER_OFF,   		/**< HX711  �µ���� */
    HX711_CTRL_POWER_ON,				/**< HX711  �ϵ���� */
    HX711_CTRL_WAIT,						/**< HX711  �ȴ��½��ؿ��� */
    HX711_CTRL_SAMPLE,					/**< HX711  �������� */
    HX711_CTRL_NUM
}enumHX711CtrlType;

extern void hx711_DataSampleCtrl(void);

#endif

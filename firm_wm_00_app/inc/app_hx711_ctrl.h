#ifndef __APP_HX711_CTRL_H__
#define __APP_HX711_CTRL_H__

#include "typedefine.h"
#include "app_led_ctrl.h"

//sample NUM
typedef enum HX711ChanelType
{
	HX711Chanel_1 = 0,  /**< HX711  1控制 */
	HX711Chanel_2 ,      /**< HX711  2控制 */
	HX711Chanel_3 ,      /**< HX711  3控制 */
	HX711Chanel_4 ,      /**< HX711  4控制 */
	HX711Chanel_5 ,      /**< HX711  5控制 */
	HX711Chanel_6 ,      /**< HX711  6控制 */
	HX711Chanel_7 ,      /**< HX711  7控制 */
	HX711Chanel_8 ,      /**< HX711  8控制 */
	HX711_CHANEL_NUM	 /**< HX711  总数量 hardware chanel number */
}enumHX711ChanelType;

#define HX711_DATA_SAMPLE_TYPE 	(25)//hx711 sample type ;25:used chanel A 128 gain
#define HX711_DATA_SAMPLE_WIDE 	(10)//us:careful not lager than 50us 

#define HX711_DATA_SAMPLE_NUM 	(10)//each chanel filter number
#define HX711_MAX_WAIT_TIME 	(500)//ms:the max wait Data line from high -> low

#define HX711_DEFAULT_DATA		(0X1FFFFFF)
#define HX711_NEGATIVE_DATA		(0X1000000)//if lager than this data ,it's negative data


#define CHANEL_FILTER_NUM	(10)
#define CHANEL_SECTION_NUM	(10)//must lager than 2
#define CHANEL_MAX_WEIGHT	(5000)
#define CHANEL_DEFAULT_K	(float)(0.001223)//defaule k
#define CHANEL_DEFAULT_B	(float)(-267.71)//default b
#define CHANEL_MAX_ERR_RANGE	(float)(2.0)//2.0g

typedef struct
{
	enumLedSeqType ledType;
	UINT8 	initFlag;
	UINT8	sampleCycle;//onme cycle sample over allow to calculate
	
	UINT8	sample_offset;
	UINT8	section_offset;
	
	INT32	sample_Arr[CHANEL_FILTER_NUM];
	INT32	sample_TotalValue;
	INT32	sample_AvgValue;
	
	INT32	section_PointSample[CHANEL_SECTION_NUM+1];
	INT32	section_PointWeight[CHANEL_SECTION_NUM+1];
	float 	section_K[CHANEL_SECTION_NUM+2];//0:degative  CHANEL_SECTION_NUM+1:out range;this 2 status use default K & B
	float 	section_B[CHANEL_SECTION_NUM+2];
	INT32	weightTen;
	INT32	weight;
} ChanelType;


//main task status
typedef enum HX711CtrlType
{
	HX711_CTRL_INIT = 0,   	/**< HX711  初始化控制 */
	HX711_CTRL_POWER_OFF,   /**< HX711  下电控制 */
	HX711_CTRL_POWER_ON,	/**< HX711  上电控制 */
	HX711_CTRL_WAIT,		/**< HX711  等待下降沿控制 */
	HX711_CTRL_SAMPLE,		/**< HX711  采样控制 */
	HX711_CTRL_NUM
}enumHX711CtrlType;

extern void hx711_init(void);
extern void hx711_MainFunction(void);
extern float hx711_getWeight(enumHX711ChanelType chanel);
extern float hx711_getWeightTen(enumHX711ChanelType chanel);
extern void sampleCalcKB(UINT8 chanel,UINT8 point,INT32 weight);
#endif


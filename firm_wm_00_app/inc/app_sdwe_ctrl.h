
#ifndef __APP_SDWE_H__
#define __APP_SDWE_H__

#include "hal_uart.h"
#include "app_hx711_ctrl.h"

#define SDWE_UART_DATA_LEN	(0X100)

//address of weight back to SDWE : (0~7)-> weight of chanel(val:g)
#define SDWE_FUNC_ASK_CHANEL_WEIGHT			(0X0160)//0x0160~0x0167

//address of COLOR back to SDWE : (0~7)-> COLOR of chanel(val=0x00:white,val=0x01:red,val=0x02:green,val=0x03:blue,val=0x04:yellowWhite)
#define SDWE_FUNC_ASK_CHANEL_WEIGHT_COLOR	(0X0168)//0x0168~0x016F

//address of set chanel number : 0->all chanel set  ; (1~8)->single chanel set
#define SDWE_FUNC_SET_CHANEL_NUM			(0X01FF)

//address of set point(weight value) of chanel : (0~9)-> point of chanel set (:g)
#define SDWE_FUNC_SET_CHANEL_POINT			(0X0200)//0x0200~0x0209

//address of COLOR back to SDWE : (0~9)-> COLOR of point of chanel set triger(val=0x00:white(not triger),val=0x01green(triger))
#define SDWE_FUNC_ASK_CHANEL_POINT_TRIG		(0X020A)//0x020A~0x0213

//address of set point of chanel triger : (0~9)-> point of chanel set triger(val=0xff(SDWE triger MCU))
#define SDWE_FUNC_SET_CHANEL_POINT_TRIG		(0X0214)//0x0214~0x021D


//==========================================================================================================================
//point sample , weight value , K ,B ,weightRemove

//start of on board flash store address
#define FLASH_STORE_ADDRESS_START				(0X0803F000)

//each chanel have 10 point : 8*10*8(sample + weight value) byte
#define FLASH_CHANEL_POINT_ADDRESS_START		(FLASH_STORE_ADDRESS_START)
#define FLASH_CHANEL_POINT_LEN					(HX711_CHANEL_NUM*CHANEL_POINT_NUM*8)
#define FLASH_CHANEL_POINT_ADDRESS_END			(FLASH_CHANEL_POINT_ADDRESS_START+FLASH_CHANEL_POINT_LEN)

//each chanel have 10 point -> 11 KB : 8*11*8(K + B) byte
#define FLASH_CHANEL_POINT_KB_ADDRESS_START		(FLASH_CHANEL_POINT_ADDRESS_END)
#define FLASH_CHANEL_POINT_KB_LEN				(HX711_CHANEL_NUM*(CHANEL_POINT_NUM+1)*8)
#define FLASH_CHANEL_POINT_KB_ADDRESS_END		(FLASH_CHANEL_POINT_KB_ADDRESS_START+FLASH_CHANEL_POINT_KB_LEN)

//each chanel remove weight:8*4
#define FLASH_CHANEL_POINT_RMW_ADDRESS_START	(FLASH_CHANEL_POINT_KB_ADDRESS_END)
#define FLASH_CHANEL_POINT_RMW_LEN				(HX711_CHANEL_NUM*4)
#define FLASH_CHANEL_POINT_RMW_ADDRESS_END		((FLASH_CHANEL_POINT_RMW_ADDRESS_START)+FLASH_CHANEL_POINT_RMW_LEN)

//end of on board flash store address
#define FLASH_STORE_ADDRESS_END					(FLASH_CHANEL_POINT_RMW_ADDRESS_END)

//store flash data : 8 * (sample value , weight value , k , b , remove value ) , crc
#define FLASH_STORE_MAX_LEN						((FLASH_STORE_ADDRESS_END/4)+1)
//==========================================================================================================================

typedef enum sdweRxFuncIdType
{
	/**< SDWE_RX_0X83 举例
	1：A5 5A 06 83 01 FF 01 00 01 ; 代表 add = 0x01ff(校准通道号选择) , len = 0x01 , data = 0x0001 
	解释：校准通道选择 (add=0x01ff , len = 1), data=0:所有通道 data=1~8:代表具体通道
	2:A5 5A 06 83 03 00 01 00 0A
	解释：对于通道下的校准点选择 (add=0x0300 , len = 1), data=1~11:具体点(十段总共11点)
	*/
	SDWE_RX_FUN_HEAD1 = 0XA5, /**< SDWE HEAD1*/
	SDWE_RX_FUN_HEAD2 = 0X5A, /**< SDWE HEAD2*/
	SDWE_RX_FUN_0X83 = 0X83, /**< SDWE 设置变量 下发给MCU*/
	SDWE_RX_FUN_NUM	 		 /**< SDWE 总数量*/
}enumsdweRxFuncIdType;

typedef enum sdweTxFuncIdType
{
	SDWE_TX_FUN_0X83 = 0X83, /**< SDWE 设置变量 下发给MCU*/
	SDWE_TX_FUN_NUM 		 /**< SDWE 总数量*/
}enumsdweTxFuncIdType;



/** 定义从机串口设备类型 */
typedef struct structSdweType
{
	UartDeviceType *pUartDevice;        /**< 串口设备 */
	UINT8 	rxData[SDWE_UART_DATA_LEN];
	UINT8 	txData[SDWE_UART_DATA_LEN];
	UINT16	RxLength;					/**< 接收字节数 */
	UINT8 	RxFinishFlag;				/**< 接收完成标志 */
	
	UINT16  sdweSetAdd;/**< 地址 */
	INT16  	sdweSetData;/**< 数据 */
	
	UINT16 	sdweCalChanel;/**< 通道 */
	UINT16 	sdweCalPoint;/**< 校准点 */
	INT32 	sdweCalPointArry[CHANEL_POINT_NUM];/**< 校准点数组 */
}SdweType;

/** ModbusRtu设备默认配置 */
#define SdweDefault             { \
	&g_UartDevice[UART_EXTERN], \
	{0}, \
	{0}, \
	0,\
	0,\
	0XFFFF,\
	0XFFFF,\
	0,\
	0,\
	{0},\
	}

#define SDWE_WEIGHR_DATA_LEN (2*HX711_CHANEL_NUM)//8 weight data + 8 color data	


extern void sdwe_init(void);
extern void sdwe_test(void);
extern void sdweSetWeightBackColor(UINT8 seq,UINT8 color);
extern void sdwe_MainFunction(UINT8 hx711DataUpgrade);
extern void readSysDataFromFlash();

#endif


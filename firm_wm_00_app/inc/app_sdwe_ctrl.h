
#ifndef __APP_SDWE_H__
#define __APP_SDWE_H__

#include "hal_uart.h"

#define SDWE_UART_DATA_LEN	(0X100)


/** 定义从机串口设备类型 */
typedef struct structSdweType
{
	UartDeviceType *pUartDevice;        /**< 串口设备 */
	UINT8 	rxData[SDWE_UART_DATA_LEN];
	UINT8 	txData[SDWE_UART_DATA_LEN];
	UINT16	RxLength;					/**< 接收字节数 */
	UINT8 	RxFinishFlag;				/**< 接收完成标志 */
}SdweType;

/** ModbusRtu设备默认配置 */
#define SdweDefault             { \
	&g_UartDevice[UART_EXTERN], \
	{0}, \
	{0}, \
	0,\
	0,\
	}


extern void sdwe_init(void);
extern void sdwe_test(void);
extern void sdwe_MainFunction(void);
#endif


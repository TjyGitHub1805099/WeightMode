
#ifndef __APP_MODBUS_RTU_H__
#define __APP_MODBUS_RTU_H__

#include "hal_uart.h"
#include "app_hx711_ctrl.h"

#define MODBUS_RTU_UART_DATA_LEN	(0X100)


/** 定义从机串口设备类型 */
typedef struct structModbusRtuType
{
	UartDeviceType *pUartDevice;        /**< 串口设备 */
	UINT8 	rxData[MODBUS_RTU_UART_DATA_LEN];
	UINT8 	txData[MODBUS_RTU_UART_DATA_LEN];
	UINT16	RxLength;					/**< 接收字节数 */
	UINT8 	RxFinishFlag;				/**< 接收完成标志 */
	
	UINT32	sysTick;

	UINT16  SetAdd;/**< 地址 */
	INT16  	DataLen;/**< 数据长度 */
	INT16  	SetData;/**< 数据 */

}ModbusRtuType;

/** ModbusRtu设备默认配置 */
#define ModbusRtuDefault   { \
	&g_UartDevice[UART_COM], \
	{0}, \
	{0}, \
	0,\
	0,\
	0XFFFF,\
	0,\
	0,\
	}
	
extern ModbusRtuType g_ModbusRtu;

extern void ModbusRtu_init(void);
extern void ModbusRtu_MainFunction(void);

#endif

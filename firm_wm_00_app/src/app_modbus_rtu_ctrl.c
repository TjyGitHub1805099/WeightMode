/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "drv_flash.h"
#include "app_main_task.h"
#include "app_modbus_rtu_ctrl.h"
#include "app_crc.h"
#include "app_hx711_ctrl.h"
#include "app_crc.h"
#include "hal_delay.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
ModbusRtuType g_ModbusRtu = ModbusRtuDefault;

/*******************************************************************************
 * Functions
 ******************************************************************************/
//==ModbusRtu initial
void ModbusRtu_init(void)
{
	UINT8 i = 0 ;
	//
	g_ModbusRtu.pUartDevice = &g_UartDevice[UART_COM];
	//
	g_ModbusRtu.pUartDevice->pRxLength = &g_ModbusRtu.RxLength;
	g_ModbusRtu.pUartDevice->pRxFinishFlag = &g_ModbusRtu.RxFinishFlag;
	g_ModbusRtu.pUartDevice->pTxBuffer = &g_ModbusRtu.rxData[0];
	g_ModbusRtu.pUartDevice->pRxBuffer = &g_ModbusRtu.rxData[0];
	//
	//
	g_ModbusRtu.RxLength = 0;					/**< 接收字节数 */
	g_ModbusRtu.RxFinishFlag = FALSE;			/**< 接收完成标志 */
	//
	g_ModbusRtu.SetAdd = 0XFFFF;/**< 地址 */
	g_ModbusRtu.DataLen = 0;/**< 数据长度 */
	g_ModbusRtu.SetData = 0;/**< 数据 */

	//
	g_ModbusRtu.pUartDevice->init(g_ModbusRtu.pUartDevice);
}
//==ModbusRtu main function
void ModbusRtu_MainFunction(void)
{
	g_ModbusRtu.sysTick++;
	if(TRUE == g_ModbusRtu.RxFinishFlag)
	{
	
		//
		g_ModbusRtu.RxFinishFlag = FALSE;
	}

	if(g_ModbusRtu.sysTick%1000 == 0)
	{
		g_ModbusRtu.txData[0] = g_ModbusRtu.sysTick/1000;
		g_ModbusRtu.txData[1] = g_ModbusRtu.sysTick;
		g_ModbusRtu.pUartDevice->tx_bytes(g_ModbusRtu.pUartDevice,&g_ModbusRtu.txData[0],2);

	}
}



#ifndef __APP_MODBUS_RTU_H__
#define __APP_MODBUS_RTU_H__

#include "hal_uart.h"
#include "app_hx711_ctrl.h"

#define MODBUS_RTU_UART_DATA_LEN	(0X100)
#define MODBUS_RTU_CRC_EN			(TRUE)


#define MODBUS_RTU_SLAVE_CHANEL_NUM	(HX711_CHANEL_NUM)
#define MODBUS_RTU_SLAVE_DATA_LEN	(4*MODBUS_RTU_SLAVE_CHANEL_NUM)

//aa bb cc dd ee ff .. hh
//aa-> sender address
//bb-> recive address
//cc-> fuction code
//dd-> data len
//ee-> data
//hh-> crc of all
#define MODBUS_RTU_SEND_ADDRESS_POS	(0)//sender ID : master(1) or slave(2~n)
#define MODBUS_RTU_ASK_ADDRESS_POS	(1)//recive ID : master(1) or slave(2~n)
#define MODBUS_RTU_FUN_CODE_POS		(2)//function code :
#define MODBUS_RTU_DATA_LEN_POS		(3)//data len
#define MODBUS_RTU_DATA_STAR_POS	(4)//data start position

//slave address type
typedef enum ModbusAddType
{
	ModbusAdd_Master	= 1 ,//1:was the master ID
	ModbusAdd_Slave_1	= 2 ,//other:was the slave ID
	ModbusAdd_Slave_Max
}enumModbusAddType;
//function code def address type
typedef enum ModbusFuncCodeType
{
	MFC_Mater_Ask_Weight_Float = 0x01 ,
	MFC_Mater_Ask_Weight_INT = 0x02 ,
	MFC_Mater_Ask_Weight_Remove = 0x03 ,
	MFC_Mater_Ask_Weight_Float_WriteHelpData = 0x04 ,
	MFC_Slave_Ans_Weight_Float = 0x81 ,
	MFC_Slave_Ans_Weight_INT = 0x82 ,
	MFC_Slave_Ans_Weight_Remove = 0x83 ,
	MFC_Slave_Ans_Weight_Float_WriteHelpData = 0x04 ,
}enumModbusFuncCodeType;
	
#define MODBUS_FUNCTION_CODE_MASTER_ASK1_DATA_LEN	(1)

#define MasterTimer_TxOrderDlay						(15)//15ms
#define MasterTimer_RxTimeOut						(60)//50ms
#define MasterTimer_CycleRead_Weight_Float			(200)//200ms
#define MasterTimer_CycleRead_Weight_Float_AndSetHelpDataToSlave1			(200)//200ms

#define SlaveTimer_TxOrderDlay						(MasterTimer_TxOrderDlay)//20ms


//master tx mask
typedef UINT16 MasterTxMaskType;
#define MasterTxMask_IDLE						(MasterTxMaskType)(0x0000)
#define MasterTxMask_CycleRead_Weight_Float		(MasterTxMaskType)(0x0001)
#define MasterTxMask_CycleRead_Weight_Int32		(MasterTxMaskType)(0x0002)
#define MasterTxMask_CycleRead_Weight_Float_AndSetHelpDataToSlave1		(MasterTxMaskType)(0x0004)
//master rx mask
typedef UINT16 MasterRxMaskType;
#define MasterRxMask_IDLE						(MasterRxMaskType)(0x0000)

//slave tx mask
typedef UINT16 SlaveTxMaskType;
#define SlaveTxMask_IDLE						(SlaveTxMaskType)(0x0000)
#define SlaveTxMask_CycleRead_Weight_Float		(SlaveTxMaskType)(0x0001)
#define SlaveTxMask_CycleRead_Weight_Int32		(SlaveTxMaskType)(0x0002)
#define SlaveTxMask_RemoveWeight				(SlaveTxMaskType)(0x0004)


//slave rx mask
typedef UINT16 SlaveRxMaskType;
#define SlaveRxMask_Idle						(SlaveRxMaskType)(0x0000)
#define SlaveRxMask_CycleRead_Weight_Float		(SlaveTxMaskType)(0x0001)
#define SlaveRxMask_CycleRead_Weight_Int32		(SlaveTxMaskType)(0x0002)
#define SlaveRxMask_RemoveWeight				(SlaveRxMaskType)(0x0004)


typedef enum MasterStateType
{
	MasterState_Idle = 0 ,
	MasterState_AllowTx = 1 ,
	MasterState_WaitRx = 2,
}enumMasterStateType;
typedef enum SlaveStateType
{
	SlaveState_Idle = 0 ,
	SlaveState_AllowTx = 1 ,
	SlaveState_WaitRx = 2,
}enumSlaveStateType;


/** 定义从机串口设备类型 */
typedef struct structModbusRtuType
{
	UartDeviceType *pUartDevice;        /**< 串口设备 */
	UINT8 	rxData[MODBUS_RTU_UART_DATA_LEN];
	UINT8 	txData[MODBUS_RTU_UART_DATA_LEN];
	UINT16	RxLength;					/**< 接收字节数 */
	UINT8 	RxFinishFlag;				/**< 接收完成标志 */	
	UINT32	sysTick;
	unionFloatInt32	MultWeightData[ModbusAdd_Slave_Max - ModbusAdd_Master][MODBUS_RTU_SLAVE_CHANEL_NUM];
	
	//master
	enumMasterStateType masterState;
	MasterTxMaskType	masterTxMask;
	MasterRxMaskType	masterRxMask;
	UINT16 				masterTxDiffTick;
	UINT16 				masterMaxWaitRxTick;

	//slave
	enumSlaveStateType	slaveState;
	SlaveTxMaskType		slaveTxMask;
	SlaveRxMaskType		slaveRxMask;
	UINT16 				slaveTxDiffTick;

	UINT8	needSendLen;
	UINT8 	slaveID;
	UINT8 	allowToSend;
	//master or slave
	
}ModbusRtuType;

/** ModbusRtu设备默认配置 */
#define ModbusRtuDefault   { \
	&g_UartDevice[UART_COM], \
	{0}, \
	{0}, \
	0,\
	0,\
	0,\
	{{0}},\
	MasterState_Idle,\
	MasterTxMask_IDLE,\
	MasterRxMask_IDLE,\
	0,\
	0,\
	SlaveState_Idle,\
	SlaveTxMask_IDLE,\
	SlaveRxMask_Idle,\
	0,\
	0,\
	0,\
	0,\
	}
	
extern ModbusRtuType g_ModbusRtu;

extern void ModbusRtu_init(void);
extern void ModbusRtu_MainFunction(void);

#endif

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
#include "app_sdwe_ctrl.h"
#include "app_syspara.h"
#include "app_t5l_ctrl.h"

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
	//
	g_ModbusRtu.pUartDevice->init(g_ModbusRtu.pUartDevice);
}



UINT8 ModbusRtu_SlaveRxMaskDeal(ModbusRtuType *pContex)
{
	UINT8 ret = FALSE;
	if((pContex->slaveRxMask & SlaveRxMask_RemoveWeight) != 0 )
	{

	}
	return ret;
}



UINT8 ModbusRtu_RxMainFunction(ModbusRtuType *pContex)
{
	UINT16 crc_ret = FALSE;
#if(TRUE == MODBUS_RTU_CRC_EN)
	UINT16 crc_data = 0 ;
#endif
	
	//rx data from bus
	if(TRUE == pContex->RxFinishFlag)
	{
		//1.clear flag
		pContex->RxFinishFlag = FALSE;
		//2.crc check
		#if(TRUE == MODBUS_RTU_CRC_EN)
			if((pContex->RxLength > 2) && (pContex->RxLength < MODBUS_RTU_UART_DATA_LEN))
			{
				crc_data = (pContex->rxData[pContex->RxLength-2]<<8)&0xff00;
				crc_data += pContex->rxData[pContex->RxLength-1];
				//
				if(crc_data == cal_crc16(pContex->rxData,(pContex->RxLength-2)))
				{
					crc_ret = TRUE;
				}
			}
		#else
			crc_ret = TRUE;
		#endif	
	}
	return crc_ret;
}
//==master mask scan
void ModbusRtu_MasterCycleReadWeightScan(ModbusRtuType *pContex)
{
	//cycle read
	//if(0 == (pContex->sysTick % MasterTimer_CycleRead_Weight_Float))
	//{
	//	pContex->masterTxMask |= MasterTxMask_CycleRead_Weight_Float;
	//}

	
	if(0 == (pContex->sysTick % MasterTimer_CycleRead_Weight_Float_AndSetHelpDataToSlave1))
	{
		pContex->masterTxMask |= MasterTxMask_CycleRead_Weight_Float_AndSetHelpDataToSlave1;
	}
}
//==master mask deal
UINT8 ModbusRtu_MasterMaskDeal(ModbusRtuType *pContex)
{
	UINT16 ret =FALSE;
#if(TRUE == MODBUS_RTU_CRC_EN)
	UINT16 crc_data = 0 ;
#endif
	if((pContex->masterTxMask & MasterTxMask_CycleRead_Weight_Float) != 0 )
	{
		//clear Mask
		pContex->masterTxMask &= (~MasterTxMask_CycleRead_Weight_Float);
		//read slave ID
		pContex->slaveID++;
		if(pContex->slaveID >= ModbusAdd_Slave_Max)
		{
			pContex->slaveID = ModbusAdd_Slave_1;
		}
		//
		pContex->txData[MODBUS_RTU_SEND_ADDRESS_POS] = ModbusAdd_Master;
		pContex->txData[MODBUS_RTU_ASK_ADDRESS_POS] = pContex->slaveID;
		pContex->txData[MODBUS_RTU_FUN_CODE_POS] = MFC_Mater_Ask_Weight_Float;
		pContex->txData[MODBUS_RTU_DATA_LEN_POS] = MODBUS_FUNCTION_CODE_MASTER_ASK1_DATA_LEN;
		pContex->txData[MODBUS_RTU_DATA_STAR_POS] = MFC_Mater_Ask_Weight_Float;
		pContex->needSendLen = MODBUS_RTU_DATA_STAR_POS+1;
		//	
		#if(TRUE == MODBUS_RTU_CRC_EN)
			crc_data = cal_crc16(pContex->txData,(pContex->needSendLen));
			pContex->txData[pContex->needSendLen++] = (crc_data>>8)&0xff;
			pContex->txData[pContex->needSendLen++] = (crc_data>>0)&0xff;
		#endif
		//
		ret = TRUE;
	}
	else if((pContex->masterTxMask & MasterTxMask_CycleRead_Weight_Float_AndSetHelpDataToSlave1) != 0 )
	{
		//clear Mask
		pContex->masterTxMask &= (~MasterTxMask_CycleRead_Weight_Float_AndSetHelpDataToSlave1);
		//read slave ID
		pContex->slaveID++;
		if(pContex->slaveID >= ModbusAdd_Slave_Max)
		{
			pContex->slaveID = ModbusAdd_Slave_1;
		}
		//
		pContex->txData[MODBUS_RTU_SEND_ADDRESS_POS] = ModbusAdd_Master;
		pContex->txData[MODBUS_RTU_ASK_ADDRESS_POS] = pContex->slaveID;
		pContex->txData[MODBUS_RTU_FUN_CODE_POS] = MFC_Mater_Ask_Weight_Float_WriteHelpData;
		pContex->txData[MODBUS_RTU_DATA_LEN_POS] = MODBUS_FUNCTION_CODE_MASTER_ASK1_DATA_LEN+2*DIFF_TO_DIWEN_DATA_LEN;
		pContex->txData[MODBUS_RTU_DATA_STAR_POS] = MFC_Mater_Ask_Weight_Float_WriteHelpData;
		readHelpDataFromSys(&pContex->txData[MODBUS_RTU_DATA_STAR_POS+1],DIFF_TO_DIWEN_DATA_LEN);
		pContex->needSendLen = MODBUS_RTU_DATA_STAR_POS+1+2*DIFF_TO_DIWEN_DATA_LEN;
		//	
#if(TRUE == MODBUS_RTU_CRC_EN)
			crc_data = cal_crc16(pContex->txData,(pContex->needSendLen));
			pContex->txData[pContex->needSendLen++] = (crc_data>>8)&0xff;
			pContex->txData[pContex->needSendLen++] = (crc_data>>0)&0xff;
#endif
		//
		ret = TRUE;
	}
	return ret;
}

//==master TX main function
void ModbusRtu_MasterTxMainFunction(ModbusRtuType *pContex)
{
	//make cycle read Mask 
	ModbusRtu_MasterCycleReadWeightScan(pContex);

	//masterState deal
	switch(pContex->masterState)
	{
		//master orderA and orderB need delay
		case MasterState_Idle:
			pContex->masterTxDiffTick++;
			if(pContex->masterTxDiffTick >= MasterTimer_TxOrderDlay)
			{
				pContex->masterState = MasterState_AllowTx;
			}
		break;
		//master allow to seed
		case MasterState_AllowTx:
			//Mask Deal
			if(TRUE == ModbusRtu_MasterMaskDeal(pContex))
			{
				pContex->pUartDevice->tx_bytes(pContex->pUartDevice,pContex->txData,pContex->needSendLen);
				//
				pContex->masterState = MasterState_WaitRx;
				pContex->masterMaxWaitRxTick = 0 ;
			}	
		break;
		//master wait RX
		case MasterState_WaitRx:
			if( pContex->masterMaxWaitRxTick++ >= MasterTimer_RxTimeOut)
			{
				pContex->masterState = MasterState_AllowTx;
			}
			//if recv slave answer , goto IDLE
			if(TRUE == pContex->RxFinishFlag)
			{
				pContex->masterState = MasterState_Idle;
			}
		break;
		default :
			pContex->masterState = MasterState_Idle;
		break;
	}
}
//==master RX main function
void ModbusRtu_MasterRxMainFunction(ModbusRtuType *pContex)
{
	UINT16 i = 0 , sender_add = 0 , recive_add = 0;

	//sender_ID , recv_ID
	sender_add = pContex->rxData[MODBUS_RTU_SEND_ADDRESS_POS];
	recive_add = pContex->rxData[MODBUS_RTU_ASK_ADDRESS_POS];
	//
	if((sender_add >= ModbusAdd_Slave_1) && (sender_add < ModbusAdd_Slave_Max) )
	{
		if((ModbusAdd_Master == recive_add) && (ModbusAdd_Master == gSystemPara.isCascade))
		{
			switch(pContex->rxData[MODBUS_RTU_FUN_CODE_POS])
			{
				case MFC_Slave_Ans_Weight_Float://master recv slave answer weight was float
					if(pContex->rxData[MODBUS_RTU_DATA_LEN_POS] == MODBUS_RTU_SLAVE_DATA_LEN)
					{
						//recv data
						for(i=0;i<MODBUS_RTU_SLAVE_CHANEL_NUM;i++)
						{
							pContex->MultWeightData[sender_add-ModbusAdd_Master][i].u_value[0] =  pContex->rxData[MODBUS_RTU_DATA_STAR_POS+4*i+0];
							pContex->MultWeightData[sender_add-ModbusAdd_Master][i].u_value[1] =  pContex->rxData[MODBUS_RTU_DATA_STAR_POS+4*i+1];
							pContex->MultWeightData[sender_add-ModbusAdd_Master][i].u_value[2] =  pContex->rxData[MODBUS_RTU_DATA_STAR_POS+4*i+2];
							pContex->MultWeightData[sender_add-ModbusAdd_Master][i].u_value[3] =  pContex->rxData[MODBUS_RTU_DATA_STAR_POS+4*i+3];
						}
					}
				break;
				case MFC_Slave_Ans_Weight_INT://master recv slave answer weight was int
					if(pContex->rxData[MODBUS_RTU_DATA_LEN_POS] == MODBUS_RTU_SLAVE_DATA_LEN)
					{
						//recv data
						for(i=0;i<MODBUS_RTU_SLAVE_CHANEL_NUM;i++)
						{
							pContex->MultWeightData[sender_add-ModbusAdd_Master][i].i_value = 0 ;
							pContex->MultWeightData[sender_add-ModbusAdd_Master][i].i_value += pContex->rxData[MODBUS_RTU_DATA_STAR_POS+4*i+0];
							pContex->MultWeightData[sender_add-ModbusAdd_Master][i].i_value <<= 8;
							pContex->MultWeightData[sender_add-ModbusAdd_Master][i].i_value += pContex->rxData[MODBUS_RTU_DATA_STAR_POS+4*i+1];
							pContex->MultWeightData[sender_add-ModbusAdd_Master][i].i_value <<= 8;
							pContex->MultWeightData[sender_add-ModbusAdd_Master][i].i_value += pContex->rxData[MODBUS_RTU_DATA_STAR_POS+4*i+2];
							pContex->MultWeightData[sender_add-ModbusAdd_Master][i].i_value <<= 8;
							pContex->MultWeightData[sender_add-ModbusAdd_Master][i].i_value += pContex->rxData[MODBUS_RTU_DATA_STAR_POS+4*i+3];
						}
					}
				break;
				default:
				break;
			}
		}
	}
}

//==slave rx main function
void ModbusRtu_SlaveRxMainFunction(ModbusRtuType *pContex)
{
	UINT16 i = 0 , sender_add = 0 , recive_add = 0;
#if(TRUE == MODBUS_RTU_CRC_EN)
	UINT16 crc_data = 0 ;
#endif
	//sender_ID , recv_ID
	sender_add = pContex->rxData[MODBUS_RTU_SEND_ADDRESS_POS];
	recive_add = pContex->rxData[MODBUS_RTU_ASK_ADDRESS_POS];
	//
	if(ModbusAdd_Master == sender_add)
	{
		if((recive_add >= ModbusAdd_Slave_1) && (recive_add < ModbusAdd_Slave_Max) && (recive_add == gSystemPara.isCascade))
		{
			switch(pContex->rxData[MODBUS_RTU_FUN_CODE_POS])
			{
				case MFC_Mater_Ask_Weight_Float://master ask slave chanel weight:float
					if(pContex->rxData[MODBUS_RTU_DATA_LEN_POS] == MODBUS_FUNCTION_CODE_MASTER_ASK1_DATA_LEN)
					{
						if(pContex->rxData[MODBUS_RTU_DATA_STAR_POS] == MFC_Mater_Ask_Weight_Float)
						{
							pContex->txData[MODBUS_RTU_SEND_ADDRESS_POS] = gSystemPara.isCascade;
							pContex->txData[MODBUS_RTU_ASK_ADDRESS_POS] = ModbusAdd_Master;
							pContex->txData[MODBUS_RTU_FUN_CODE_POS] = MFC_Slave_Ans_Weight_Float;
							pContex->txData[MODBUS_RTU_DATA_LEN_POS] = MODBUS_RTU_SLAVE_DATA_LEN;
							//===================push data to MultWeightData.....
							for(i=0;i<MODBUS_RTU_SLAVE_CHANEL_NUM;i++)
							{
								pContex->MultWeightData[recive_add-ModbusAdd_Master][i].f_value = hx711_getWeight((enumHX711ChanelType)i);
							}
							//===================push data to txData.....
							for(i=0;i<MODBUS_RTU_SLAVE_CHANEL_NUM;i++)
							{
								pContex->txData[MODBUS_RTU_DATA_STAR_POS+4*i+0] = pContex->MultWeightData[recive_add-ModbusAdd_Master][i].u_value[0];
								pContex->txData[MODBUS_RTU_DATA_STAR_POS+4*i+1] = pContex->MultWeightData[recive_add-ModbusAdd_Master][i].u_value[1];
								pContex->txData[MODBUS_RTU_DATA_STAR_POS+4*i+2] = pContex->MultWeightData[recive_add-ModbusAdd_Master][i].u_value[2];
								pContex->txData[MODBUS_RTU_DATA_STAR_POS+4*i+3] = pContex->MultWeightData[recive_add-ModbusAdd_Master][i].u_value[3];
							}
							pContex->needSendLen = MODBUS_RTU_DATA_STAR_POS+4*i;
							//crc
							#if(TRUE == MODBUS_RTU_CRC_EN)
								crc_data = cal_crc16(pContex->txData,(pContex->needSendLen));
								pContex->txData[pContex->needSendLen++] = (crc_data>>8)&0xff;
								pContex->txData[pContex->needSendLen++] = (crc_data>>0)&0xff;
							#endif
							//
							pContex->slaveTxMask |= SlaveTxMask_CycleRead_Weight_Float;
						}
					}
				break;
				case MFC_Mater_Ask_Weight_Float_WriteHelpData:
					if(pContex->rxData[MODBUS_RTU_DATA_LEN_POS] == (MODBUS_FUNCTION_CODE_MASTER_ASK1_DATA_LEN+2*DIFF_TO_DIWEN_DATA_LEN))
					{
						if(pContex->rxData[MODBUS_RTU_DATA_STAR_POS] == MFC_Mater_Ask_Weight_Float_WriteHelpData)
						{
							//rx data
							writeHelpDataFromCom(&pContex->rxData[MODBUS_RTU_DATA_STAR_POS+1],DIFF_TO_DIWEN_DATA_LEN);

							//tx data
							pContex->txData[MODBUS_RTU_SEND_ADDRESS_POS] = gSystemPara.isCascade;
							pContex->txData[MODBUS_RTU_ASK_ADDRESS_POS] = ModbusAdd_Master;
							pContex->txData[MODBUS_RTU_FUN_CODE_POS] = MFC_Slave_Ans_Weight_Float;
							pContex->txData[MODBUS_RTU_DATA_LEN_POS] = MODBUS_RTU_SLAVE_DATA_LEN;
							//===================push data to MultWeightData.....
							for(i=0;i<MODBUS_RTU_SLAVE_CHANEL_NUM;i++)
							{
								pContex->MultWeightData[recive_add-ModbusAdd_Master][i].f_value = hx711_getWeight((enumHX711ChanelType)i);
							}
							//===================push data to txData.....
							for(i=0;i<MODBUS_RTU_SLAVE_CHANEL_NUM;i++)
							{
								pContex->txData[MODBUS_RTU_DATA_STAR_POS+4*i+0] = pContex->MultWeightData[recive_add-ModbusAdd_Master][i].u_value[0];
								pContex->txData[MODBUS_RTU_DATA_STAR_POS+4*i+1] = pContex->MultWeightData[recive_add-ModbusAdd_Master][i].u_value[1];
								pContex->txData[MODBUS_RTU_DATA_STAR_POS+4*i+2] = pContex->MultWeightData[recive_add-ModbusAdd_Master][i].u_value[2];
								pContex->txData[MODBUS_RTU_DATA_STAR_POS+4*i+3] = pContex->MultWeightData[recive_add-ModbusAdd_Master][i].u_value[3];
							}
							pContex->needSendLen = MODBUS_RTU_DATA_STAR_POS+4*i;
							//crc
		#if(TRUE == MODBUS_RTU_CRC_EN)
								crc_data = cal_crc16(pContex->txData,(pContex->needSendLen));
								pContex->txData[pContex->needSendLen++] = (crc_data>>8)&0xff;
								pContex->txData[pContex->needSendLen++] = (crc_data>>0)&0xff;
		#endif
							//
							pContex->slaveTxMask |= SlaveTxMask_CycleRead_Weight_Float;
						}
					}
				break;
				case MFC_Mater_Ask_Weight_INT://master ask slave chanel weight:int32
					if(pContex->rxData[MODBUS_RTU_DATA_LEN_POS] == MODBUS_FUNCTION_CODE_MASTER_ASK1_DATA_LEN)
					{
						if(pContex->rxData[MODBUS_RTU_DATA_STAR_POS] == MFC_Mater_Ask_Weight_INT)
						{
							pContex->txData[MODBUS_RTU_SEND_ADDRESS_POS] = gSystemPara.isCascade;
							pContex->txData[MODBUS_RTU_ASK_ADDRESS_POS] = ModbusAdd_Master;
							pContex->txData[MODBUS_RTU_FUN_CODE_POS] = MFC_Slave_Ans_Weight_INT;
							pContex->txData[MODBUS_RTU_DATA_LEN_POS] = MODBUS_RTU_SLAVE_DATA_LEN;
							//===================push data to MultWeightData.....
							for(i=0;i<MODBUS_RTU_SLAVE_CHANEL_NUM;i++)
							{
								pContex->MultWeightData[recive_add-ModbusAdd_Master][i].f_value = hx711_getWeight((enumHX711ChanelType)i);
							}
							//===================push data to txData.....
							for(i=0;i<MODBUS_RTU_SLAVE_CHANEL_NUM;i++)
							{
								pContex->txData[MODBUS_RTU_DATA_STAR_POS+4*i+0] = (((INT32)pContex->MultWeightData[recive_add-ModbusAdd_Master][i].f_value)>>24)&0xff;
								pContex->txData[MODBUS_RTU_DATA_STAR_POS+4*i+1] = (((INT32)pContex->MultWeightData[recive_add-ModbusAdd_Master][i].f_value)>>16)&0xff;
								pContex->txData[MODBUS_RTU_DATA_STAR_POS+4*i+2] = (((INT32)pContex->MultWeightData[recive_add-ModbusAdd_Master][i].f_value)>>8)&0xff;
								pContex->txData[MODBUS_RTU_DATA_STAR_POS+4*i+3] = (((INT32)pContex->MultWeightData[recive_add-ModbusAdd_Master][i].f_value)>>0)&0xff;
							}
							pContex->needSendLen = MODBUS_RTU_DATA_STAR_POS+4*i;
							//crc
							#if(TRUE == MODBUS_RTU_CRC_EN)
								crc_data = cal_crc16(pContex->txData,(pContex->needSendLen));
								pContex->txData[pContex->needSendLen++] = (crc_data>>8)&0xff;
								pContex->txData[pContex->needSendLen++] = (crc_data>>0)&0xff;
							#endif
							//
							pContex->slaveTxMask |= SlaveTxMask_CycleRead_Weight_Int32;
						}
					}
				break;
				case MFC_Mater_Ask_Weight_Remove://remove weight
					pContex->slaveTxMask |= SlaveRxMask_RemoveWeight;
					pContex->txData[MODBUS_RTU_SEND_ADDRESS_POS] = gSystemPara.isCascade;
					pContex->txData[MODBUS_RTU_ASK_ADDRESS_POS] = ModbusAdd_Master;
					pContex->txData[MODBUS_RTU_FUN_CODE_POS] = MFC_Slave_Ans_Weight_INT;
					pContex->txData[MODBUS_RTU_DATA_LEN_POS] = 1;
					pContex->txData[MODBUS_RTU_DATA_STAR_POS] = MFC_Slave_Ans_Weight_Remove;
					pContex->needSendLen = MODBUS_RTU_DATA_STAR_POS+1;
					//crc
					#if(TRUE == MODBUS_RTU_CRC_EN)
						crc_data = cal_crc16(pContex->txData,(pContex->needSendLen));
						pContex->txData[pContex->needSendLen++] = (crc_data>>8)&0xff;
						pContex->txData[pContex->needSendLen++] = (crc_data>>0)&0xff;
					#endif
					//
					pContex->slaveTxMask |= SlaveTxMask_RemoveWeight;
					pContex->slaveRxMask |= SlaveRxMask_RemoveWeight;
				break;
				default:
				break;
			}
			//
		}
	}
}
//==slave tx mask deal function
UINT8 ModbusRtu_SlaveTxMaskDeal(ModbusRtuType *pContex)
{
	UINT8 ret = 0 ;
	if(pContex->slaveTxMask != SlaveTxMask_IDLE )
	{
		pContex->slaveTxMask = SlaveTxMask_IDLE;
		ret =TRUE;
	}
	return ret;
}
//==slave tx main function
void ModbusRtu_SlaveTxMainFunction(ModbusRtuType *pContex)
{		
	//slaveState deal
	switch(pContex->slaveState)
	{
		//slave orderA and orderB need delay
		case SlaveState_Idle:
			pContex->slaveTxDiffTick++;
			if(pContex->slaveTxDiffTick >= SlaveTimer_TxOrderDlay)
			{
				pContex->slaveState = SlaveState_AllowTx;
			}
		break;
		case SlaveState_AllowTx :
			if(TRUE ==ModbusRtu_SlaveTxMaskDeal(pContex))
			{
				pContex->pUartDevice->tx_bytes(pContex->pUartDevice,pContex->txData,pContex->needSendLen);
				pContex->slaveState = SlaveState_Idle;
			}
		break;
		default:
			pContex->slaveState = SlaveState_Idle;
		break;
	}
}

//==master main function
void ModbusRtu_MasterMainFunction(ModbusRtuType *pContex)
{
	ModbusRtu_MasterTxMainFunction(pContex);
	if(TRUE == ModbusRtu_RxMainFunction(pContex))
	{
		ModbusRtu_MasterRxMainFunction(pContex);
	}
}
//==slave main function
void ModbusRtu_SlaveMainFunction(ModbusRtuType *pContex)
{
	ModbusRtu_SlaveTxMainFunction(pContex);
	if(TRUE == ModbusRtu_RxMainFunction(pContex))
	{
		ModbusRtu_SlaveRxMainFunction(pContex);
	}
}
//==main function
void ModbusRtu_MainFunction(void)
{
	ModbusRtuType *pContex = &g_ModbusRtu;
	//
	pContex->sysTick++;
	//
	if(0 != gSystemPara.isCascade)//not cascade , only one
	{
		if(ModbusAdd_Master == gSystemPara.isCascade)//cascade : master Device
		{
			ModbusRtu_MasterMainFunction(pContex);
		}else if ((gSystemPara.isCascade >= ModbusAdd_Slave_1) && (gSystemPara.isCascade < ModbusAdd_Slave_Max))////cascade : slave Device
		{
			ModbusRtu_SlaveMainFunction(pContex);
		}
	}
}


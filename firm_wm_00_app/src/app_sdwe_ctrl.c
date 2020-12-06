#include "app_main_task.h"
#include "app_sdwe_ctrl.h"
#include "app_crc.h"
#include "app_hx711_ctrl.h"

SdweType g_sdwe = SdweDefault;
static INT32 defaultChanelSamplePoint[CHANEL_SECTION_NUM+1] = {0,50,100,200,500,1000,2000,3000,3500,4000,5000};

void sdwe_init(void)
{
	UINT8 i = 0 ;
	//
	g_sdwe.pUartDevice->pRxLength = &g_sdwe.RxLength;
	g_sdwe.pUartDevice->pRxFinishFlag = &g_sdwe.RxFinishFlag;
	g_sdwe.pUartDevice->pTxBuffer = &g_sdwe.rxData[0];
	g_sdwe.pUartDevice->pRxBuffer = &g_sdwe.rxData[0];
	//
	for(i=0;i<(CHANEL_SECTION_NUM+1);i++)
	{
		g_sdwe.sdweCalPointArry[i] = defaultChanelSamplePoint[i];
	}
	//
	g_sdwe.pUartDevice->init(g_sdwe.pUartDevice);
}


void sdweWrite(UINT16 address, UINT16 *pData ,UINT16 len ,UINT8 crcEn)
{
	UINT16 i = 0 ,l_data = 0 , total_len = 0 , crc = 0;
	if(address < 0x7ff)
	{
		if((len>0)&&(len<0x7f))
		{
			//head
			g_sdwe.txData[0]=0XA5;
			g_sdwe.txData[1]=0X5A;
			//data len
			g_sdwe.txData[2]=0X03+2*len;
			//order:write
			g_sdwe.txData[3]=0X82;
			//address
			g_sdwe.txData[4]=0xff&(address>>8);
			g_sdwe.txData[5]=0xff&(address>>0);
			//data
			for(i=0;i<len;i++)
			{
				l_data = *pData++;
				g_sdwe.txData[6+2*i+0] = 0xff&(l_data>>8);
				g_sdwe.txData[6+2*i+1] = 0xff&(l_data>>0);
			}
			//crc
			if(TRUE == crcEn)
			{
				crc = cal_crc16(&g_sdwe.txData[3],(3+2*len));
				g_sdwe.txData[6+2*i+0] = 0xff&(crc>>8);
				g_sdwe.txData[6+2*i+1] = 0xff&(crc>>0);
				//total len
				total_len = 6+2*len+2;
			}
			else
			{
				//total len
				total_len = 6+2*len;
			}
			//send
			g_sdwe.pUartDevice->tx_bytes(g_sdwe.pUartDevice,&g_sdwe.txData[0],total_len);
		}
	}

}




void sdwe_test(void)
{
	UINT8 i=0;
	static UINT8 status = 1 ,teat_data=0;
	static UINT16 wm_data[8];
	
	teat_data++;
	
	/*
	*/
	if(0==status)
	{
		//read verdion
		//0xA5 0x5A 0x03 0x81 0x03 0x02
		g_sdwe.txData[0]=0XA5;
		g_sdwe.txData[1]=0X5A;
		g_sdwe.txData[2]=0X03;
		g_sdwe.txData[3]=0X81;
		g_sdwe.txData[4]=0X03;
		g_sdwe.txData[5]=0X02;
		
		g_sdwe.pUartDevice->tx_bytes(g_sdwe.pUartDevice,&g_sdwe.txData[0],6);
	}
	else
	{
		wm_data[0]=teat_data;
		for(i=1;i<8;i++)
		{
			wm_data[i]=(i+1)*teat_data;
		}
		sdweWrite(0x160,&wm_data[0],8,0);
	}


}
void sdweRxDeal(void)
{
	UINT16 dataReg = 0 ;
	if(TRUE == g_sdwe.RxFinishFlag)
	{
		//A5 5A 06 83 01 FF 01 00 01
		if((SDWE_RX_FUN_HEAD1 == g_sdwe.rxData[0]) && (SDWE_RX_FUN_HEAD2 == g_sdwe.rxData[1]))
		{
			if((9 == g_sdwe.RxLength) && (6 == g_sdwe.rxData[2] ))
			{
				if(SDWE_RX_FUN_0X83 == g_sdwe.rxData[3])
				{
					//sdweSetAdd
					dataReg = 0 ;
					dataReg = g_sdwe.rxData[4];
					dataReg <<= 8;
					dataReg &= 0xff00;
					dataReg += g_sdwe.rxData[5];
					g_sdwe.sdweSetAdd = dataReg;
					//sdweSetData
					dataReg = 0 ;
					dataReg = g_sdwe.rxData[7];
					dataReg <<= 8;
					dataReg &= 0xff00;
					dataReg += g_sdwe.rxData[8];
					g_sdwe.sdweSetData = dataReg;
				}
			}
		}
		//
		g_sdwe.RxFinishFlag = FALSE;
	}
}
void app_uart_extern_msg_packet_process( UartDeviceType *pUartDevice )
{
	UINT8 X=0;
	X=X+1;
	sdweRxDeal();	
}

void sdwe_MainCalFunction(void)
{
	UINT8 i = 0 ;
	UINT8 point;
	INT32 weight;
	SdweType *pSdwe = &g_sdwe;
	switch(pSdwe->sdweSetAdd)
	{
		case SDWE_FUNC_CAL_CHANEL_ADD:
			if(pSdwe->sdweSetData <= HX711_CHANEL_NUM)
			{
				pSdwe->sdweCalChanel = pSdwe->sdweSetData;//chanel
			}
		break;
		
		case SDWE_FUNC_CAL_CHANEL_POINT_ADD://point
			if((pSdwe->sdweSetData > 0) && 
				(pSdwe->sdweSetData <= (CHANEL_SECTION_NUM)))
			{
				//point
				pSdwe->sdweCalPoint = pSdwe->sdweSetData;//point
				point = pSdwe->sdweCalPoint-1;
				//weight
				weight = pSdwe->sdweCalPointArry[point];
				//cal set
				if((pSdwe->sdweCalChanel <= HX711_CHANEL_NUM) && (pSdwe->sdweCalChanel >= 0))
				{
					if(0 == pSdwe->sdweCalChanel)//all chanel set
					{
						for(i=0;i<(HX711_CHANEL_NUM);i++)//8通道
						{
							sampleCalcKB(i,point,weight);
						}
					}
					else//single chanel set
					{
						sampleCalcKB((pSdwe->sdweCalChanel-1),point,weight);
					}
				}
			}
		break;
		default:
		break;
	}
	//clr address
	pSdwe->sdweSetAdd = 0xffff;
}


#define SDWE_WEIGHR_DATA_LEN 12	
UINT16 g_sdwe_dis_data[SDWE_WEIGHR_DATA_LEN]={0};

void sdwe_MainFunction(void)
{
	UINT16 *pSendData= &g_sdwe_dis_data[0] , i = 0 , j = 0 ,sameA = 5, sameB = 8;
	INT16 weight[HX711_CHANEL_NUM],weightTen[HX711_CHANEL_NUM];
	//
	for(i=0;i<HX711_CHANEL_NUM;i++)
	{
		weight[i] = (INT16)hx711_getWeight(i);
		weightTen[i] = (INT16)hx711_getWeightTen(i);
		//
		pSendData[i] = weightTen[i];
	}
	//
	pSendData[i++] = sameA;
	pSendData[i++] = weight[sameA];
	pSendData[i++] = sameB;
	pSendData[i++] = weight[sameB];
	
	//void sdweWrite(UINT16 address, UINT16 *pData ,UINT16 len ,UINT8 crcEn)
	sdweWrite(0X160,pSendData,i,0);	
}


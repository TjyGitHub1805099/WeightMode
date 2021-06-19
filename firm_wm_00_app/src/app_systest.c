#include "typedefine.h"
#include "app_systest.h"
#include "hal_gpio.h"
#include "app_led_ctrl.h"
#include "app_hx711_ctrl.h"
#include "app_main_task.h"
#include "app_key_ctrl.h"
#include "app_sdwe_ctrl.h"
#include "app_modbus_rtu_ctrl.h"

//============================================================================================
//========================================LED TEST START======================================
//============================================================================================
#if LED_CTRL_TEST//test
//==led test
void LedSysTest(UINT32 ms_tick)
{
	static UINT16 l_led_test_cycle = 1000;
	static enumLedColorType color = LED_COLOR_REG;
	enumLedSeqType seq = LED_SEQ_1;
	
	//every 1s change color:reg->yellow->blue->green
	if(0 == (ms_tick%l_led_test_cycle))
	{
		for(seq = LED_SEQ_1 ; seq < LED_SEQ_NUM ; seq++)
		{
			LedDataSet(seq,color);
		}
		
		color++;
		if( color >= LED_COLOR_NUM )
		{
			color = LED_COLOR_REG;
		}
	}
}
#endif
//============================================================================================
//========================================LED TEST END======================================
//============================================================================================

















//============================================================================================
//========================================T5L TEST START======================================
//============================================================================================
#if T5L_VOICE_PRITF
void T5L_VoicePritfTest(UINT32 sysTick)
{
	static tT5LVoinceType testVoiceId = VoiceTypeNum_1;
	if(1==(sysTick%4000))
	{
		sdwe_VoicePrintfPush(testVoiceId,(tT5LVoinceType)((testVoiceId+1)%VoiceTypeYu_13));
		{
			testVoiceId++;
			if(testVoiceId >= VoiceTypeYu_13)
			{
				testVoiceId = VoiceTypeNum_1;
			}
		}
	}
}
#endif

#if T5L_WEIGHT_COLOR_TEST
//==write varible data to SDWE thought UART
void sdweWriteVarible_dw(UINT16 varAdd, INT16 *pData ,UINT16 varlen ,UINT8 crcEn)
{
	//A5 5A 05 82 00 03 00 01:向0x0003地址写入数据0x0001
	UINT16 i = 0 ,l_data = 0 , total_len = 0 , crc = 0;
	if(varAdd < 0xFFFF)
	{
		if(((varAdd+varlen)>0)&&((varAdd+varlen)<0xFFFF))
		{
			//head
			g_T5L.txData[cmdPosHead1]=0x5A;
			g_T5L.txData[cmdPosHead2]=0XA5;
			//data len
			g_T5L.txData[cmdPosDataLen]=0X03+2*varlen;
			//order:write
			g_T5L.txData[cmdPosCommand]=cmdWriteSWDEVariable;
			//varAdd
			g_T5L.txData[cmdPosVarWriteAddress1]=0xff&(varAdd>>8);
			g_T5L.txData[cmdPosVarWriteAddress2]=0xff&(varAdd>>0);
			//data
			for(i=0;i<varlen;i++)
			{
				l_data = *pData++;
				g_T5L.txData[cmdPosVarWriteData+2*i+0] = 0xff&(l_data>>8);
				g_T5L.txData[cmdPosVarWriteData+2*i+1] = 0xff&(l_data>>0);
			}
			//crc
			if(TRUE == crcEn)
			{
				crc = cal_crc16(&g_T5L.txData[cmdPosCommand],(3+2*varlen));
				g_T5L.txData[cmdPosVarWriteData+2*varlen+0] = 0xff&(crc>>8);
				g_T5L.txData[cmdPosVarWriteData+2*varlen+1] = 0xff&(crc>>0);
				//total len
				total_len = cmdPosVarWriteData+2*varlen+2;
			}
			else
			{
				//total len
				total_len = cmdPosVarWriteData+2*varlen;
			}
			//send
			g_T5L.pUartDevice->tx_bytes(g_T5L.pUartDevice,&g_T5L.txData[0],total_len);
			//
			hal_delay_ms(1);
		}
	}
}

void sdwe_MainFunctionTest(void)
{
	//测试流程
	//1.chanelTestEn=1
	//0x2200后面10个地址为，校验模式，参考点 设定值
	//0x2300后面10个地址为，校验模式，参考点 校验状态设定值
	//0x2400后面10个地址为，校验模式，参考点 实际采样值
	
	//0x3000后面12个地址为，配平模式，各通道采样转换后的值
	//0x3100后面12个地址为，配平模式，各通道背景色值
	static UINT8 chanelTestEn= 0 ;
	static UINT32 cnt = 0 ;
	UINT8 i = 0 ;
	
	static UINT8 chanelDataEn= 0 ;
	static INT16 chanelDataAddress = 0x3000 ;//转换后数据
	static INT16 chanelDataTest[32] ={0};
	
	static UINT8 chanelColorEn = 0 ;
	static INT16 chanelColorAddress = 0x3100 ;//背景色
	static INT16 chanelColorTest[32] ={0};
	
	static INT16 chanelDataLen = 6;
	
	cnt++;
	if((1 == chanelTestEn) && 0 == (cnt%1000))
	{
		if(1 == chanelDataEn)
		{
			for(i=0;i<chanelDataLen;i++)
			{
				chanelDataTest[i]=cnt;
			}
			sdweWriteVarible_dw(chanelDataAddress,&chanelDataTest[0],chanelDataLen,0);
		}
		else if(1 == chanelColorEn)
		{
			for(i=0;i<chanelDataLen;i++)
			{
				switch(chanelColorTest[i])
				{
					case 0:chanelColorTest[i]=1;break;
					case 1:chanelColorTest[i]=2;break;
					case 2:chanelColorTest[i]=3;break;
					case 3:chanelColorTest[i]=4;break;
					case 4:chanelColorTest[i]=0;break;
					default:chanelColorTest[i]=0;break;
				}
			}
			sdweWriteVarible_dw(chanelColorAddress,&chanelColorTest[0],chanelDataLen,0);
		}
	}
}
#endif

//============================================================================================
//========================================T5L TEST END======================================
//============================================================================================


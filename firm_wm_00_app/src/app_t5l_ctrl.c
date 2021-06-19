/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "drv_flash.h"
#include "app_main_task.h"
#include "app_sdwe_ctrl.h"
#include "app_crc.h"
#include "app_hx711_ctrl.h"
#include "app_crc.h"
#include "hal_delay.h"
#include "app_modbus_rtu_ctrl.h"
#include "app_syspara.h"
#include "app_password.h"
#include "app_t5l_ctrl.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
T5LType g_T5L = T5LDataDefault;
//sdwe 10 point triger color data:arr0 is triger flag ,arr1 is color ,arr2 is sample data
static INT16 g_sdwe_triger_data[4][CHANEL_POINT_NUM]={{0},{0},{0}};
//sdwe 8 weight data + 8 color data	
INT16 g_t5l_dis_data[T5L_WEIGHT_DATA_LEN]={0};
INT16 g_t5l_dis_data_buff[T5L_WEIGHT_DATA_LEN]={0};

//1.chanel num :0~x HX711_CHANEL_NUM
//2.trigerStarus , back color , point avg Sample , point set weight
//3.point num
static INT16 g_t5l_triger_data[HX711_CHANEL_NUM+1][DMG_TRIGER_SAMPLE_MAX_NUM][CHANEL_POINT_NUM];

//voice printf buff
tT5LVoinceType g_T5L_VoiceBuff[T5L_VOICE_MAX_PRINTF_NUM][3];
UINT8 u8T5LVoiceBuffPush_i = 0 ,u8T5LVoiceBuffPop_i = 0;


/*******************************************************************************
 * Functions
 ******************************************************************************/
//==sdwe->mcu rx callback ,not used
void app_uart_extern_msg_packet_process( UartDeviceType *pUartDevice )
{
	//not used
}

//==sdwe initial
void screenT5L_Init(void)
{
	UINT8 i = 0 ;
	//
	g_T5L.readSdweInit = FALSE;
	g_T5L.pUartDevice = &g_UartDevice[UART_EXTERN];
	g_T5L.version = 0;//SDWE version
	g_T5L.allowCompare = FALSE;
	//
	g_T5L.pUartDevice->pRxLength = &g_T5L.RxLength;
	g_T5L.pUartDevice->pRxFinishFlag = &g_T5L.RxFinishFlag;
	g_T5L.pUartDevice->pTxBuffer = &g_T5L.rxData[0];
	g_T5L.pUartDevice->pRxBuffer = &g_T5L.rxData[0];
	//
	//
	g_T5L.RxLength = 0;					/**< 接收字节数 */
	g_T5L.RxFinishFlag = FALSE;			/**< 接收完成标志 */
	//
	g_T5L.SetAdd = 0XFFFF;/**< 地址 */
	g_T5L.DataLen = 0;/**< 数据长度 */
	g_T5L.SetData = 0;/**< 数据 */

	g_T5L.ColorClen=FALSE;/**< 通道切换SDWE颜色清除 */
	g_T5L.CalibrateChanel=88;/**< 通道 */
	g_T5L.CalibratePoint=0;/**< 校准点 */


	g_T5L.ResetTrigerValid = FALSE;
	//
	for(i=0;i<CHANEL_POINT_NUM;i++)
	{
		g_T5L.CalibratePointArry[i] = defaultChanelSamplePoint[i];/**< 校准点数组 */
	}
	//
	g_T5L.pUartDevice->init(g_T5L.pUartDevice);
}
//==read register data from SDWE thought UART
void sdweReadRegister(UINT8 regAdd ,UINT8 regLen ,UINT8 crcEn)
{
	//A5 5A 03 81 03 02:读取03和04号寄存器数据
	//A5 5A (03) 81 XX len
	UINT16 total_len = 0 , crc = 0 ;
	
	if(regAdd < 0xFF)
	{
		if(((regAdd+regLen)>0)&&((regAdd+regLen)<0xFf))
		{
			//head
			g_T5L.txData[cmdPosHead1]=T5L_RX_FUN_HEAD1;
			g_T5L.txData[cmdPosHead2]=T5L_RX_FUN_HEAD2;
			//data len
			g_T5L.txData[cmdPosDataLen]=0X03;
			//order:write register
			g_T5L.txData[cmdPosCommand]=cmdReadSWDERegister;
			//address
			g_T5L.txData[cmdPosRegReadAddress]=regAdd;
			//len
			g_T5L.txData[cmdPosRegReadLen]=regLen;
			//crc
			if(TRUE == crcEn)
			{
				crc = cal_crc16(&g_T5L.txData[cmdPosCommand],(3));
				g_T5L.txData[cmdPosRegReadLen+1] = 0xff&(crc>>8);
				g_T5L.txData[cmdPosRegReadLen+2] = 0xff&(crc>>0);
				//total len
				total_len =cmdPosRegReadLen+3;
			}
			else
			{
				//total len
				total_len = cmdPosRegReadLen+1;
			}
			//send
			g_T5L.pUartDevice->tx_bytes(g_T5L.pUartDevice,&g_T5L.txData[0],total_len);
			//
			//hal_delay_ms(5);
		}
	}
}
//==write register data to SDWE thought UART
void sdweWriteRegister(UINT8 regAdd, UINT8 *pData ,UINT8 regLen ,UINT8 crcEn)
{
	//A5 5A 04 80 03 00 01:向03和04号寄存器写入数据00和01
	//A5 5A (02+n*DD) 80 XX n*DD
	UINT8 reg_i = 0 ;
	UINT16 total_len = 0 , crc = 0 ;
	if(regAdd < 0xFF)
	{
		if(((regAdd+regLen)>0)&&((regAdd+regLen)<0xFF))
		{
			//head
			g_T5L.txData[cmdPosHead1]=T5L_RX_FUN_HEAD1;
			g_T5L.txData[cmdPosHead2]=T5L_RX_FUN_HEAD2;
			//data len
			g_T5L.txData[cmdPosDataLen]=0X02+regLen;
			//order:write register
			g_T5L.txData[cmdPosCommand]=cmdWriteSWDERegister;
			//address
			g_T5L.txData[cmdPosRegWriteAddress]=regAdd;
			//data
			for(reg_i = 0 ; reg_i < regLen ; reg_i++)
			{
				g_T5L.txData[cmdPosRegWritesData+reg_i] = *(pData+reg_i);
			}
			//crc
			if(TRUE == crcEn)
			{
				crc = cal_crc16(&g_T5L.txData[cmdPosCommand],(2+1*regLen));
				g_T5L.txData[cmdPosRegWritesData+regLen+0] = 0xff&(crc>>8);
				g_T5L.txData[cmdPosRegWritesData+regLen+1] = 0xff&(crc>>0);
				//total len
				total_len = cmdPosRegWritesData+regLen+2;
			}
			else
			{
				//total len
				total_len = cmdPosRegWritesData+regLen;
			}
			//send
			g_T5L.pUartDevice->tx_bytes(g_T5L.pUartDevice,&g_T5L.txData[0],total_len);
			//
			hal_delay_ms(1);
		}
	}
}
//==read varible data from SDWE thought UART
void sdweReadVarible(UINT16 varAdd ,UINT16 varlen ,UINT8 crcEn)
{
	//A5 5A 04 83 00 03 02:读取0x0003和0x0004两个寄存器
	//A5 5A (04) 83 XX XX len
	UINT16 total_len = 0 , crc = 0;
	if(varAdd < 0xFFFF)
	{
		if(((varAdd+varlen)>0)&&((varAdd+varlen)<0xFFFF))
		{
			//head
			g_T5L.txData[cmdPosHead1]=T5L_RX_FUN_HEAD1;
			g_T5L.txData[cmdPosHead2]=T5L_RX_FUN_HEAD2;
			//data len
			g_T5L.txData[cmdPosDataLen]=0X04;
			//order:write
			g_T5L.txData[cmdPosCommand]=cmdReadSWDEVariable;
			//varAdd
			g_T5L.txData[cmdPosVarReadAddress1]=0xff&(varAdd>>8);
			g_T5L.txData[cmdPosVarReadAddress2]=0xff&(varAdd>>0);
			//len
			g_T5L.txData[cmdPosVarReadLen]=0xff&(varlen>>0);
			//crc
			if(TRUE == crcEn)
			{
				crc = cal_crc16(&g_T5L.txData[cmdPosCommand],(4));
				g_T5L.txData[cmdPosVarReadLen+1] = 0xff&(crc>>8);
				g_T5L.txData[cmdPosVarReadLen+2] = 0xff&(crc>>0);
				//total len
				total_len = cmdPosVarReadLen+3;
			}
			else
			{
				//total len
				total_len = cmdPosVarReadLen+1;
			}
			//send
			g_T5L.pUartDevice->tx_bytes(g_T5L.pUartDevice,&g_T5L.txData[0],total_len);
			//
			hal_delay_ms(1);
		}
	}
}












//========================================================================================check:20210619
//==write varible data to SDWE thought UART
void t5lWriteVarible(UINT16 varAdd, INT16 *pData ,UINT16 varlen ,UINT8 crcEn)
{
	//A5 5A 05 82 00 03 00 01:向0x0003地址写入数据0x0001
	UINT16 i = 0 ,l_data = 0 , total_len = 0 , crc = 0;
	if(varAdd < 0xFFFF)
	{
		if(((varAdd+varlen)>0)&&((varAdd+varlen)<0xFFFF))
		{
			//head
			g_T5L.txData[cmdPosHead1]=T5L_RX_FUN_HEAD1;
			g_T5L.txData[cmdPosHead2]=T5L_RX_FUN_HEAD2;
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
			g_T5L.LastSendTick = g_T5L.CurTick;
			//
			hal_delay_ms(1);
		}
	}
}

//==write data to screen , have delay contrl
UINT8 t5lWriteData(UINT16 varAdd, INT16 *pData ,UINT16 varlen ,UINT8 crcEn)
{
	UINT8 ret = FALSE;
	//A5 5A 05 82 00 03 00 01:向0x0003地址写入数据0x0001
	UINT16 i = 0 ,l_data = 0 , total_len = 0 , crc = 0;
	if(((g_T5L.LastSendTick > g_T5L.CurTick)&&((g_T5L.LastSendTick-g_T5L.CurTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER))||
		((g_T5L.LastSendTick < g_T5L.CurTick)&&((g_T5L.CurTick - g_T5L.LastSendTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER)))
	{
		if(varAdd < 0xFFFF)
		{
			if(((varAdd+varlen)>0)&&((varAdd+varlen)<0xFFFF))
			{
				//head
				g_T5L.txData[cmdPosHead1]=T5L_RX_FUN_HEAD1;
				g_T5L.txData[cmdPosHead2]=T5L_RX_FUN_HEAD2;
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
				g_T5L.LastSendTick = g_T5L.CurTick;
				//
				ret = TRUE;
			}
		}
	}
	return ret;
}

//if screen at calibration page point trigerd
void pointWeightTrigerDataSet(UINT8 localChanel , UINT8 point , INT16 value)
{
	if(localChanel > HX711_CHANEL_NUM)
		return ;

	if(point < CHANEL_POINT_NUM)
	{
		g_t5l_triger_data[localChanel][DMG_TRIGER_SAMPLE_OF_ASK_WEIGHT][point] = value;//point weight triger	
	}
}
//if screen at calibration page point trigerd
void pointSampleTrigerDataSet(UINT8 localChanel , UINT8 point , INT16 value)
{
	if(localChanel > HX711_CHANEL_NUM)
		return ;

	if(point < CHANEL_POINT_NUM)
	{
		g_t5l_triger_data[localChanel][DMG_TRIGER_SAMPLE_OF_AVG_SAMPLE][point] = value;//point sample triger	
	}
}

//if screen at calibration page point trigerd
void pointTrigerDataSet(UINT8 localChanel , UINT8 point , UINT8 value ,INT16 avgSampleValue)
{	
	if(localChanel > HX711_CHANEL_NUM)
		return ;

	if(point < CHANEL_POINT_NUM)
	{
		g_t5l_triger_data[localChanel][DMG_TRIGER_SAMPLE_OF_STATUS][point] = TRUE;//point triger need answer flag	
		g_t5l_triger_data[localChanel][DMG_TRIGER_SAMPLE_OF_ASK_COLOR][point] = value;//point triger color answer	
		g_t5l_triger_data[localChanel][DMG_TRIGER_SAMPLE_OF_AVG_SAMPLE][point] = avgSampleValue;//point triger avg sample value answer
	}
}

//if sreen calibtion point triger
UINT8 pointTrigerDeal()
{
	static UINT8 inerStatus = 0 , localChanel = 0 ;	
	INT16 *pSendData= 0;
	UINT8 result = 0 ;

	if(g_T5L.CalibrateChanel > HX711_CHANEL_NUM)
		return 0 ;

	//chanel get
	if(0 == g_T5L.CalibrateChanel)
	{
		localChanel = HX711_CHANEL_NUM ;
	}
	else if(g_T5L.CalibrateChanel <= HX711_CHANEL_NUM)
	{
		localChanel = g_T5L.CalibrateChanel - 1 ;
	}

	//status
	switch(inerStatus)
	{
		case 0://send Color
			if(((g_T5L.LastSendTick > g_T5L.CurTick)&&((g_T5L.LastSendTick-g_T5L.CurTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER))||
				((g_T5L.LastSendTick < g_T5L.CurTick)&&((g_T5L.CurTick - g_T5L.LastSendTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER)))
			{
				pSendData= &g_t5l_triger_data[localChanel][DMG_TRIGER_SAMPLE_OF_ASK_COLOR][0];//color:1 green 0:white
				t5lWriteVarible(DMG_FUNC_ASK_CHANEL_POINT_TRIG_BACK_COLOR_ADDRESS,pSendData,(CHANEL_POINT_NUM),0);
				//
				inerStatus++ ;
			}
		break;
		case 1://send sample data
			{
				if(((g_T5L.LastSendTick > g_T5L.CurTick)&&((g_T5L.LastSendTick-g_T5L.CurTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER))||
					((g_T5L.LastSendTick < g_T5L.CurTick)&&((g_T5L.CurTick - g_T5L.LastSendTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER)))
				{
					pSendData= &g_t5l_triger_data[localChanel][DMG_TRIGER_SAMPLE_OF_AVG_SAMPLE][0];//data
					t5lWriteVarible(DMG_FUNC_ASK_CHANEL_POINT_TRIG_SAMPLE_DATA_ADDRESS,pSendData,(CHANEL_POINT_NUM),0);
					//
					inerStatus++ ;
				}
			}
			break;
		default:
			inerStatus = 0 ;
			result = 1 ;
			break;
	}

	return result;
}


//==read SDWE version
void sdweReadVersion()
{
	sdweReadRegister(0x00 ,1 ,FALSE);
	//at sdwe answer
	//g_T5L.version = version;
}
//==when sdwe point triger,update g_sdwe_triger_data[]
void sdwePointTrigerUpdata(UINT8 point , UINT8 value ,INT16 avgSampleValue)
{
	if(point < CHANEL_POINT_NUM)
	{
		g_sdwe_triger_data[0][point] = TRUE;//point triger need answer flag	
		g_sdwe_triger_data[1][point] = value;//point triger color answer
		g_sdwe_triger_data[2][point] = avgSampleValue;//point triger avg sample value answer	
	}
}

//==get if not sdwe point triger
UINT8 getSdwePointTriger()
{
	UINT8 ret = 0 , i = 0 ;
	
	for(i=0;i<CHANEL_POINT_NUM;i++)
	{
		ret = g_sdwe_triger_data[0][i];//point triger need answer flag	
		if(1 == ret)
		{
			break;
		}
	}
	return ret;
}
//==sdwe triger point changed ,clear point triger
void clrSdwePointTriger()
{
	UINT8 i = 0 ;
	for(i=0;i<CHANEL_POINT_NUM;i++)
	{
		g_sdwe_triger_data[0][i] = FALSE ;//clr point triger need answer flag	
	}
}
//==sdwe triger chanel changed ,clear all
void clrSdwePointTrigerAll()
{
	UINT8 i = 0 ;
	for(i=0;i<CHANEL_POINT_NUM;i++)
	{
		g_sdwe_triger_data[0][i] = 0 ;//clr triger flag	
		g_sdwe_triger_data[1][i] = 0 ;//clr color data	
		g_sdwe_triger_data[2][i] = 0 ;//clr avg sample data	
	}
}









//==updata sdwe weight color
void sdweSetWeightBackColor(UINT8 seq,UINT8 color)
{
	if(seq < HX711_CHANEL_NUM)
	{
		//0~HX711_CHANEL_NUM:weight
		//HX711_CHANEL_NUM~2*HX711_CHANEL_NUM:color
		g_t5l_dis_data[HX711_CHANEL_NUM+seq] = color;
	}
}
void color_clearAllColor(void)
{
	UINT8 seq = HX711Chanel_1;
	for(seq=HX711Chanel_1;seq<HX711_CHANEL_NUM;seq++)
	{
		g_t5l_dis_data[HX711_CHANEL_NUM+seq] = LED_COLOR_NONE;
	}
}
//==recv sdwe register ask deal
UINT8 sdweAskRegData(UINT8 regAdd, UINT8 regData)
{
	UINT8 needStore = FALSE ;
	T5LType *pSdwe = &g_T5L;
	if(0 == regAdd)
	{
		pSdwe->version = regData;
		pSdwe->readSdweInit = TRUE;
	}
	return needStore;
}
//clear data to screen at calibration page
void clearLocalCalibrationRecordData(UINT8 sreen_chanel)
{
	UINT8 chane_i = 0 , point_j = 0 ;
	
	if(0 == sreen_chanel)//clear all
	{
		for(chane_i=0;chane_i<(HX711_CHANEL_NUM+1);chane_i++)
		{
			//==1:clear sreen needs back color and sample data
			for(point_j=0;point_j<CHANEL_POINT_NUM;point_j++)
			{
				//back color
				g_t5l_triger_data[chane_i][DMG_TRIGER_SAMPLE_OF_ASK_COLOR][point_j] = 0 ;//color:1 green 0:white
				//data
				g_t5l_triger_data[chane_i][DMG_TRIGER_SAMPLE_OF_AVG_SAMPLE][point_j] = 0 ;//sample data = 0
			}
		}
	}
	else if(sreen_chanel <= HX711_CHANEL_NUM)
	{
		chane_i = sreen_chanel - 1 ;
		//==1:clear sreen needs back color and sample data
		for(point_j=0;point_j<CHANEL_POINT_NUM;point_j++)
		{
			//back color
			g_t5l_triger_data[chane_i][DMG_TRIGER_SAMPLE_OF_ASK_COLOR][point_j] = 0 ;//color:1 green 0:white
			//data
			g_t5l_triger_data[chane_i][DMG_TRIGER_SAMPLE_OF_AVG_SAMPLE][point_j] = 0 ;//sample data = 0
		}
	}
}

//clear local recode k and b and sample data
void clearLocalCalibrationKAndBAndSample(UINT8 sreen_chanel)
{
	UINT8 chane_i = 0 , point_j = 0 ;
	ChanelType *pChanel=0;
	
	if(0 == sreen_chanel)//clear all
	{
		for(chane_i=0;chane_i<HX711_CHANEL_NUM;chane_i++)
		{
			//==1:clear local sample data and k and b
			//get chanel
			pChanel = getChanelStruct(chane_i);
			//clear local sample point
			for(point_j=0;point_j<CHANEL_POINT_NUM;point_j++)
			{
				pChanel->section_PointSample[point_j] = 0 ;
			}
			//clear local k & b
			for(point_j=0;point_j<(CHANEL_POINT_NUM+1);point_j++)
			{
				pChanel->section_K[point_j] = 0.0 ;
				pChanel->section_B[point_j] = 0.0 ;
			}
		}
	}
	else if(sreen_chanel <= HX711_CHANEL_NUM)
	{
		chane_i = sreen_chanel - 1 ;

		//==1:clear local sample data and k and b
		//get chanel
		pChanel = getChanelStruct(chane_i);
		//clear local sample point
		for(point_j=0;point_j<CHANEL_POINT_NUM;point_j++)
		{
			pChanel->section_PointSample[point_j] = 0 ;
		}
		//clear local k & b
		for(point_j=0;point_j<(CHANEL_POINT_NUM+1);point_j++)
		{
			pChanel->section_K[point_j] = 0.0 ;
			pChanel->section_B[point_j] = 0.0 ;
		}
	}	
}

void storeSysPara_3030(UINT16 varAdd, UINT16 varData)
{
	switch(varAdd)
	{
		case DMG_FUNC_SET_UNIT_ADDRESS://		(0X1000)//0x1000
			gSystemPara.uint = varData;
		break;
		case DMG_FUNC_SET_MIN_RANGE_ADDRESS://		(0X100A)//0x100A
		break;
		case  DMG_FUNC_SET_MAX_RANGE_ADDRESS://		(0X100B)//0x100B
		break;
		case DMG_FUNC_SET_ERR_RANGE_ADDRESS://		(0X100C)//0x100C
		break;
		case  DMG_FUNC_SET_isCascade_ADDRESS://		(0X100D)//0x100D
		break;
		case  DMG_FUNC_SET_isLedIndicate_ADDRESS://	(0X100E)//0x100E
		break;
		case  DMG_FUNC_SET_COLOR_START_ADDRESS://	(0X100F)//0x100F
		break;
		case DMG_FUNC_SET_COLOR_END_ADDRESS://		(0X1012)//0x1012
		break;
		case DMG_FUNC_SET_ZERO_RANGE_ADDRESS://		(0X1013)//0x1013
		break;
		default:
		break;
	}

}

//==recv sdwe variable ask deal
UINT8 sdweAskVaribleData(UINT16 varAdd, UINT16 varData)
{
	UINT8 needStore = FALSE ;
	UINT8 i = 0 , point = 0;
	INT32 weight=0,avgSampleValue=0;
	T5LType *pSdwe = &g_T5L;
	//
	pSdwe->SetAdd = varAdd ;
	pSdwe->SetData = varData ;
	//receive address from SDWE
	if(0xffff != pSdwe->SetAdd)
	{
		//==(update:20210411):sys para
		if(((pSdwe->SetAdd >= DMG_FUNC_SET_UNIT_ADDRESS)&&(pSdwe->SetAdd <= (DMG_FUNC_SET_ZERO_RANGE_ADDRESS)))
			|| (pSdwe->SetAdd == DMG_FUNC_PASSORD_SET_ADDRESS))
		{	
			switch(pSdwe->SetAdd)
			{
				case DMG_FUNC_PASSORD_SET_ADDRESS:
					STM32CheckPassWord(pSdwe->SetData);/**< 密码 */
				break;
				case DMG_FUNC_SET_UNIT_ADDRESS://			(0X1000)//0x1000
					gSystemPara.uint = pSdwe->SetData;/**< 单位 */
				break;
				case DMG_FUNC_SET_MIN_RANGE_ADDRESS://		(0X100A)//0x100A
					gSystemPara.minWeight = pSdwe->SetData;/**< 最小量程 */
				break;
				case  DMG_FUNC_SET_MAX_RANGE_ADDRESS://		(0X100B)//0x100B
					gSystemPara.maxWeight = pSdwe->SetData;/**< 最大量程 */
				break;
				case  DMG_FUNC_SET_ERR_RANGE_ADDRESS://		(0X100C)//0x100C
					gSystemPara.errRange = pSdwe->SetData;/**< 误差范围 */
				break;
				case  DMG_FUNC_SET_isCascade_ADDRESS://		(0X100D)//0x100D
					gSystemPara.isCascade = pSdwe->SetData;/**< 是否级联 */
				break;
				case  DMG_FUNC_SET_isLedIndicate_ADDRESS://	(0X100E)//0x100E
					gSystemPara.isLedIndicate = pSdwe->SetData;/**< 是否LED指示 */
				break;
				case  DMG_FUNC_SET_ZERO_RANGE_ADDRESS://		(0X1013)//0x1013
					gSystemPara.zeroRange = pSdwe->SetData;/**< 零点范围 */
				break;
				default:
					if((pSdwe->SetAdd >= DMG_FUNC_SET_COLOR_START_ADDRESS)&&(pSdwe->SetAdd <= (DMG_FUNC_SET_COLOR_END_ADDRESS)))
					{
						gSystemPara.userColorSet[pSdwe->SetAdd-DMG_FUNC_SET_COLOR_START_ADDRESS] = pSdwe->SetData;/**< 颜色1~4 */
					}
				break;
			}
			//
			needStore |= DMG_TRIGER_SAVE_SECOTOR_2 ;
		}
		//==(update:20210515):Balancing JUMP CLEAN PAGE
		else if(DMG_FUNC_Balancing_CLEARPAGE_SET_ADDRESS == pSdwe->SetAdd)
		{
			if(DMG_FUNC_Balancing_CLEARPAGE_SET_VALUE == (UINT16)pSdwe->SetData)
			{
				pSdwe->sdweJumpBalancing_cleanpagee = TRUE;
			}
		}
		//==(update:20210515):Balancing JUMP
		else if(DMG_FUNC_Balancing_HOME_SET_ADDRESS == pSdwe->SetAdd)
		{
			if(DMG_FUNC_Balancing_HOME_SET_VALUE == (UINT16)pSdwe->SetData)
			{
				pSdwe->sdweJumpBalancing_home = TRUE;
			}
		}
		//==(update:20210515):Balancing JUMP
		else if(DMG_FUNC_Balancing_SET_ADDRESS == pSdwe->SetAdd)
		{
			if(DMG_FUNC_Balancing_SET_VALUE == (UINT16)pSdwe->SetData)
			{
				pSdwe->sdweJumpBalancing = TRUE;
			}
		}
		//==(update:20210328):chanel choice:0->all chanel , 1~8:single chanel
		else if(DMG_FUNC_SET_CHANEL_NUM == pSdwe->SetAdd)
		{
			pSdwe->ResetTrigerValid = FALSE;/*重新校准取消*/
			if(pSdwe->CalibrateChanel != pSdwe->SetData)
			{
				pSdwe->sdweChanelChanged = TRUE;
				if(pSdwe->SetData <= HX711_CHANEL_NUM)
				{
					pSdwe->CalibrateChanel = pSdwe->SetData;//chanel
				}
			}
		}//==(update:20210328):reset calibration
		else if(DMG_FUNC_RESET_CALIBRATION_ADDRESS == pSdwe->SetAdd)
		{
			if(DMG_FUNC_RESET_CALIBRATION_VAL == (UINT16)pSdwe->SetData)
			{
				pSdwe->sdweResetTriger = TRUE;
				pSdwe->ResetTrigerValid = TRUE;
				clearLocalCalibrationRecordData(pSdwe->CalibrateChanel);
				clearLocalCalibrationKAndBAndSample(pSdwe->CalibrateChanel);
			}
		}//==(update:20210428):reset calibration
		else if(DMG_FUNC_JUNPTO_CALIBRATION_ADDRESS == pSdwe->SetAdd)
		{
			if(DMG_FUNC_JUNPTO_CALIBRATION_VAL == (UINT16)pSdwe->SetData)
			{
				pSdwe->sdweJumpToCalitrationPage = TRUE;
			}
			else if(DMG_FUNC_JUNPTO_ACTIVE_VAL == (UINT16)pSdwe->SetData)
			{
				pSdwe->sdweJumpActivePage = TRUE;
			}
		}//==(update:20210328):remove all weight value
		else if(DMG_FUNC_REMOVE_WEIGHT_ADDRESS == pSdwe->SetAdd)
		{
			if(DMG_FUNC_REMOVE_WEIGHT_VAL == (UINT16)pSdwe->SetData)
			{
				hx711_setAllRemoveWeight();
				pSdwe->sdweRemoveWeightTriger = TRUE;
			}
		}//==(update:20210328):chanel point weight value set
		else if((pSdwe->SetAdd >= DMG_FUNC_SET_CHANEL_POINT_ADDRESS)&&(pSdwe->SetAdd < (DMG_FUNC_SET_CHANEL_POINT_ADDRESS + CHANEL_POINT_NUM )))
		{
			needStore = DMG_TRIGER_SAVE_SECOTOR_1 ;
			//point
			pSdwe->CalibratePoint = (pSdwe->SetAdd -DMG_FUNC_SET_CHANEL_POINT_ADDRESS) ;//point
			point = pSdwe->CalibratePoint;
			pSdwe->CalibratePointArry[point] = pSdwe->SetData;
			//weight
			weight = pSdwe->SetData;
		
			if(0 == pSdwe->CalibrateChanel)//all chanel point weight value set
			{
				for(i=0;i<HX711_CHANEL_NUM;i++)//8通道
				{
					setSampleWeightValue(i,point,weight);
					pointWeightTrigerDataSet(i,point,weight);
				}
				pointWeightTrigerDataSet(i,point,weight);
			}
			else//single chanel point weight value set
			{
				setSampleWeightValue((pSdwe->CalibrateChanel-1),point,weight);
				pointWeightTrigerDataSet((pSdwe->CalibrateChanel-1),point,weight);
			}
		}//triger calculate
		else if((TRUE == pSdwe->ResetTrigerValid)&&(pSdwe->SetAdd >= DMG_FUNC_SET_CHANEL_POINT_TRIG_ADDRESS)&&(pSdwe->SetAdd < (DMG_FUNC_SET_CHANEL_POINT_TRIG_ADDRESS + CHANEL_POINT_NUM )))
		{
			//value = 0x12fe
			if(DMG_FUNC_SET_CHANEL_POINT_TRIG_VAL == pSdwe->SetData)
			{
				//	
				pSdwe->sdwePointTriger = TRUE;
				//
				needStore = DMG_TRIGER_SAVE_SECOTOR_1 ;
				point = ( pSdwe->SetAdd - DMG_FUNC_SET_CHANEL_POINT_TRIG_ADDRESS );
				
				if(0 == pSdwe->CalibrateChanel)//all chanel caculate	K & B
				{
					//avgSampleValue = hx711_getAvgSample(pSdwe->CalibrateChanel)/512;
					for(i=0;i<HX711_CHANEL_NUM;i++)//eight chanel
					{
						avgSampleValue = hx711_getAvgSample((enumHX711ChanelType)i)/512;
						trigerCalcKB(i,point);
						pointTrigerDataSet(i,point,1,avgSampleValue);
					}
					pointTrigerDataSet(HX711_CHANEL_NUM,point,1,avgSampleValue);
					
				}
				else if(HX711_CHANEL_NUM >= pSdwe->CalibrateChanel)//single chanel caculate  K & B
				{
					avgSampleValue = hx711_getAvgSample((enumHX711ChanelType)(pSdwe->CalibrateChanel-1))/512;
					trigerCalcKB((pSdwe->CalibrateChanel-1),point);
					pointTrigerDataSet((pSdwe->CalibrateChanel-1),point,1,avgSampleValue);
				}
				//sdwePointTrigerUpdata(point,1,avgSampleValue);
			}
		}
		//clr address
		pSdwe->SetAdd = 0xffff;
	}
	return needStore;
}
//if need jump to active page 
UINT8 jumpToActivePage()
{
	UINT8 result = 0 ;
	//5A A5 07 82 0084 5A01 page
	INT16 pageChangeOrderAndData[2]={0x5A01,56};//56 page
	if(((g_T5L.LastSendTick > g_T5L.CurTick)&&((g_T5L.LastSendTick-g_T5L.CurTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER))||
		((g_T5L.LastSendTick < g_T5L.CurTick)&&((g_T5L.CurTick - g_T5L.LastSendTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER)))
	{
		t5lWriteVarible((0X0084),pageChangeOrderAndData,2,0);
		result = 1;
	}
	return result;
}
//if need jump to active page 
UINT8 jumpToBalancingPage()
{
	UINT8 result = 0 ;
	//5A A5 07 82 0084 5A01 page
	INT16 pageChangeOrderAndData[2]={0x5A01,DMG_FUNC_Balancing_6_PAGE};//49 page

	if(TRUE == gSystemPara.isCascade)
	{
		pageChangeOrderAndData[1] = DMG_FUNC_Balancing_12_PAGE;
	}
	if(((g_T5L.LastSendTick > g_T5L.CurTick)&&((g_T5L.LastSendTick-g_T5L.CurTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER))||
		((g_T5L.LastSendTick < g_T5L.CurTick)&&((g_T5L.CurTick - g_T5L.LastSendTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER)))
	{
		t5lWriteVarible((0X0084),pageChangeOrderAndData,2,0);
		result = 1;
	}
	return result;
}
//if need jump to active page 
UINT8 jumpToBalancingHomePage()
{
	UINT8 result = 0 ;
	//5A A5 07 82 0084 5A01 page
	INT16 pageChangeOrderAndData[2]={0x5A01,DMG_FUNC_Balancing_6_HOME_PAGE};

	if(TRUE == gSystemPara.isCascade)
	{
		pageChangeOrderAndData[1] = DMG_FUNC_Balancing_12_HOME_PAGE;
	}
	if(((g_T5L.LastSendTick > g_T5L.CurTick)&&((g_T5L.LastSendTick-g_T5L.CurTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER))||
		((g_T5L.LastSendTick < g_T5L.CurTick)&&((g_T5L.CurTick - g_T5L.LastSendTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER)))
	{
		t5lWriteVarible((0X0084),pageChangeOrderAndData,2,0);
		result = 1;
	}
	return result;
}
//if need jump to active page 
UINT8 jumpToBalancingCleanPage()
{
	UINT8 result = 0 ;
	//5A A5 07 82 0084 5A01 page
	INT16 pageChangeOrderAndData[2]={0x5A01,DMG_FUNC_Balancing_6_PAGE};

	if(TRUE == gSystemPara.isCascade)
	{
		pageChangeOrderAndData[1] = DMG_FUNC_Balancing_12_PAGE;
	}
	if(((g_T5L.LastSendTick > g_T5L.CurTick)&&((g_T5L.LastSendTick-g_T5L.CurTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER))||
		((g_T5L.LastSendTick < g_T5L.CurTick)&&((g_T5L.CurTick - g_T5L.LastSendTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER)))
	{
		t5lWriteVarible((0X0084),pageChangeOrderAndData,2,0);
		result = 1;
	}
	return result;
}

//if need jump to calibration page 
UINT8 jumpToCalibrationPage()
{
	UINT8 result = 0 ;
	//5A A5 07 82 0084 5A01 page
	INT16 pageChangeOrderAndData[2]={0x5A01,53};//53 page
	if(((g_T5L.LastSendTick > g_T5L.CurTick)&&((g_T5L.LastSendTick-g_T5L.CurTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER))||
		((g_T5L.LastSendTick < g_T5L.CurTick)&&((g_T5L.CurTick - g_T5L.LastSendTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER)))
	{
		t5lWriteVarible((0X0084),pageChangeOrderAndData,2,0);
		result = 1;
	}
	return result;
}
//if need jump to home page 
UINT8 jumpToHomePage()
{
	UINT8 result = 0 ;
	//5A A5 07 82 0084 5A01 page
	INT16 pageChangeOrderAndData[2]={0x5A01,54};//54 page
	if(((g_T5L.LastSendTick > g_T5L.CurTick)&&((g_T5L.LastSendTick-g_T5L.CurTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER))||
		((g_T5L.LastSendTick < g_T5L.CurTick)&&((g_T5L.CurTick - g_T5L.LastSendTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER)))
	{
		t5lWriteVarible((0X0084),pageChangeOrderAndData,2,0);
		result = 1;
	}
	return result;
}
//if need jump to Banling page 
UINT8 jumpToBanlingPage()
{
	UINT8 result = 0 ;
	//5A A5 07 82 0084 5A01 page
	INT16 pageChangeOrderAndData[2]={0x5A01,DMG_FUNC_Balancing_6_PAGE};//49 page

	if(TRUE == gSystemPara.isCascade)
	{
		pageChangeOrderAndData[1] = DMG_FUNC_Balancing_12_PAGE;
	}
	if(((g_T5L.LastSendTick > g_T5L.CurTick)&&((g_T5L.LastSendTick-g_T5L.CurTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER))||
		((g_T5L.LastSendTick < g_T5L.CurTick)&&((g_T5L.CurTick - g_T5L.LastSendTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER)))
	{
		t5lWriteVarible((0X0084),pageChangeOrderAndData,2,0);
		result = 1;
	}
	return result;
}




//if reset calibration valid 
//prepare DMG display of color and sample avg data
UINT8 resetCalibrationTrigerDeal()
{
	static UINT8 inerStatus = 0 , localChanel = 0 ;	

	INT16 *pSendData= 0;
	UINT8 result = 0 ;

	if(g_T5L.CalibrateChanel > HX711_CHANEL_NUM)
		return 0 ;

	//chanel get
	if(0 == g_T5L.CalibrateChanel)
	{
		localChanel = 0 ;
	}
	else if(g_T5L.CalibrateChanel <= HX711_CHANEL_NUM)
	{
		localChanel = g_T5L.CalibrateChanel ;
	}
	//status
	switch(inerStatus)
	{
		case 0://send Color
			if(((g_T5L.LastSendTick > g_T5L.CurTick)&&((g_T5L.LastSendTick-g_T5L.CurTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER))||
				((g_T5L.LastSendTick < g_T5L.CurTick)&&((g_T5L.CurTick - g_T5L.LastSendTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER)))
			{
				pSendData= &g_t5l_triger_data[localChanel][DMG_TRIGER_SAMPLE_OF_ASK_COLOR][0];//color:1 green 0:white
				t5lWriteVarible(DMG_FUNC_ASK_CHANEL_POINT_TRIG_BACK_COLOR_ADDRESS,pSendData,(CHANEL_POINT_NUM),0);
				//
				inerStatus++ ;
			}
		break;
		case 1://send data
			{
				if(((g_T5L.LastSendTick > g_T5L.CurTick)&&((g_T5L.LastSendTick-g_T5L.CurTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER))||
					((g_T5L.LastSendTick < g_T5L.CurTick)&&((g_T5L.CurTick - g_T5L.LastSendTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER)))
				{
					pSendData= &g_t5l_triger_data[localChanel][DMG_TRIGER_SAMPLE_OF_AVG_SAMPLE][0];//data
					t5lWriteVarible(DMG_FUNC_ASK_CHANEL_POINT_TRIG_SAMPLE_DATA_ADDRESS,pSendData,(CHANEL_POINT_NUM),0);
					//
					inerStatus++ ;
				}
			}
			break;
		default:
			inerStatus = 0 ;
			result = 1 ;
			break;
	}

	return result;
}


//if sreen chanel changed
UINT8 chanelChangedTrigerDeal()
{
	static UINT8 inerStatus = 0 , localChanel = 0 ;	

	INT16 *pSendData= 0 ;
	UINT8 result = 0 ;
	
	if(g_T5L.CalibrateChanel > HX711_CHANEL_NUM)
		return 0 ;

	//chanel get
	if(0 == g_T5L.CalibrateChanel)
	{
		localChanel = HX711_CHANEL_NUM ;
	}
	else if(g_T5L.CalibrateChanel <= HX711_CHANEL_NUM)
	{
		localChanel = g_T5L.CalibrateChanel - 1 ;
	}
	
	//status
	switch(inerStatus)
	{
		case 0://send back Color
			if(((g_T5L.LastSendTick > g_T5L.CurTick)&&((g_T5L.LastSendTick-g_T5L.CurTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER))||
				((g_T5L.LastSendTick < g_T5L.CurTick)&&((g_T5L.CurTick - g_T5L.LastSendTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER)))
			{
				pSendData= &g_t5l_triger_data[localChanel][DMG_TRIGER_SAMPLE_OF_ASK_COLOR][0];//color:1 green 0:white
				t5lWriteVarible(DMG_FUNC_ASK_CHANEL_POINT_TRIG_BACK_COLOR_ADDRESS,pSendData,(CHANEL_POINT_NUM),0);
				//
				inerStatus++ ;
			}
		break;
		case 1://send avg sample data
			{
				if(((g_T5L.LastSendTick > g_T5L.CurTick)&&((g_T5L.LastSendTick-g_T5L.CurTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER))||
					((g_T5L.LastSendTick < g_T5L.CurTick)&&((g_T5L.CurTick - g_T5L.LastSendTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER)))
				{
					pSendData= &g_t5l_triger_data[localChanel][DMG_TRIGER_SAMPLE_OF_AVG_SAMPLE][0];//avg sample data
					t5lWriteVarible(DMG_FUNC_ASK_CHANEL_POINT_TRIG_SAMPLE_DATA_ADDRESS,pSendData,(CHANEL_POINT_NUM),0);
					//
					inerStatus++ ;
				}
			}
			break;
		case 2://send weight point
			{
				if(((g_T5L.LastSendTick > g_T5L.CurTick)&&((g_T5L.LastSendTick-g_T5L.CurTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER))||
					((g_T5L.LastSendTick < g_T5L.CurTick)&&((g_T5L.CurTick - g_T5L.LastSendTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER)))
				{
					pSendData= &g_t5l_triger_data[localChanel][DMG_TRIGER_SAMPLE_OF_ASK_WEIGHT][0];//weight point data
					t5lWriteVarible(DMG_FUNC_SET_CHANEL_POINT_ADDRESS,pSendData,(CHANEL_POINT_NUM),0);
					//
					inerStatus++ ;
				}
			}
			break;
		case 3://send chanel
			{
				if(((g_T5L.LastSendTick > g_T5L.CurTick)&&((g_T5L.LastSendTick-g_T5L.CurTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER))||
					((g_T5L.LastSendTick < g_T5L.CurTick)&&((g_T5L.CurTick - g_T5L.LastSendTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER)))
				{

					pSendData = (INT16 *)&(g_T5L.CalibrateChanel);		
					t5lWriteVarible(DMG_FUNC_SET_CHANEL_NUM,pSendData,1,0);
					//
					inerStatus++ ;
				}
			}
			break;
		default:
			inerStatus = 0 ;
			result = 1 ;
			break;
	}

	return result;

}


UINT8 removeWeightTrigerDeal()
{
	INT16 *pSendData= &g_t5l_dis_data[0];
	INT16 weight[HX711_CHANEL_NUM]; 
	enumHX711ChanelType chanel = HX711Chanel_1;
	
	UINT8 result = 0 ;
	static UINT8 inerStatus = 0 ; 

	//=============================================================weight value and color
	pSendData= &g_t5l_dis_data[0];
	for(chanel=HX711Chanel_1;chanel<HX711_CHANEL_NUM;chanel++)
	{
		weight[chanel] = (INT16)(hx711_getWeight(chanel)+0.5f);
		pSendData[chanel] = weight[chanel];
	}

	switch(inerStatus)
	{
		case 0:
			if(((g_T5L.LastSendTick > g_T5L.CurTick)&&((g_T5L.LastSendTick-g_T5L.CurTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER))||
				((g_T5L.LastSendTick < g_T5L.CurTick)&&((g_T5L.CurTick - g_T5L.LastSendTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER)))
			{
				t5lWriteVarible(DMG_FUNC_ASK_CHANEL_WEIGHT_ADDRESS,pSendData,HX711_CHANEL_NUM,0);
				//
				inerStatus=1;
				//
				//inerStatus=0;
				//need_send = FALSE;
			}
		break;
		case 1:
			if(((g_T5L.LastSendTick > g_T5L.CurTick)&&((g_T5L.LastSendTick-g_T5L.CurTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER))||
				((g_T5L.LastSendTick < g_T5L.CurTick)&&((g_T5L.CurTick - g_T5L.LastSendTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER)))
			{
			
				pSendData= &g_t5l_dis_data[HX711_CHANEL_NUM];
				
				t5lWriteVarible(DMG_FUNC_ASK_CHANEL_COLOR_ADDRESS,pSendData,HX711_CHANEL_NUM,0);
				//
				inerStatus=2;
			}
		break;
		default:
			inerStatus = 0 ;
			result = TRUE;
		break;
	}
	return result;
}

//at BALANCING Page , auto to judge the remaining chanel weight minus
//to help user to caculate
//1.find out the remaining chanel
//2.find out the closed group(minus was smallest)
//3.send to DIWEN Screen to display
#define DIFF_JUDGE_GROUP_NUM	(2)//2 group display 
#define DIFF_JUDGE_DATA_NUM		(3)//num1 num2 minus

void sendHelpDataDiff()
{
	//===================================================================
	enumHX711ChanelType chanel = HX711Chanel_1;
	float weight[HX711_CHANEL_NUM];	
	enumHX711ChanelType chanleArry[HX711_CHANEL_NUM];
	//JUDGE
	float fMinus=0;
	INT16 i16Number1[DIFF_JUDGE_GROUP_NUM]={0,0};
	INT16 i16Number2[DIFF_JUDGE_GROUP_NUM]={0,0};
	INT16 i16Min[DIFF_JUDGE_GROUP_NUM]={0,0};
	UINT8 u8Min_i=0;
	//
	UINT8 i = 0;
	//locate
	static INT16 si16Number1[DIFF_JUDGE_GROUP_NUM]={0,0};
	static INT16 si16Number2[DIFF_JUDGE_GROUP_NUM]={0,0};
	static INT16 si16Min[DIFF_JUDGE_GROUP_NUM]={0,0};
	//
	static UINT8 need_send = 0;
	
	INT16 sendData[DIFF_JUDGE_GROUP_NUM*DIFF_JUDGE_DATA_NUM];
	
	//get weight and chanel num
	for(chanel=HX711Chanel_1;chanel<HX711_CHANEL_NUM;chanel++)
	{
		//weight[chanel] = hx711_getWeight(chanel);
		weight[chanel] = g_t5l_dis_data_buff[chanel];
		chanleArry[chanel] = chanel;
	}

	//remove peiping
	#if 0
		BubbleSort(weight,chanleArry,HX711_CHANEL_NUM);
		for(chanel=HX711Chanel_1;chanel<(HX711_CHANEL_NUM-1);chanel++)
		{
			//judge allready peiping
			fMinus = weight[chanel+1] - weight[chanel];
			if(fMinus <= gSystemPara.errRange)
			{
				weight[chanel+1] = 0 ;
				weight[chanel] = 0 ;
				//
				chanel++;
			}
		}
	#else
		for(chanel=HX711Chanel_1;chanel<(HX711_CHANEL_NUM);chanel++)
		{
			if(g_t5l_dis_data_buff[HX711_CHANEL_NUM+chanel] != LED_COLOR_NONE)
			{
				weight[chanel] = 0 ;
			}
		}
	#endif	

	//Sort
	BubbleSort(weight,(INT16 *)chanleArry,(UINT8)HX711_CHANEL_NUM);
	
	//
	for(chanel=HX711Chanel_1;chanel<(HX711_CHANEL_NUM-1);chanel++)
	{
		//judge allready peiping
		fMinus = weight[chanel+1] - weight[chanel];
		if(fMinus <= gSystemPara.errRange)
		{
			chanel++;
		}
		else
		{
			if((weight[chanel+1] >= gSystemPara.zeroRange) || 
				(weight[chanel+1] <= -gSystemPara.zeroRange) )
			{
				if((weight[chanel] >= gSystemPara.zeroRange) || 
					(weight[chanel] <= -gSystemPara.zeroRange) )
				{
					fMinus = weight[chanel+1] - weight[chanel];
					if(fMinus > gSystemPara.errRange)
					{
						i16Number1[u8Min_i] = chanleArry[chanel+1]+1;
						i16Number2[u8Min_i] = chanleArry[chanel]+1;
						i16Min[u8Min_i] = (INT16)fMinus;
						//
						chanel++;
						//
						u8Min_i++;
						if(u8Min_i>=DIFF_JUDGE_GROUP_NUM)
						{
							break;
						}
					}
				}
			
			}
		}
	}
	
	//compare help data if equal
	for(i=0;i<DIFF_JUDGE_GROUP_NUM;i++)
	{
		if((i16Number1[i]!=si16Number1[i])||
			(i16Number2[i]!=si16Number2[i])||
			(i16Min[i]!=si16Min[i]))
		{
			si16Number1[i] = i16Number1[i];
			si16Number2[i] = i16Number2[i];
			si16Min[i] = i16Min[i];
			need_send = 1 ;
		}
	}

	if(1 == need_send)
	{
		if(((g_T5L.LastSendTick > g_T5L.CurTick)&&((g_T5L.LastSendTick-g_T5L.CurTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER))||
			((g_T5L.LastSendTick < g_T5L.CurTick)&&((g_T5L.CurTick - g_T5L.LastSendTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER)))
		{
			//
			for(i=0;i<DIFF_JUDGE_GROUP_NUM;i++)
			{
				sendData[i*DIFF_JUDGE_DATA_NUM+0] = si16Number1[i];
				sendData[i*DIFF_JUDGE_DATA_NUM+1] = si16Number2[i];
				sendData[i*DIFF_JUDGE_DATA_NUM+2] = si16Min[i];
			}
			//
			t5lWriteVarible(DMG_FUNC_HELP_TO_JUDGE_SET_ADDRESS,sendData,(DIFF_JUDGE_GROUP_NUM*DIFF_JUDGE_DATA_NUM),0);
			need_send = FALSE;
		}
	}

}



float GetFloatBalancingModelData(enumModbusAddType slaveId,enumHX711ChanelType chanel)
{
	float weight = 0.0;
	ModbusRtuType *pContex = &g_ModbusRtu;
	if((chanel < HX711_CHANEL_NUM ) 
		&& ( slaveId >= ModbusAdd_Slave_1 )
		&& ( slaveId < ModbusAdd_Slave_Max ))
	{
		weight = pContex->MultWeightData[slaveId-ModbusAdd_Master][chanel].f_value;
	}
	return weight;
}



//
INT16 g_i16DataBuff[T5L_WEIGHT_DATA_LEN]={0};//data send to DIWEN
INT16 g_i16DataBuffPre[T5L_WEIGHT_DATA_LEN]={0};
//
INT16 g_i16ColorBuff[T5L_WEIGHT_COLOR_LEN]={0};//color send to DIWEN
INT16 g_i16ColorBuffPre[T5L_WEIGHT_COLOR_LEN]={0};
INT16 g_i16OtherChanel[T5L_WEIGHT_COLOR_LEN]={0};//other chanel need +1 , chanel = 1~x
//
float g_fDataBuffCaculate[T5L_WEIGHT_DATA_LEN]={0};
INT16 g_i16OtherChanelCaculate[T5L_WEIGHT_DATA_LEN]={0};//other chanel need +1 , chanel = 1~x

UINT8 preWeightDataAndJudgeIfNeedSend(INT16 *pData,INT16 *pDataPre,UINT8 chanel_len)
{
	UINT8 ret = FALSE , offset = 0;
	//
	enumHX711ChanelType chanel = HX711Chanel_1;
	//
	if(chanel_len <= T5L_WEIGHT_DATA_LEN)
	{
		//not ji lian
		if(0 == gSystemPara.isCascade)
		{
			for(chanel=HX711Chanel_1;chanel<HX711_CHANEL_NUM;chanel++)
			{
				offset = chanel;
				pData[offset] = (INT16)(hx711_getWeight(chanel)+0.5f);
				//
				if(pData[offset] != pDataPre[offset])
				{
					pDataPre[offset] = pData[offset];
					ret = TRUE ;
				}
			}
		}else if(ModbusAdd_Master == gSystemPara.isCascade)
		{
			//master local data
			for(chanel=HX711Chanel_1;chanel<HX711_CHANEL_NUM;chanel++)
			{
				offset = chanel;
				pData[offset] = (INT16)(hx711_getWeight(chanel)+0.5f);
				//
				if(pData[offset] != pDataPre[offset])
				{
					pDataPre[offset] = pData[offset];
					ret = TRUE ;
				}
			}

			//ModbusAdd_Slave_1 recv data
			for(chanel=HX711Chanel_1;chanel<HX711_CHANEL_NUM;chanel++)
			{
				offset = HX711_CHANEL_NUM*(ModbusAdd_Slave_1 - ModbusAdd_Master)+chanel;
				pData[offset] = (INT16)(GetFloatBalancingModelData(ModbusAdd_Slave_1,chanel)+0.5f);
				//
				if(pData[offset] != pDataPre[offset])
				{
					pDataPre[offset] = pData[offset];
					ret = TRUE ;
				}
			}
		}
	}
	//
	return ret;
	
}

enumLedColorType getSysColorWhichUsable(void)
{
	enumLedColorType ret = LED_COLOR_NONE;
	UINT8 i = 0 ;
	for(i=0;i<SYS_COLOR_GROUP_NUM;i++)
	{
		if(SYS_COLOR_USED_FLAG != gSystemPara.userColorUsed[i])
		{
			gSystemPara.userColorUsed[i] = SYS_COLOR_USED_FLAG;
			ret = (enumLedColorType)gSystemPara.userColorSet[i]; 
			break;
		}
	}
	return ret;
}
void releaseSysColor(enumLedColorType color)
{
	UINT8 i = 0 ;
	for(i=0;i<SYS_COLOR_GROUP_NUM;i++)
	{
		if(color == gSystemPara.userColorSet[i])
		{
			gSystemPara.userColorUsed[i] = 0;
			break;
		}
	}
}


void holdSysColor(enumLedColorType color)
{
	UINT8 i = 0 ;
	for(i=0;i<SYS_COLOR_GROUP_NUM;i++)
	{
		if(color == gSystemPara.userColorSet[i])
		{
			gSystemPara.userColorUsed[i] = SYS_COLOR_USED_FLAG;
			break;
		}
	}
		

}


//==20210609
UINT8 preColorDataAndJudgeIfNeedSend(INT16 *pData,INT16 *pColor,INT16 *pColorPre,INT16 *pOtherCh,UINT8 chanel_len)
{
	UINT8 ret = FALSE ,release = FALSE;
	UINT8 i = 0 , sortArry_num = 0 ,chanel_a = 0 , chanel_b = 0 , chanel = 0;
	//
	float *sortWeight = &g_fDataBuffCaculate[0];
	INT16 *sortArry = &g_i16OtherChanelCaculate[0];
	//
	UINT8 compare_i = 0 ;
	//
	enumLedColorType colorVld = LED_COLOR_NONE;
	//
	if(chanel_len <= T5L_WEIGHT_DATA_LEN)
	{
//=====================================already display color of preColor , judge again
		for(chanel=0;chanel<chanel_len;chanel++)
		{
			chanel_a = chanel;//self chanel
			chanel_b = pOtherCh[chanel_a]%chanel_len;//other chanel
			//
			if(LED_COLOR_NONE != pColor[chanel_a])
			{
				



			
				//is equal
				if( ((pData[chanel_a] < -gSystemPara.zeroRange) || (pData[chanel_a] > gSystemPara.zeroRange)) &&
					((pData[chanel_b] < -gSystemPara.zeroRange) || (pData[chanel_b] > gSystemPara.zeroRange)) &&
					(((pData[chanel_a] - pData[chanel_b]) > -gSystemPara.errRange) && ((pData[chanel_a] - pData[chanel_b]) < gSystemPara.errRange) ) )
				{
					pColor[chanel_b] = pColor[chanel_a];
					pOtherCh[chanel_a] = chanel_b%chanel_len;
					pOtherCh[chanel_b] = chanel_a%chanel_len;
				}
				else
				{
					release = TRUE;
				}
				if(TRUE == release)
				{
					//clear self and other color display : case weight not match
					releaseSysColor((enumLedColorType)pColor[chanel_a]);
					releaseSysColor((enumLedColorType)pColor[chanel_b]);
					pColor[chanel_a] = LED_COLOR_NONE;
					pColor[chanel_b] = LED_COLOR_NONE;
					//clear self and other otherChn : case weight not match
					pOtherCh[chanel_a] = 0 ;
					pOtherCh[chanel_b] = 0 ;
				}
			}
		}
//=====================================use pColor ==  LED_COLOR_NONE , to triger need judge weight
		sortArry_num = 0 ;
		for(chanel=0;chanel<chanel_len;chanel++)
		{
			if(LED_COLOR_NONE == pColor[chanel])
			{
				sortWeight[sortArry_num] = pData[chanel];
				sortArry[sortArry_num] = chanel;//===========================================very important
				sortArry_num++;
			}
		}
//=====================================use weight caculate color 2,calibrate
		//sequence 
		BubbleSort(sortWeight,sortArry,sortArry_num);
		//set color
		for(compare_i=0;compare_i<(sortArry_num-1);compare_i++)
		{
			chanel_a = sortArry[compare_i];
			chanel_b = sortArry[compare_i+1];
			if(( chanel_a < chanel_len) && ( chanel_b < chanel_len) )
			{
				//is equal
				if( ((pData[chanel_a] < -gSystemPara.zeroRange) || (pData[chanel_a] > gSystemPara.zeroRange)) &&
					((pData[chanel_b] < -gSystemPara.zeroRange) || (pData[chanel_b] > gSystemPara.zeroRange)) &&
					(((pData[chanel_a] - pData[chanel_b]) > -gSystemPara.errRange) && ((pData[chanel_a] - pData[chanel_b]) < gSystemPara.errRange) ) )
				{
					//set the same color
					colorVld = getSysColorWhichUsable();
					pColor[chanel_a] = colorVld;
					pColor[chanel_b] = colorVld;
					//otherChn recode
					pOtherCh[chanel_a] = chanel_b%chanel_len;
					pOtherCh[chanel_b] = chanel_a%chanel_len;
				}
			}
		}
//=====================================updata pColorPre from pColor
		for(i=0;i<chanel_len;i++)
		{
			if(pColor[i] != pColorPre[i])
			{
				pColorPre[i] = pColor[i];
				ret = TRUE;//need send data
			}
		}
		//need send
		if(TRUE == ret)
		{


		}
	}
	return ret;	
}

void sendBalancingWeightAndColor619()
{
	static UINT8 dataSendFlag = FALSE , colorSendFlag = FALSE;
	//
	INT16 *pData = &g_i16DataBuff[0];
	INT16 *pDataPre = &g_i16DataBuffPre[0];
	//
	INT16 *pColor = &g_i16ColorBuff[0];
	INT16 *pColorPre = &g_i16ColorBuffPre[0];
	INT16 *pOtherCh = &g_i16OtherChanel[0];
	//
	UINT8 chanel_len = 0 ;
	if(0 == gSystemPara.isCascade)
	{
		chanel_len = HX711_CHANEL_NUM;
	}
	else
	{
		chanel_len = 2*HX711_CHANEL_NUM;
	}
		
	//=================prepare weight data
	if(TRUE == preWeightDataAndJudgeIfNeedSend(pData,pDataPre,chanel_len))
	{
		dataSendFlag = TRUE;
	}
	
	//=================send weight data
	if(TRUE == dataSendFlag)
	{
		if(TRUE ==t5lWriteData(DMG_FUNC_ASK_CHANEL_WEIGHT_ADDRESS,pData,T5L_WEIGHT_DATA_LEN,0))
		{
			dataSendFlag = FALSE;
		}
	}
	else
	{
		//=================prepare color data
		//preColorDataAndJudgeIfNeedSend(INT16 *pData,INT16 *pColor,INT16 *pColorPre,INT16 *pOtherCh,UINT8 chanel_len)
		if(TRUE == preColorDataAndJudgeIfNeedSend(pData,pColor,pColorPre,pOtherCh,chanel_len))
		{
			colorSendFlag = TRUE;
		}
		//=================send color data
		if(TRUE == colorSendFlag)
		{
			if(TRUE ==t5lWriteData(DMG_FUNC_ASK_CHANEL_COLOR_ADDRESS,pColor,T5L_WEIGHT_DATA_LEN,0))
			{
				colorSendFlag = FALSE;
			}
		}
	}
}


void sendBalancingModelData()
{
	static UINT8 need_send = 0;
	INT16 *pSendData= &g_t5l_dis_data[0];
	INT16 weight[HX711_CHANEL_NUM];	
	static INT16 weightPre[HX711_CHANEL_NUM]={1,1,1,1,1,1}; 
	enumHX711ChanelType chanel = HX711Chanel_1;
	
	static UINT8 inerStatus = 0 ;	
	
	UINT8 i = 0;

	//=============================================================weight value and color
	pSendData= &g_t5l_dis_data[0];
	for(chanel=HX711Chanel_1;chanel<HX711_CHANEL_NUM;chanel++)
	{
		weight[chanel] = (INT16)(hx711_getWeight(chanel)+0.5f);
		pSendData[chanel] = weight[chanel];
		//
		if(weight[chanel] != weightPre[chanel])
		{
			weightPre[chanel] = weight[chanel];
			need_send = TRUE ;
			inerStatus = 0 ;
		}
	}

	//
	if(TRUE == need_send)
	{

		switch(inerStatus)
		{
			case 0:
				if(((g_T5L.LastSendTick > g_T5L.CurTick)&&((g_T5L.LastSendTick-g_T5L.CurTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER))||
					((g_T5L.LastSendTick < g_T5L.CurTick)&&((g_T5L.CurTick - g_T5L.LastSendTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER)))
				{
					if(0 == gSystemPara.isCascade)
					{
						for(chanel=HX711Chanel_1;chanel<HX711_CHANEL_NUM;chanel++)
						{
							g_t5l_dis_data_buff[chanel] = g_t5l_dis_data[chanel];
						}
						t5lWriteVarible(DMG_FUNC_ASK_CHANEL_WEIGHT_ADDRESS,pSendData,HX711_CHANEL_NUM,0);
					}
					else if(ModbusAdd_Master == gSystemPara.isCascade)
					{
						for(chanel=HX711Chanel_1;chanel<HX711_CHANEL_NUM;chanel++)
						{
							g_t5l_dis_data_buff[chanel] = g_t5l_dis_data[chanel];
						}
						t5lWriteVarible(DMG_FUNC_ASK_CHANEL_WEIGHT_ADDRESS,pSendData,HX711_CHANEL_NUM,0);
					}
					//
					inerStatus=1;
					//
					//inerStatus=0;
					//need_send = FALSE;
				}
			break;
			case 1:
				if(((g_T5L.LastSendTick > g_T5L.CurTick)&&((g_T5L.LastSendTick-g_T5L.CurTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER))||
					((g_T5L.LastSendTick < g_T5L.CurTick)&&((g_T5L.CurTick - g_T5L.LastSendTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER)))
				{
					//
					pSendData= &g_t5l_dis_data[HX711_CHANEL_NUM];
					for(i=0;i<HX711_CHANEL_NUM;i++)
					{
						g_t5l_dis_data_buff[HX711_CHANEL_NUM+i] = g_t5l_dis_data[HX711_CHANEL_NUM+i];
					}
					t5lWriteVarible(DMG_FUNC_ASK_CHANEL_COLOR_ADDRESS,pSendData,HX711_CHANEL_NUM,0);
					//
					inerStatus=2;
					//
					need_send = FALSE;
				}
			break;
			default:
				inerStatus = 0 ;
				need_send = FALSE;
			break;
		}
	}
}




//if sreen chanel changed
UINT8 sendSysParaDataToDiwen()
{
	static UINT8 inerStatus = 0;	
	INT16 sendData[20],len=0;
	UINT8 result = 0 ;
	
	//0x1000	4096	10	单位
	//0X100A	4106	1	最小量程
	//0X100B	4107	1	最大量程
	//0X100C	4108	1	误差范围
	//0X100D	4109	1	是否级联
	//0X100E	4110	1	LED开关
	//0X100F	4111	1	配平色1
	//0X1010	4112	1	配平色2
	//0X1011	4113	1	配平色3
	//0X1012	4114	1	配平色4
	//0X1013	4115	1	零点范围
	//0X1501				MCU设备ID
	//0X1510				密码

	switch(inerStatus)
	{
		case 0://send 0x1000 单位
			if(((g_T5L.LastSendTick > g_T5L.CurTick)&&((g_T5L.LastSendTick-g_T5L.CurTick) >= 10*DMG_MIN_DIFF_OF_TWO_SEND_ORDER))||
				((g_T5L.LastSendTick < g_T5L.CurTick)&&((g_T5L.CurTick - g_T5L.LastSendTick) >= 10*DMG_MIN_DIFF_OF_TWO_SEND_ORDER)))
			{
				len=0;
				sendData[len++] = gSystemPara.uint;
				t5lWriteVarible((0x1000),sendData,len,0);
				inerStatus++;
			}
		break;
		case 1://send 0X100A~0X1013
			if(((g_T5L.LastSendTick > g_T5L.CurTick)&&((g_T5L.LastSendTick-g_T5L.CurTick) >= 10*DMG_MIN_DIFF_OF_TWO_SEND_ORDER))||
				((g_T5L.LastSendTick < g_T5L.CurTick)&&((g_T5L.CurTick - g_T5L.LastSendTick) >= 10*DMG_MIN_DIFF_OF_TWO_SEND_ORDER)))
			{
				len = 0 ;
				sendData[len++] = gSystemPara.minWeight;/**< 最小量程 */
				sendData[len++] = gSystemPara.maxWeight;/**< 最大量程 */
				sendData[len++] = gSystemPara.errRange;/**< 误差范围 */
				sendData[len++] = gSystemPara.isCascade;/**< 是否级联 */
				sendData[len++] = gSystemPara.isLedIndicate;/**< 是否LED指示 */
				sendData[len++] = gSystemPara.userColorSet[0];/**< 配平色1 */
				sendData[len++] = gSystemPara.userColorSet[1];/**< 配平色2 */
				sendData[len++] = gSystemPara.userColorSet[2];/**< 配平色3 */
				sendData[len++] = gSystemPara.userColorSet[3];/**< 配平色4 */
				sendData[len++] = gSystemPara.zeroRange;/**< 零点范围 */
				t5lWriteVarible((0x100A),sendData,len,0);
				inerStatus++;
			}
		break;
		case 2://send 0X1501
			if(((g_T5L.LastSendTick > g_T5L.CurTick)&&((g_T5L.LastSendTick-g_T5L.CurTick) >= 10*DMG_MIN_DIFF_OF_TWO_SEND_ORDER))||
				((g_T5L.LastSendTick < g_T5L.CurTick)&&((g_T5L.CurTick - g_T5L.LastSendTick) >= 10*DMG_MIN_DIFF_OF_TWO_SEND_ORDER)))
			{
				len=0;
				sendData[len++] = g_passWordId&0XFFFF;
				t5lWriteVarible((0x1500),sendData,len,0);
				inerStatus++;
			}
		break;
		case 3://send 1510
			if(((g_T5L.LastSendTick > g_T5L.CurTick)&&((g_T5L.LastSendTick-g_T5L.CurTick) >= 10*DMG_MIN_DIFF_OF_TWO_SEND_ORDER))||
				((g_T5L.LastSendTick < g_T5L.CurTick)&&((g_T5L.CurTick - g_T5L.LastSendTick) >= 10*DMG_MIN_DIFF_OF_TWO_SEND_ORDER)))
			{
				len=0;
				sendData[len++] = g_passWordStore&0XFFFF;
				t5lWriteVarible((0x1510),sendData,len,0);
				inerStatus++;
			}
		break;
		case 4://send 2100 DMG_FUNC_SET_CHANEL_NUM
			if(((g_T5L.LastSendTick > g_T5L.CurTick)&&((g_T5L.LastSendTick-g_T5L.CurTick) >= 10*DMG_MIN_DIFF_OF_TWO_SEND_ORDER))||
				((g_T5L.LastSendTick < g_T5L.CurTick)&&((g_T5L.CurTick - g_T5L.LastSendTick) >= 10*DMG_MIN_DIFF_OF_TWO_SEND_ORDER)))
			{
				len=0;
				sendData[len++] = g_T5L.CalibrateChanel;
				t5lWriteVarible(DMG_FUNC_SET_CHANEL_NUM,sendData,len,0);
				inerStatus++;
			}
		break;
		default:
			result = 1;
		break;
	}
	return result;
}

//if need jump to Banling page 
UINT8 trigerVoice(UINT8 test_id)
{
	UINT8 result = 0 ;
	//5A A5 07 82 0084 5A01 page
	//5A A5 07 82 00A0 3101 4000
	INT16 pageChangeOrderAndData[2]={0x3101,0X6400};//64音量100 00速度
	
	
	pageChangeOrderAndData[0] = ((test_id%15)<<8)+(1);//音乐序号 1：整段音乐
	if(((g_T5L.LastSendTick > g_T5L.CurTick)&&((g_T5L.LastSendTick-g_T5L.CurTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER))||
		((g_T5L.LastSendTick < g_T5L.CurTick)&&((g_T5L.CurTick - g_T5L.LastSendTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER)))
	{
		t5lWriteVarible((0X00A0),pageChangeOrderAndData,2,0);
		result = 1;
	}
	return result;
}

//===========================T5L Voice Printf Manage
void sdwe_VoicePrintfPush(tT5LVoinceType u8Voice1 ,tT5LVoinceType u8Voice2)
{
	g_T5L_VoiceBuff[u8T5LVoiceBuffPush_i][0] = u8Voice1;
	g_T5L_VoiceBuff[u8T5LVoiceBuffPush_i][1] = u8Voice2;
	g_T5L_VoiceBuff[u8T5LVoiceBuffPush_i][2] = VoiceTypePeiPin_14;
	u8T5LVoiceBuffPush_i = (u8T5LVoiceBuffPush_i+1)%T5L_VOICE_MAX_PRINTF_NUM;
}
UINT8 sdwe_VoicePrintfPop(tT5LVoinceType *u8Voice1 , tT5LVoinceType *u8Voice2 , tT5LVoinceType *u8Voice3)
{
	UINT8 ret = FALSE;
	*u8Voice1 = g_T5L_VoiceBuff[u8T5LVoiceBuffPop_i][0];
	g_T5L_VoiceBuff[u8T5LVoiceBuffPop_i][0] =T5L_VoiceTypeNum_0;
	*u8Voice2 = g_T5L_VoiceBuff[u8T5LVoiceBuffPop_i][1];
	g_T5L_VoiceBuff[u8T5LVoiceBuffPop_i][1] = T5L_VoiceTypeNum_0;
	*u8Voice3 = g_T5L_VoiceBuff[u8T5LVoiceBuffPop_i][2];
	g_T5L_VoiceBuff[u8T5LVoiceBuffPop_i][2] = T5L_VoiceTypeNum_0;
	u8T5LVoiceBuffPop_i = (u8T5LVoiceBuffPop_i+1)%T5L_VOICE_MAX_PRINTF_NUM;
	//
	if((T5L_VoiceTypeNum_0 != *u8Voice1) && (T5L_VoiceTypeNum_0 != *u8Voice2))
	{
		ret = TRUE;
	}
	return ret;
}
//if need jump to Banling page 
UINT8 screenT5L_OutputVoice(UINT8 voiceId)
{
	UINT8 result = 0 ;
	//5A A5 07 82 00A0 3101 4000
	INT16 pageChangeOrderAndData[2]={0x3101,0X6400};//64音量100 00速度
	//
	pageChangeOrderAndData[0] = ((voiceId%VoiceTypeMax)<<8)+(1);//音乐序号 1：整段音乐
	if(((g_T5L.LastSendTick > g_T5L.CurTick)&&((g_T5L.LastSendTick-g_T5L.CurTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER))||
		((g_T5L.LastSendTick < g_T5L.CurTick)&&((g_T5L.CurTick - g_T5L.LastSendTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER)))
	{
		t5lWriteVarible((0X00A0),pageChangeOrderAndData,2,0);
		result = 1;
	}
	return result;
}
//
void screenT5L_VoicePrintfMainfunction(void)
{
	static UINT8 u8Vstatus = 0 ;
	static tT5LVoinceType u8Voice1 = T5L_VoiceTypeNum_0 ,u8Voice2 = T5L_VoiceTypeNum_0 ,u8Voice3 = T5L_VoiceTypeNum_0 ;
	static UINT16 u16Ticks = 0 ;
	//
	switch(u8Vstatus)
	{
		case 0:
			if(TRUE == sdwe_VoicePrintfPop(&u8Voice1,&u8Voice2,&u8Voice3))
			{
				u8Vstatus++;
			}
		break;
		//===========V1
		case 1://printf V1
			if(TRUE == screenT5L_OutputVoice(u8Voice1))
			{
				u8Vstatus++;
				u16Ticks = 0 ;
			}
		break;
		case 2://wait time
			if(u16Ticks++ > 1000)
			{
				u8Vstatus++;
			}
		break;
		//===========yu
		case 3://printf yu
			if(TRUE == screenT5L_OutputVoice(VoiceTypeYu_13))
			{
				u8Vstatus++;
				u16Ticks = 0 ;
			}
		break;
		case 4://wait time
			if(u16Ticks++ > 1000)
			{
				u8Vstatus++;
			}
		break;
		//===========V2
		case 5://printf v2
			if(TRUE == screenT5L_OutputVoice(u8Voice2))
			{
				u8Vstatus++;
				u16Ticks = 0 ;
			}
		break;
		case 6://wait time
			if(u16Ticks++ > 1000)
			{
				u8Vstatus++;
			}
		break;
		//==========pei pin cheng gong
		case 7://printf v1 v2 success
			if(TRUE == screenT5L_OutputVoice(u8Voice3))
			{
				u8Vstatus++;
				u16Ticks = 0 ;
			}
		break;
		case 8://wait time
			if(u16Ticks++ > 1500)
			{
				u8Vstatus++;
			}
		break;
		default:
			u8Vstatus = 0 ;
		break;
	}
}

//==prepare TX data
void screenT5L_TxFunction(void)
{
	//voice printf mainfunction
	screenT5L_VoicePrintfMainfunction();

	//==send initial data to DIWEN to display
	if(FALSE == g_T5L.sendSdweInit)
	{
		if(0!= sendSysParaDataToDiwen())
		{
			g_T5L.sendSdweInit = 123;
		}
	}//==M1 event arrive:jump to HOME Page
	else if(TRUE == g_T5L.sdweJumpToHomePage)
	{
		if(0 != jumpToHomePage())
		{
			g_T5L.sdweJumpToHomePage = FALSE;
		}
	}//==M2 event arrive:jump to BALANCING Page
	else if(TRUE == g_T5L.sdweJumpToBanlingPage)
	{
		if(0 != jumpToBanlingPage())
		{
			g_T5L.sdweJumpToBanlingPage = FALSE;
		}
	}//==M3 event arrive:jump to CALITRATION Page
	else if(TRUE == g_T5L.sdweJumpToCalitrationPage)
	{
		if(0 != jumpToCalibrationPage())
		{
			g_T5L.sdweJumpToCalitrationPage = FALSE;
		}
	}//==M4 event arrive:jump to ACTIVE Page
	else if(TRUE == g_T5L.sdweJumpActivePage)
	{
		if(0 != jumpToActivePage())
		{
			g_T5L.sdweJumpActivePage = FALSE;
		}
	}//==M2-1 event arrive: jump to BALANCING Page
	else if(TRUE == g_T5L.sdweJumpBalancing)
	{
		if(0!= jumpToBalancingPage())
		{
			g_T5L.sdweJumpBalancing = FALSE;
		}
	}//==M2-2 event arrive:jump to BALANCING (clean)page
	else if(TRUE == g_T5L.sdweJumpBalancing_cleanpagee)
	{
		if(0!= jumpToBalancingCleanPage())
		{
			g_T5L.sdweJumpBalancing_cleanpagee = FALSE;
		}
	}//==M2-3 event arrive:jump to BALANCING (home)page
	else if(TRUE == g_T5L.sdweJumpBalancing_home)
	{
		if(0!= jumpToBalancingHomePage())
		{
			g_T5L.sdweJumpBalancing_home = FALSE;
		}
	}//==C1 event arrive:At Calibration Page , chanel changed trigerd
	else if(TRUE == g_T5L.sdweChanelChanged)
	{
		if(0 != chanelChangedTrigerDeal())
		{
			 g_T5L.sdweChanelChanged = FALSE;
		}
	}//==C2 event arrive:At Calibration Page , calibration reset trigerd 
	else if(TRUE == g_T5L.sdweResetTriger)
	{
		if(0 != resetCalibrationTrigerDeal())
		{
			g_T5L.sdweResetTriger = FALSE;
		}
	}//==C3 event arrive:At Calibration Page , point trigerd
	else if(TRUE == g_T5L.sdwePointTriger)
	{
		if(0 != pointTrigerDeal())
		{
			g_T5L.sdwePointTriger = FALSE;
		}
	}//==B1 event arrive:At Balancing Page , remove weight trigerd
	else if(TRUE == g_T5L.sdweRemoveWeightTriger)
	{
		if(0 != removeWeightTrigerDeal())
		{
			g_T5L.sdweRemoveWeightTriger = FALSE;
		}
	}
	//==SYS LOCK CHARGE
	else if(g_sysLocked == STM32MCU_UNLOCKED)
	{
		//sendBalancingModelData();
		sendBalancingWeightAndColor619();
		sendHelpDataDiff();
	}	

}


//==SDWE UART data deal
void screenT5L_RxFunction(void)
{
	UINT8 needStore = FALSE ;
	UINT16 regLen = 0 , reg_i = 0 , regAdd = 0 , regData = 0;
	UINT16 varLen = 0 , var_i = 0 , varAdd = 0 , varData = 0;
	if(TRUE == g_T5L.RxFinishFlag)
	{
		//A5 5A
		if((T5L_RX_FUN_HEAD1 == g_T5L.rxData[cmdPosHead1]) && (T5L_RX_FUN_HEAD2 == g_T5L.rxData[cmdPosHead2]))
		{
			//2 head + 1 len + last 3(cmd:1 add:1-2 data:1-n) data 
			if(( g_T5L.RxLength >= 6 ) && ((g_T5L.RxLength-3) == g_T5L.rxData[cmdPosDataLen]) )
			{
				switch(g_T5L.rxData[cmdPosCommand])
				{
					case cmdWriteSWDERegister:
					break;
					case cmdReadSWDERegister://each register is 8 bits
						//send:A5 5A 03 cmdReadSWDERegister XX YY (XX:address YY:len)
						//rec :A5 5A (03+YY) cmdReadSWDERegister XX YY DD^YY (XX:address YY:len DD:data)
						//if((g_T5L.RxLength-3) == g_T5L.rxData[cmdPosDataLen])//remove 2 head + 1 data len
						{
							regLen = g_T5L.rxData[cmdPosReadRegAskLen];
							if(((g_T5L.rxData[cmdPosDataLen]-3)/1) == regLen)
							{
								regAdd = 0 ;
								regAdd = g_T5L.rxData[cmdPosRegAddress];
								//mult varible deal
								for(reg_i = 0 ; reg_i < regLen ;reg_i++)
								{
									regData = 0 ;
									regData = g_T5L.rxData[cmdPosRegData+reg_i];
									//deal
									needStore |= sdweAskRegData((regAdd+reg_i),regData);
								}
							}
						}
					break;
					case cmdWriteSWDEVariable:
					break;
					case cmdReadSWDEVariable://each variable is 16 bits
						//send:A5 5A 04 cmdReadSWDEVariable XX XX YY (XX XX:address YY:len)
						//rec :A5 5A (04+2*YY) cmdReadSWDEVariable XX XX YY DD DD^YY (XX XX:address YY:len DD DD:data)
						//if((g_T5L.RxLength-3) == g_T5L.rxData[cmdPosDataLen])//remove 2 head + 1 data len
						{
							varLen = g_T5L.rxData[cmdPosReadVarAskLen];
							if(((g_T5L.rxData[cmdPosDataLen]-4)/2) == varLen)
							{
								varAdd = 0 ;
								varAdd = g_T5L.rxData[cmdPosVarAddress1];					
								varAdd <<= 8 ;
								varAdd &= 0xff00;
								varAdd += g_T5L.rxData[cmdPosVarAddress2];
								//mult varible deal
								for(var_i = 0 ; var_i < varLen ;var_i++)
								{
									varData = 0 ;
									varData = g_T5L.rxData[cmdPosVarData1+2*var_i+0];					
									varData <<= 8 ;
									varData &= 0xff00;
									varData += g_T5L.rxData[cmdPosVarData1+2*var_i+1];
									//deal
									needStore |= sdweAskVaribleData((varAdd+var_i),varData);
								}
							}
						}						
					break;
					default:
					break;
				}
			}

			//store in flash
			if(g_T5L.CurTick > 5000)
			{
				if(0 != (DMG_TRIGER_SAVE_SECOTOR_1&needStore))
				{
					storeSysDataToFlash();
				}
				//store in flash
				else if(0 != (DMG_TRIGER_SAVE_SECOTOR_2&needStore))
				{
					storeSysDataToFlash_3030();
				}
			}
		}
		//
		g_T5L.RxFinishFlag = FALSE;
	}
}



//==sdwe main function
void sreenT5L_MainFunction(void)
{
	g_T5L.CurTick++;
	if(g_T5L.CurTick > 3000)
	{
		//deal rx data from SDWE
		screenT5L_RxFunction();
		
		//prepare data and send to SDWE
		screenT5L_TxFunction();
	}
}


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

/*******************************************************************************
 * Definitions
 ******************************************************************************/
SdweType g_sdwe = SdweDefault;
//store flash data : 8 * (sample value , weight value , k , b , remove value ) , last one is crc
unionFloatInt32 flashStoreDataBuf[FLASH_STORE_MAX_LEN]={0};
//sdwe 10 point triger color data:arr0 is triger flag ,arr1 is color ,arr2 is sample data
static INT16 g_sdwe_triger_data[4][CHANEL_POINT_NUM]={{0},{0},{0}};
//sdwe 8 weight data + 8 color data	
INT16 g_sdwe_dis_data[SDWE_WEIGHR_DATA_LEN]={0};

//1.chanel num :0~x HX711_CHANEL_NUM
//2.trigerStarus , back color , point avg Sample , point set weight
//3.point num
static INT16 g_dmg_triger_data[HX711_CHANEL_NUM+1][DMG_TRIGER_SAMPLE_MAX_NUM][CHANEL_POINT_NUM];


/*******************************************************************************
 * Functions
 ******************************************************************************/
//==sdwe initial
void sdwe_init(void)
{
	UINT8 i = 0 ;
	//
	g_sdwe.readSdweInit = FALSE;
	g_sdwe.pUartDevice = &g_UartDevice[UART_EXTERN];
	g_sdwe.version = 0;//SDWE version
	g_sdwe.allowCompare = FALSE;
	//
	g_sdwe.pUartDevice->pRxLength = &g_sdwe.RxLength;
	g_sdwe.pUartDevice->pRxFinishFlag = &g_sdwe.RxFinishFlag;
	g_sdwe.pUartDevice->pTxBuffer = &g_sdwe.rxData[0];
	g_sdwe.pUartDevice->pRxBuffer = &g_sdwe.rxData[0];
	//
	//
	g_sdwe.RxLength = 0;					/**< 接收字节数 */
	g_sdwe.RxFinishFlag = FALSE;			/**< 接收完成标志 */
	//
	g_sdwe.sdweSetAdd = 0XFFFF;/**< 地址 */
	g_sdwe.sdwetDataLen = 0;/**< 数据长度 */
	g_sdwe.sdweSetData = 0;/**< 数据 */

	g_sdwe.sdweColorClen=FALSE;/**< 通道切换SDWE颜色清除 */
	g_sdwe.sdweCalChanel=0;/**< 通道 */
	g_sdwe.sdweCalPoint=0;/**< 校准点 */
	//
	for(i=0;i<CHANEL_POINT_NUM;i++)
	{
		g_sdwe.sdweCalPointArry[i] = defaultChanelSamplePoint[i];/**< 校准点数组 */
	}
	//
	g_sdwe.pUartDevice->init(g_sdwe.pUartDevice);
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
			g_sdwe.txData[cmdPosHead1]=SDWE_RX_FUN_HEAD1;
			g_sdwe.txData[cmdPosHead2]=SDWE_RX_FUN_HEAD2;
			//data len
			g_sdwe.txData[cmdPosDataLen]=0X03;
			//order:write register
			g_sdwe.txData[cmdPosCommand]=cmdReadSWDERegister;
			//address
			g_sdwe.txData[cmdPosRegReadAddress]=regAdd;
			//len
			g_sdwe.txData[cmdPosRegReadLen]=regLen;
			//crc
			if(TRUE == crcEn)
			{
				crc = cal_crc16(&g_sdwe.txData[cmdPosCommand],(3));
				g_sdwe.txData[cmdPosRegReadLen+1] = 0xff&(crc>>8);
				g_sdwe.txData[cmdPosRegReadLen+2] = 0xff&(crc>>0);
				//total len
				total_len =cmdPosRegReadLen+3;
			}
			else
			{
				//total len
				total_len = cmdPosRegReadLen+1;
			}
			//send
			g_sdwe.pUartDevice->tx_bytes(g_sdwe.pUartDevice,&g_sdwe.txData[0],total_len);
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
			g_sdwe.txData[cmdPosHead1]=SDWE_RX_FUN_HEAD1;
			g_sdwe.txData[cmdPosHead2]=SDWE_RX_FUN_HEAD2;
			//data len
			g_sdwe.txData[cmdPosDataLen]=0X02+regLen;
			//order:write register
			g_sdwe.txData[cmdPosCommand]=cmdWriteSWDERegister;
			//address
			g_sdwe.txData[cmdPosRegWriteAddress]=regAdd;
			//data
			for(reg_i = 0 ; reg_i < regLen ; reg_i++)
			{
				g_sdwe.txData[cmdPosRegWritesData+reg_i] = *(pData+reg_i);
			}
			//crc
			if(TRUE == crcEn)
			{
				crc = cal_crc16(&g_sdwe.txData[cmdPosCommand],(2+1*regLen));
				g_sdwe.txData[cmdPosRegWritesData+regLen+0] = 0xff&(crc>>8);
				g_sdwe.txData[cmdPosRegWritesData+regLen+1] = 0xff&(crc>>0);
				//total len
				total_len = cmdPosRegWritesData+regLen+2;
			}
			else
			{
				//total len
				total_len = cmdPosRegWritesData+regLen;
			}
			//send
			g_sdwe.pUartDevice->tx_bytes(g_sdwe.pUartDevice,&g_sdwe.txData[0],total_len);
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
			g_sdwe.txData[cmdPosHead1]=SDWE_RX_FUN_HEAD1;
			g_sdwe.txData[cmdPosHead2]=SDWE_RX_FUN_HEAD2;
			//data len
			g_sdwe.txData[cmdPosDataLen]=0X04;
			//order:write
			g_sdwe.txData[cmdPosCommand]=cmdReadSWDEVariable;
			//varAdd
			g_sdwe.txData[cmdPosVarReadAddress1]=0xff&(varAdd>>8);
			g_sdwe.txData[cmdPosVarReadAddress2]=0xff&(varAdd>>0);
			//len
			g_sdwe.txData[cmdPosVarReadLen]=0xff&(varlen>>0);
			//crc
			if(TRUE == crcEn)
			{
				crc = cal_crc16(&g_sdwe.txData[cmdPosCommand],(4));
				g_sdwe.txData[cmdPosVarReadLen+1] = 0xff&(crc>>8);
				g_sdwe.txData[cmdPosVarReadLen+2] = 0xff&(crc>>0);
				//total len
				total_len = cmdPosVarReadLen+3;
			}
			else
			{
				//total len
				total_len = cmdPosVarReadLen+1;
			}
			//send
			g_sdwe.pUartDevice->tx_bytes(g_sdwe.pUartDevice,&g_sdwe.txData[0],total_len);
			//
			hal_delay_ms(1);
		}
	}
}
//==write varible data to SDWE thought UART
void sdweWriteVarible(UINT16 varAdd, INT16 *pData ,UINT16 varlen ,UINT8 crcEn)
{
	//A5 5A 05 82 00 03 00 01:向0x0003地址写入数据0x0001
	UINT16 i = 0 ,l_data = 0 , total_len = 0 , crc = 0;
	if(varAdd < 0xFFFF)
	{
		if(((varAdd+varlen)>0)&&((varAdd+varlen)<0xFFFF))
		{
			//head
			g_sdwe.txData[cmdPosHead1]=SDWE_RX_FUN_HEAD1;
			g_sdwe.txData[cmdPosHead2]=SDWE_RX_FUN_HEAD2;
			//data len
			g_sdwe.txData[cmdPosDataLen]=0X03+2*varlen;
			//order:write
			g_sdwe.txData[cmdPosCommand]=cmdWriteSWDEVariable;
			//varAdd
			g_sdwe.txData[cmdPosVarWriteAddress1]=0xff&(varAdd>>8);
			g_sdwe.txData[cmdPosVarWriteAddress2]=0xff&(varAdd>>0);
			//data
			for(i=0;i<varlen;i++)
			{
				l_data = *pData++;
				g_sdwe.txData[cmdPosVarWriteData+2*i+0] = 0xff&(l_data>>8);
				g_sdwe.txData[cmdPosVarWriteData+2*i+1] = 0xff&(l_data>>0);
			}
			//crc
			if(TRUE == crcEn)
			{
				crc = cal_crc16(&g_sdwe.txData[cmdPosCommand],(3+2*varlen));
				g_sdwe.txData[cmdPosVarWriteData+2*varlen+0] = 0xff&(crc>>8);
				g_sdwe.txData[cmdPosVarWriteData+2*varlen+1] = 0xff&(crc>>0);
				//total len
				total_len = cmdPosVarWriteData+2*varlen+2;
			}
			else
			{
				//total len
				total_len = cmdPosVarWriteData+2*varlen;
			}
			//send
			g_sdwe.pUartDevice->tx_bytes(g_sdwe.pUartDevice,&g_sdwe.txData[0],total_len);
			g_sdwe.sdweLastSendTick = g_sdwe.sdweTick;
			//
			hal_delay_ms(1);
		}
	}
}









//if screen at calibration page point trigerd
void pointWeightTrigerDataSet(UINT8 localChanel , UINT8 point , INT16 value)
{
	if(localChanel > HX711_CHANEL_NUM)
		return ;

	if(point < CHANEL_POINT_NUM)
	{
		g_dmg_triger_data[localChanel][DMG_TRIGER_SAMPLE_OF_ASK_WEIGHT][point] = value;//point weight triger	
	}
}
//if screen at calibration page point trigerd
void pointSampleTrigerDataSet(UINT8 localChanel , UINT8 point , INT16 value)
{
	if(localChanel > HX711_CHANEL_NUM)
		return ;

	if(point < CHANEL_POINT_NUM)
	{
		g_dmg_triger_data[localChanel][DMG_TRIGER_SAMPLE_OF_AVG_SAMPLE][point] = value;//point sample triger	
	}
}

//if screen at calibration page point trigerd
void pointTrigerDataSet(UINT8 localChanel , UINT8 point , UINT8 value ,INT16 avgSampleValue)
{	
	if(localChanel > HX711_CHANEL_NUM)
		return ;

	if(point < CHANEL_POINT_NUM)
	{
		g_dmg_triger_data[localChanel][DMG_TRIGER_SAMPLE_OF_STATUS][point] = TRUE;//point triger need answer flag	
		g_dmg_triger_data[localChanel][DMG_TRIGER_SAMPLE_OF_ASK_COLOR][point] = value;//point triger color answer	
		g_dmg_triger_data[localChanel][DMG_TRIGER_SAMPLE_OF_AVG_SAMPLE][point] = avgSampleValue;//point triger avg sample value answer
	}
}

//if sreen calibtion point triger
UINT8 pointTrigerDeal()
{
	static UINT8 inerStatus = 0 , localChanel = 0 ;	
	INT16 *pSendData= 0;
	UINT8 result = 0 ;

	if(g_sdwe.sdweCalChanel > HX711_CHANEL_NUM)
		return 0 ;

	//chanel get
	if(0 == g_sdwe.sdweCalChanel)
	{
		localChanel = HX711_CHANEL_NUM ;
	}
	else if(g_sdwe.sdweCalChanel <= HX711_CHANEL_NUM)
	{
		localChanel = g_sdwe.sdweCalChanel - 1 ;
	}

	//status
	switch(inerStatus)
	{
		case 0://send Color
			if(((g_sdwe.sdweLastSendTick > g_sdwe.sdweTick)&&((g_sdwe.sdweLastSendTick-g_sdwe.sdweTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER))||
				((g_sdwe.sdweLastSendTick < g_sdwe.sdweTick)&&((g_sdwe.sdweTick - g_sdwe.sdweLastSendTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER)))
			{
				pSendData= &g_dmg_triger_data[localChanel][DMG_TRIGER_SAMPLE_OF_ASK_COLOR][0];//color:1 green 0:white
				sdweWriteVarible(DMG_FUNC_ASK_CHANEL_POINT_TRIG_BACK_COLOR_ADDRESS,pSendData,(CHANEL_POINT_NUM),0);
				//
				inerStatus++ ;
			}
		break;
		case 1://send sample data
			{
				if(((g_sdwe.sdweLastSendTick > g_sdwe.sdweTick)&&((g_sdwe.sdweLastSendTick-g_sdwe.sdweTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER))||
					((g_sdwe.sdweLastSendTick < g_sdwe.sdweTick)&&((g_sdwe.sdweTick - g_sdwe.sdweLastSendTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER)))
				{
					pSendData= &g_dmg_triger_data[localChanel][DMG_TRIGER_SAMPLE_OF_AVG_SAMPLE][0];//data
					sdweWriteVarible(DMG_FUNC_ASK_CHANEL_POINT_TRIG_SAMPLE_DATA_ADDRESS,pSendData,(CHANEL_POINT_NUM),0);
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
	//g_sdwe.version = version;
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
//==store set data to flash
void storeSysDataToFlash()
{
	static UINT16 storeTick = 0 ; 
	ChanelType *pChanel = 0;	
	unionFloatInt32 *pWordInt32Float=&flashStoreDataBuf[0];
	UINT8 *pChar = 0 ;
	INT32 *pInt32 = 0 ;
	float *pFloat = 0;
	UINT32 crc = 0 ;
	UINT16 chanel_i = 0 ,start_i = 0 , end_i = 0;
	//get ram buf
	for(chanel_i=0;chanel_i<HX711_CHANEL_NUM;chanel_i++)
	{
		//get chanel
		pChanel = getChanelStruct(chanel_i);
		//==point sample value value
		start_i = end_i ;
		end_i = start_i+CHANEL_POINT_NUM;
		pInt32 = (INT32 *)&(pChanel->section_PointSample[0]);
		for(;start_i<end_i;start_i++)
		{
			if(start_i < (FLASH_STORE_MAX_LEN - 1))
			{
				pWordInt32Float[start_i].i_value = *pInt32++;
			}
		}
		//==point weight value
		start_i = end_i ;
		end_i = start_i+CHANEL_POINT_NUM;
		pInt32 = (INT32 *)&(pChanel->section_PointWeight[0]);
		for(;start_i<end_i;start_i++)
		{
			if(start_i < (FLASH_STORE_MAX_LEN - 1))
			{
				pWordInt32Float[start_i].i_value = *pInt32++;
			}
		}
		//==point K
		start_i = end_i ;
		end_i = start_i+CHANEL_POINT_NUM+1;
		pFloat = (float *)&(pChanel->section_K[0]);
		for(;start_i<end_i;start_i++)
		{
			if(start_i < (FLASH_STORE_MAX_LEN - 1))
			{
				pWordInt32Float[start_i].f_value = *pFloat++;
			}
		}
		//==point B
		start_i = end_i ;
		end_i = start_i+CHANEL_POINT_NUM+1;
		pFloat = (float *)&(pChanel->section_B[0]);
		for(;start_i<end_i;start_i++)
		{
			if(start_i < (FLASH_STORE_MAX_LEN - 1))
			{
				pWordInt32Float[start_i].f_value = *pFloat++;
			}
		}
		//==point remove weight
		start_i = end_i ;
		end_i = start_i+1;
		pFloat = (float *)&(pChanel->weightRemove);
		for(;start_i<end_i;start_i++)
		{
			if(start_i < (FLASH_STORE_MAX_LEN - 1))
			{
				pWordInt32Float[start_i].f_value = *pFloat++;
			}
		}
		//==point weight dirction
		start_i = end_i ;
		end_i = start_i+1;
		pInt32 = (INT32 *)&(pChanel->weightDir);
		for(;start_i<end_i;start_i++)
		{
			if(start_i < (FLASH_STORE_MAX_LEN - 1))
			{
				pWordInt32Float[start_i].i_value = *pInt32++;
			}
		}
	}
	//
	pChar = (UINT8 *)(&pWordInt32Float[0].u_value[0]);
	crc = cal_crc16(pChar,(4*start_i));
	pWordInt32Float[start_i].i_value = crc;
	start_i++;
	//write flash
	if(start_i <= FLASH_STORE_MAX_LEN)
	{	
		storeTick++;
		drv_flash_unlock();
		drv_flash_erase_sector(FLASH_STORE_ADDRESS_START);
		drv_flash_write_words( FLASH_STORE_ADDRESS_START, (UINT32 *)(&pWordInt32Float[0].i_value), (start_i) );
		drv_flash_lock();
	}
}

//==read data from flash
void readSysDataFromFlash(void)
{
	ChanelType *pChanel = 0;	
	unionFloatInt32 readflashDataBuf[FLASH_STORE_MAX_LEN]={0};
	INT32 *pInt32 = 0;
	float *pFloat = 0;
	UINT32 crc = 0 ;
	UINT16 chanel_i = 0 ,start_i = 0 , end_i = 0;
	UINT8 point_i = 0 ;
	//read data from flash
	drv_flash_read_words( FLASH_STORE_ADDRESS_START, (UINT32 *)(&readflashDataBuf[0].i_value), FLASH_STORE_MAX_LEN);

	//crc
	crc = readflashDataBuf[FLASH_STORE_MAX_LEN-1].i_value;
	if(crc == cal_crc16(((UINT8 *)&readflashDataBuf[0].u_value),(4*(FLASH_STORE_MAX_LEN-1)))) 
	{
		start_i = 0 ;
		end_i = 0 ;
		//get ram buf
		for(chanel_i=0;chanel_i<HX711_CHANEL_NUM;chanel_i++)
		{
				//get chanel
				pChanel = getChanelStruct(chanel_i);
				//==point sample value
				point_i = 0 ;
				start_i = end_i ;
				end_i = start_i + CHANEL_POINT_NUM;
				pInt32 = &(pChanel->section_PointSample[0]);
				for(;start_i<end_i;start_i++)
				{
					*pInt32++ = readflashDataBuf[start_i].i_value;
					pointSampleTrigerDataSet(chanel_i,(point_i++),readflashDataBuf[start_i].i_value);
				}
				//==point weight value
				point_i = 0;
				start_i = end_i ;
				end_i = start_i + CHANEL_POINT_NUM;
				pInt32 = &(pChanel->section_PointWeight[0]);
				for(;start_i<end_i;start_i++)
				{
					*pInt32++ = readflashDataBuf[start_i].i_value;
					pointWeightTrigerDataSet(chanel_i,(point_i++),readflashDataBuf[start_i].i_value);
				}	
				//==point K value
				start_i = end_i ;
				end_i = start_i + CHANEL_POINT_NUM + 1;
				pFloat = &(pChanel->section_K[0]);
				for(;start_i<end_i;start_i++)
				{
					*pFloat++ = readflashDataBuf[start_i].f_value;
				}
				//==point B value
				start_i = end_i ;
				end_i = start_i + CHANEL_POINT_NUM + 1;
				pFloat = &(pChanel->section_B[0]);
				for(;start_i<end_i;start_i++)
				{
					*pFloat++ = readflashDataBuf[start_i].f_value;
				}
				//==point remove value
				start_i = end_i ;
				end_i = start_i + 1;
				pFloat = &(pChanel->weightRemove);
				for(;start_i<end_i;start_i++)
				{
					*pFloat++ = readflashDataBuf[start_i].f_value;
				}
				//==point weight direction
				start_i = end_i ;
				end_i = start_i + 1;
				pInt32 = &(pChanel->weightDir);
				for(;start_i<end_i;start_i++)
				{
					*pInt32++ = readflashDataBuf[start_i].i_value;
				}
			}
		}
}

//=======================sys parameter read :unit min max....
#ifdef FLASH_SYS_PARA_STORE_MAX_LEN
static UINT32 sysPara[FLASH_SYS_PARA_STORE_MAX_LEN-1]={0};
static unionFloatInt32 flashSysParaStoreDataBuf[FLASH_SYS_PARA_STORE_MAX_LEN]={0};
UINT32 getSysPara(enumHX711SystemParaType offset)
{	
	UINT32 readBuf = 0 ;
	if(offset<HX711SystemPara_NUM)
	{
		readBuf = sysPara[offset];
	}
	return readBuf;
}
void setSysPara(enumHX711SystemParaType offset,UINT32 val)
{	
	if(offset<HX711SystemPara_NUM)
	{
		sysPara[offset] = val;
	}
}
void readSysParaFromFlsh(void)
{
	unionFloatInt32 readflashDataBuf[FLASH_SYS_PARA_STORE_MAX_LEN]={0};
	INT32 *pInt32 = 0;
	float *pFloat = 0;
	UINT32 crc = 0 ;
	UINT16 start_i = 0 , end_i = 0;
	//read data from flash
	drv_flash_read_words( FLASH_SYS_PARA_STORE_ADDRESS_START, (UINT32 *)(&readflashDataBuf[0].i_value), FLASH_SYS_PARA_STORE_MAX_LEN);

	//crc
	crc = readflashDataBuf[FLASH_SYS_PARA_STORE_MAX_LEN-1].i_value;
	if(crc == cal_crc16(((UINT8 *)&readflashDataBuf[0].u_value),(4*(FLASH_SYS_PARA_STORE_MAX_LEN-1)))) 
	{
		start_i = 0 ;
		end_i = FLASH_SYS_PARA_STORE_MAX_LEN-1;
		pInt32 = &(sysPara[0]);
		for(;start_i<end_i;start_i++)
		{
			*pInt32++ = readflashDataBuf[start_i].i_value;
		}
	}
}
void storeSysParaFromFlsh(void)
{
	static UINT16 storeTick = 0 ; 
	unionFloatInt32 *pWordInt32Float=&flashSysParaStoreDataBuf[0];
	UINT8 *pChar = 0 ;
	INT32 *pInt32 = 0 ;
	float *pFloat = 0;
	UINT32 crc = 0 ;
	UINT16 start_i = 0 , end_i = 0;

	//get ram buf
	start_i = 0 ;
	end_i = FLASH_SYS_PARA_STORE_MAX_LEN-1;
	pInt32 = &(sysPara[0]);
	for(;start_i<end_i;start_i++)
	{
		if(start_i < (FLASH_SYS_PARA_STORE_MAX_LEN-1))
		{
			pWordInt32Float[start_i].i_value = *pInt32++;
		}
	}
	
	//
	pChar = (UINT8 *)(&pWordInt32Float[0].u_value[0]);
	crc = cal_crc16(pChar,(4*start_i));
	pWordInt32Float[start_i].i_value = crc;
	start_i++;

	//write flash
	if(start_i <= FLASH_SYS_PARA_STORE_MAX_LEN)
	{	
		storeTick++;
		drv_flash_unlock();
		drv_flash_erase_sector(FLASH_SYS_PARA_STORE_ADDRESS_START);
		drv_flash_write_words( FLASH_SYS_PARA_STORE_ADDRESS_START, (UINT32 *)(&pWordInt32Float[0].i_value), (start_i) );
		drv_flash_lock();
	}
}
#endif

//==updata sdwe weight color
void sdweSetWeightBackColor(UINT8 seq,UINT8 color)
{
	if(seq < HX711_CHANEL_NUM)
	{
		//0~HX711_CHANEL_NUM:weight
		//HX711_CHANEL_NUM~2*HX711_CHANEL_NUM:color
		g_sdwe_dis_data[HX711_CHANEL_NUM+seq] = color;
	}
}
//==recv sdwe register ask deal
UINT8 sdweAskRegData(UINT8 regAdd, UINT8 regData)
{
	UINT8 needStore = FALSE ;
	SdweType *pSdwe = &g_sdwe;
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
				g_dmg_triger_data[chane_i][DMG_TRIGER_SAMPLE_OF_ASK_COLOR][point_j] = 0 ;//color:1 green 0:white
				//data
				g_dmg_triger_data[chane_i][DMG_TRIGER_SAMPLE_OF_AVG_SAMPLE][point_j] = 0 ;//sample data = 0
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
			g_dmg_triger_data[chane_i][DMG_TRIGER_SAMPLE_OF_ASK_COLOR][point_j] = 0 ;//color:1 green 0:white
			//data
			g_dmg_triger_data[chane_i][DMG_TRIGER_SAMPLE_OF_AVG_SAMPLE][point_j] = 0 ;//sample data = 0
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



//==recv sdwe variable ask deal
UINT8 sdweAskVaribleData(UINT16 varAdd, UINT16 varData)
{
	UINT8 needStore = FALSE ;
	UINT8 i = 0 , point = 0 , point_i = 0;
	INT32 weight=0,avgSampleValue=0;
	SdweType *pSdwe = &g_sdwe;
	//
	pSdwe->sdweSetAdd = varAdd ;
	pSdwe->sdweSetData = varData ;
	//receive address from SDWE
	if(0xffff != pSdwe->sdweSetAdd)
	{
		//==(update:20210328):chanel choice:0->all chanel , 1~8:single chanel
		if(DMG_FUNC_SET_CHANEL_NUM == pSdwe->sdweSetAdd)
		{
			if(pSdwe->sdweCalChanel != pSdwe->sdweSetData)
			{
				pSdwe->sdweChanelChanged = TRUE;
				if(pSdwe->sdweSetData <= HX711_CHANEL_NUM)
				{
					pSdwe->sdweCalChanel = pSdwe->sdweSetData;//chanel
				}
			}
		}//==(update:20210328):reset calibration
		else if(DMG_FUNC_RESET_CALIBRATION_ADDRESS == pSdwe->sdweSetAdd)
		{
			if(DMG_FUNC_RESET_CALIBRATION_VAL == (UINT16)pSdwe->sdweSetData)
			{
				pSdwe->sdweResetTriger = TRUE;
				clearLocalCalibrationRecordData(pSdwe->sdweCalChanel);
				clearLocalCalibrationKAndBAndSample(pSdwe->sdweCalChanel);
			}
		}//==(update:20210328):remove all weight value
		else if(DMG_FUNC_REMOVE_WEIGHT_ADDRESS == pSdwe->sdweSetAdd)
		{
			if(DMG_FUNC_REMOVE_WEIGHT_VAL == (UINT16)pSdwe->sdweSetData)
			{
				hx711_setAllRemoveWeight();
			}
		}//==(update:20210328):chanel point weight value set
		else if((pSdwe->sdweSetAdd >= DMG_FUNC_SET_CHANEL_POINT_ADDRESS)&&(pSdwe->sdweSetAdd < (DMG_FUNC_SET_CHANEL_POINT_ADDRESS + CHANEL_POINT_NUM )))
		{
			needStore = 1 ;
			//point
			pSdwe->sdweCalPoint = (pSdwe->sdweSetAdd -DMG_FUNC_SET_CHANEL_POINT_ADDRESS) ;//point
			point = pSdwe->sdweCalPoint;
			pSdwe->sdweCalPointArry[point] = pSdwe->sdweSetData;
			//weight
			weight = pSdwe->sdweSetData;
		
			if(0 == pSdwe->sdweCalChanel)//all chanel point weight value set
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
				setSampleWeightValue((pSdwe->sdweCalChanel-1),point,weight);
				pointWeightTrigerDataSet((pSdwe->sdweCalChanel-1),point,weight);
			}
		}//triger calculate
		else if((pSdwe->sdweSetAdd >= DMG_FUNC_SET_CHANEL_POINT_TRIG_ADDRESS)&&(pSdwe->sdweSetAdd < (DMG_FUNC_SET_CHANEL_POINT_TRIG_ADDRESS + CHANEL_POINT_NUM )))
		{
			//value = 0x12fe
			if(DMG_FUNC_SET_CHANEL_POINT_TRIG_VAL == pSdwe->sdweSetData)
			{
				//	
				pSdwe->sdwePointTriger = TRUE;
				//
				needStore = 1 ;
				point = ( pSdwe->sdweSetAdd - DMG_FUNC_SET_CHANEL_POINT_TRIG_ADDRESS );
				
				if(0 == pSdwe->sdweCalChanel)//all chanel caculate	K & B
				{
					//avgSampleValue = hx711_getAvgSample(pSdwe->sdweCalChanel)/512;
					for(i=0;i<HX711_CHANEL_NUM;i++)//eight chanel
					{
						avgSampleValue = hx711_getAvgSample(i)/512;
						trigerCalcKB(i,point);
						pointTrigerDataSet(i,point,1,avgSampleValue);
					}
					pointTrigerDataSet(HX711_CHANEL_NUM,point,1,avgSampleValue);
					
				}
				else if(HX711_CHANEL_NUM >= pSdwe->sdweCalChanel)//single chanel caculate  K & B
				{
					avgSampleValue = hx711_getAvgSample(pSdwe->sdweCalChanel-1)/512;
					trigerCalcKB((pSdwe->sdweCalChanel-1),point);
					pointTrigerDataSet((pSdwe->sdweCalChanel-1),point,1,avgSampleValue);
				}
				//sdwePointTrigerUpdata(point,1,avgSampleValue);
			}
		}
		//clr address
		pSdwe->sdweSetAdd = 0xffff;
	}
	return needStore;
}




//if reset calibration valid 
//prepare DMG display of color and sample avg data
UINT8 resetCalibrationTrigerDeal()
{
	static UINT8 inerStatus = 0 , localChanel = 0 ;	

	INT16 *pSendData= 0;
	UINT8 result = 0 ;

	if(g_sdwe.sdweCalChanel > HX711_CHANEL_NUM)
		return 0 ;

	//chanel get
	if(0 == g_sdwe.sdweCalChanel)
	{
		localChanel = 0 ;
	}
	else if(g_sdwe.sdweCalChanel <= HX711_CHANEL_NUM)
	{
		localChanel = g_sdwe.sdweCalChanel ;
	}
	//status
	switch(inerStatus)
	{
		case 0://send Color
			if(((g_sdwe.sdweLastSendTick > g_sdwe.sdweTick)&&((g_sdwe.sdweLastSendTick-g_sdwe.sdweTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER))||
				((g_sdwe.sdweLastSendTick < g_sdwe.sdweTick)&&((g_sdwe.sdweTick - g_sdwe.sdweLastSendTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER)))
			{
				pSendData= &g_dmg_triger_data[localChanel][DMG_TRIGER_SAMPLE_OF_ASK_COLOR][0];//color:1 green 0:white
				sdweWriteVarible(DMG_FUNC_ASK_CHANEL_POINT_TRIG_BACK_COLOR_ADDRESS,pSendData,(CHANEL_POINT_NUM),0);
				//
				inerStatus++ ;
			}
		break;
		case 1://send data
			{
				if(((g_sdwe.sdweLastSendTick > g_sdwe.sdweTick)&&((g_sdwe.sdweLastSendTick-g_sdwe.sdweTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER))||
					((g_sdwe.sdweLastSendTick < g_sdwe.sdweTick)&&((g_sdwe.sdweTick - g_sdwe.sdweLastSendTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER)))
				{
					pSendData= &g_dmg_triger_data[localChanel][DMG_TRIGER_SAMPLE_OF_AVG_SAMPLE][0];//data
					sdweWriteVarible(DMG_FUNC_ASK_CHANEL_POINT_TRIG_SAMPLE_DATA_ADDRESS,pSendData,(CHANEL_POINT_NUM),0);
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
	
	if(g_sdwe.sdweCalChanel > HX711_CHANEL_NUM)
		return 0 ;

	//chanel get
	if(0 == g_sdwe.sdweCalChanel)
	{
		localChanel = HX711_CHANEL_NUM ;
	}
	else if(g_sdwe.sdweCalChanel <= HX711_CHANEL_NUM)
	{
		localChanel = g_sdwe.sdweCalChanel - 1 ;
	}
	
	//status
	switch(inerStatus)
	{
		case 0://send back Color
			if(((g_sdwe.sdweLastSendTick > g_sdwe.sdweTick)&&((g_sdwe.sdweLastSendTick-g_sdwe.sdweTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER))||
				((g_sdwe.sdweLastSendTick < g_sdwe.sdweTick)&&((g_sdwe.sdweTick - g_sdwe.sdweLastSendTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER)))
			{
				pSendData= &g_dmg_triger_data[localChanel][DMG_TRIGER_SAMPLE_OF_ASK_COLOR][0];//color:1 green 0:white
				sdweWriteVarible(DMG_FUNC_ASK_CHANEL_POINT_TRIG_BACK_COLOR_ADDRESS,pSendData,(CHANEL_POINT_NUM),0);
				//
				inerStatus++ ;
			}
		break;
		case 1://send avg sample data
			{
				if(((g_sdwe.sdweLastSendTick > g_sdwe.sdweTick)&&((g_sdwe.sdweLastSendTick-g_sdwe.sdweTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER))||
					((g_sdwe.sdweLastSendTick < g_sdwe.sdweTick)&&((g_sdwe.sdweTick - g_sdwe.sdweLastSendTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER)))
				{
					pSendData= &g_dmg_triger_data[localChanel][DMG_TRIGER_SAMPLE_OF_AVG_SAMPLE][0];//avg sample data
					sdweWriteVarible(DMG_FUNC_ASK_CHANEL_POINT_TRIG_SAMPLE_DATA_ADDRESS,pSendData,(CHANEL_POINT_NUM),0);
					//
					inerStatus++ ;
				}
			}
			break;
		case 2://send weight point
			{
				if(((g_sdwe.sdweLastSendTick > g_sdwe.sdweTick)&&((g_sdwe.sdweLastSendTick-g_sdwe.sdweTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER))||
					((g_sdwe.sdweLastSendTick < g_sdwe.sdweTick)&&((g_sdwe.sdweTick - g_sdwe.sdweLastSendTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER)))
				{
					pSendData= &g_dmg_triger_data[localChanel][DMG_TRIGER_SAMPLE_OF_ASK_WEIGHT][0];//weight point data
					sdweWriteVarible(DMG_FUNC_SET_CHANEL_POINT_ADDRESS,pSendData,(CHANEL_POINT_NUM),0);
					//
					inerStatus++ ;
				}
			}
			break;
		case 3://send chanel
			{
				if(((g_sdwe.sdweLastSendTick > g_sdwe.sdweTick)&&((g_sdwe.sdweLastSendTick-g_sdwe.sdweTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER))||
					((g_sdwe.sdweLastSendTick < g_sdwe.sdweTick)&&((g_sdwe.sdweTick - g_sdwe.sdweLastSendTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER)))
				{

					pSendData = &(g_sdwe.sdweCalChanel);		
					sdweWriteVarible(DMG_FUNC_SET_CHANEL_NUM,pSendData,1,0);
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


void sendBalancingModelData(void)
{
	static UINT16 ticks = 0 ;
	static UINT8 need_send = 0;
	INT16 *pSendData= &g_sdwe_dis_data[0];
	INT16 weight[HX711_CHANEL_NUM];	
	static INT16 weightPre[HX711_CHANEL_NUM]; 
	enumHX711ChanelType chanel = HX711Chanel_1;


	//=============================================================weight value and color
	pSendData= &g_sdwe_dis_data[0];
	for(chanel=HX711Chanel_1;chanel<HX711_CHANEL_NUM;chanel++)
	{
		weight[chanel] = (INT16)(hx711_getWeight(chanel)+0.5f);
		pSendData[chanel] = weight[chanel];
		//
		if(weight[chanel] != weightPre[chanel])
		{
			weightPre[chanel] = weight[chanel];
			need_send = TRUE ;
		}
	}

	//
	if(TRUE == need_send)
	{
		if(((g_sdwe.sdweLastSendTick > g_sdwe.sdweTick)&&((g_sdwe.sdweLastSendTick-g_sdwe.sdweTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER))||
			((g_sdwe.sdweLastSendTick < g_sdwe.sdweTick)&&((g_sdwe.sdweTick - g_sdwe.sdweLastSendTick) >= DMG_MIN_DIFF_OF_TWO_SEND_ORDER)))
		{
			sdweWriteVarible(DMG_FUNC_ASK_CHANEL_WEIGHT_ADDRESS,pSendData,HX711_CHANEL_NUM,0);
			//
			need_send = FALSE;
		}
	}
}




//==prepare TX data
void sdwe_TxFunction(void)
{
	//==if calibration page: chanel changed trigerd
	if(TRUE == g_sdwe.sdweChanelChanged)
	{
		if(0 != chanelChangedTrigerDeal())
		{
			 g_sdwe.sdweChanelChanged = FALSE;
		}
	}
	//==if calibration page: reset calibration trigerd
	else if(TRUE == g_sdwe.sdweResetTriger)
	{
		if(0 != resetCalibrationTrigerDeal())
		{
			g_sdwe.sdweResetTriger = FALSE;
		}
	}
	//==if calibration page: point trigerd
	else if(TRUE == g_sdwe.sdwePointTriger)
	{
		if(0 != pointTrigerDeal())
		{
			g_sdwe.sdwePointTriger = FALSE;
		}
	}
	else 
	{
		sendBalancingModelData();
	}	

#if 0

	if( (TRUE == getSdwePointTriger() ) || (TRUE == g_sdwe.sdweColorClen) )
	{
		if(TRUE == g_sdwe.sdweColorClen)
		{


		}
	
		#if 0
		//=============================================================point triger ask
		//clr
		//clear triger
		if(TRUE == getSdwePointTriger())
		{
			clrSdwePointTriger();
		}
		//clr triger ,color, avg sample value
		if(TRUE == g_sdwe.sdweColorClen)
		{
			//if chanel changed set SDWE back color to white
			clrSdwePointTrigerAll();
			g_sdwe.sdweColorClen = FALSE;
		}
		//color get
		pSendData = &g_sdwe_triger_data[1][0];//color:1 green 0:white
		if(0 == g_sdwe.sdweCalChanel)
		{
			sdweWriteVarible(SDWE_FUNC_ASK_CHANEL_POINT_TRIG,pSendData,(3*CHANEL_POINT_NUM),0);
		}
		else
		{
			sdweWriteVarible(SDWE_FUNC_ASK_CHANEL_POINT_TRIG,pSendData,(2*CHANEL_POINT_NUM),0);
		}
		#else
		

		
		#endif
	}
	else if((ticks%1000) == 0 )
	{
		//============================================================sdwe version and chanel and weight read
		if(FALSE == g_sdwe.readSdweInit)
		{
			sdweReadRegister(0x00,1 ,FALSE);//version read
		}
		else if(TRUE == g_sdwe.readSdweInit)//if version was read , readSdweInit = TRUE
		{
			sdweReadVarible(0X01FF,11,FALSE);//read sdwe : chanel num and 10 point weight value
			g_sdwe.readSdweInit = 2;
		}
	}	
	else if(1 == need_send)
	{
		//============================================================if weight changed , send weight and color value to SDWE
		need_send = 0 ;
		//color was set at useWeightUpdateLedAndSdweColor 
		sdweWriteVarible(SDWE_FUNC_ASK_CHANEL_WEIGHT,pSendData,SDWE_WEIGHR_DATA_LEN,0);
	}	
	#endif
}

//==SDWE UART data deal
void sdwe_RxFunction(void)
{
	UINT8 needStore = FALSE ;
	UINT16 regLen = 0 , reg_i = 0 , regAdd = 0 , regData = 0;
	UINT16 varLen = 0 , var_i = 0 , varAdd = 0 , varData = 0;
	if(TRUE == g_sdwe.RxFinishFlag)
	{
		//A5 5A
		if((SDWE_RX_FUN_HEAD1 == g_sdwe.rxData[cmdPosHead1]) && (SDWE_RX_FUN_HEAD2 == g_sdwe.rxData[cmdPosHead2]))
		{
			//2 head + 1 len + last 3(cmd:1 add:1-2 data:1-n) data 
			if(( g_sdwe.RxLength >= 6 ) && ((g_sdwe.RxLength-3) == g_sdwe.rxData[cmdPosDataLen]) )
			{
				switch(g_sdwe.rxData[cmdPosCommand])
				{
					case cmdWriteSWDERegister:
					break;
					case cmdReadSWDERegister://each register is 8 bits
						//send:A5 5A 03 cmdReadSWDERegister XX YY (XX:address YY:len)
						//rec :A5 5A (03+YY) cmdReadSWDERegister XX YY DD^YY (XX:address YY:len DD:data)
						//if((g_sdwe.RxLength-3) == g_sdwe.rxData[cmdPosDataLen])//remove 2 head + 1 data len
						{
							regLen = g_sdwe.rxData[cmdPosReadRegAskLen];
							if(((g_sdwe.rxData[cmdPosDataLen]-3)/1) == regLen)
							{
								regAdd = 0 ;
								regAdd = g_sdwe.rxData[cmdPosRegAddress];
								//mult varible deal
								for(reg_i = 0 ; reg_i < regLen ;reg_i++)
								{
									regData = 0 ;
									regData = g_sdwe.rxData[cmdPosRegData+reg_i];
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
						//if((g_sdwe.RxLength-3) == g_sdwe.rxData[cmdPosDataLen])//remove 2 head + 1 data len
						{
							varLen = g_sdwe.rxData[cmdPosReadVarAskLen];
							if(((g_sdwe.rxData[cmdPosDataLen]-4)/2) == varLen)
							{
								varAdd = 0 ;
								varAdd = g_sdwe.rxData[cmdPosVarAddress1];					
								varAdd <<= 8 ;
								varAdd &= 0xff00;
								varAdd += g_sdwe.rxData[cmdPosVarAddress2];
								//mult varible deal
								for(var_i = 0 ; var_i < varLen ;var_i++)
								{
									varData = 0 ;
									varData = g_sdwe.rxData[cmdPosVarData1+2*var_i+0];					
									varData <<= 8 ;
									varData &= 0xff00;
									varData += g_sdwe.rxData[cmdPosVarData1+2*var_i+1];
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
			if((TRUE == needStore) && (g_sdwe.sdweTick > 5000 ))
			{
				storeSysDataToFlash();
			}
		}
		//
		g_sdwe.RxFinishFlag = FALSE;
	}
}
//==sdwe->mcu rx callback ,not used
void app_uart_extern_msg_packet_process( UartDeviceType *pUartDevice )
{
	//not used
	//sdweRxDeal();	
}

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
			g_sdwe.txData[cmdPosHead1]=0x5A;
			g_sdwe.txData[cmdPosHead2]=0XA5;
			//data len
			g_sdwe.txData[cmdPosDataLen]=0X03+2*varlen;
			//order:write
			g_sdwe.txData[cmdPosCommand]=cmdWriteSWDEVariable;
			//varAdd
			g_sdwe.txData[cmdPosVarWriteAddress1]=0xff&(varAdd>>8);
			g_sdwe.txData[cmdPosVarWriteAddress2]=0xff&(varAdd>>0);
			//data
			for(i=0;i<varlen;i++)
			{
				l_data = *pData++;
				g_sdwe.txData[cmdPosVarWriteData+2*i+0] = 0xff&(l_data>>8);
				g_sdwe.txData[cmdPosVarWriteData+2*i+1] = 0xff&(l_data>>0);
			}
			//crc
			if(TRUE == crcEn)
			{
				crc = cal_crc16(&g_sdwe.txData[cmdPosCommand],(3+2*varlen));
				g_sdwe.txData[cmdPosVarWriteData+2*varlen+0] = 0xff&(crc>>8);
				g_sdwe.txData[cmdPosVarWriteData+2*varlen+1] = 0xff&(crc>>0);
				//total len
				total_len = cmdPosVarWriteData+2*varlen+2;
			}
			else
			{
				//total len
				total_len = cmdPosVarWriteData+2*varlen;
			}
			//send
			g_sdwe.pUartDevice->tx_bytes(g_sdwe.pUartDevice,&g_sdwe.txData[0],total_len);
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
//==sdwe main function
void sdwe_MainFunction(UINT8 hx711DataUpgrade)
{
	g_sdwe.sdweTick++;
	//deal rx data from SDWE
	sdwe_RxFunction();
	
	//prepare data and send to SDWE
	sdwe_TxFunction();

	//test
	//sdwe_MainFunctionTest();	
}


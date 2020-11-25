#include "hal_gpio.h"
#include "hal_delay.h"
#include "app_hx711_ctrl.h"

INT32 g_HX711_Data[HX711_CHANEL_NUM]={HX711_DEFAULT_DATA};
INT32 l_sampleBuff[HX711_CHANEL_NUM][HX711_DATA_SAMPLE_NUM]={0};
INT32 l_sampleTotal[HX711_CHANEL_NUM] = {0} ;



float g_Weirgt[HX711_CHANEL_NUM]={0.0};


//power off hx711
UINT8 hx711_PowerOff(enumHX711ChanelType chanel)
{
	UINT8 ret = 0 ;//1:success
	if( chanel < HX711_CHANEL_NUM)
	{
		//set HX711 CLK high larger than 60 us
		hal_gpio_set_do_high( (enumDoLineType)(HX711_CLK_1 + chanel) );
		hal_delay_us(200);//200us
		ret = 1 ;
	}
	return ret;
}
//power on hx711
UINT8 hx711_PowerOn(enumHX711ChanelType chanel)
{
	UINT8 ret = 0 ;//1:success
	if( chanel < HX711_CHANEL_NUM)
	{
		//set HX711 CLK low
		hal_gpio_set_do_low( (enumDoLineType)(HX711_CLK_1 + chanel) );
		ret = 1 ;
	}
	return ret;
}

//reset hx711
UINT8 hx711_reset(enumHX711ChanelType chanel)
{
	UINT8 ret = 0 ;//1:sucess
	if(1 == hx711_PowerOff(chanel))
	{
		ret = hx711_PowerOn(chanel);
	}
	return ret;
}


void hx711_DataSample()
{
	static UINT8 sample_i = 0 ;
	UINT8 clk_i = 0 ,chanel_i = 0 ,chanel_i_data = 0;
	float chanel_i_data_g = 0 ;
	for(chanel_i=0;chanel_i<HX711_CHANEL_NUM;chanel_i++)
	{
		l_sampleBuff[chanel_i][sample_i] =0 ;
	}
	
	for(clk_i = 0 ; clk_i < 27 ; clk_i++)
	{
		if( clk_i < HX711_DATA_SAMPLE_TYPE )
		{
			//CLK rising 0->1
			for(chanel_i=0;chanel_i<HX711_CHANEL_NUM;chanel_i++)
			{
				hal_gpio_set_do_low( (enumDoLineType)(HX711_CLK_1 + chanel_i));
			}
			hal_delay_us(HX711_DATA_SAMPLE_WIDE);
			for(chanel_i=0;chanel_i<HX711_CHANEL_NUM;chanel_i++)
			{
				hal_gpio_set_do_high( (enumDoLineType)(HX711_CLK_1 + chanel_i));
			}
			hal_delay_us(HX711_DATA_SAMPLE_WIDE);			
			
			//sample
			for(chanel_i=0;chanel_i<HX711_CHANEL_NUM;chanel_i++)
			{
				l_sampleBuff[chanel_i][sample_i] <<= 1 ;
				chanel_i_data = hal_di_get((enumDiLineType)(HX711_DATA_1+chanel_i));
				l_sampleBuff[chanel_i][sample_i] += chanel_i_data;
			}
			

			//negative data judge			
			for(chanel_i=0;chanel_i<HX711_CHANEL_NUM;chanel_i++)
			{
				if(l_sampleBuff[chanel_i][sample_i] >= HX711_NEGATIVE_DATA)
				{
					l_sampleBuff[chanel_i][sample_i] = -(HX711_DEFAULT_DATA - l_sampleBuff[chanel_i][sample_i] + 1) ;
				}
			}
			
			//CLK rising 1->0
			for(chanel_i=0;chanel_i<HX711_CHANEL_NUM;chanel_i++)
			{
				hal_gpio_set_do_low( (enumDoLineType)(HX711_CLK_1 + chanel_i));
			}
			
		}
		else
		{
			hal_delay_us(2*HX711_DATA_SAMPLE_WIDE);			
		}
	}
	
	for(chanel_i=0;chanel_i<HX711_CHANEL_NUM;chanel_i++)
	{
		//total
		l_sampleTotal[chanel_i] += l_sampleBuff[chanel_i][sample_i];
		l_sampleTotal[chanel_i] -= l_sampleBuff[chanel_i][(1 + sample_i)%HX711_DATA_SAMPLE_NUM];
		//output
		g_HX711_Data[chanel_i] = l_sampleTotal[chanel_i] / (HX711_DATA_SAMPLE_NUM-1);
		
		chanel_i_data_g = g_HX711_Data[chanel_i];
		
		chanel_i_data_g *= CHANEL_DEFAULT_K;
		chanel_i_data_g += CHANEL_DEFAULT_B;
		
		g_Weirgt[chanel_i] = chanel_i_data_g;
		
	}


	//
	sample_i++;
	if(sample_i>=HX711_DATA_SAMPLE_NUM)
	{
		sample_i = 0 ;
	}
}
void hx711_MainFunction(void);
void hx711_DataSampleCtrl(void)
{
	static enumHX711CtrlType status = HX711_CTRL_INIT;
	static UINT8 l_max_wait_time = 0;
	UINT8 i = 0 ,ret = 0;
	switch(status)
	{
		case HX711_CTRL_INIT:
					for(i=0;i<HX711_CHANEL_NUM;i++)
					{
						l_sampleTotal[i]=0;
					}
					status = HX711_CTRL_POWER_OFF;
		break;
		case HX711_CTRL_POWER_OFF:
					for(i=0;i<HX711_CHANEL_NUM;i++)
					{
						hal_gpio_set_do_high( (enumDoLineType)(HX711_CLK_1 + i) );
					}
					status = HX711_CTRL_POWER_ON;
		break;
		case HX711_CTRL_POWER_ON:
					for(i=0;i<HX711_CHANEL_NUM;i++)
					{
						hal_gpio_set_do_low( (enumDoLineType)(HX711_CLK_1 + i) );
					}
					status = HX711_CTRL_WAIT;
					l_max_wait_time = 0;
		break;
		case HX711_CTRL_WAIT:
			l_max_wait_time+=2;
			if( l_max_wait_time >= HX711_MAX_WAIT_TIME)
			{
					l_max_wait_time = 0 ;
					status = HX711_CTRL_POWER_OFF;
			}
			else
			{
				for(i=0;i<HX711_CHANEL_NUM;i++)
				{
					//wait DATA faling 1->0
					if(1 == hal_di_get((enumDoLineType)(HX711_DATA_1+i)))
					{
						ret = 1;
						break;
					}
				}
				//
				if(ret == 1)
				{
					status = HX711_CTRL_WAIT;
				}
				else
				{
					status = HX711_CTRL_SAMPLE;
				}
			}
		break;
		case HX711_CTRL_SAMPLE:
				 //hx711_DataSample();
				 hx711_MainFunction();
				 status = HX711_CTRL_WAIT;
				 l_max_wait_time = 0;
		break;
		default :
				status = HX711_CTRL_POWER_OFF;
		break;
	}
}




//==========================================================================================
static ChanelType HX711Chanel[CHANEL_NUM];
void sampleDataPush(ChanelType *pChanel , UINT32 sampleData)
{
	if( pChanel->sample_offset >= CHANEL_FILTER_NUM )
	{
		pChanel->sample_offset = 0 ;
		pChanel->sampleCycle = TRUE;
	}
	//
	if(sampleData >= HX711_NEGATIVE_DATA )
	{
		pChanel->sample_Arr[pChanel->sample_offset] = -(HX711_DEFAULT_DATA - sampleData + 1);
	}
	else
	{
		pChanel->sample_Arr[pChanel->sample_offset] = sampleData;
	}
	//calculate Total
	if(TRUE == pChanel->sampleCycle)
	{
		pChanel->sample_TotalValue += pChanel->sample_Arr[pChanel->sample_offset];
		pChanel->sample_TotalValue -= pChanel->sample_Arr[((CHANEL_FILTER_NUM + 1 + pChanel->sample_offset)%CHANEL_FILTER_NUM)];
	}
	else
	{
		pChanel->sample_TotalValue += pChanel->sample_Arr[pChanel->sample_offset];
	}
	//
	pChanel->sample_offset++;
}
void sampleCalcKB(ChanelType *pChanel,UINT8 point,UINT32 weight)
{
	float k=0.0,b=0.0;
	//
	if(point <= (CHANEL_SECTION_NUM-2))
	{
		pChanel->section_PointWeight[point] = weight ;
		pChanel->section_PointSample[point] = pChanel->sample_AvgValue;
		//
		if((0 == point) || ((CHANEL_SECTION_NUM-2) == point))
		{
			//k
			k = CHANEL_DEFAULT_K;
			//b
			b = pChanel->section_PointWeight[point];
			b -= k*pChanel->section_PointSample[point];
		}
		else
		{
			//k
			k = 0.0;
			k = (pChanel->section_PointWeight[point] - pChanel->section_PointWeight[point-1]);
			k = k / (pChanel->section_PointSample[point]-pChanel->section_PointSample[point-1]);
			//b
			b = pChanel->section_PointWeight[point] - k*pChanel->section_PointSample[point];
		}
		//
		pChanel->section_K[point] = k;
		pChanel->section_B[point] = b;
	}
}
void hx711_SigChanelAvrgAndWeightCalc(ChanelType *pChanel)
{
	UINT8 i = 0 ;
	float weight = 0 ;
	//if sample cycle complete
	if(TRUE == pChanel->sampleCycle)
	{
		//calculate average
		pChanel->sample_AvgValue = pChanel->sample_TotalValue / CHANEL_FILTER_NUM;
		
		//find out k & b
		for( i = 0 ; i < (CHANEL_SECTION_NUM-1) ; i++ )
		{
			if( pChanel->sample_AvgValue <= pChanel->section_PointSample[i] )
			{
				break;
			}
		}
		//calculate weight
		weight = pChanel->section_K[i];
		weight *= pChanel->sample_AvgValue;
		weight += pChanel->section_B[i];
		//
		pChanel->weight = weight + 0.5;
	}
}

void hx711_AllChanelSample(void)
{
	ChanelType *pChanel=&HX711Chanel[0];
	UINT8 clk_i = 0 ,chanel_i = 0 ,chanel_i_data = 0;
	UINT32 sampleDataBuf[HX711_CHANEL_NUM];
	//clear buf
	for(chanel_i=0;chanel_i<HX711_CHANEL_NUM;chanel_i++)
	{
		sampleDataBuf[chanel_i] = 0;
	}
	//sample each chanel
	for(clk_i = 0 ; clk_i < 27 ; clk_i++)
	{
		if( clk_i < HX711_DATA_SAMPLE_TYPE )
		{
			//CLK rising 0->1
			for(chanel_i=0;chanel_i<HX711_CHANEL_NUM;chanel_i++)
			{
				hal_gpio_set_do_low( (enumDoLineType)(HX711_CLK_1 + chanel_i));
			}
			hal_delay_us(HX711_DATA_SAMPLE_WIDE);
			for(chanel_i=0;chanel_i<HX711_CHANEL_NUM;chanel_i++)
			{
				hal_gpio_set_do_high( (enumDoLineType)(HX711_CLK_1 + chanel_i));
			}
			hal_delay_us(HX711_DATA_SAMPLE_WIDE);			
			
			//sample
			for(chanel_i=0;chanel_i<HX711_CHANEL_NUM;chanel_i++)
			{
				sampleDataBuf[chanel_i] <<= 1 ;
				chanel_i_data = hal_di_get((enumDiLineType)(HX711_DATA_1+chanel_i));
				sampleDataBuf[chanel_i] += chanel_i_data;
			}
			
			//CLK rising 1->0
			for(chanel_i=0;chanel_i<HX711_CHANEL_NUM;chanel_i++)
			{
				hal_gpio_set_do_low( (enumDoLineType)(HX711_CLK_1 + chanel_i));
			}
			
		}
		else
		{
			hal_delay_us(2*HX711_DATA_SAMPLE_WIDE);			
		}
	}
	//push data in arry
	for(chanel_i=0;chanel_i<HX711_CHANEL_NUM;chanel_i++)
	{
		sampleDataPush(&pChanel[chanel_i],sampleDataBuf[chanel_i]);
	}
}
void hx711_init()
{
	ChanelType *pChanel=&HX711Chanel[0];
	UINT8 chanel_i = 0 ,sample_i = 0 ,section_i = 0 ;
	for(chanel_i=0;chanel_i<HX711_CHANEL_NUM;chanel_i++)
	{
		pChanel[chanel_i].sampleCycle = FALSE;
		for(sample_i=0;sample_i<HX711_DATA_SAMPLE_NUM;sample_i++)
		{
			pChanel[chanel_i].sample_Arr[sample_i] = 0 ;
		}
		pChanel[chanel_i].sample_TotalValue = 0 ;
		pChanel[chanel_i].sample_AvgValue = 0 ;
		pChanel[chanel_i].sample_offset = 0 ;
		pChanel[chanel_i].section_offset = 0 ;
		for(section_i=0;section_i<CHANEL_SECTION_NUM;section_i++)
		{
		
			pChanel[chanel_i].section_K[section_i] = CHANEL_DEFAULT_K ;
			pChanel[chanel_i].section_B[section_i] = CHANEL_DEFAULT_B ;
		}
		pChanel[chanel_i].weight = 0 ;
		//
		pChanel[chanel_i].initFlag = TRUE ;
	}

}
//hx711 main function
void hx711_MainFunction(void)
{
	ChanelType *pChanel=&HX711Chanel[0];
	UINT8 chanel_i = 0 ;
	static float dataT[100];
	static UINT8 i=0;
	hx711_AllChanelSample();
	for(chanel_i=0;chanel_i<HX711_CHANEL_NUM;chanel_i++)
	{
		hx711_SigChanelAvrgAndWeightCalc(&pChanel[chanel_i]);
	}
	dataT[i++%100] = pChanel[0].weight;
}



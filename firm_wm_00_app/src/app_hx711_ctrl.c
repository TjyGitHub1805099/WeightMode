#include "hal_gpio.h"
#include "hal_delay.h"
#include "app_main_task.h"
#include "app_hx711_ctrl.h"
#include "app_led_ctrl.h"

const INT32 defaultChanelSamplePoint[CHANEL_POINT_NUM] = {0,50,100,200,500,1000,2000,3000,4000,5000};

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

//==========================================================================================
static ChanelType HX711Chanel[HX711_CHANEL_NUM];
//init
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
		pChanel[chanel_i].ledType = (enumLedSeqType)chanel_i;
		pChanel[chanel_i].sample_TotalValue = 0 ;
		pChanel[chanel_i].sample_AvgValue = 0 ;
		pChanel[chanel_i].sample_offset = 0 ;
		pChanel[chanel_i].section_offset = 0 ;
		for(section_i=0;section_i<(CHANEL_POINT_NUM+1);section_i++)
		{
		
			pChanel[chanel_i].section_K[section_i] = CHANEL_DEFAULT_K ;
			pChanel[chanel_i].section_B[section_i] = CHANEL_DEFAULT_B ;
		}
		for(section_i=0;section_i<(CHANEL_POINT_NUM);section_i++)
		{
		
			pChanel[chanel_i].section_PointWeight[section_i] = defaultChanelSamplePoint[section_i] ;
		}		
		pChanel[chanel_i].weight = 0 ;		
		pChanel[chanel_i].weightPre = 0;
		pChanel[chanel_i].weightRemove = 0;
		//
		pChanel[chanel_i].initFlag = TRUE ;
	}
}
ChanelType* getChanelStruct(UINT8 chanel_i)
{
	ChanelType *pChanel=&HX711Chanel[0];
	if(chanel_i < HX711_CHANEL_NUM)
	{
		pChanel = &HX711Chanel[chanel_i];
	}
	return pChanel;
}


//sample data push
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
//set chanel point sample value
void setSampleWeightValue(UINT8 chanel,UINT8 point,INT32 weight)
{
	ChanelType *pChanel = &HX711Chanel[0];
	//
	if( (chanel < HX711_CHANEL_NUM) && (point < CHANEL_POINT_NUM) )
	{
		pChanel = &HX711Chanel[chanel];
		pChanel->section_PointWeight[point] = weight ;
	}
}
//set chanel point weight value
void setSampleValue(UINT8 chanel,UINT8 point,INT32 sample)
{
	ChanelType *pChanel = &HX711Chanel[0];
	//
	if( (chanel < HX711_CHANEL_NUM) && (point < CHANEL_POINT_NUM) )
	{
		pChanel = &HX711Chanel[chanel];
		pChanel->section_PointSample[point] = sample ;
	}
}

//thought SDWE point triger cacluate K & B
void trigerCalcKB(UINT8 chanel,UINT8 point)
{
	ChanelType *pChanel = &HX711Chanel[0];
	float k=0.0,b=0.0;
	//
	if( (chanel < HX711_CHANEL_NUM) && (point < CHANEL_POINT_NUM) )
	{
		pChanel = &HX711Chanel[chanel];
		//load weight and sample
		pChanel->section_PointSample[point] = pChanel->sample_AvgValue;

		//cal each k b : point form 1~(CHANEL_POINT_NUM-1)
		if(0 != point)
		{
			//k
			k = 0.0f;
			k = (pChanel->section_PointWeight[point] - pChanel->section_PointWeight[point-1]);
			k = k / (pChanel->section_PointSample[point]-pChanel->section_PointSample[point-1]);
			//b
			b = pChanel->section_PointWeight[point] - k*pChanel->section_PointSample[point];
			//
			pChanel->section_K[point] = k;
			pChanel->section_B[point] = b;

			if((CHANEL_POINT_NUM-1) > point)
			{
				//k
				k = 0.0f;
				k = (pChanel->section_PointWeight[point+1] - pChanel->section_PointWeight[point]);
				k = k / (pChanel->section_PointSample[point+1]-pChanel->section_PointSample[point]);
				//b
				b = pChanel->section_PointWeight[point+1] - k*pChanel->section_PointSample[point+1];
				//
				pChanel->section_K[point+1] = k;
				pChanel->section_B[point+1] = b;
			}

		}

		//special deal : first point
		if(1 == point )
		{
			pChanel->section_K[point-1] = pChanel->section_K[point];
			pChanel->section_B[point-1] = pChanel->section_B[point];
		}
		//special deal : last point
		if((CHANEL_POINT_NUM-1) == point)
		{
			pChanel->section_K[point+1] = pChanel->section_K[point];
			pChanel->section_B[point+1] = pChanel->section_B[point];
		}
	}
}
//calculate K & B
void sampleCalcKB(UINT8 chanel,UINT8 point,INT32 weight)
{
	ChanelType *pChanel = &HX711Chanel[0];

	float k=0.0,b=0.0;
	//
	if(chanel >= HX711_CHANEL_NUM)
		return;

	//
	pChanel = &HX711Chanel[chanel];

	//
	if(point < CHANEL_POINT_NUM)
	{
		//load weight and sample
		pChanel->section_PointWeight[point] = weight ;
		pChanel->section_PointSample[point] = pChanel->sample_AvgValue;

		//cal each k b : point form 1~(CHANEL_POINT_NUM-1)
		if(0 != point)
		{
			//k
			k = 0.0f;
			k = (pChanel->section_PointWeight[point] - pChanel->section_PointWeight[point-1]);
			k = k / (pChanel->section_PointSample[point]-pChanel->section_PointSample[point-1]);
			//b
			b = pChanel->section_PointWeight[point] - k*pChanel->section_PointSample[point];
			//
			pChanel->section_K[point] = k;
			pChanel->section_B[point] = b;

			if((CHANEL_POINT_NUM-1) > point)
			{
				//k
				k = 0.0f;
				k = (pChanel->section_PointWeight[point+1] - pChanel->section_PointWeight[point]);
				k = k / (pChanel->section_PointSample[point+1]-pChanel->section_PointSample[point]);
				//b
				b = pChanel->section_PointWeight[point+1] - k*pChanel->section_PointSample[point+1];
				//
				pChanel->section_K[point+1] = k;
				pChanel->section_B[point+1] = b;
			}

		}

		//special deal : first point
		if(1 == point )
		{
			pChanel->section_K[point-1] = pChanel->section_K[point];
			pChanel->section_B[point-1] = pChanel->section_B[point];
		}
		//special deal : last point
		if((CHANEL_POINT_NUM-1) == point)
		{
			pChanel->section_K[point+1] = pChanel->section_K[point];
			pChanel->section_B[point+1] = pChanel->section_B[point];
		}
	}
}

//calculate avrg value and weight
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
		for( i = 0 ; i < CHANEL_POINT_NUM ; i++ )
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
		pChanel->weightPre = pChanel->weight;
		pChanel->weight = weight;
	}
}
//
float hx711_getWeight(enumHX711ChanelType chanel)
{
	float ret = 0 ;
	ChanelType *pChanel=&HX711Chanel[0];
	if( chanel < HX711_CHANEL_NUM )
	{
		//remove tare 去皮
		ret = pChanel[chanel].weight - pChanel[chanel].weightRemove;
	}
	return ret;
}
//sample all chanel data
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

//hx711 data sample ctrl
void hx711_DataSampleCtrl(void)
{
	UINT8 chanel_i = 0 ;
	ChanelType *pChanel=&HX711Chanel[0];
	//sample
	hx711_AllChanelSample();
	//cal weight
	for(chanel_i=0;chanel_i<HX711_CHANEL_NUM;chanel_i++)
	{
		hx711_SigChanelAvrgAndWeightCalc(&pChanel[chanel_i]);
	}
}
//hx711 main function
UINT8 hx711_MainFunction(void)
{
	static enumHX711CtrlType status = HX711_CTRL_INIT;
	static UINT8 l_max_wait_time = 0;
	UINT8 i = 0 ,stillWait = 0 , retn = 0 ;
	//
	switch(status)
	{
		case HX711_CTRL_INIT:
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
						stillWait = 1;
						break;
					}
				}
				//
				if(stillWait == 1)
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
			 hx711_DataSampleCtrl();
			 status = HX711_CTRL_WAIT;
			 l_max_wait_time = 0;
			 retn = 1 ;
		break;
		default :
			status = HX711_CTRL_POWER_OFF;
		break;
	}
	//
	return retn;
}


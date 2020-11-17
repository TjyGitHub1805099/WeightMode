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
	if( chanel < HX711Chanel_NUM)
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
	if( chanel < HX711Chanel_NUM)
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
	for(chanel_i=0;chanel_i<HX711Chanel_NUM;chanel_i++)
	{
		l_sampleBuff[chanel_i][sample_i] =0 ;
	}
	
	for(clk_i = 0 ; clk_i < 27 ; clk_i++)
	{
		if( clk_i < HX711_DATA_SAMPLE_TYPE )
		{
			//CLK rising 0->1
			for(chanel_i=0;chanel_i<HX711Chanel_NUM;chanel_i++)
			{
				hal_gpio_set_do_low( (enumDoLineType)(HX711_CLK_1 + chanel_i));
			}
			hal_delay_us(HX711_DATA_SAMPLE_WIDE);
			for(chanel_i=0;chanel_i<HX711Chanel_NUM;chanel_i++)
			{
				hal_gpio_set_do_high( (enumDoLineType)(HX711_CLK_1 + chanel_i));
			}
			hal_delay_us(HX711_DATA_SAMPLE_WIDE);			
			
			//sample
			for(chanel_i=0;chanel_i<HX711Chanel_NUM;chanel_i++)
			{
				l_sampleBuff[chanel_i][sample_i] <<= 1 ;
				chanel_i_data = hal_di_get((enumDiLineType)(HX711_DATA_1+chanel_i));
				l_sampleBuff[chanel_i][sample_i] += chanel_i_data;
			}
			

			//negative data judge			
			for(chanel_i=0;chanel_i<HX711Chanel_NUM;chanel_i++)
			{
				if(l_sampleBuff[chanel_i][sample_i] >= HX711_NEGATIVE_DATA)
				{
					l_sampleBuff[chanel_i][sample_i] = -(HX711_DEFAULT_DATA - l_sampleBuff[chanel_i][sample_i] + 1) ;
				}
			}
			
			//CLK rising 1->0
			for(chanel_i=0;chanel_i<HX711Chanel_NUM;chanel_i++)
			{
				hal_gpio_set_do_low( (enumDoLineType)(HX711_CLK_1 + chanel_i));
			}
			
		}
		else
		{
			hal_delay_us(2*HX711_DATA_SAMPLE_WIDE);			
		}
	}
	
	for(chanel_i=0;chanel_i<HX711Chanel_NUM;chanel_i++)
	{
		//total
		l_sampleTotal[chanel_i] += l_sampleBuff[chanel_i][sample_i];
		l_sampleTotal[chanel_i] -= l_sampleBuff[chanel_i][(1 + sample_i)%HX711_DATA_SAMPLE_NUM];
		//output
		g_HX711_Data[chanel_i] = l_sampleTotal[chanel_i] / (HX711_DATA_SAMPLE_NUM-1);
		
		chanel_i_data_g = g_HX711_Data[chanel_i];
		
		chanel_i_data_g *= WM_LINEAR_K;
		chanel_i_data_g += WM_LINEAR_B;
		
		g_Weirgt[chanel_i] = chanel_i_data_g;
		
	}


	//
	sample_i++;
	if(sample_i>=HX711_DATA_SAMPLE_NUM)
	{
		sample_i = 0 ;
	}
}

void hx711_DataSampleCtrl(void)
{
	static enumHX711CtrlType status = HX711_CTRL_INIT;
	static UINT8 l_max_wait_time = 0;
	UINT8 i = 0 ,ret = 0;
	switch(status)
	{
		case HX711_CTRL_INIT:
					for(i=0;i<HX711Chanel_NUM;i++)
					{
						l_sampleTotal[i]=0;
					}
					status = HX711_CTRL_POWER_OFF;
		break;
		case HX711_CTRL_POWER_OFF:
					for(i=0;i<HX711Chanel_NUM;i++)
					{
						hal_gpio_set_do_high( (enumDoLineType)(HX711_CLK_1 + i) );
					}
					status = HX711_CTRL_POWER_ON;
		break;
		case HX711_CTRL_POWER_ON:
					for(i=0;i<HX711Chanel_NUM;i++)
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
						for(i=0;i<HX711Chanel_NUM;i++)
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
				 hx711_DataSample();
				 status = HX711_CTRL_WAIT;
				 l_max_wait_time = 0;
		break;
		default :
				status = HX711_CTRL_POWER_OFF;
		break;
	}
}

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "hal_gpio.h"
#include "app_led_ctrl.h"
#include "app_hx711_ctrl.h"
#include "app_sdwe_ctrl.h"
#include "app_main_task.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
UINT8 g_led_ctrl_data[LED_CTRL_DATA_LEN]={0};

/*******************************************************************************
 * Functions
 ******************************************************************************/
//==led delay
void Led_delay( UINT32 TIME )
{
	while(TIME>0)
	{
		TIME--;
	}
}
//==led send pulse
void LedCtrlSendPulse(enumDoLineType offset,UINT8 type)
{
		UINT8 TIME = 100;
		if(1==type)//rising
		{
			hal_gpio_set_do_low( offset );
			Led_delay( TIME );
			hal_gpio_set_do_high( offset );
			Led_delay( TIME );
			hal_gpio_set_do_low( offset );
			Led_delay( TIME );
		}
		else//falling
		{
			hal_gpio_set_do_high( offset );
			Led_delay( TIME );
			hal_gpio_set_do_low( offset );
			Led_delay( TIME );
			hal_gpio_set_do_high( offset );
			Led_delay( TIME );
		}
}
//==led mode init
void led_init(void)
{
	//default LED output
	hal_gpio_set_do_high( LED_DO_SER0 );
	hal_gpio_set_do_high( LED_DO_OE );//disable LED OUT
	hal_gpio_set_do_low( LED_DO_RCLK );//init CLK = high
	hal_gpio_set_do_low( LED_DO_SRCLK );//init LOCK CLK = high
	hal_gpio_set_do_low( LED_DO_SRCLR );//clear ALL LED shift regester

	//1rd:disable LED output
	hal_gpio_set_do_high( LED_DO_OE );

	//2nd:clear all shift reg = 0
	LedCtrlSendPulse(LED_DO_SRCLR,0);

	//2rd:lock data form shift reg to store reg
	LedCtrlSendPulse(LED_DO_RCLK,1);

	//4th:enable LED output
	hal_gpio_set_do_low( LED_DO_OE );
}
//==set LED light 
UINT8 LedDataSet(enumLedSeqType seq , enumLedColorType color)
{	
	UINT8 ret = 0 ;//1:success
	UINT8 cloorData = 0 ;
	UINT8 offset = 0 ;//数组偏移
	UINT8 lsb_flag = 0;//低4位 or 高4位
	UINT8 l_data = 0;
	
	//color judge
	switch(color)
	{
		case LED_COLOR_REG:		cloorData = 0x08;/**< LED 红 控制 */
		break;
		case LED_COLOR_WHITE:	cloorData = 0x01;/**< LED 白 控制 */
		break;
		case LED_COLOR_BLUE:	cloorData = 0x02;/**< LED 蓝 控制 */
		break;
		case LED_COLOR_GREEN:	cloorData = 0x04;/**< LED 绿 控制 */	
		break;
		default :			 	cloorData = 0x00;/**< LED    控制 */
		break;
	}
	
	//data change
	if(seq<LED_SEQ_NUM)
	{
		//arry offset
		offset = (LED_SEQ_NUM - 1 - seq)/2;
		l_data = g_led_ctrl_data[offset];
		
		if( offset < LED_CTRL_DATA_LEN )
		{
			//lsb or msb
			lsb_flag = 	(LED_SEQ_NUM - 1 - seq)%2;

			if(0 == lsb_flag)//low 4 bits
			{
				l_data &= 0xf0;
				l_data |= cloorData;
			}
			else//high 4 bits
			{
				l_data &= 0x0f;
				l_data |= ((cloorData<<4)&0xf0);
			}
			
			//
			g_led_ctrl_data[offset] = l_data;
			ret = 1;
		}
	}
	return ret;
}
//==冒泡排序
void BubbleSort(float a[],enumHX711ChanelType arry[] ,int n)
{
	UINT8	flag = 0;
    int 	i = 0, j = 0;
	float 	temp = 0.0;
	enumHX711ChanelType chanelTemp = HX711Chanel_1;
    for( i = 0 ; i < n ; i++ )
	{
        flag=0;              //表示本趟冒泡是否发生交换的标志
        for( j = 1 ; j < n-i ; j++)
		{         //j的起始位置为1，终止位置为n-i  
            if(a[j]<a[j-1])
			{
				temp = a[j-1];
				a[j-1] = a[j];
				a[j] = temp;

				chanelTemp = arry[j-1];
				arry[j-1] = arry[j];
				arry[j] = chanelTemp;
				
               	flag=1;
            }
        }
        if(flag==0)             //未交换，说明已经有序，停止排序
        {
            return;
        }
    }          
}

#define CHANEL_COMPARED_FLAG_MASK	0XF000
#define CHANEL_COMPARED_OTHER_MASK	0X0F00
#define CHANEL_COMPARED_COLOR_MASK	0X00F0

#define CHANEL_COMPARED_FLAG_BIT	(12)
#define CHANEL_COMPARED_OTHER_BIT	(8)
#define CHANEL_COMPARED_COLOR_BIT	(4)

void useWeightUpdataOutColor(UINT8 hx711DataUpgrade)
{
	enumHX711ChanelType chanel = HX711Chanel_1,chanel_a,chanel_b;
	float curWeight[HX711_CHANEL_NUM]={0.0};
	static UINT16 chanelCompareInfo[HX711_CHANEL_NUM]={0};
	static UINT8  colorLock[LED_COLOR_NUM]={FALSE};
	enumLedColorType color_i = LED_COLOR_REG,color;

	UINT8 sortArry_num = 0;
	enumHX711ChanelType sortArry[HX711_CHANEL_NUM];
	float sortWeight[HX711_CHANEL_NUM]={0.0};
	UINT8 compare_i = 0 ;
	if(TRUE == hx711DataUpgrade)
	{
		//get current weight
		for(chanel = HX711Chanel_1;chanel<HX711_CHANEL_NUM;chanel++)
		{
			curWeight[chanel] = hx711_getWeight(chanel);
		}
		//check allready equal chanel,judge again
		for(chanel = HX711Chanel_1;chanel<HX711_CHANEL_NUM;chanel++)
		{
			//get a(self_chanel) and b(other_chanel) chanel
			chanel_a = chanel;
			chanel_b = ((chanelCompareInfo[chanel]&CHANEL_COMPARED_OTHER_MASK)>>CHANEL_COMPARED_OTHER_BIT)%HX711_CHANEL_NUM;
			color_i = ((chanelCompareInfo[chanel]&CHANEL_COMPARED_COLOR_MASK)>>CHANEL_COMPARED_COLOR_BIT)%LED_COLOR_NUM;
			//is lock
			if(chanelCompareInfo[chanel] & CHANEL_COMPARED_FLAG_MASK)//0xFxxx
			{
				//if a or b changed lager than CHANEL_MAX_ERR_RANGE
				#if 0
				if( ((curWeight[chanel_a] - curWeight[chanel_b]) > CHANEL_MAX_ERR_RANGE) ||
					((curWeight[chanel_a] - curWeight[chanel_b]) < -CHANEL_MAX_ERR_RANGE) )
				#else
				if( ((curWeight[chanel_a] - curWeight[chanel_b]) > CHANEL_MAX_ERR_RANGE) ||
					((curWeight[chanel_a] - curWeight[chanel_b]) < -CHANEL_MAX_ERR_RANGE) ||
					((curWeight[chanel_a] > -CHANEL_MAX_ERR_RANGE) && (curWeight[chanel_a] < CHANEL_MAX_ERR_RANGE)) ||
					((curWeight[chanel_b] > -CHANEL_MAX_ERR_RANGE) && (curWeight[chanel_b] < CHANEL_MAX_ERR_RANGE)) )
				#endif
				{
					//chanel unlock,other..,color unlock
					chanelCompareInfo[chanel] = 0;
					//color unlock
					colorLock[color_i] = 0 ;
					
					//clear chanel_a color
					LedDataSet(chanel_a, LED_COLOR_NONE);
					sdweSetWeightBackColor(chanel_a, LED_COLOR_NONE);
					//clear chanel_b color
					LedDataSet(chanel_b, LED_COLOR_NONE);
					sdweSetWeightBackColor(chanel_b, LED_COLOR_NONE);
				}
				else
				{
					//update chanelCompareInfo[]
					chanelCompareInfo[chanel] = 0X0F ;
					chanelCompareInfo[chanel] <<= 4;
					chanelCompareInfo[chanel] += chanel_b;
					chanelCompareInfo[chanel] <<= 4;
					chanelCompareInfo[chanel] += color_i;
					chanelCompareInfo[chanel] <<= 4;
					//color lock
					colorLock[color_i] = TRUE ;
					
					//set chanel_a color
					LedDataSet(chanel_a, color_i);
					sdweSetWeightBackColor(chanel_a, color_i);
					//set chanel_b color
					LedDataSet(chanel_b, color_i);
					sdweSetWeightBackColor(chanel_b, color_i);
				}
			}
			else//not lock
			{
				//chanel unlock,other..,color unlock
				chanelCompareInfo[chanel] = 0;
				//color unlock
				colorLock[color_i] = 0 ;
				
				//clear self color
				LedDataSet(chanel, LED_COLOR_NONE);
				sdweSetWeightBackColor(chanel, LED_COLOR_NONE);
			}
		}
		//get unlock chanel num and weight
		sortArry_num = 0 ;
		for(chanel = HX711Chanel_1;chanel<HX711_CHANEL_NUM;chanel++)
		{
			if(CHANEL_COMPARED_FLAG_MASK != (chanelCompareInfo[chanel]&CHANEL_COMPARED_FLAG_MASK))
			{
				sortArry[sortArry_num] = chanel;
				sortWeight[sortArry_num] = curWeight[chanel];
				sortArry_num++;
			}
		}
		//sequence 
		BubbleSort(sortWeight,sortArry,sortArry_num);
		for(compare_i=0;compare_i<(sortArry_num-1);compare_i++)
		{
			chanel_a = sortArry[compare_i];
			chanel_b = sortArry[compare_i+1];
			//is equal
			if(	((curWeight[chanel_a] < -CHANEL_MAX_ERR_RANGE) || (curWeight[chanel_a] > CHANEL_MAX_ERR_RANGE)) &&
				((curWeight[chanel_b] < -CHANEL_MAX_ERR_RANGE) || (curWeight[chanel_b] > CHANEL_MAX_ERR_RANGE)) &&
				(((curWeight[chanel_b] - curWeight[chanel_a]) > -CHANEL_MAX_ERR_RANGE) && ((curWeight[chanel_b] - curWeight[chanel_a]) < CHANEL_MAX_ERR_RANGE) ) )
			{
				//set color
				//find color
				for(color_i=LED_COLOR_REG;color_i<LED_COLOR_NUM;color_i++)
				{
					if(FALSE == colorLock[color_i])
					{
						//color unlock
						color = color_i;
						colorLock[color_i] = TRUE ;
						//lock chanel,other ,color
						//update chanelCompareInfo[]
						chanelCompareInfo[chanel_a] = 0X0F ;
						chanelCompareInfo[chanel_a] <<= 4;
						chanelCompareInfo[chanel_a] += chanel_b;
						chanelCompareInfo[chanel_a] <<= 4;
						chanelCompareInfo[chanel_a] += color_i;
						chanelCompareInfo[chanel_a] <<= 4;
						//update chanelCompareInfo[]
						chanelCompareInfo[chanel_b] = 0X0F ;
						chanelCompareInfo[chanel_b] <<= 4;
						chanelCompareInfo[chanel_b] += chanel_a;
						chanelCompareInfo[chanel_b] <<= 4;
						chanelCompareInfo[chanel_b] += color_i;
						chanelCompareInfo[chanel_b] <<= 4;

						//light color
						LedDataSet(chanel_a, color);//light same color
						sdweSetWeightBackColor(chanel_a, color);//light same color
						LedDataSet(chanel_b, color);//light same color
						sdweSetWeightBackColor(chanel_b, color);//light same color

						//
						compare_i++;
						break;
					}
				}
			}
			else
			{
				//chanel unlock,other..,color unlock
				chanelCompareInfo[chanel_a] = 0;
				//clear self color
				LedDataSet(chanel_a, LED_COLOR_NONE);
				sdweSetWeightBackColor(chanel_a, LED_COLOR_NONE);

				if((sortArry_num-2) == compare_i)
				{
					//chanel unlock,other..,color unlock
					chanelCompareInfo[chanel_b] = 0;
					//clear self color
					LedDataSet(chanel_b, LED_COLOR_NONE);
					sdweSetWeightBackColor(chanel_b, LED_COLOR_NONE);
				}
			}
		}
	}
}
//==update color
void useWeightUpdateLedAndSdweColor(UINT8 hx711DataUpgrade)
{
	enumHX711ChanelType chanel = HX711Chanel_1;
	enumHX711ChanelType arry[HX711_CHANEL_NUM];
	enumLedSeqType ledSeq = LED_SEQ_1; 	
	enumLedColorType color = LED_COLOR_REG ;
	float weight[HX711_CHANEL_NUM]={0.0};
	//static float preWeight[HX711_CHANEL_NUM]={0.0};
	//if weight data changed
	if(1 == hx711DataUpgrade)
	{
		//get each chanel weight
		for(chanel = HX711Chanel_1;chanel<HX711_CHANEL_NUM;chanel++)
		{
			arry[chanel] = chanel;
			weight[chanel] = hx711_getWeight(chanel);
		}
		//sequence
		BubbleSort(weight,arry,HX711_CHANEL_NUM);
		//
		for(ledSeq = LED_SEQ_1;ledSeq<(LED_SEQ_NUM-1);ledSeq++)
		{
			if(((weight[ledSeq+1] - weight[ledSeq]) < CHANEL_MAX_ERR_RANGE) &&
				((weight[ledSeq+1] - weight[ledSeq]) > -CHANEL_MAX_ERR_RANGE) &&
				((weight[ledSeq] < -CHANEL_MAX_ERR_RANGE) || (weight[ledSeq] > CHANEL_MAX_ERR_RANGE)) &&
				((weight[ledSeq+1] < -CHANEL_MAX_ERR_RANGE) || (weight[ledSeq+1] > CHANEL_MAX_ERR_RANGE)) )
			{
				LedDataSet((enumLedSeqType)arry[ledSeq], color);//light same color
				sdweSetWeightBackColor(arry[ledSeq], color);
				LedDataSet((enumLedSeqType)(arry[ledSeq+1]), color);//light same color
				sdweSetWeightBackColor((enumLedSeqType)(arry[ledSeq+1]), color);//light same color
				ledSeq++;
				color++;
			}
			else
			{
				LedDataSet((enumLedSeqType)arry[ledSeq], LED_COLOR_NONE);//not light
				sdweSetWeightBackColor(arry[ledSeq], LED_COLOR_NONE);//not light
				if((LED_SEQ_NUM-2) == ledSeq)
				{
					LedDataSet((enumLedSeqType)(arry[ledSeq+1]), LED_COLOR_NONE);//not light
					sdweSetWeightBackColor((enumLedSeqType)(arry[ledSeq+1]), LED_COLOR_NONE);//not light
				}
			}
		}
	}
}
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
//==led cycle contrl
void led_MainFunction(UINT8 hx711DataUpgrade)
{
	static UINT8 led_data[LED_CTRL_DATA_LEN]={0};
	UINT8 *pData=&g_led_ctrl_data[0];
	UINT8 i = 0,j = 0,set = 0,l_data = 0;

	//if weight data changed
	if(1 == hx711DataUpgrade)
	{	
		//check data change and store g_led_ctrl_data
		for(i=0;i<LED_CTRL_DATA_LEN;i++)
		{
			if(led_data[i] != pData[i])
			{
				led_data[i] = pData[i];
				set = 1;
			}
		}
		
		//if data changed
		if(1 == set)
		{
			for(i=0;i<LED_CTRL_DATA_LEN;i++)//byte
			{
				l_data = led_data[i];	
				for(j=0;j<8;j++)//8 bit
				{		
					//set SER
					if(0x01 == (l_data&0x01))
					{
						hal_gpio_set_do_high( LED_DO_SER0 );
					}
					else
					{
						hal_gpio_set_do_low( LED_DO_SER0 );
					}
					l_data>>=1;
		
					//send SCK shift data
					LedCtrlSendPulse( LED_DO_SRCLK ,1); 
				}		
			}
			
			//send RCK lock data
			LedCtrlSendPulse( LED_DO_RCLK ,1);	
		}
	
	}
}


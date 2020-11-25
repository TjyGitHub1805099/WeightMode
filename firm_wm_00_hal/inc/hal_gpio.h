/**
  *******************************************************************
  * @file    hal_gpio.h
  * @author  MKdriver
  * @version V1.0.0
  * @date    9-Oct-2015
  * @brief   GPIO头文件(抽象层)
  *******************************************************************
  * @attention
  *
  * 
  *
  * <h2><center>&copy; COPYRIGHT 2015 MKdriver</center></h2>
  *******************************************************************
  */
#ifndef __HAL_GPIO_H__
#define __HAL_GPIO_H__

#include "drv_gpio.h"

//test
#define TEST_GPIO_PORT				GPIOB
#define TEST_GPIO_CLK				RCC_AHBENR_GPIOBEN
#define TEST_GPIO_PIN				GPIO_PIN_12

//=============================================LED DO口线配置
//LED_SER0
#define DO0_GPIO_PORT				GPIOC
#define DO0_GPIO_CLK				RCC_AHBENR_GPIOCEN
#define DO0_GPIO_PIN				GPIO_PIN_0

//LED_OE
#define DO1_GPIO_PORT				GPIOC
#define DO1_GPIO_CLK				RCC_AHBENR_GPIOCEN
#define DO1_GPIO_PIN				GPIO_PIN_2

//LED_RCLK
#define DO2_GPIO_PORT				GPIOC
#define DO2_GPIO_CLK				RCC_AHBENR_GPIOCEN
#define DO2_GPIO_PIN				GPIO_PIN_3

//LED_SRCLK
#define DO3_GPIO_PORT				GPIOC
#define DO3_GPIO_CLK				RCC_AHBENR_GPIOCEN
#define DO3_GPIO_PIN				GPIO_PIN_4

//LED_SRCLR
#define DO4_GPIO_PORT				GPIOC
#define DO4_GPIO_CLK				RCC_AHBENR_GPIOCEN
#define DO4_GPIO_PIN				GPIO_PIN_5

//=============================================HX711 CLK 配置
//HX711_CLK_1
#define DO5_GPIO_PORT				GPIOA
#define DO5_GPIO_CLK				RCC_AHBENR_GPIOAEN
#define DO5_GPIO_PIN				GPIO_PIN_0

//HX711_CLK_2
#define DO6_GPIO_PORT				GPIOA
#define DO6_GPIO_CLK				RCC_AHBENR_GPIOAEN
#define DO6_GPIO_PIN				GPIO_PIN_1

//HX711_CLK_3
#define DO7_GPIO_PORT				GPIOA
#define DO7_GPIO_CLK				RCC_AHBENR_GPIOAEN
#define DO7_GPIO_PIN				GPIO_PIN_2

//HX711_CLK_4
#define DO8_GPIO_PORT				GPIOA
#define DO8_GPIO_CLK				RCC_AHBENR_GPIOAEN
#define DO8_GPIO_PIN				GPIO_PIN_3

//HX711_CLK_5
#define DO9_GPIO_PORT				GPIOA
#define DO9_GPIO_CLK				RCC_AHBENR_GPIOAEN
#define DO9_GPIO_PIN				GPIO_PIN_4

//HX711_CLK_6
#define DO10_GPIO_PORT			GPIOA
#define DO10_GPIO_CLK				RCC_AHBENR_GPIOAEN
#define DO10_GPIO_PIN				GPIO_PIN_5

//HX711_CLK_7
#define DO11_GPIO_PORT			GPIOA
#define DO11_GPIO_CLK				RCC_AHBENR_GPIOAEN
#define DO11_GPIO_PIN				GPIO_PIN_6

//HX711_CLK_8
#define DO12_GPIO_PORT			GPIOA
#define DO12_GPIO_CLK				RCC_AHBENR_GPIOAEN
#define DO12_GPIO_PIN				GPIO_PIN_7

//=============================================LED DI口线配置
//LED_QH3
#define DI0_GPIO_PORT				GPIOC
#define DI0_GPIO_CLK				RCC_AHBENR_GPIOCEN
#define DI0_GPIO_PIN				GPIO_PIN_1

//B13
#define DI1_GPIO_PORT				GPIOB
#define DI1_GPIO_CLK				RCC_AHBENR_GPIOBEN
#define DI1_GPIO_PIN				GPIO_PIN_13

//B14
#define DI2_GPIO_PORT				GPIOB
#define DI2_GPIO_CLK				RCC_AHBENR_GPIOBEN
#define DI2_GPIO_PIN				GPIO_PIN_14

//B15
#define DI3_GPIO_PORT				GPIOB
#define DI3_GPIO_CLK				RCC_AHBENR_GPIOBEN
#define DI3_GPIO_PIN				GPIO_PIN_15

//=============================================HX711 DATA
//HX711_DATA_1
#define DI4_GPIO_PORT				GPIOB
#define DI4_GPIO_CLK				RCC_AHBENR_GPIOBEN
#define DI4_GPIO_PIN				GPIO_PIN_0

//HX711_DATA_2
#define DI5_GPIO_PORT				GPIOB
#define DI5_GPIO_CLK				RCC_AHBENR_GPIOBEN
#define DI5_GPIO_PIN				GPIO_PIN_1

//HX711_DATA_3
#define DI6_GPIO_PORT				GPIOB
#define DI6_GPIO_CLK				RCC_AHBENR_GPIOBEN
#define DI6_GPIO_PIN				GPIO_PIN_2

//HX711_DATA_4
#define DI7_GPIO_PORT				GPIOB
#define DI7_GPIO_CLK				RCC_AHBENR_GPIOBEN
#define DI7_GPIO_PIN				GPIO_PIN_3

//HX711_DATA_5
#define DI8_GPIO_PORT				GPIOB
#define DI8_GPIO_CLK				RCC_AHBENR_GPIOBEN
#define DI8_GPIO_PIN				GPIO_PIN_4

//HX711_DATA_6
#define DI9_GPIO_PORT				GPIOB
#define DI9_GPIO_CLK				RCC_AHBENR_GPIOBEN
#define DI9_GPIO_PIN				GPIO_PIN_5

//HX711_DATA_7
#define DI10_GPIO_PORT			GPIOB
#define DI10_GPIO_CLK				RCC_AHBENR_GPIOBEN
#define DI10_GPIO_PIN				GPIO_PIN_6

//HX711_DATA_8
#define DI11_GPIO_PORT			GPIOB
#define DI11_GPIO_CLK				RCC_AHBENR_GPIOBEN
#define DI11_GPIO_PIN				GPIO_PIN_7

//=============================================KEY
//B8
#define DI12_GPIO_PORT			GPIOB
#define DI12_GPIO_CLK				RCC_AHBENR_GPIOBEN
#define DI12_GPIO_PIN				GPIO_PIN_8

//B9
#define DI13_GPIO_PORT			GPIOB
#define DI13_GPIO_CLK				RCC_AHBENR_GPIOBEN
#define DI13_GPIO_PIN				GPIO_PIN_9

//output gpio type
typedef enum DoLineType
{
	  LED_DO_SER0=0,
    LED_DO_OE,
    LED_DO_RCLK,
    LED_DO_SRCLK,
    LED_DO_SRCLR,
	  HX711_CLK_1,
	  HX711_CLK_2,
	  HX711_CLK_3,
	  HX711_CLK_4,
	  HX711_CLK_5,
	  HX711_CLK_6,
	  HX711_CLK_7,
	  HX711_CLK_8,
	  DO_GPIO_NUMBER
}enumDoLineType;

//input gpio type
typedef enum DiLineType
{
	  LED_DI_QH3 = 0,       /**< LED数据输入 */
		LED_B13,
		LED_B14,
		LED_B15,
	  HX711_DATA_1,
	  HX711_DATA_2,
	  HX711_DATA_3,
	  HX711_DATA_4,
	  HX711_DATA_5,
	  HX711_DATA_6,
	  HX711_DATA_7,
	  HX711_DATA_8,
		SYS_KEY_1,
		SYS_KEY_2,
    DI_GPIO_NUMBER
}enumDiLineType;

typedef struct
{
    UINT16 PulseModeL : 1;
    UINT16 PulseModeH : 1;
    UINT16 ExtPulseDir : 1;
    UINT16 SelectUartCan : 1;
    UINT16 SelectRs485Rs422 : 1;
    UINT16 SelectEzEqepZ : 1;
    UINT16 EqeqDir : 1;
    UINT16 Warn : 1;
    UINT16 Fan : 1;
    UINT16 Res : 7;
}structDoStatusType;

typedef union
{
    structDoStatusType bits;
    UINT16 all;
}DoStatusType;


extern void wm_hal_gpio_init( void );
extern UINT8 hal_di_get( UINT8 offset );
extern void hal_gpio_set_do_high( enumDoLineType Do );
extern void hal_gpio_set_do_low( enumDoLineType Do );
#endif

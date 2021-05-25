
#include "typedefine.h"
#include "hal_clock.h"
#include "hal_delay.h"
#include "hal_timer.h"
#include "hal_gpio.h"
#include "hal_uart.h"
#include "drv_cpu.h"
#include "app_main_task.h"
#include "app_led_ctrl.h"
#include "app_sdwe_ctrl.h"
#include "app_hx711_ctrl.h"
#include "app_key_ctrl.h"

/*定义STM32 MCU的类型*/
typedef enum{
STM32F0,
STM32F1,
STM32F2,
STM32F3,
STM32F4,
STM32F7,
STM32L0,
STM32L1,
STM32L4,
STM32H7,
}MCUTypedef;
 
UINT32 idAddr[]={0x1FFFF7AC,/*STM32F0唯一ID起始地址*/
0x1FFFF7E8,/*STM32F1唯一ID起始地址*/
0x1FFF7A10,/*STM32F2唯一ID起始地址*/
0x1FFFF7AC,/*STM32F3唯一ID起始地址*/
0x1FFF7A10,/*STM32F4唯一ID起始地址*/
0x1FF0F420,/*STM32F7唯一ID起始地址*/
0x1FF80050,/*STM32L0唯一ID起始地址*/
0x1FF80050,/*STM32L1唯一ID起始地址*/
0x1FFF7590,/*STM32L4唯一ID起始地址*/
0x1FF0F420};/*STM32H7唯一ID起始地址*/
 
/*获取MCU的唯一ID*/
UINT32 STM32McuId[3];
void STM32MCUIDGet(UINT32 *id,MCUTypedef type)
{
	if(id!=0)
	{
		id[0]=*(UINT32*)(idAddr[type]);
		id[1]=*(UINT32*)(idAddr[type]+4);
		id[2]=*(UINT32*)(idAddr[type]+8);
	}
}

INT32 g_passWordId = 0;
INT32 g_passWordStore = 0;
INT32 g_sysLocked = STM32MCU_LOCKED;

UINT8 STM32CheckPassWord(INT32 passwordIn)
{
	UINT8 ret = FALSE;
	INT32 passwordBase=(STM32McuId[0]+STM32McuId[1]+STM32McuId[2])&0xffff;
	//
	g_passWordId = passwordBase;//display at screen
	//
	passwordBase = (passwordBase * 2021 ) &0xfff;
	if(passwordIn == passwordBase)
	{
		g_passWordStore = passwordIn;
		g_sysLocked = STM32MCU_UNLOCKED;
		ret = TRUE;
	}
	return ret;
}

/**
 * @brief  系统初始化
 * @retval 无
 */
void system_init( void )
{
    UINT32 l_VectStartAddress = FLASH_BASE, l_VectOffset = 0;

#ifdef DEBUG_RAM
    l_VectStartAddress = SRAM_BASE;
#endif	

#ifdef PROJ_BOOTLOADER
    l_VectOffset = 0x4000;				// 16KB
#endif

    hal_clock_init( l_VectStartAddress + l_VectOffset );
    hal_delay_init( );
	
	//weight mode gpio init
	wm_hal_gpio_init();
	
    hal_timer_init( 1000 );
    //wdg_init( 4000 );
}

/**
 * @brief  系统主函数
 * @retval 无
 */
int main(void)
{
	system_init();
	key_init();
	led_init();
	hx711_init();
	sdwe_init();
	readSysDataFromFlash();
	readSysDataFromFlash_3030();
	STM32MCUIDGet(&STM32McuId[0],STM32F1);
	STM32CheckPassWord(g_passWordStore);
	while(1)
	{}
}


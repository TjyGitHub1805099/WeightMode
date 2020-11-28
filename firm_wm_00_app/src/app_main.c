
#include "typedefine.h"
#include "hal_clock.h"
#include "hal_delay.h"
#include "hal_timer.h"
#include "hal_gpio.h"
#include "hal_uart.h"
#include "drv_cpu.h"
#include "app_led_ctrl.h"
#include "app_sdwe_ctrl.h"
#include "app_hx711_ctrl.h"
#include "app_key_ctrl.h"

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
	while(1)
	{}
}


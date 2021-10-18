#ifndef __APP_MAIN_TASK_H__
#define __APP_MAIN_TASK_H__

#include "typedefine.h"
#include "app_led_ctrl.h"

#define COLOR_ALT_20210328_DEFINE	(FALSE)
#define COLOR_ALT_20210414_DEFINE	(FALSE)
#define COLOR_ALT_20210427_DEFINE	(FALSE)
#define COLOR_ALT_20210606_DEFINE	(FALSE)

#define SYS_HX711_ONLINE_CHECK_TIME	(2000)//when power on 2000ms start check HX711
#define SYS_REMOVE_WEIGHT_TIME		(3300)//when power on 3300ms remove weight
#define SYS_POWER_REDAY_TIME		(3500)//when power on 3500ms send data to T5L , do not change

#endif

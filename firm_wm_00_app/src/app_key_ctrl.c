#include "app_main_task.h"
#include "app_key_ctrl.h"

SysKeyType SysKey[SYS_KEY_NUM];

//key init
void key_init(void)
{
	UINT8 i = 0 ;
	SysKeyType *pSysKeyType = &SysKey[0];
	for(i=0;i<SYS_KEY_NUM;i++)
	{
		pSysKeyType[i].type = (enumDiLineType)(SYS_KEY_1+i);
		pSysKeyType[i].count = 0;
		pSysKeyType[i].preSample = SYS_KEY_INVALUED ;
		pSysKeyType[i].curSample = SYS_KEY_INVALUED ;
		pSysKeyType[i].keyOutput = SYS_KEY_INVALUED;
		pSysKeyType[i].initFlag = TRUE;
	}
}

//key filter
void key_MainFunction(void)
{
	UINT8 i = 0;
	SysKeyType *pSysKeyType = &SysKey[0];
	//
	for(i=0;i<SYS_KEY_NUM;i++)
	{
		pSysKeyType[i].curSample = hal_di_get(pSysKeyType[i].type);
		//
		if( pSysKeyType[i].preSample == pSysKeyType[i].curSample)
		{
			pSysKeyType[i].count++;
			//output
			if( pSysKeyType[i].count >= SYS_KEY_FILTER_NUM )
			{
				pSysKeyType[i].count = SYS_KEY_FILTER_NUM;
				pSysKeyType[i].keyOutput = pSysKeyType[i].curSample;
			}
		}
		else
		{
			pSysKeyType[i].count = 0 ;
		}
		pSysKeyType[i].preSample = pSysKeyType[i].curSample ;
	}
}

UINT8 key_FilterGet(enumDiLineType type)
{
	UINT8 value = SYS_KEY_INVALUED;
	if( (type >= SYS_KEY_1) && (type < (SYS_KEY_1+SYS_KEY_NUM) ) )
	{
		value =  SysKey[type-SYS_KEY_1].keyOutput;
	}
	return value;
}

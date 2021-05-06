
#ifndef __APP_SDWE_H__
#define __APP_SDWE_H__

#include "hal_uart.h"
#include "app_hx711_ctrl.h"

#define SDWE_UART_DATA_LEN	(0X100)

//address of weight back to SDWE : (0~7)-> weight of chanel(val:g)
#define SDWE_FUNC_ASK_CHANEL_WEIGHT			(0X0160)//0x0160~0x0167

//address of COLOR back to SDWE : (0~7)-> COLOR of chanel(val=0x00:white,val=0x01:red,val=0x02:green,val=0x03:blue,val=0x04:yellowWhite)
#define SDWE_FUNC_ASK_CHANEL_WEIGHT_COLOR	(0X0168)//0x0168~0x016F



//address of COLOR back to SDWE : (0~9)-> COLOR of point of chanel set triger(val=0x00:white(not triger),val=0x01green(triger))
#define SDWE_FUNC_ASK_CHANEL_POINT_TRIG		(0X020A)//0x020A~0x0213



#define DMG_TRIGER_SAVE_SECOTOR_1			(0X01)
#define DMG_TRIGER_SAVE_SECOTOR_2			(0X02)


//==(update:20210328):DIWEN reserve (uodate to v3:2021.03.26)
#define DMG_MIN_DIFF_OF_TWO_SEND_ORDER			(10)//ms


//==(update:20210328):address of set chanel number : 0->all chanel set  ; (1~8)->single chanel set
#define DMG_FUNC_SET_CHANEL_NUM					(0X2100)

//==(update:20210328):address of reset calibration of choice chanel number : 0->all chanel set  ; (1~x)->single chanel set
#define DMG_FUNC_RESET_CALIBRATION_ADDRESS		(0X2101)
//==(update:20210328):value of reset calibration of choice chanel number:0XAEEA reset calibration
#define DMG_FUNC_RESET_CALIBRATION_VAL	 		(2021)//(0XAEEA)

//==(update:20210328):address of remove weight
#define DMG_FUNC_REMOVE_WEIGHT_ADDRESS			(0X2102)
#define DMG_FUNC_REMOVE_WEIGHT_VAL				(0XA55A)

//==(update:20210428):address of remove weight
#define DMG_FUNC_JUNPTO_CALIBRATION_ADDRESS		(0X2103)
#define DMG_FUNC_JUNPTO_CALIBRATION_VAL			(2021)
#define DMG_FUNC_JUNPTO_ACTIVE_VAL				(1202)

<<<<<<< HEAD


=======
>>>>>>> 1cea0e2fea91edc540f4538cdd162cbac04c019b
//==(update:20210328):address of set point(weight value) of chanel : (0~9)-> point of chanel set (:g)
#define DMG_FUNC_SET_CHANEL_POINT_ADDRESS		(0X2200)//0x2200~0x2209

//==(update:20210328):address of set point of chanel triger : (0~9)-> point of chanel set triger(val=0x12FE(DMG triger MCU))
#define DMG_FUNC_SET_CHANEL_POINT_TRIG_ADDRESS	(0X2500)//0x2500~0x2509
#define DMG_FUNC_SET_CHANEL_POINT_TRIG_VAL		(0X12FE)


//==(update:20210328):address of triger COLOR back to DMG : (0~9)-> COLOR of point of chanel set triger(val=0x00:white(not triger),val=0x01green(triger))
#define DMG_FUNC_ASK_CHANEL_POINT_TRIG_BACK_COLOR_ADDRESS	(0X2300)//0x2300~0x2309

//==(update:20210328):address of triger sample back to DMG : (0~9)-> COLOR of point of chanel set triger(val=0x00:white(not triger),val=0x01green(triger))
#define DMG_FUNC_ASK_CHANEL_POINT_TRIG_SAMPLE_DATA_ADDRESS	(0X2400)//0x2400~0x2409

//==(update:20210328):address of weight back to DMG : (0~5)-> weight of chanel(val:g)
#define DMG_FUNC_ASK_CHANEL_WEIGHT_ADDRESS		(0X3000)//0x3000~0x3005
//==(update:20210328):address of color back to DMG : (0~5)-> color of chanel(val:g)
#define DMG_FUNC_ASK_CHANEL_COLOR_ADDRESS		(0X3100)//0x3100~0x3105


//==(update:20210411):address of unit min max ...
#define DMG_FUNC_SET_UNIT_ADDRESS			(0X1000)//0x1000
#define DMG_FUNC_SET_MIN_RANGE_ADDRESS		(0X100A)//0x100A
#define DMG_FUNC_SET_MAX_RANGE_ADDRESS		(0X100B)//0x100B
#define DMG_FUNC_SET_ERR_RANGE_ADDRESS		(0X100C)//0x100C
#define DMG_FUNC_SET_isCascade_ADDRESS		(0X100D)//0x100D
#define DMG_FUNC_SET_isLedIndicate_ADDRESS	(0X100E)//0x100E
#define DMG_FUNC_SET_COLOR_START_ADDRESS	(0X100F)//0x100F
#define DMG_FUNC_SET_COLOR_END_ADDRESS		(0X1012)//0x1012
#define DMG_FUNC_SET_ZERO_RANGE_ADDRESS		(0X1013)//0x1013

#define DMG_FUNC_MCUID_ADDRESS				(0X1500)//0x1500
#define DMG_FUNC_PASSORD_SET_ADDRESS		(0X1510)//0x1510

//system parameter
typedef enum HX711SystemParaType
{
	HX711SystemPara_UNIT = 0,  		/**< HX711  系统设置-单位 */
	HX711SystemPara_MIN_RANGE = 1,  /**< HX711  系统设置-最小量程 */
	HX711SystemPara_MAX_RANGE = 2,  /**< HX711  系统设置-最大量程 */
	HX711SystemPara_ERR_RANGE = 3,	/**< HX711	系统设置-误差 */
	HX711SystemPara_CASCADE = 4,  	/**< HX711  系统设置-级联 */	
	HX711SystemPara_LED_DIS_EN = 5,		/**< HX711	系统设置-LED指示 */
	HX711SystemPara_COLOR1 = 6,	/**< HX711	系统设置-颜色1 */
	HX711SystemPara_COLOR2 = 7,	/**< HX711	系统设置-颜色2 */
	HX711SystemPara_COLOR3 = 8,	/**< HX711	系统设置-颜色3 */
	HX711SystemPara_COLOR4 = 9,	/**< HX711	系统设置-颜色4 */	
	HX711SystemPara_ZERO_RANGE = 10, /**< HX711	零点范围 */ 
	HX711SystemPara_NUM  			/**< HX711  系统设置-最大长度 */
}enumHX711SystemParaType;

//ask calibration page data
typedef enum CalibrationAskParaType
{
	DMG_TRIGER_SAMPLE_OF_STATUS = 0 ,		/* trigerStarus */
	DMG_TRIGER_SAMPLE_OF_ASK_COLOR = 1 ,	/* back color of point*/
	DMG_TRIGER_SAMPLE_OF_AVG_SAMPLE = 2 ,	/* avg sample of point*/
	DMG_TRIGER_SAMPLE_OF_ASK_WEIGHT = 3 ,	/* set weight of point */
	DMG_TRIGER_SAMPLE_MAX_NUM
}enumCalibrationAskParaType;
	
//==========================================================================================================================
//==================code area
//start of on board flash store address
//0x0800 0000 ~ 0x0803 0000

//===================================important:each need store data need 4 byte=============================================
//==================parameter1:system control of unit , min , max , err , cascade
//start of on board flash store address
//0X0803E000 ~ 0X0803E7FF
//start of on board sys para flash store address
#define FLASH_SYS_PARA_STORE_ADDRESS_START				(0X0803E000)
#define FLASH_SYS_PASSWORD_ADDRESS_START	FLASH_SYS_PARA_STORE_ADDRESS_START
#define FLASH_SYS_PASSWORD_ADDRESS_LED		(4)
#define FLASH_SYS_PASSWORD_ADDRESS_END		(FLASH_SYS_PASSWORD_ADDRESS_START+FLASH_SYS_PASSWORD_ADDRESS_LED)
//unit:g or ml
#define FLASH_SYS_UNIT_ADDRESS_START	FLASH_SYS_PASSWORD_ADDRESS_END
#define FLASH_SYS_UNIT_LEN				(HX711SystemPara_NUM*4)
#define FLASH_SYS_UNIT_ADDRESS_END		(FLASH_SYS_UNIT_ADDRESS_START+FLASH_SYS_UNIT_LEN)

//end of on board sys para flash store address
#define FLASH_SYS_PARA_STORE_ADDRESS_END					(FLASH_SYS_UNIT_ADDRESS_END)

//store flash data : PASSWORD unit , min , max , cascade ,... , crc
#define FLASH_SYS_PARA_STORE_MAX_LEN						(((FLASH_SYS_PARA_STORE_ADDRESS_END-FLASH_SYS_PARA_STORE_ADDRESS_START)/4)+1)

//==========================================================================================================================
//==================parameter2:HX711 point sample , weight value , K ,B ,weightRemove,weightDir
//start of on board flash store address
//0X0803F000 ~ 0X0803F7FF
//start of on board flash store address
#define FLASH_STORE_ADDRESS_START				(0X0803F000)

//each chanel have 10 point : HX711_CHANEL_NUM*10*8(sample + weight value) byte
#define FLASH_CHANEL_POINT_ADDRESS_START		(FLASH_STORE_ADDRESS_START)
#define FLASH_CHANEL_POINT_LEN					(HX711_CHANEL_NUM*CHANEL_POINT_NUM*8)
#define FLASH_CHANEL_POINT_ADDRESS_END			(FLASH_CHANEL_POINT_ADDRESS_START+FLASH_CHANEL_POINT_LEN)

//each chanel have 10 point -> 11 KB : HX711_CHANEL_NUM*11*8(K + B) byte
#define FLASH_CHANEL_POINT_KB_ADDRESS_START		(FLASH_CHANEL_POINT_ADDRESS_END)
#define FLASH_CHANEL_POINT_KB_LEN				(HX711_CHANEL_NUM*(CHANEL_POINT_NUM+1)*8)
#define FLASH_CHANEL_POINT_KB_ADDRESS_END		(FLASH_CHANEL_POINT_KB_ADDRESS_START+FLASH_CHANEL_POINT_KB_LEN)

//each chanel remove weight:HX711_CHANEL_NUM*4
#define FLASH_CHANEL_POINT_RMW_ADDRESS_START	(FLASH_CHANEL_POINT_KB_ADDRESS_END)
#define FLASH_CHANEL_POINT_RMW_LEN				(HX711_CHANEL_NUM*4)
#define FLASH_CHANEL_POINT_RMW_ADDRESS_END		((FLASH_CHANEL_POINT_RMW_ADDRESS_START)+FLASH_CHANEL_POINT_RMW_LEN)

//each chanel sensor direction :HX711_CHANEL_NUM*4
#define FLASH_CHANEL_SERNSER_DIR_ADDRESS_START	(FLASH_CHANEL_POINT_RMW_ADDRESS_END)
#define FLASH_CHANEL_SERNSER_DIR_LEN			(HX711_CHANEL_NUM*4)
#define FLASH_CHANEL_SERNSER_DIR_ADDRESS_END	((FLASH_CHANEL_SERNSER_DIR_ADDRESS_START)+FLASH_CHANEL_SERNSER_DIR_LEN)

//end of on board flash store address
#define FLASH_STORE_ADDRESS_END					(FLASH_CHANEL_SERNSER_DIR_ADDRESS_END)

//store flash data : HX711_CHANEL_NUM * (sample value , weight value , k , b , remove value , weightDir ) , crc
#define FLASH_STORE_MAX_LEN						(((FLASH_STORE_ADDRESS_END-FLASH_STORE_ADDRESS_START)/4)+1)
//==========================================================================================================================



typedef enum sdweRxFuncIdType
{
	/**< SDWE_RX_0X83 举例
	1：A5 5A 06 83 01 FF 01 00 01 ; 代表 add = 0x01ff(校准通道号选择) , len = 0x01 , data = 0x0001 
	解释：校准通道选择 (add=0x01ff , len = 1), data=0:所有通道 data=1~8:代表具体通道
	2:A5 5A 06 83 03 00 01 00 0A
	解释：对于通道下的校准点选择 (add=0x0300 , len = 1), data=1~11:具体点(十段总共11点)
	*/
	SDWE_RX_FUN_HEAD1 = 0X5A, /**< SDWE HEAD1*/
	SDWE_RX_FUN_HEAD2 = 0XA5, /**< SDWE HEAD2*/
	SDWE_RX_FUN_0X83 = 0X83, /**< SDWE 设置变量 下发给MCU*/
	SDWE_RX_FUN_NUM	 		 /**< SDWE 总数量*/
}enumsdweRxFuncIdType;


typedef enum
{
	cmdWriteSWDERegister = 0x80 ,
	cmdReadSWDERegister = 0x81 ,
	cmdWriteSWDEVariable = 0x82 ,
	cmdReadSWDEVariable = 0x83 ,
}enumSDWEcmdType;

typedef enum
{
	cmdPosHead1  = 0 ,//A5
	cmdPosHead2  = 1 ,//5A
	cmdPosDataLen= 2 ,//last data len
	cmdPosCommand= 3 ,//command position

	//=======MCU->SDWE order
	//read register 
	cmdPosRegReadAddress= 4 ,//reg address one byte position
	cmdPosRegReadLen= 5 ,//reg address one byte position
	//write register 
	cmdPosRegWriteAddress= 4 ,//reg address one byte position
	cmdPosRegWritesData= 5 ,//reg address one byte position

	//read varible 
	cmdPosVarReadAddress1= 4 ,//val address two byte position
	cmdPosVarReadAddress2= 5 ,//val address two byte position
	cmdPosVarReadLen= 6 ,//val address two byte position
	//write varible 
	cmdPosVarWriteAddress1= 4 ,//val address two byte position
	cmdPosVarWriteAddress2= 5 ,//val address two byte position
	cmdPosVarWriteData= 6 ,//val address two byte position

	//=======SDWE->MCU order
	//read register
	cmdPosRegAddress= 4 ,//reg address one byte position
	cmdPosReadRegAskLen= 5 ,//when read data ask data len position
	cmdPosRegData= 6 ,//reg address one byte position
	//read varible
	cmdPosVarAddress1= 4 ,//val address two byte position
	cmdPosVarAddress2= 5 ,//val address two byte position
	cmdPosReadVarAskLen= 6 ,//when read data ask data len position
	cmdPosVarData1= 7 ,//val address two byte position
}enumSDWEcmdPosType;


/** 定义从机串口设备类型 */
typedef struct structSdweType
{
	UINT8 	sendSdweInit;
	UINT8 	readSdweInit;
	UartDeviceType *pUartDevice;        /**< 串口设备 */
	UINT8 	version;//SDWE version
	UINT8 	allowCompare;
	UINT8 	rxData[SDWE_UART_DATA_LEN];
	UINT8 	txData[SDWE_UART_DATA_LEN];
	UINT16	RxLength;					/**< 接收字节数 */
	UINT8 	RxFinishFlag;				/**< 接收完成标志 */
	
	UINT16  sdweSetAdd;/**< 地址 */
	INT16  	sdwetDataLen;/**< 数据长度 */
	INT16  	sdweSetData;/**< 数据 */

	UINT16 	sdweRemoveWeightTriger;/**< 去皮 */
	UINT16 	sdwePointTriger;/**< 点触发校准 */
	UINT16 	sdweResetTriger;/**< 重新校准 */
	UINT16 	sdweResetTrigerValid;/**< 重新校准有效 */
	UINT16 	sdweChanelChanged;/**< 通道改变 */
	UINT16 	sdweColorClen;/**< 通道改变时清颜色 */
	UINT16 	sdweCalChanel;/**< 通道 */
	UINT16 	sdweCalPoint;/**< 校准点 */
	INT32 	sdweCalPointArry[CHANEL_POINT_NUM];/**< 校准点数组 */
	UINT32	sdweTick;
	UINT32	sdweLastSendTick;
	UINT16 	sdweJumpToCalitrationPage;/**< 跳转至校准页面 */
<<<<<<< HEAD
	UINT16	sdweJumpToHomePage;
	UINT16	sdweJumpToBanlingPage;
	UINT16 	sdweJumpToActivePage;/**< 跳转至激活界面 */
=======
	UINT16 	sdweJumpToActivePage;/**< 跳转至激活码 */
	UINT16	sdweJumpToHomePage;
	UINT16	sdweJumpToBanlingPage;
>>>>>>> 1cea0e2fea91edc540f4538cdd162cbac04c019b
}SdweType;

/** ModbusRtu设备默认配置 */
#define SdweDefault   { \
	0,\
	0,\
	&g_UartDevice[UART_EXTERN], \
	0,\
	FALSE,\
	{0}, \
	{0}, \
	0,\
	0,\
	0XFFFF,\
	0,\
	0,\
	0,\
	0,\
	0,\
	0,\
	0,\
	0,\
	88,\
	0,\
	{0},\
	0,\
	0,\
	0,\
	0,\
	0,\
	0,\
	}

#define SDWE_WEIGHR_DATA_LEN (2*HX711_CHANEL_NUM)//8 weight data + 8 color data	


extern void sdwe_init(void);
extern void sdwe_test(void);
extern void sdweSetWeightBackColor(UINT8 seq,UINT8 color);
extern void sdwe_MainFunction(UINT8 hx711DataUpgrade);
extern void readSysDataFromFlash(void);





//===================sys para functions
extern UINT32 getSysPara(enumHX711SystemParaType offset);
extern void setSysPara(enumHX711SystemParaType offset,UINT32 val);
extern void readSysParaFromFlsh(void);
extern void storeSysParaFromFlsh(void);
extern void readSysParaFromFlsh_3030(void);
extern void storeSysParaFromFlsh_3030(void);

extern void color_clearAllColor();


extern SdweType g_sdwe;

#endif

#include "bsp_gpio.h"

GPIO_TypeDef *bsp_gpio[64]={GPIOE,GPIOE,GPIOE,GPIOE,GPIOE,GPIOE,GPIOB,GPIOB,GPIOB,GPIOB,
														GPIOB,GPIOB,GPIOB,GPIOG,GPIOG,GPIOG,GPIOG,GPIOG,GPIOG,GPIOG,
														GPIOD,GPIOD,GPIOD,GPIOD,GPIOD,GPIOD,GPIOD,GPIOD,GPIOC,GPIOC,
														GPIOC,GPIOA,GPIOB,GPIOB,GPIOE,GPIOE,GPIOE,GPIOE,GPIOE,GPIOE,
														GPIOE,GPIOE,GPIOE,GPIOG,GPIOG,GPIOF,GPIOF,GPIOF,GPIOF,GPIOF,
														GPIOB,GPIOB,GPIOB,GPIOC,GPIOC,GPIOA,GPIOA,GPIOA,GPIOA,GPIOA,
														GPIOC,GPIOC,GPIOC,GPIOC};

uint32_t bsp_pin[64]={GPIO_PIN_5,GPIO_PIN_4,GPIO_PIN_3,GPIO_PIN_2,GPIO_PIN_1,GPIO_PIN_0,GPIO_PIN_9,GPIO_PIN_8,GPIO_PIN_7,GPIO_PIN_6,
											GPIO_PIN_5,GPIO_PIN_4,GPIO_PIN_3,GPIO_PIN_15,GPIO_PIN_14,GPIO_PIN_13,GPIO_PIN_12,GPIO_PIN_11,GPIO_PIN_10,GPIO_PIN_9,
											GPIO_PIN_7,GPIO_PIN_6,GPIO_PIN_5,GPIO_PIN_4,GPIO_PIN_3,GPIO_PIN_2,GPIO_PIN_1,GPIO_PIN_0,GPIO_PIN_12,GPIO_PIN_11,
											GPIO_PIN_10,GPIO_PIN_15,GPIO_PIN_11,GPIO_PIN_10,GPIO_PIN_15,GPIO_PIN_14,GPIO_PIN_13,GPIO_PIN_12,GPIO_PIN_11,GPIO_PIN_10,
											GPIO_PIN_9,GPIO_PIN_8,GPIO_PIN_7,GPIO_PIN_1,GPIO_PIN_0,GPIO_PIN_15,GPIO_PIN_14,GPIO_PIN_13,GPIO_PIN_12,GPIO_PIN_11,
											GPIO_PIN_2,GPIO_PIN_1,GPIO_PIN_0,GPIO_PIN_5,GPIO_PIN_4,GPIO_PIN_7,GPIO_PIN_6,GPIO_PIN_5,GPIO_PIN_4,GPIO_PIN_1,
											GPIO_PIN_3,GPIO_PIN_2,GPIO_PIN_1,GPIO_PIN_0}; 

uint32_t detect_pin[8]={GPIO_PIN_0,GPIO_PIN_1,GPIO_PIN_2,GPIO_PIN_3,GPIO_PIN_4,GPIO_PIN_5,GPIO_PIN_6,GPIO_PIN_7};//GPIOF
uint32_t line_pin[8]={GPIO_PIN_8,GPIO_PIN_9,GPIO_PIN_10,GPIO_PIN_11,GPIO_PIN_12,GPIO_PIN_13,GPIO_PIN_14,GPIO_PIN_15};//GPIOD

uint8_t lockStatus[64];
uint8_t stopUnolck;//停止开锁标志
uint8_t lastUnlock;//记录暂停开锁当前锁号
uint8_t busyStatus;//当前正在开锁标志

button_sta STATUS;//按钮当前状态

uint16_t lockCnt;

extern uint8_t rx1Buff[1024];

extern uint8_t displaylock;

extern UART_HandleTypeDef huart1;
extern IWDG_HandleTypeDef hiwdg;

uint8_t delock(uint8_t lock)
{
	uint8_t res;
	uint8_t lineNUM = lock%8;
	if(lineNUM == 0)
		lineNUM = 8;
	HAL_GPIO_WritePin(GPIOD, line_pin[(lock-1)/8], GPIO_PIN_RESET);
	res = HAL_GPIO_ReadPin(GPIOF, detect_pin[lineNUM-1]);
	HAL_GPIO_WritePin(GPIOD, line_pin[(lock-1)/8], GPIO_PIN_SET);
//	mylog("%dlock:%d",lock,res);
	res = !res;
	return res;
}

uint8_t unLock(uint8_t lock)
{
	displaylock = lock;
	SETLED_ON;
	if(lock<=64)
	{
		HAL_GPIO_WritePin(bsp_gpio[lock-1], bsp_pin[lock-1], GPIO_PIN_SET);
		lockCnt = 1;
		while(lockCnt)
		{
			BSP_Display(CABINETNO,displaylock);
		}
		HAL_GPIO_WritePin(bsp_gpio[lock-1], bsp_pin[lock-1], GPIO_PIN_RESET);
		displaylock = 0;
		SETLED_OFF;
		return delock(lock);
	}
	else
		return 0;
}

void allLine_Up(void)
{
	uint8_t i;
	for(i=0;i<8;i++)
	{
		HAL_GPIO_WritePin(GPIOD, line_pin[i], GPIO_PIN_SET);
	}
}
extern uint8_t errPlaceSlot;//记录有多少错位的货道
extern uint8_t PlaceSlot[160];//记录错位的货道
void allCheck(void)
{
	uint8_t i;
	uint8_t j;
	errPlaceSlot = 0;
	memset(lockStatus,0,sizeof(lockStatus));
//	mylog("start check");
	for(i=0;i<8;i++)
	{
		HAL_GPIO_WritePin(GPIOD, line_pin[i], GPIO_PIN_RESET);
		HAL_Delay(10);
		for(j=0;j<8;j++)
		{
			lockStatus[i*8+j] = HAL_GPIO_ReadPin(GPIOF,detect_pin[j]);
//			mylog("%dlock:%d",i*8+j,lockStatus[i*8+j]);
			if(lockStatus[i*8+j] == 0)
			{
				PlaceSlot[errPlaceSlot++] = i*8+j+1;
			}				
		}
		HAL_GPIO_WritePin(GPIOD, line_pin[i], GPIO_PIN_SET);
	}
//	mylog("stop check");
}

void allOpen(void)//全开锁
{
	uint8_t i;
	busyStatus = 1;//先关闭心跳
	printf("OpenRelay");//副芯片打开继电器
	HAL_UART_Transmit(&huart1,"OpenRelay",strlen("OpenRelay"), 0xFFFF);
	HAL_Delay(200);
	if(strstr((char *)rx1Buff,"OpenRelayOK"))
	{
		for(i=lastUnlock;i<64;i++)
		{
				HAL_UART_Transmit(&huart1,"OpenRelay",strlen("OpenRelay"), 0xFFFF);
				unLock(i+1);
				//HAL_IWDG_Refresh(&hiwdg);
//				mylog("open==>>%d",i+1);
				if(STATUS == ALLOPEN)
				{
					busyStatus = 1;
				}
				else if(STATUS == PAUSE)
				{
					lastUnlock = i+1;
					busyStatus = 0;
					break;
				}
				else if(STATUS == RECOVER)
				{
					busyStatus = 1;
				}
				else if(STATUS == RELEASE)
				{
					break;
				}
		}
	}
	//else
	if(STATUS != PAUSE)
	{
		busyStatus = 0;
		STATUS = RELEASE;
		lastUnlock = 0;
	}
	HAL_UART_Transmit(&huart1,"CloseRelay",strlen("CloseRelay"), 0xFFFF);
}
void testfun(void)
{
	HAL_UART_Transmit(&huart1,"OpenRelay",strlen("OpenRelay"), 0xFFFF);
	HAL_Delay(200);
	//HAL_GPIO_WritePin(bsp_gpio[38], bsp_pin[38], GPIO_PIN_SET);
//	HAL_GPIO_WritePin(bsp_gpio[2], bsp_pin[2], GPIO_PIN_SET);
//	HAL_GPIO_WritePin(bsp_gpio[2], bsp_pin[2], GPIO_PIN_SET);
//	HAL_GPIO_WritePin(bsp_gpio[3], bsp_pin[3], GPIO_PIN_SET);
//	HAL_GPIO_WritePin(bsp_gpio[0], bsp_pin[0], GPIO_PIN_SET);
	HAL_Delay(1000);
////	HAL_GPIO_WritePin(bsp_gpio[38], bsp_pin[38], GPIO_PIN_RESET);
//	HAL_GPIO_WritePin(bsp_gpio[2], bsp_pin[2], GPIO_PIN_RESET);
//	HAL_GPIO_WritePin(bsp_gpio[2], bsp_pin[2], GPIO_PIN_RESET);
//	HAL_GPIO_WritePin(bsp_gpio[3], bsp_pin[3], GPIO_PIN_RESET);
//	HAL_GPIO_WritePin(bsp_gpio[0], bsp_pin[0], GPIO_PIN_RESET);
//	HAL_UART_Transmit(&huart1,"CloseRelay",strlen("CloseRelay"), 0xFFFF);
	HAL_Delay(200);
}
uint8_t testdisplay;
void test1fun(void)
{
	HAL_UART_Transmit(&huart1,"OpenRelay",strlen("OpenRelay"), 0xFFFF);
	HAL_Delay(200);
}
void test2fun(void)
{
	testdisplay = 1;
}
void test3fun(void)
{
	HAL_UART_Transmit(&huart1,"OpenRelay",strlen("OpenRelay"), 0xFFFF);
	HAL_Delay(200);
	HAL_GPIO_WritePin(bsp_gpio[1], bsp_pin[1], GPIO_PIN_SET);
	HAL_GPIO_WritePin(bsp_gpio[2], bsp_pin[2], GPIO_PIN_SET);
	HAL_GPIO_WritePin(bsp_gpio[4], bsp_pin[4], GPIO_PIN_SET);
	HAL_GPIO_WritePin(bsp_gpio[3], bsp_pin[3], GPIO_PIN_SET);
	HAL_GPIO_WritePin(bsp_gpio[0], bsp_pin[0], GPIO_PIN_SET);
	HAL_GPIO_WritePin(bsp_gpio[5], bsp_pin[5], GPIO_PIN_SET);
	HAL_Delay(1000);
	HAL_GPIO_WritePin(bsp_gpio[1], bsp_pin[1], GPIO_PIN_RESET);
	HAL_GPIO_WritePin(bsp_gpio[2], bsp_pin[2], GPIO_PIN_RESET);
	HAL_GPIO_WritePin(bsp_gpio[4], bsp_pin[4], GPIO_PIN_RESET);
	HAL_GPIO_WritePin(bsp_gpio[3], bsp_pin[3], GPIO_PIN_RESET);
	HAL_GPIO_WritePin(bsp_gpio[0], bsp_pin[0], GPIO_PIN_RESET);
	HAL_GPIO_WritePin(bsp_gpio[5], bsp_pin[5], GPIO_PIN_RESET);
	HAL_UART_Transmit(&huart1,"CloseRelay",strlen("CloseRelay"), 0xFFFF);
	HAL_Delay(200);
}
void test4fun(void)
{
	HAL_UART_Transmit(&huart1,"OpenRelay",strlen("OpenRelay"), 0xFFFF);
	HAL_Delay(200);
	HAL_GPIO_WritePin(bsp_gpio[1], bsp_pin[1], GPIO_PIN_SET);
	HAL_GPIO_WritePin(bsp_gpio[2], bsp_pin[2], GPIO_PIN_SET);
	HAL_GPIO_WritePin(bsp_gpio[4], bsp_pin[4], GPIO_PIN_SET);
	HAL_GPIO_WritePin(bsp_gpio[3], bsp_pin[3], GPIO_PIN_SET);
	HAL_GPIO_WritePin(bsp_gpio[0], bsp_pin[0], GPIO_PIN_SET);
	HAL_GPIO_WritePin(bsp_gpio[5], bsp_pin[5], GPIO_PIN_SET);
	HAL_Delay(200);
}
void test5fun(void)
{
	HAL_UART_Transmit(&huart1,"CloseRelay",strlen("CloseRelay"), 0xFFFF);
	HAL_Delay(200);
}
void test6fun(void)
{
	testdisplay = 0;
}
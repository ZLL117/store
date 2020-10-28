#ifndef __BSP_GPIO_H
#define __BSP_GPIO_H

#include "main.h"
//SET灯
#define SETLED_ON (HAL_GPIO_WritePin(GPIOG, GPIO_PIN_3, GPIO_PIN_SET))
#define SETLED_OFF (HAL_GPIO_WritePin(GPIOG, GPIO_PIN_3, GPIO_PIN_RESET))
//复位副芯片
#define RESET_ON (HAL_GPIO_WritePin(GPIOG, GPIO_PIN_4, GPIO_PIN_SET))
#define RESET_OFF (HAL_GPIO_WritePin(GPIOG, GPIO_PIN_4, GPIO_PIN_RESET))

uint8_t unLock(uint8_t lock);
void allLine_Up(void);
void allCheck(void);
uint8_t delock(uint8_t lock);
void allOpen(void);//全开锁
void testfun(void);
void test1fun(void);
void test2fun(void);
void test3fun(void);
void test4fun(void);
void test5fun(void);
void test6fun(void);

typedef enum{
	RELEASE = 0,
	ALLOPEN = 1,
	PAUSE   = 2,
	RECOVER = 3
}button_sta;

#endif

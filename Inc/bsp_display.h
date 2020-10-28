#ifndef __BSP_DISPLAY_H
#define __BSP_DISPLAY_H

#include "main.h"

#define HC595_SCLK_RESET (HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET))
#define HC595_SCLK_SET (HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET))
#define HC595_DIO_RESET (HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET))
#define HC595_DIO_SET (HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET))
#define HC595_RCLK_RESET (HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET))
#define HC595_RCLK_SET (HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET))

void HC595_SendData(uint8_t data);
void BSP_Display(uint8_t iadress,uint8_t ilock);
void Clear_Display(void);

#endif

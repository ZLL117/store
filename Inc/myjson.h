#ifndef __MYJSON_H
#define __MYJSON_H

#include "main.h"
#include "bsp_gpio.h"

void JSON_decode(uint8_t* arr,char* key);
void caseone(void);
void casetwo(void);
void JSON_Handle(uint8_t *arr);
uint8_t toHEX(void);

#endif

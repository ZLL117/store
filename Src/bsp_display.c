#include "bsp_display.h"

const uint8_t digDisplay[10]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90};//Ñô¼«

void HC595_SendData(uint8_t data)
{ 
	uint8_t i;
    for(i=0; i<8; i++) 
    {
        HC595_SCLK_RESET;
        if( (data & 0x80) == 0x80)
        {
            HC595_DIO_SET;          
        }
        else
        {
             HC595_DIO_RESET;     
        }
        data = data << 1;    
        HC595_SCLK_SET;      
    }
//    HC595_RCLK_RESET;
//    HC595_RCLK_SET;
}

void BSP_Display(uint8_t iadress,uint8_t ilock)
{
//	uint8_t DisplayArr[4];
//	DisplayArr[0] = ilock%10;
//	DisplayArr[1] = ilock/10;
//	DisplayArr[2] = iadress%10;
//	DisplayArr[3] = iadress/10;
	HC595_SendData(digDisplay[ilock/10]);
	HC595_SendData(0x01);
	HC595_RCLK_RESET;    
	HC595_RCLK_SET;
	HC595_SendData(digDisplay[ilock%10]);
	HC595_SendData(0x02);
	HC595_RCLK_RESET;    
	HC595_RCLK_SET;
	HC595_SendData(digDisplay[iadress/10]);
	HC595_SendData(0x04);
	HC595_RCLK_RESET;    
	HC595_RCLK_SET;
	HC595_SendData(digDisplay[iadress%10]);
	HC595_SendData(0x08);
	HC595_RCLK_RESET;    
	HC595_RCLK_SET;
}

void Clear_Display(void)
{
	HC595_SendData(0x00);
	HC595_SendData(0x10);
	HC595_RCLK_RESET;    
	HC595_RCLK_SET;
}

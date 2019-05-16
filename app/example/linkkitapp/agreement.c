#include "agreement.h"
#include "string.h"
#include <aos/hal/uart.h>
#include <aos/hal/gpio.h>
#include "crc.h"
#include "time.h"
#include "iot_export.h"
//#include <aos/hal/timer.h>


#define UART1_PORT_NUM  1
#define UART_BUF_SIZE   20
#define UART_TX_TIMEOUT 10
#define UART_RX_TIMEOUT 10
#define HEARTBEAT_BUF_SIZE 110
#define TIMER1_PORT_NUM 1

uart_dev_t uart1;
aos_timer_t  g_timer;
aos_timer_t  a_timer;
gpio_dev_t led;
//timer_dev_t timer1;

int count   = 0;
int ret     = -1;
int i       = 0;
int rx_size = 0;
uint8_t UpData[UART_BUF_SIZE];
char NULLARR[1];
uint16_t CRCNUM;
uint8_t idex=0x01;
uint8_t hbcmd;
uint8_t ErrorCmd;//初始是零，出错是1，重连是2
uint8_t PortPro;
uint8_t TimeInt;
uint8_t HeartPoint;
uint8_t LedPoint;


////////////////////////////////////////////下行帧数组//////////////////////////////////////////

uint8_t HeartbeatDown[10]={0xEE,0xEE,0x00,0x06,0x01,0x01,0x75,0x20,0xFF,0xFF};
uint8_t DistributionNetworkDown[10]={0xEE,0xEE,0x00,0x06,0x02,0x00,0x45,0xE1,0xFF,0xFF};
uint8_t WorkGearSetDown[10]={0xEE,0xEE,0x00,0x06,0x03,0x03,0xD4,0xA0,0xFF,0xFF};
uint8_t WaterboxSetDown[11]={0xEE,0xEE,0x00,0x07,0x04,0x14,0x14,0xBA,0x6B,0xFF,0xFF};
uint8_t RoomTemperatureSetDown[12]={0xEE,0xEE,0x00,0x08,0x05,0x14,0x14,0x14,0x1D,0xAE,0xFF,0xFF};
uint8_t WaterpumpTemperatureSetDown[10]={0xEE,0xEE,0x00,0x06,0x06,0x14,0x8A,0xE3,0xFF,0xFF};
uint8_t TimerTimeSetDown[18]={0xEE,0xEE,0x00,0x0C,0x07,0x01,0x01,0x00,0x02,0x00,0x14,0x0A,0x0A,0x0A,0x63,0x84,0xFF,0xFF};
uint8_t TimerSwitchSetDown[10]={0xEE,0xEE,0x00,0x06,0x08,0x01,0x25,0x26,0xFF,0xFF};
uint8_t ControlModeSetDown[10]={0xEE,0xEE,0x00,0x06,0x09,0x01,0xB5,0x27,0xFF,0xFF};
uint8_t SwitchSetDown[10]={0xEE,0xEE,0x00,0x06,0x0A,0x01,0x45,0x27,0xFF,0xFF};

///////////////////////////////////////////////////////////////////////////////////////////////

void Timer_Init(void)//2S发送一次心跳包
{
	ret = aos_timer_new(&g_timer, timer_handler, NULL, 1000, 1);
	ret = aos_timer_new(&a_timer, timer_handlerII, NULL, 100, 1);
}
static void timer_handler(void *arg1, void* arg2)
{
	if(HeartPoint)
	{
	AgrMentDownFun(HeartbeatCmd,NULLARR); 
	memset(UpData,0,sizeof(UpData));
	AgrMentUpHandle(HeartbeatCmd,UpData);
	}
	
}
static void timer_handlerII(void *arg1, void* arg2)
{
	if(!LedPoint)
	{
		hal_gpio_output_toggle(&led);
	}
}
void Uart1_Init(void)
{
	

    /* uart port set */
    uart1.port = UART1_PORT_NUM;

    /* uart attr config */
    uart1.config.baud_rate    = 115200;
    uart1.config.data_width   = DATA_WIDTH_8BIT;
    uart1.config.parity       = NO_PARITY;
    uart1.config.stop_bits    = STOP_BITS_1;
    uart1.config.flow_control = FLOW_CONTROL_DISABLED;
    uart1.config.mode         = MODE_TX_RX;

	ret = hal_uart_init(&uart1);
    if (ret != 0) {
        HAL_Printf("uart1 init error !\n");
	}
}
void GPIO_INIT(void)
{
	
	led.port = 2;
	led.config = OUTPUT_PUSH_PULL;
	ret = hal_gpio_init(&led);
    if (ret != 0) {
        printf("gpio init error !\n");
		}
}

///////////////////////////上行帧处理///////////////////////////////

void heartbeatbufhandle(uint8_t *rxdata)
{
	for(i=0;i<10;i++)
	{
	//ret = krhino_buf_queue_recv(&uart1, 0x10, rxdata, &rx_size);
		CRCNUM=CalCrc16(rxdata+2,rx_size-6,0xFFFF);
		if(CRCNUM==(rxdata[rx_size-4]<<8)+rxdata[rx_size-3])
		{
			hbcmd=rxdata[5];
			if(ErrorCmd==1)ErrorCmd=2;
			switch(hbcmd)
			{
				case 0x01:
				memcpy(&heartbeatbuf01,rxdata+6,10);
				//PortPro=1;
				//user_post_property();
				//HAL_Printf("AAAAAAAAAAAAAAAAAA\n");
				break;
				case 0x02:
				memcpy(&heartbeatbuf02,rxdata+6,6);
				//PortPro=2;
				break;
				case 0x03:
				memcpy(&heartbeatbuf03,rxdata+6,9);
				//PortPro=3;
				break;
				case 0x04:
				memcpy(&heartbeatbuf04,rxdata+6,9);
				//PortPro=4;
				break;
				case 0x05:
				memcpy(&heartbeatbuf05,rxdata+6,9);
				//PortPro=5;
				break;
				case 0x06:
				memcpy(&heartbeatbuf06,rxdata+6,9);
				//PortPro=6;
				break;
				case 0x07:
				memcpy(&heartbeatbuf07,rxdata+6,9);
				//PortPro=7;
				break;
				case 0x08:
				memcpy(&heartbeatbuf08,rxdata+6,9);
				//PortPro=8;
				break;
				case 0x09:
				memcpy(&heartbeatbuf09,rxdata+6,9);
				//PortPro=9;
				break;
				case 0x0A:
				memcpy(&heartbeatbuf0A,rxdata+6,9);
				//PortPro=10;
				break;
				case 0x0B:
				memcpy(&heartbeatbuf0B,rxdata+6,8);

				if(heartbeatbuf0B.ReDistributionNetwork)
				{
					LedPoint=0;
					networkdata=0;
					AgrMentDownFun(DistributionNetworkCmd,NULLARR);
					memset(UpData,0,sizeof(UpData));
					AgrMentUpHandle(DistributionNetworkCmd,UpData);
				}
				break;
				default:
				break;
			}
			break;
		}
		else
		{
			memset(rxdata,0,rx_size);
			ret = hal_uart_send(&uart1, HeartbeatDown, sizeof(HeartbeatDown), UART_TX_TIMEOUT);
			ret = hal_uart_recv_II(&uart1, rxdata, UART_BUF_SIZE,
                               &rx_size, UART_RX_TIMEOUT);
		}
	}
	if(i==10)
	{
		ErrorCmd=1;
		user_post_event();
	}
}
void DistributionUpHandle(uint8_t *rxdata)
{
	for(i=0;i<10;i++)
	{
		CRCNUM=CalCrc16(rxdata+2,4,0xFFFF);
		if(CRCNUM==(rxdata[6]<<8)+rxdata[7])
		{
			if(rxdata[5]==0xAA)break;
			else
			{
				ret = hal_uart_send(&uart1, DistributionNetworkDown, sizeof(DistributionNetworkDown), UART_TX_TIMEOUT);	
				ret = hal_uart_recv_II(&uart1, rxdata, UART_BUF_SIZE,
                               &rx_size, UART_RX_TIMEOUT);
			}
		}			
		else
		{
			ret = hal_uart_send(&uart1, DistributionNetworkDown, sizeof(DistributionNetworkDown), UART_TX_TIMEOUT);	
			ret = hal_uart_recv_II(&uart1, rxdata, UART_BUF_SIZE,
                               &rx_size, UART_RX_TIMEOUT);
		}
	}
	if(i==10)
	{
		ErrorCmd=1;
		user_post_event();
	}
}
void GearUpHandle(uint8_t *rxdata)
{
	for(i=0;i<10;i++)
	{
		CRCNUM=CalCrc16(rxdata+2,4,0xFFFF);
		if(CRCNUM==(rxdata[6]<<8)+rxdata[7])
		{
			if((rxdata[5]==0xAA)||(rxdata[5]==0xCC))break;
		//	else AgrMentDownFun(WorkGearCmd,NULLARR);
			else
			{
				ret = hal_uart_send(&uart1, WorkGearSetDown, sizeof(WorkGearSetDown), UART_TX_TIMEOUT);	
				ret = hal_uart_recv_II(&uart1, rxdata, UART_BUF_SIZE,
                               &rx_size, UART_RX_TIMEOUT);
			}
		}			
		else
		{
			ret = hal_uart_send(&uart1, WorkGearSetDown, sizeof(WorkGearSetDown), UART_TX_TIMEOUT);	
			ret = hal_uart_recv_II(&uart1, rxdata, UART_BUF_SIZE,
                               &rx_size, UART_RX_TIMEOUT);
		}
	}
	if(i==10)
	{
		ErrorCmd=1;
		user_post_event();
	}
}
void WaterboxSetUpHandle(uint8_t *rxdata)
{
	for(i=0;i<10;i++)
	{
		CRCNUM=CalCrc16(rxdata+2,4,0xFFFF);
		if(CRCNUM==(rxdata[6]<<8)+rxdata[7])
		{
			if((rxdata[5]==0xAA)||(rxdata[5]==0xCC))break;
			else
			{
				ret = hal_uart_send(&uart1, WaterboxSetDown, sizeof(WaterboxSetDown), UART_TX_TIMEOUT);	
				ret = hal_uart_recv_II(&uart1, rxdata, UART_BUF_SIZE,
                               &rx_size, UART_RX_TIMEOUT);
			}
		}			
		else
		{
			ret = hal_uart_send(&uart1, WaterboxSetDown, sizeof(WaterboxSetDown), UART_TX_TIMEOUT);	
			ret = hal_uart_recv_II(&uart1, rxdata, UART_BUF_SIZE,
                               &rx_size, UART_RX_TIMEOUT);
		}
	}
	if(i==10)
	{
		ErrorCmd=1;
		user_post_event();
	}
}
void RoomTemperUpHandle(uint8_t *rxdata)
{
	for(i=0;i<10;i++)
	{
		CRCNUM=CalCrc16(rxdata+2,4,0xFFFF);
		if(CRCNUM==(rxdata[6]<<8)+rxdata[7])
		{
			if((rxdata[5]==0xAA)||(rxdata[5]==0xCC))break;
			else
			{
				ret = hal_uart_send(&uart1, RoomTemperatureSetDown, sizeof(RoomTemperatureSetDown), UART_TX_TIMEOUT);	
				ret = hal_uart_recv_II(&uart1, rxdata, UART_BUF_SIZE,
                               &rx_size, UART_RX_TIMEOUT);
			}
		}			
		else
		{
			ret = hal_uart_send(&uart1, RoomTemperatureSetDown, sizeof(RoomTemperatureSetDown), UART_TX_TIMEOUT);	
			ret = hal_uart_recv_II(&uart1, rxdata, UART_BUF_SIZE,
                               &rx_size, UART_RX_TIMEOUT);
		}
	}
	if(i==10)
	{
		ErrorCmd=1;
		user_post_event();
	}
}
void WaterpumpTemperUpHandle(uint8_t *rxdata)
{
	for(i=0;i<10;i++)
	{
		CRCNUM=CalCrc16(rxdata+2,4,0xFFFF);
		if(CRCNUM==(rxdata[6]<<8)+rxdata[7])
		{
			if((rxdata[5]==0xAA)||(rxdata[5]==0xCC))break;
			else
			{
				ret = hal_uart_send(&uart1, WaterpumpTemperatureSetDown, sizeof(WaterpumpTemperatureSetDown), UART_TX_TIMEOUT);	
				ret = hal_uart_recv_II(&uart1, rxdata, UART_BUF_SIZE,
                               &rx_size, UART_RX_TIMEOUT);
			}
		}			
		else
		{
			ret = hal_uart_send(&uart1, WaterpumpTemperatureSetDown, sizeof(WaterpumpTemperatureSetDown), UART_TX_TIMEOUT);	
			ret = hal_uart_recv_II(&uart1, rxdata, UART_BUF_SIZE,
                               &rx_size, UART_RX_TIMEOUT);
		}
	}
	if(i==10)
	{
		ErrorCmd=1;
		user_post_event();
	}
}
void TimerSetUpHandle(uint8_t *rxdata)
{
	for(i=0;i<10;i++)
	{
		CRCNUM=CalCrc16(rxdata+2,4,0xFFFF);
		if(CRCNUM==(rxdata[6]<<8)+rxdata[7])
		{
			if((rxdata[5]==0xAA)||(rxdata[5]==0xCC))break;
			else
			{
				ret = hal_uart_send(&uart1, TimerTimeSetDown, sizeof(TimerTimeSetDown), UART_TX_TIMEOUT);	
				ret = hal_uart_recv_II(&uart1, rxdata, UART_BUF_SIZE,
                               &rx_size, UART_RX_TIMEOUT);
			}
		}			
		else
		{
			ret = hal_uart_send(&uart1, TimerTimeSetDown, sizeof(TimerTimeSetDown), UART_TX_TIMEOUT);	
			ret = hal_uart_recv_II(&uart1, rxdata, UART_BUF_SIZE,
                               &rx_size, UART_RX_TIMEOUT);
		}
	}
	if(i==10)
	{
		ErrorCmd=1;
		user_post_event();
	}
}
void TimerSwitchUpHandle(uint8_t *rxdata)
{
	for(i=0;i<10;i++)
	{
		CRCNUM=CalCrc16(rxdata+2,4,0xFFFF);
		if(CRCNUM==(rxdata[6]<<8)+rxdata[7])
		{
			if(rxdata[5]==0xAA)break;
			else
			{
				ret = hal_uart_send(&uart1, TimerSwitchSetDown, sizeof(TimerSwitchSetDown), UART_TX_TIMEOUT);	
				ret = hal_uart_recv_II(&uart1, rxdata, UART_BUF_SIZE,
                               &rx_size, UART_RX_TIMEOUT);
			}
		}			
		else
		{
			ret = hal_uart_send(&uart1, TimerSwitchSetDown, sizeof(TimerSwitchSetDown), UART_TX_TIMEOUT);	
			ret = hal_uart_recv_II(&uart1, rxdata, UART_BUF_SIZE,
                               &rx_size, UART_RX_TIMEOUT);
		}
	}
	if(i==10)
	{
		ErrorCmd=1;
		user_post_event();
	}
}
void ControlModeUpHandle(uint8_t *rxdata)
{
	for(i=0;i<10;i++)
	{
		CRCNUM=CalCrc16(rxdata+2,4,0xFFFF);
		if(CRCNUM==(rxdata[6]<<8)+rxdata[7])
		{
			if(rxdata[5]==0xAA)break;
			else
			{
				ret = hal_uart_send(&uart1, ControlModeSetDown, sizeof(ControlModeSetDown), UART_TX_TIMEOUT);	
				ret = hal_uart_recv_II(&uart1, rxdata, UART_BUF_SIZE,
                               &rx_size, UART_RX_TIMEOUT);
			}
		}			
		else
		{
			ret = hal_uart_send(&uart1, ControlModeSetDown, sizeof(ControlModeSetDown), UART_TX_TIMEOUT);	
			ret = hal_uart_recv_II(&uart1, rxdata, UART_BUF_SIZE,
                               &rx_size, UART_RX_TIMEOUT);
		}
	}
	if(i==10)
	{
		ErrorCmd=1;
		user_post_event();
	}
}
void SwitchUpHandle(uint8_t *rxdata)
{
	for(i=0;i<10;i++)
	{
		CRCNUM=CalCrc16(rxdata+2,4,0xFFFF);
		if(CRCNUM==(rxdata[6]<<8)+rxdata[7])
		{
			if(rxdata[5]==0xAA)break;
			else
			{
				ret = hal_uart_send(&uart1, SwitchSetDown, sizeof(SwitchSetDown), UART_TX_TIMEOUT);	
				ret = hal_uart_recv_II(&uart1, rxdata, UART_BUF_SIZE,
                               &rx_size, UART_RX_TIMEOUT);
			}
		}			
		else
		{
			ret = hal_uart_send(&uart1, SwitchSetDown, sizeof(SwitchSetDown), UART_TX_TIMEOUT);	
			ret = hal_uart_recv_II(&uart1, rxdata, UART_BUF_SIZE,
                               &rx_size, UART_RX_TIMEOUT);
		}
	}
	if(i==10)
	{
		ErrorCmd=1;
		user_post_event();
	}
}
////////////////////////////////////////////////////////////////////

///////////////////////////根据服务器发送来的操作发送下行帧////////////////////////////////
void HeartbeatDownFun(void)
{
	HeartbeatDown[5]=idex;
	CRCNUM=CalCrc16(HeartbeatDown+2,4,0xFFFF);
	HeartbeatDown[6]=CRCNUM/(1<<8);
	HeartbeatDown[7]=CRCNUM%(1<<8);
	ret = hal_uart_send(&uart1, HeartbeatDown, 10, UART_TX_TIMEOUT);
	idex++;
	if(idex>11)idex=0x01;
}
void DistributionDownFun(void)
{
	if(networkdata==1)//未定义
	{
		DistributionNetworkDown[5]=0x01;
		CRCNUM=CalCrc16(DistributionNetworkDown+2,4,0xFFFF);
		DistributionNetworkDown[6]=CRCNUM/(1<<8);
		DistributionNetworkDown[7]=CRCNUM%(1<<8);
	}
	else if(networkdata==2)
	{
		DistributionNetworkDown[5]=0x02;
		CRCNUM=CalCrc16(DistributionNetworkDown+2,4,0xFFFF);
		DistributionNetworkDown[6]=CRCNUM/(1<<8);
		DistributionNetworkDown[7]=CRCNUM%(1<<8);
	}
	else
	{
		DistributionNetworkDown[5]=0x00;
		CRCNUM=CalCrc16(DistributionNetworkDown+2,4,0xFFFF);
		DistributionNetworkDown[6]=CRCNUM/(1<<8);
		DistributionNetworkDown[7]=CRCNUM%(1<<8);
		awss_report_reset();
		netmgr_clear_ap_config();

    HAL_Reboot();
		
		//awss_config_press();
	}
	ret = hal_uart_send(&uart1, DistributionNetworkDown, sizeof(DistributionNetworkDown), UART_TX_TIMEOUT);
}
void GearDownFun(uint8_t *RequestARR)
{
	WorkGearSetDown[5]=(RequestARR[8]-48);
	CRCNUM=CalCrc16(WorkGearSetDown+2,4,0xFFFF);
	WorkGearSetDown[6]=CRCNUM/(1<<8);
	WorkGearSetDown[7]=CRCNUM%(1<<8);
	ret = hal_uart_send(&uart1, WorkGearSetDown, sizeof(WorkGearSetDown), UART_TX_TIMEOUT);
}
void TargetTemperatureSetDownFun(uint8_t *RequestARR)
{
	WaterboxSetDown[5]=(RequestARR[21]-48)*10+(RequestARR[22]-48);
}
void DiffSetDownFun(uint8_t *RequestARR)
{
	if(RequestARR[9]=='}')
	WaterboxSetDown[6]=RequestARR[8]-48;
	else
	{
		WaterboxSetDown[6]=(RequestARR[8]-48)*10+(RequestARR[9]-48);
	}	
}
void RoomTemSetDownFun(uint8_t *RequestARR)
{
	RoomTemperatureSetDown[5]=(RequestARR[38]-48)*10+(RequestARR[39]-48);
	RoomTemperatureSetDown[6]=(RequestARR[18]-48)*10+(RequestARR[19]-48);
	RoomTemperatureSetDown[7]=(RequestARR[58]-48)*10+(RequestARR[59]-48);
	CRCNUM=CalCrc16(RoomTemperatureSetDown+2,6,0xFFFF);
	RoomTemperatureSetDown[8]=CRCNUM/(1<<8);
	RoomTemperatureSetDown[9]=CRCNUM%(1<<8);
}
void WaterpumpTemperSetDownFun(uint8_t *RequestARR)
{
	WaterpumpTemperatureSetDown[5]=(RequestARR[17]-48)*10+(RequestARR[18]-48);
	CRCNUM=CalCrc16(WaterpumpTemperatureSetDown+2,4,0xFFFF);
	WaterpumpTemperatureSetDown[6]=CRCNUM/(1<<8);
	WaterpumpTemperatureSetDown[7]=CRCNUM%(1<<8);
}
void TimeSetDownFun(uint8_t *RequestARR)
{
	struct tm *OffTime;
    time_t lt;
    lt =(RequestARR[17]-48)*1000000000+(RequestARR[18]-48)*100000000+
		(RequestARR[19]-48)*10000000+(RequestARR[20]-48)*1000000+
		(RequestARR[21]-48)*100000+(RequestARR[22]-48)*10000+
		(RequestARR[23]-48)*1000+(RequestARR[24]-48)*100+
		(RequestARR[25]-48)*10+(RequestARR[26]-48)+(8*3600);
    OffTime=localtime(&lt);
		TimerTimeSetDown[8]=OffTime->tm_hour;
		TimerTimeSetDown[9]=OffTime->tm_min;
	struct tm *OnTime;
	  time_t tl;
    tl =(RequestARR[67]-48)*1000000000+(RequestARR[68]-48)*100000000+
		(RequestARR[69]-48)*10000000+(RequestARR[70]-48)*1000000+
		(RequestARR[71]-48)*100000+(RequestARR[72]-48)*10000+
		(RequestARR[73]-48)*1000+(RequestARR[74]-48)*100+
		(RequestARR[75]-48)*10+(RequestARR[76]-48)+(8*3600);
    OnTime=localtime(&tl);
        //HAL_Printf("second:%d\n",ptr->tm_sec);
        //HAL_Printf("minute:%d\n",ptr->tm_min);
        //HAL_Printf("hour:%d\n",ptr->tm_hour);
        //HAL_Printf("mday:%d\n",ptr->tm_mday);
        //HAL_Printf("month:%d\n",ptr->tm_mon+1);
        //HAL_Printf("year:%d\n",ptr->tm_year+1900);
	TimerTimeSetDown[5]=RequestARR[114]-48;
	TimerTimeSetDown[6]=OnTime->tm_hour;
	TimerTimeSetDown[7]=OnTime->tm_min;
	
	TimerTimeSetDown[10]=(RequestARR[155]-48)*10+(RequestARR[156]-48);
	TimerTimeSetDown[11]=(RequestARR[99]-48)*10+(RequestARR[100]-48);
	TimerTimeSetDown[12]=(RequestARR[49]-48)*10+(RequestARR[50]-48);
	TimerTimeSetDown[13]=(RequestARR[133]-48)*10+(RequestARR[134]-48);
	CRCNUM=CalCrc16(TimerTimeSetDown+2,12,0xFFFF);
	TimerTimeSetDown[14]=CRCNUM/(1<<8);
	TimerTimeSetDown[15]=CRCNUM%(1<<8);
}
void TimeSwitchSetDownFun(uint8_t *RequestARR)
{
	TimerSwitchSetDown[5]=RequestARR[14]-48;
	CRCNUM=CalCrc16(TimerSwitchSetDown+2,4,0xFFFF);
	TimerSwitchSetDown[6]=CRCNUM/(1<<8);
	TimerSwitchSetDown[7]=CRCNUM%(1<<8);
}
void ContrlModeSetDownFun(uint8_t *RequestARR)
{
	ControlModeSetDown[5]=RequestARR[15]+1-48;
	CRCNUM=CalCrc16(ControlModeSetDown+2,4,0xFFFF);
	ControlModeSetDown[6]=CRCNUM/(1<<8);
	ControlModeSetDown[7]=CRCNUM%(1<<8);
}
void SwitchSetDownFun(uint8_t *RequestARR)
{
	SwitchSetDown[5]=RequestARR[15]-48;
	CRCNUM=CalCrc16(SwitchSetDown+2,4,0xFFFF);
	SwitchSetDown[6]=CRCNUM/(1<<8);
	SwitchSetDown[7]=CRCNUM%(1<<8);
}
void ForceSetDownFun(void)
{
	ControlModeSetDown[5]=0x03;
	CRCNUM=CalCrc16(ControlModeSetDown+2,4,0xFFFF);
	ControlModeSetDown[6]=CRCNUM/(1<<8);
	ControlModeSetDown[7]=CRCNUM%(1<<8);
}

///////////////////////////////////////////////////////////////////////////////////////

/////////////////////通讯由下行帧发起，上行帧回应。一应一答的方式///////////////////////////
/////////////////////////////////发送下行帧//////////////////////////////////////////////

void AgrMentDownFun(uint8_t downcmd,uint8_t *requestarr)
{	
	switch(downcmd)
	{
		case HeartbeatCmd:
		HeartbeatDownFun();
		break;
		case DistributionNetworkCmd:
		DistributionDownFun();
		break;
		case WorkGearCmd:
		GearDownFun(requestarr);
		break;
		case WaterboxSetCmd:
		if(strstr(requestarr,"TargetTemperature"))
			TargetTemperatureSetDownFun(requestarr);
		if(strstr(requestarr,"diff"))
			DiffSetDownFun(requestarr);
		CRCNUM=CalCrc16(WaterboxSetDown+2,5,0xFFFF);
		WaterboxSetDown[7]=CRCNUM/(1<<8);
		WaterboxSetDown[8]=CRCNUM%(1<<8);
		ret = hal_uart_send(&uart1, WaterboxSetDown, sizeof(WaterboxSetDown), UART_TX_TIMEOUT);
		break;
		case RoomTemperatureSetCmd:
		RoomTemSetDownFun(requestarr);
		ret = hal_uart_send(&uart1, RoomTemperatureSetDown, sizeof(RoomTemperatureSetDown), UART_TX_TIMEOUT);
		break;
		case WaterpumpTempertureCmd:
		WaterpumpTemperSetDownFun(requestarr);
		ret = hal_uart_send(&uart1, WaterpumpTemperatureSetDown, sizeof(WaterpumpTemperatureSetDown), UART_TX_TIMEOUT);
		break;
		case TimerSetCmd:
		TimeSetDownFun(requestarr);
		ret = hal_uart_send(&uart1, TimerTimeSetDown, sizeof(TimerTimeSetDown), UART_TX_TIMEOUT);
		break;
		case TimerSwitchCmd:
		TimeSwitchSetDownFun(requestarr);
		ret = hal_uart_send(&uart1, TimerSwitchSetDown, sizeof(TimerSwitchSetDown), UART_TX_TIMEOUT);
		break;
		case ControlModeCmd:
		ContrlModeSetDownFun(requestarr);
		ret = hal_uart_send(&uart1, ControlModeSetDown, sizeof(ControlModeSetDown), UART_TX_TIMEOUT);
		break;
		case SwitchCmd:
		SwitchSetDownFun(requestarr);
		ret = hal_uart_send(&uart1, SwitchSetDown, sizeof(SwitchSetDown), UART_TX_TIMEOUT);
		break;
		case ForceCmd:
		ForceSetDownFun();
		break;
		default:
		break;
	}
}

///////////////////////////////////////////////上行帧处理////////////////////////////////////////////
uint8_t UpCmd;
void AgrMentUpHandle(uint8_t UpCmd,uint8_t *rxdata)
{
	ret = hal_uart_recv_II(&uart1, rxdata, UART_BUF_SIZE,
                               &rx_size, UART_RX_TIMEOUT);
	switch(UpCmd)
	{
		case HeartbeatCmd:
		heartbeatbufhandle(rxdata);
		break;
		case DistributionNetworkCmd:
		DistributionUpHandle(rxdata);
		break;
		case WorkGearCmd:
		GearUpHandle(rxdata);
		break;
		case WaterboxSetCmd:
		WaterboxSetUpHandle(rxdata);
		break;
		case RoomTemperatureSetCmd:
		RoomTemperUpHandle(rxdata);
		break;
		case WaterpumpTempertureCmd:
		WaterpumpTemperUpHandle(rxdata);
		break;
		case TimerSetCmd:
		TimerSetUpHandle(rxdata);
		break;
		case TimerSwitchCmd:
		TimerSwitchUpHandle(rxdata);
		break;
		case ControlModeCmd:
		ControlModeUpHandle(rxdata);
		break;
		case SwitchCmd:
		SwitchUpHandle(rxdata);
		break;
		default:
		break;
	}	
}

//////////////////////////////////////////////////////////////
void test(void)
{
	ret = hal_uart_recv_II(&uart1, UpData, UART_BUF_SIZE,
                               &rx_size, UART_RX_TIMEOUT);
	
	if(strstr(UpData,"net"))
	{
		ret = hal_uart_send(&uart1, UpData, sizeof(UpData), UART_TX_TIMEOUT);
		memset(UpData,0,sizeof(UpData));
		ret = hal_uart_recv_II(&uart1, UpData, UART_BUF_SIZE,
                               &rx_size, UART_RX_TIMEOUT);
		
		return UpData;
	}
}



#ifndef __AGREEMENT_H__
#define __AGREEMENT_H__

#include "aos/kernel.h"
#include "stdint.h"

enum SetCmd
{
	HeartbeatCmd=0x01,
	DistributionNetworkCmd,
	WorkGearCmd,
	WaterboxSetCmd,
	RoomTemperatureSetCmd,
	WaterpumpTempertureCmd,
	TimerSetCmd,
	TimerSwitchCmd,
	ControlModeCmd,
	SwitchCmd,
	ForceCmd
};

typedef struct 
{
	uint8_t SwitchStatus;
	uint8_t WaterboxSetTemperature;
	uint8_t WaterboxRealTemperature;
	uint8_t Room1SetTemperature;
	uint8_t Room2SetTemperature;
	uint8_t Room3SetTemperature;
	uint8_t Room1RealTemperature;
	uint8_t Room2RealTemperature;
	uint8_t Room3RealTemperature;
	uint8_t Diff;
}HeartBeatBuf01;

HeartBeatBuf01 heartbeatbuf01;

typedef struct 
{
	uint8_t HeaterWorkStaus;
	uint8_t HeaterGear;
	uint8_t WaterPumpTemperature;
	uint8_t WaterPumpWorkStatus;
	uint8_t HeatingPeriod;
	uint8_t TimerSwitch;
}HeartBeatBuf02;

HeartBeatBuf02 heartbeatbuf02;

typedef struct 
{
	uint8_t ENTimer;
	uint8_t OnHour;
	uint8_t OnMin;
	uint8_t OffHour;
	uint8_t OffMin;
	uint8_t TargetTemper;
	uint8_t Room1Temper;
	uint8_t Room2Temper;
	uint8_t Room3Temper;
}HeartBeatBuf03;

HeartBeatBuf03 heartbeatbuf03;

typedef struct 
{
	uint8_t ENTimer;
	uint8_t OnHour;
	uint8_t OnMin;
	uint8_t OffHour;
	uint8_t OffMin;
	uint8_t TargetTemper;
	uint8_t Room1Temper;
	uint8_t Room2Temper;
	uint8_t Room3Temper;
}HeartBeatBuf04;

HeartBeatBuf04 heartbeatbuf04;

typedef struct 
{
	uint8_t ENTimer;
	uint8_t OnHour;
	uint8_t OnMin;
	uint8_t OffHour;
	uint8_t OffMin;
	uint8_t TargetTemper;
	uint8_t Room1Temper;
	uint8_t Room2Temper;
	uint8_t Room3Temper;
}HeartBeatBuf05;

HeartBeatBuf05 heartbeatbuf05;

typedef struct 
{
	uint8_t ENTimer;
	uint8_t OnHour;
	uint8_t OnMin;
	uint8_t OffHour;
	uint8_t OffMin;
	uint8_t TargetTemper;
	uint8_t Room1Temper;
	uint8_t Room2Temper;
	uint8_t Room3Temper;
}HeartBeatBuf06;

HeartBeatBuf06 heartbeatbuf06;

typedef struct 
{
	uint8_t ENTimer;
	uint8_t OnHour;
	uint8_t OnMin;
	uint8_t OffHour;
	uint8_t OffMin;
	uint8_t TargetTemper;
	uint8_t Room1Temper;
	uint8_t Room2Temper;
	uint8_t Room3Temper;
}HeartBeatBuf07;

HeartBeatBuf07 heartbeatbuf07;

typedef struct 
{
	uint8_t ENTimer;
	uint8_t OnHour;
	uint8_t OnMin;
	uint8_t OffHour;
	uint8_t OffMin;
	uint8_t TargetTemper;
	uint8_t Room1Temper;
	uint8_t Room2Temper;
	uint8_t Room3Temper;
}HeartBeatBuf08;

HeartBeatBuf08 heartbeatbuf08;

typedef struct 
{
	uint8_t ENTimer;
	uint8_t OnHour;
	uint8_t OnMin;
	uint8_t OffHour;
	uint8_t OffMin;
	uint8_t TargetTemper;
	uint8_t Room1Temper;
	uint8_t Room2Temper;
	uint8_t Room3Temper;
}HeartBeatBuf09;

HeartBeatBuf09 heartbeatbuf09;

typedef struct 
{
	uint8_t ENTimer;
	uint8_t OnHour;
	uint8_t OnMin;
	uint8_t OffHour;
	uint8_t OffMin;
	uint8_t TargetTemper;
	uint8_t Room1Temper;
	uint8_t Room2Temper;
	uint8_t Room3Temper;
}HeartBeatBuf0A;

HeartBeatBuf0A heartbeatbuf0A;

typedef struct 
{
	uint8_t Mode;
	uint8_t ReDistributionNetwork;
	uint8_t WaterLevelWarnning;
	uint8_t SensorWarnning;
	uint8_t LeakageWarnning;
	uint8_t OutTemperWarnning;
	uint8_t ForcedOperationMode;
	uint8_t FreConverMode;
}HeartBeatBuf0B;

HeartBeatBuf0B heartbeatbuf0B;

typedef enum
{
	RxNetwork,
	NetworkSuccessed,
	NetworkFailed
}DistributionNetworkdata;

DistributionNetworkdata networkdata;

void AgrMentUpHandle(uint8_t UpCmd,uint8_t *rxdata);
void AgrMentDownFun(uint8_t downcmd,uint8_t *requestarr);
void Uart1_Init(void);
static void timer_handler(void *arg1, void* arg2);
static void timer_handlerII(void *arg1, void* arg2);
void Timer_Init(void);
void test(void);
void GPIO_INIT(void);


#endif

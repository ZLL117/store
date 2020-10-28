#include "myjson.h"
#include "math.h"
extern UART_HandleTypeDef huart1;

uint8_t str[1024];//JSON数据缓存
uint8_t strr[1024];//获取JSON数据
uint8_t* p; 
uint8_t* end;
uint8_t* commom;
uint8_t* oldCommom;
uint8_t tempstr[160][100];//存放JSON数组数据
uint8_t returnStr[1024];//存放反馈数据
int temp;
uint32_t s;
uint8_t totalPoint;
uint8_t arrNum;//数组下标
uint8_t arrItems;//数组大小
uint8_t slot;
uint8_t slotNum;
uint16_t ti;
uint8_t tj;
uint16_t tempCnt;
uint16_t itempCnt;
uint16_t olditempCnt;
void JSON_decode(uint8_t* arr,char* key)//拆分json元素,将元素存入str中
{
tempCnt=0;
itempCnt=0;
olditempCnt=1;
	p = (uint8_t*)strstr((char*)arr,key);
	s = strlen(key);
	switch(*(p+s+2))
	{
	case '{':		
		for(ti=(p+s+2-arr);ti<strlen((char*)arr);ti++)
		{
			if(arr[ti]=='{')
				totalPoint++;
			if(arr[ti]=='}')
				totalPoint--;
			str[tempCnt++] = arr[ti];
			if(totalPoint==0)
				break;
		}
		break;
	case '[':
		end = (uint8_t*)strstr((char*)arr,"]");
		if(*(p+s+3)=='{')
		{
			for(tj=0;tj<160;tj++)
			{
				if(olditempCnt!=itempCnt)
					olditempCnt = itempCnt;
				else
				{
					arrItems = tj;
					break;
				}
				tempCnt = 0;
				if(p+s+3+itempCnt<end)
				{
					for(ti=(p+s+3+itempCnt-arr);ti<(end-arr);ti++)
					{
						if(arr[ti]=='{')
							totalPoint++;
						if(arr[ti]=='}')
							totalPoint--;
						tempstr[tj][tempCnt++] = arr[ti];
						if(totalPoint==0)
						{
							itempCnt = itempCnt+tempCnt+1;
							break;
						}
					}
				}
				else
				{
					arrItems=tj;
					break;
				}
			}	
		}
		else
		{
			commom = p+s+2;
			for(tj=0;tj<160;tj++)
			{
				oldCommom = commom;
				commom = (uint8_t*)strstr((char*)oldCommom+1,",");
				if((commom<end)&&(commom!=0))
				{
					memcpy(tempstr[tj],oldCommom+1,commom-oldCommom-1);
				}
				else
				{
					memcpy(tempstr[tj],oldCommom+1,end-oldCommom-1);
					arrItems = tj+1;
					break;
				}
			}
		}
			
		break;
	case '"':
		end = (uint8_t*)strstr((char*)p+s+3,"\"");
		memcpy(str,p+s+2,end-p-s-1);
		break;
	default:
		temp = 0;
		while((*(p+temp+s+2)>='0')&&(*(p+temp+s+2)<='9'))
		{
			temp++;
		}
		end = p+s+2+temp-1;
		memcpy(str,p+s+2,end-p-s-1);
		break;
	}
	memcpy(strr,str,1024);
	memset(str,0,1024);
}


uint8_t toHEX(void)
{
	uint8_t result=0;
	uint8_t i;
	uint8_t len=strlen((char*)strr);
	for(i=0;i<len;i++)
	{
		result += (strr[i]-48)*pow(10,len-i-1);
	}
	return result;
}
	uint8_t sItems[100][100];
	uint8_t eItems[10][100];
	uint8_t k;
	uint8_t CMD[50];
	uint8_t orderNomber[50];
	uint8_t PID[50];
	uint8_t OPID[50];
	uint8_t TID[50];
	uint8_t Ttime[50];
	uint8_t orderResult[20];
	uint16_t totalResult=0;
	uint16_t expectTotalResult=0;
	uint8_t errorSlot;
	uint8_t sTimes;//存放成功次数
uint8_t errPlaceSlot;//记录有多少错位的货道
uint8_t PlaceSlot[160];//记录错位的货道
uint8_t successPlaceSlot[160];//正确归位的货道
uint8_t successPlaceCnt;
uint8_t failPlaceSlot[160];//归位失败的货道
uint8_t failPlaceCnt;
void JSON_Handle(uint8_t* arr)
{
	for(k=0;k<100;k++)
	{
		memset(sItems[k],0,100);
	}
	for(k=0;k<10;k++)
	{
		memset(eItems[k],0,100);
	}
	memset(CMD,0,sizeof(CMD));
	memset(orderNomber,0,sizeof(orderNomber));
	memset(PID,0,sizeof(PID));
	memset(TID,0,sizeof(TID));
	memset(Ttime,0,sizeof(Ttime));
	memset(orderResult,0,sizeof(orderResult));
	totalResult=0;
	expectTotalResult=0;
	errorSlot=0;
	memset(returnStr,0,1024);
	JSON_decode(arr,"cabinetNo");
	if(toHEX() == CABINETNO)
	{
		JSON_decode(arr,"ttime");
		//稍作处理
		strcpy((char*)Ttime,(char*)strr);
		JSON_decode(arr,"pid");
		strcpy((char*)PID,(char*)strr);
		JSON_decode(arr,"tid");
		strcpy((char*)TID,(char*)strr);	
		JSON_decode(arr,"cmd");
		strcpy((char*)CMD,(char*)strr);
		
		if(strstr((char*)CMD,"healthCheck"))
		{
			sprintf((char*)returnStr,"%c{\"cmd\":\"healthCheckR\",\"cabinetNo\":%d,\"result\":\"ok\",\"pid\":%s,\"tid\":%s,\"ttime\":%s}",0xEE,CABINETNO,PID,TID,Ttime);
			uint16_t CRCCOUNT = CalCrc16(returnStr+1, strlen((char *)returnStr)-1,0xFFFF);
			sprintf((char*)returnStr,"%s%c%c%c",returnStr,(uint8_t)(CRCCOUNT>>8),(uint8_t)CRCCOUNT,0xFF);
	//		HAL_Delay(1000);
		}
		else if(strstr((char*)CMD,"slotCheck"))
		{
			allCheck();
			if(errPlaceSlot == 0)//没有错位的货道
			{
				sprintf((char*)returnStr,"%c{\"cmd\":\"slotCheckR\",\"cabinetNo\":%d,\"faultSlotItems\":[],\"pid\":%s,\"tid\":%s,\"ttime\":%s}",0xEE,CABINETNO,PID,TID,Ttime);
				uint16_t CRCCOUNT = CalCrc16(returnStr+1, strlen((char *)returnStr)-1,0xFFFF);
				sprintf((char*)returnStr,"%s%c%c%c",returnStr,(uint8_t)(CRCCOUNT>>8),(uint8_t)CRCCOUNT,0xFF);
			}
			else
			{
				sprintf((char*)returnStr,"%c{\"cmd\":\"slotCheckR\",\"cabinetNo\":%d,\"faultSlotItems\":[{\"slotNo\":%d,\"faultReason\":\"dislocate\"}",0xEE,CABINETNO,PlaceSlot[0]);
				if(errPlaceSlot>1)
				{
					for(k=1;k<errPlaceSlot;k++)
					{
						sprintf((char*)returnStr,"%s,{\"slotNo\":%d,\"faultReason\":\"dislocate\"}",returnStr,PlaceSlot[k]);
					}
				}
				sprintf((char*)returnStr,"%s],\"pid\":%s,\"tid\":%s,\"ttime\":%s}",returnStr,PID,TID,Ttime);
				uint16_t CRCCOUNT = CalCrc16(returnStr+1, strlen((char *)returnStr)-1,0xFFFF);
				sprintf((char*)returnStr,"%s%c%c%c",returnStr,(uint8_t)(CRCCOUNT>>8),(uint8_t)CRCCOUNT,0xFF);
			}
		}		
//		else if(strstr((char*)CMD,"slotReset"))
//		{
//			failPlaceCnt = 0;
//			successPlaceCnt = 0;
//			for(k=0;k<160;k++)
//			{
//				memset(tempstr[k],0,100);
//			}
//			JSON_decode(arr,"resetSlots");
//			for(k=0;k<arrItems;k++)
//			{
//				memset(strr,0,sizeof(strr));
//				memcpy(strr,tempstr[k],strlen((char*)tempstr[k]));
//				slot = toHEX();
//				sTimes = unLock(slot);
//				if(sTimes)
//				{
//					successPlaceSlot[successPlaceCnt++] = slot;
//				}
//				else 
//				{
//					failPlaceSlot[failPlaceCnt++] = slot;
//				}
//			}
//			if(successPlaceCnt == arrItems)
//			{
//				sprintf((char*)returnStr,"{\"cmd\":\"slotResetR\",\"result\":\"allSuccess\",\"successSlots\":[%d",successPlaceSlot[0]);
//				if(successPlaceCnt > 1)
//				{
//					for(k=1;k<successPlaceCnt;k++)
//					{
//						sprintf((char*)returnStr,"%s,%d",returnStr,successPlaceSlot[k]);
//					}
//				}
//				sprintf((char*)returnStr,"%s],\"errorSlots\":[],\"pid\":%s,\"tid\":%s,\"ttime\":%s}",returnStr,PID,TID,Ttime);
//			}
//			else if(successPlaceCnt == 0)
//			{
//				sprintf((char*)returnStr,"{\"cmd\":\"slotResetR\",\"result\":\"allTimeout\",\"successSlots\":[],\"errorSlots\":[%d",failPlaceSlot[0]);
//				if(failPlaceCnt > 1)
//				{
//					for(k=1;k<failPlaceCnt;k++)
//					{
//						sprintf((char*)returnStr,"%s,%d",returnStr,failPlaceSlot[k]);
//					}
//				}
//				sprintf((char*)returnStr,"%s],\"pid\":%s,\"tid\":%s,\"ttime\":%s}",returnStr,PID,TID,Ttime);
//			}
//			else if(successPlaceCnt < arrItems)
//			{
//				sprintf((char*)returnStr,"{\"cmd\":\"slotResetR\",\"result\":\"partialSuccess\",\"successSlots\":[%d",successPlaceSlot[0]);
//				if(successPlaceCnt > 1)
//				{
//					for(k=1;k<successPlaceCnt;k++)
//					{
//						sprintf((char*)returnStr,"%s,%d",returnStr,successPlaceSlot[k]);
//					}
//				}
//				sprintf((char*)returnStr,"%s],\"errorSlots\":[%d",returnStr,failPlaceSlot[0]);
//				if(failPlaceCnt > 1)
//				{
//					for(k=1;k<failPlaceCnt;k++)
//					{
//						sprintf((char*)returnStr,"%s,%d",returnStr,failPlaceSlot[k]);
//					}
//				}
//				sprintf((char*)returnStr,"%s],\"pid\":%s,\"tid\":%s,\"ttime\":%s}",returnStr,PID,TID,Ttime);
//			}
//		}
		else if(strstr((char*)CMD,"orderProcess"))
		{
			if(strstr((char *)OPID,(char *)PID)==0)
			{
				memcpy(OPID,PID,50);
				JSON_decode(arr,"orderNo");
				strcpy((char*)orderNomber,(char*)strr);
				JSON_decode(arr,"reqItems");
				for(k=0;k<arrItems;k++)
				{
					JSON_decode(tempstr[k],"slotNo");
					slot = toHEX();
					JSON_decode(tempstr[k],"quantity");
					slotNum = toHEX();
					HAL_UART_Transmit(&huart1,"OpenRelay",strlen("OpenRelay"), 0xFFFF);
					sTimes = unLock(slot);//返回成功次数
					HAL_UART_Transmit(&huart1,"CloseRelay",strlen("CloseRelay"), 0xFFFF);
					totalResult += sTimes;
					expectTotalResult += slotNum;
					
					if(sTimes < slotNum)
						sprintf((char*)eItems[errorSlot++],"{\"slotNo\":%d,\"exceptionOrder\":%d,\"exception\":\"%s\"}",slot,sTimes+1,"timeout");			
					
					sprintf((char*)sItems[k],"{\"slotNo\":%d,\"successTimes\":%d}",slot,sTimes);
				}
				
				if(totalResult == expectTotalResult)
					strcpy((char*)orderResult,"success");
				else if(totalResult == 0)
					strcpy((char*)orderResult,"allTimeOut");
				else if(totalResult < expectTotalResult)
					strcpy((char*)orderResult,"partialSuccess");
					
				sprintf((char*)returnStr,"%c{\"cmd\":\"orderProcessR\",\"cabinetNo\":%d,\"orderNo\":%s,\"orderResult\":\"%s\",\
	\"ttime\":%s,\"pid\":%s,\"tid\":%s,\"respItems\":[%s",\
				0xEE,CABINETNO,orderNomber,orderResult,Ttime,PID,TID,sItems[0]);
				
				if(arrItems>1)
				{
					for(k=1;k<arrItems;k++)
					{
						sprintf((char*)returnStr,"%s,%s",returnStr,sItems[k]);
					}
				}
				if(totalResult == expectTotalResult)
					sprintf((char*)returnStr,"%s],\"slotException\":[]}",returnStr);
				else
				{
					sprintf((char*)returnStr,"%s],\"slotException\":[%s",returnStr,eItems[0]);
					if(errorSlot>1)
					{
						for(k=1;k<errorSlot;k++)
						{
							sprintf((char*)returnStr,"%s,%s",returnStr,eItems[k]);
						}
					}
					sprintf((char*)returnStr,"%s]}",returnStr);
				}
				uint16_t CRCCOUNT = CalCrc16(returnStr+1, strlen((char *)returnStr)-1,0xFFFF);
				sprintf((char*)returnStr,"%s%c%c%c",returnStr,(uint8_t)(CRCCOUNT>>8),(uint8_t)CRCCOUNT,0xFF);
			}
		}
		printf("%s",returnStr);
	}
}

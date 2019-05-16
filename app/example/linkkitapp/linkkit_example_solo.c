/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */
#ifdef DEPRECATED_LINKKIT
#include "deprecated/solo.c"
#else
#include "stdio.h"
#include "string.h"
#include "iot_export.h"
#include "iot_import.h"
#include "cJSON.h"
#include "app_entry.h"
#include "agreement.h"
#include "crc.h"
#include <aos/hal/uart.h>
#include <aos/hal/gpio.h>
#include <aos/hal/flash.h>
#include "time.h"

#define RTC1_PORT_NUM 1
#define USE_CUSTOME_DOMAIN      (0)


//#define PRODUCT_KEY      "a1taoUnV8ie"
//#define PRODUCT_SECRET   "uiMvEhpb6dlhZtNB"
//#define DEVICE_NAME      "Elaycdi12SpWaegMfyna"
//#define DEVICE_SECRET    "UP6YNH0ahEHT0rcszmbjCFrHuA4dkF6C"

uint8_t product_key[12]="a1taoUnV8ie";
uint8_t product_secret[17]="uiMvEhpb6dlhZtNB";
uint8_t device_name[21]="Elaycdi12SpWaegMfyna";
uint8_t device_secrect[33]="UP6YNH0ahEHT0rcszmbjCFrHuA4dkF6C";

extern uint8_t LedPoint;
extern uint8_t HeartPoint;
extern uart_dev_t uart1;
extern gpio_dev_t led;
extern uint8_t UpData[20];
extern aos_timer_t  g_timer;
extern uint8_t ErrorCmd;
extern uint8_t PortPro;
extern uint8_t TimeInt;
extern char NULLARR[1];
uint8_t TimeIntMin=0;
uint32_t TimeChange;
uint8_t portidex=0;
uint8_t portindex;
uint8_t Times=0;
uint8_t ProEvent=0;
uint8_t FlashData[4096];

uint8_t testt[20];
uint8_t testtt[32];
#if USE_CUSTOME_DOMAIN
    #define CUSTOME_DOMAIN_MQTT     "iot-as-mqtt.cn-shanghai.aliyuncs.com"
    #define CUSTOME_DOMAIN_HTTP     "iot-auth.cn-shanghai.aliyuncs.com"
#endif

#define USER_EXAMPLE_YIELD_TIMEOUT_MS (200)

/*#define EXAMPLE_TRACE(...)                               \
    do {                                                     \
        HAL_Printf("\033[1;32;40m%s.%d: ", __func__, __LINE__);  \
        HAL_Printf(__VA_ARGS__);                                 \
        HAL_Printf("\033[0m\r\n");                                   \
    } while (0)*/

typedef struct {
    int master_devid;
    int cloud_connected;
    int master_initialized;
} user_example_ctx_t;

uint8_t Requestarr[200];


static user_example_ctx_t g_user_example_ctx;
static uint64_t user_update_sec(void)
{
    static uint64_t time_start_ms = 0;

    if (time_start_ms == 0) {
        time_start_ms = HAL_UptimeMs();
    }

    return (HAL_UptimeMs() - time_start_ms) / 1000;
}
static user_example_ctx_t *user_example_get_ctx(void)
{
    return &g_user_example_ctx;
}

void *example_malloc(size_t size)
{
    return HAL_Malloc(size);
}

void example_free(void *ptr)
{
    HAL_Free(ptr);
}

static int user_connected_event_handler(void)
{
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();
    LedPoint=1;
    hal_gpio_output_high(&led);
    //EXAMPLE_TRACE("Cloud Connected");
    user_example_ctx->cloud_connected = 1;
    networkdata=1;
    AgrMentDownFun(DistributionNetworkCmd,NULLARR);
    return 0;
}

static int user_disconnected_event_handler(void)
{
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();
    LedPoint=0;
    //EXAMPLE_TRACE("Cloud Disconnected");
    user_example_ctx->cloud_connected = 0;
    networkdata=2;
    AgrMentDownFun(DistributionNetworkCmd,NULLARR);
    return 0;
}

static int user_down_raw_data_arrived_event_handler(const int devid, const unsigned char *payload,
        const int payload_len)
{
    //EXAMPLE_TRACE("Down Raw Message, Devid: %d, Payload Length: %d", devid, payload_len);
    return 0;
}

static int user_service_request_event_handler(const int devid, const char *serviceid, const int serviceid_len,
        const char *request, const int request_len,
        char **response, int *response_len)
{
    aos_timer_stop(&g_timer);
    int contrastratio = 0, to_cloud = 0;
    cJSON *root = NULL, *item_transparency = NULL, *item_from_cloud = NULL;
    //EXAMPLE_TRACE("Service Request Received, Devid: %d, Service ID: %.*s, Payload: %s", devid, serviceid_len,
                //  serviceid,request);

    /* Parse Root */
    root = cJSON_Parse(request);
    if (root == NULL || !cJSON_IsObject(root)) {
        //EXAMPLE_TRACE("JSON Parse Error");
        return -1;
    }

    if (strlen("Custom") == serviceid_len && memcmp("Custom", serviceid, serviceid_len) == 0) {
        /* Parse Item */
        const char *response_fmt = "{\"Contrastratio\":%d}";
        item_transparency = cJSON_GetObjectItem(root, "transparency");
        if (item_transparency == NULL || !cJSON_IsNumber(item_transparency)) {
            cJSON_Delete(root);
            return -1;
        }
        //EXAMPLE_TRACE("transparency: %d", item_transparency->valueint);
        contrastratio = item_transparency->valueint + 1;

        /* Send Service Response To Cloud */
        *response_len = strlen(response_fmt) + 10 + 1;
        *response = (char *)HAL_Malloc(*response_len);
        if (*response == NULL) {
            //EXAMPLE_TRACE("Memory Not Enough");
            return -1;
        }
        memset(*response, 0, *response_len);
        //HAL_Snprintf(*response, *response_len, response_fmt, contrastratio);
        *response_len = strlen(*response);
    } else if (strlen("SyncService") == serviceid_len && memcmp("SyncService", serviceid, serviceid_len) == 0) {
        /* Parse Item */
        const char *response_fmt = "{\"ToCloud\":%d}";
        item_from_cloud = cJSON_GetObjectItem(root, "FromCloud");
        if (item_from_cloud == NULL || !cJSON_IsNumber(item_from_cloud)) {
            cJSON_Delete(root);
            return -1;
        }
        //EXAMPLE_TRACE("FromCloud: %d", item_from_cloud->valueint);
        to_cloud = item_from_cloud->valueint + 1;

        /* Send Service Response To Cloud */
        *response_len = strlen(response_fmt) + 10 + 1;
        *response = (char *)HAL_Malloc(*response_len);
        if (*response == NULL) {
            //EXAMPLE_TRACE("Memory Not Enough");
            return -1;
        }
        memset(*response, 0, *response_len);
        //HAL_Snprintf(*response, *response_len, response_fmt, to_cloud);
        *response_len = strlen(*response);
    }
    cJSON_Delete(root);
    strcpy(Requestarr,request);
    if(strstr(request,"room")&&(!(strstr(request,"time"))))
    {
        AgrMentDownFun(RoomTemperatureSetCmd,Requestarr);
        memset(UpData,0,sizeof(UpData));
        AgrMentUpHandle(RoomTemperatureSetCmd,UpData);
    }
    if(strstr(request,"time"))
    {
        AgrMentDownFun(TimerSetCmd,Requestarr);
        memset(UpData,0,sizeof(UpData));
        AgrMentUpHandle(TimerSetCmd,UpData);
    }
    aos_timer_start(&g_timer);
    return 0;
}

static int user_property_set_event_handler(const int devid, const char *request, const int request_len)
{
    aos_timer_stop(&g_timer);
    int res = 0;
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();
    //EXAMPLE_TRACE("Property Set Received, Devid: %d, Request: %s", devid, request);

    res = IOT_Linkkit_Report(user_example_ctx->master_devid, ITM_MSG_POST_PROPERTY,
                             (unsigned char *)request, request_len);
    //EXAMPLE_TRACE("Post Property Message ID: %d", res);
    strcpy(Requestarr,request);
    if(strstr(request,"gear"))//工作档位设置
    {
       AgrMentDownFun(WorkGearCmd,Requestarr);
       memset(UpData,0,sizeof(UpData));
       AgrMentUpHandle(WorkGearCmd,UpData); 
    }
    if(strstr(request,"TargetTemperature")||strstr(request,"diff"))
    {
        AgrMentDownFun(WaterboxSetCmd,Requestarr);
        memset(UpData,0,sizeof(UpData));
        AgrMentUpHandle(WaterboxSetCmd,UpData); 
    }
    if(strstr(request,"PumpTemperSet"))
    {
        AgrMentDownFun(WaterpumpTempertureCmd,Requestarr);
        memset(UpData,0,sizeof(UpData));
        AgrMentUpHandle(WaterpumpTempertureCmd,UpData); 
    }
    if(strstr(request,"tankSwtich"))
    {
        AgrMentDownFun(TimerSwitchCmd,Requestarr);
        memset(UpData,0,sizeof(UpData));
        AgrMentUpHandle(TimerSwitchCmd,UpData); 
    }
    if(strstr(request,"workModeSet"))
    {
        AgrMentDownFun(ControlModeCmd,Requestarr);
        memset(UpData,0,sizeof(UpData));
        AgrMentUpHandle(ControlModeCmd,UpData); 
    }
    if(strstr(request,"PowerSwitch"))
    {
        AgrMentDownFun(SwitchCmd,Requestarr);
        memset(UpData,0,sizeof(UpData));
        AgrMentUpHandle(SwitchCmd,UpData); 
    }
    if(strstr(request,"pumpModeForce"))
    {
        AgrMentDownFun(ForceCmd,Requestarr);
        memset(UpData,0,sizeof(UpData));
        AgrMentUpHandle(ControlModeCmd,UpData);
    }
    aos_timer_start(&g_timer);
    return 0;
}

static int user_property_get_event_handler(const int devid, const char *request, const int request_len, char **response,
        int *response_len)
{
    
    HAL_Printf("getevent\n");
    cJSON *request_root = NULL, *item_propertyid = NULL;
    cJSON *response_root = NULL;
    int index = 0;
    //EXAMPLE_TRACE("Property Get Received, Devid: %d, Request: %s", devid, request);

    /* Parse Request */
    request_root = cJSON_Parse(request);
    if (request_root == NULL || !cJSON_IsArray(request_root)) {
        //EXAMPLE_TRACE("JSON Parse Error");
        return -1;
    }

    /* Prepare Response */
    response_root = cJSON_CreateObject();
    if (response_root == NULL) {
        //EXAMPLE_TRACE("No Enough Memory");
        cJSON_Delete(request_root);
        return -1;
    }

    for (index = 0; index < cJSON_GetArraySize(request_root); index++) {
        item_propertyid = cJSON_GetArrayItem(request_root, index);
        if (item_propertyid == NULL || !cJSON_IsString(item_propertyid)) {
           // EXAMPLE_TRACE("JSON Parse Error");
            cJSON_Delete(request_root);
            cJSON_Delete(response_root);
            return -1;
        }

        //EXAMPLE_TRACE("Property ID, index: %d, Value: %s", index, item_propertyid->valuestring);

        /*
        if (strcmp("WIFI_Tx_Rate", item_propertyid->valuestring) == 0) {
            cJSON_AddNumberToObject(response_root, "WIFI_Tx_Rate", 1111);
        } else if (strcmp("WIFI_Rx_Rate", item_propertyid->valuestring) == 0) {
            cJSON_AddNumberToObject(response_root, "WIFI_Rx_Rate", 2222);
        } else if (strcmp("RGBColor", item_propertyid->valuestring) == 0) {
            cJSON *item_rgbcolor = cJSON_CreateObject();
            if (item_rgbcolor == NULL) {
                cJSON_Delete(request_root);
                cJSON_Delete(response_root);
                return -1;
            }
            cJSON_AddNumberToObject(item_rgbcolor, "Red", 100);
            cJSON_AddNumberToObject(item_rgbcolor, "Green", 100);
            cJSON_AddNumberToObject(item_rgbcolor, "Blue", 100);
            cJSON_AddItemToObject(response_root, "RGBColor", item_rgbcolor);
        } else if (strcmp("HSVColor", item_propertyid->valuestring) == 0) {
            cJSON *item_hsvcolor = cJSON_CreateObject();
            if (item_hsvcolor == NULL) {
                cJSON_Delete(request_root);
                cJSON_Delete(response_root);
                return -1;
            }
            cJSON_AddNumberToObject(item_hsvcolor, "Hue", 50);
            cJSON_AddNumberToObject(item_hsvcolor, "Saturation", 50);
            cJSON_AddNumberToObject(item_hsvcolor, "Value", 50);
            cJSON_AddItemToObject(response_root, "HSVColor", item_hsvcolor);
        } else if (strcmp("HSLColor", item_propertyid->valuestring) == 0) {
            cJSON *item_hslcolor = cJSON_CreateObject();
            if (item_hslcolor == NULL) {
                cJSON_Delete(request_root);
                cJSON_Delete(response_root);
                return -1;
            }
            cJSON_AddNumberToObject(item_hslcolor, "Hue", 70);
            cJSON_AddNumberToObject(item_hslcolor, "Saturation", 70);
            cJSON_AddNumberToObject(item_hslcolor, "Lightness", 70);
            cJSON_AddItemToObject(response_root, "HSLColor", item_hslcolor);
        } else if (strcmp("WorkMode", item_propertyid->valuestring) == 0) {
            cJSON_AddNumberToObject(response_root, "WorkMode", 4);
        } else if (strcmp("NightLightSwitch", item_propertyid->valuestring) == 0) {
            cJSON_AddNumberToObject(response_root, "NightLightSwitch", 1);
        } else if (strcmp("Brightness", item_propertyid->valuestring) == 0) {
            cJSON_AddNumberToObject(response_root, "Brightness", 30);
        } else if (strcmp("LightSwitch", item_propertyid->valuestring) == 0) {
            cJSON_AddNumberToObject(response_root, "LightSwitch", 1);
        } else if (strcmp("ColorTemperature", item_propertyid->valuestring) == 0) {
            cJSON_AddNumberToObject(response_root, "ColorTemperature", 2800);
        } else if (strcmp("PropertyCharacter", item_propertyid->valuestring) == 0) {
            cJSON_AddStringToObject(response_root, "PropertyCharacter", "testprop");
        } else if (strcmp("Propertypoint", item_propertyid->valuestring) == 0) {
            cJSON_AddNumberToObject(response_root, "Propertypoint", 50);
        } else if (strcmp("LocalTimer", item_propertyid->valuestring) == 0) {
            cJSON *array_localtimer = cJSON_CreateArray();
            if (array_localtimer == NULL) {
                cJSON_Delete(request_root);
                cJSON_Delete(response_root);
                return -1;
            }

            cJSON *item_localtimer = cJSON_CreateObject();
            if (item_localtimer == NULL) {
                cJSON_Delete(request_root);
                cJSON_Delete(response_root);
                cJSON_Delete(array_localtimer);
                return -1;
            }
            cJSON_AddStringToObject(item_localtimer, "Timer", "10 11 * * * 1 2 3 4 5");
            cJSON_AddNumberToObject(item_localtimer, "Enable", 1);
            cJSON_AddNumberToObject(item_localtimer, "IsValid", 1);
            cJSON_AddItemToArray(array_localtimer, item_localtimer);
            cJSON_AddItemToObject(response_root, "LocalTimer", array_localtimer);
        }*/
        //cJSON_AddItemToObject(response_root, "CurrentTemperature", 25);
    }
    cJSON_Delete(request_root);

    *response = cJSON_PrintUnformatted(response_root);
    if (*response == NULL) {
        //EXAMPLE_TRACE("No Enough Memory");
        cJSON_Delete(response_root);
        return -1;
    }
    cJSON_Delete(response_root);
    *response_len = strlen(*response);

    //EXAMPLE_TRACE("Property Get Response: %s", *response);
    
    return SUCCESS_RETURN;
}

static int user_report_reply_event_handler(const int devid, const int msgid, const int code, const char *reply,
        const int reply_len)
{
    const char *reply_value = (reply == NULL) ? ("NULL") : (reply);
    const int reply_value_len = (reply_len == 0) ? (strlen("NULL")) : (reply_len);

    //EXAMPLE_TRACE("Message Post Reply Received, Devid: %d, Message ID: %d, Code: %d, Reply: %.*s", devid, msgid, code,
    //reply_value_len,reply_value);
    return 0;
}

static int user_trigger_event_reply_event_handler(const int devid, const int msgid, const int code, const char *eventid,
        const int eventid_len, const char *message, const int message_len)
{
    //EXAMPLE_TRACE("Trigger Event Reply Received, Devid: %d, Message ID: %d, Code: %d, EventID: %.*s, Message: %.*s", devid,
                //  msgid, code,eventid_len,eventid, message_len, message);

    return 0;
}

static int user_timestamp_reply_event_handler(const char *timestamp)
{
    //EXAMPLE_TRACE("Current Timestamp: %s", timestamp);

    return 0;
}

static int user_initialized(const int devid)
{
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();
    //EXAMPLE_TRACE("Device Initialized, Devid: %d", devid);

    if (user_example_ctx->master_devid == devid) {
        user_example_ctx->master_initialized = 1;
    }

    return 0;
}

/** type:
  *
  * 0 - new firmware exist
  *
  */
static int user_fota_event_handler(int type, const char *version)
{
    char buffer[128] = {0};
    int buffer_length = 128;
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();

    if (type == 0) {
        //EXAMPLE_TRACE("New Firmware Version: %s", version);

        IOT_Linkkit_Query(user_example_ctx->master_devid, ITM_MSG_QUERY_FOTA_DATA, (unsigned char *)buffer, buffer_length);
    }

    return 0;
}

/** type:
  *
  * 0 - new config exist
  *
  */
static int user_cota_event_handler(int type, const char *config_id, int config_size, const char *get_type,
                                   const char *sign, const char *sign_method, const char *url)
{
    char buffer[128] = {0};
    int buffer_length = 128;
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();

    if (type == 0) {
        //EXAMPLE_TRACE("New Config ID: %s", config_id);
       // EXAMPLE_TRACE("New Config Size: %d", config_size);
        //EXAMPLE_TRACE("New Config Type: %s", get_type);
        //EXAMPLE_TRACE("New Config Sign: %s", sign);
        //EXAMPLE_TRACE("New Config Sign Method: %s", sign_method);
        //EXAMPLE_TRACE("New Config URL: %s", url);

        IOT_Linkkit_Query(user_example_ctx->master_devid, ITM_MSG_QUERY_COTA_DATA, (unsigned char *)buffer, buffer_length);
    }

    return 0;
}



void user_post_property(void)
{
    static int example_index = 0;
    int res = 0;
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();
    
if(example_index==0)  
    {
        uint8_t PS[18]="{\"PowerSwitch\":0}";
        uint8_t PowerStatus;
        PowerStatus=heartbeatbuf01.SwitchStatus;
        PS[15]=PowerStatus+48;
        char *property_payload;
        property_payload=(const char *)PS;
        example_index++;
        res = IOT_Linkkit_Report(user_example_ctx->master_devid, ITM_MSG_POST_PROPERTY,
                            (unsigned char *)property_payload, strlen(property_payload));
    }
    else if(example_index==1)
    {
        uint8_t TT[25]="{\"TargetTemperature\":25}";
        uint8_t targettemperture=heartbeatbuf01.WaterboxSetTemperature;
        TT[21]=targettemperture/10+48;
        TT[22]=targettemperture%10+48;
        char *property_payload;
        property_payload=(const char *)TT;
        example_index++;
        res = IOT_Linkkit_Report(user_example_ctx->master_devid, ITM_MSG_POST_PROPERTY,
                            (unsigned char *)property_payload, strlen(property_payload));
    }
    else if(example_index==2)
    {
        uint8_t currenttemperature=heartbeatbuf01.WaterboxRealTemperature;
        if(currenttemperature>9)
        {
            uint8_t ppp[26]="{\"CurrentTemperature\":25}";
            ppp[22]=currenttemperature/10+48;
            ppp[23]=currenttemperature%10+48;
            char *property_payload;
            property_payload=(const char *)ppp;
            res = IOT_Linkkit_Report(user_example_ctx->master_devid, ITM_MSG_POST_PROPERTY,
                                (unsigned char *)property_payload, strlen(property_payload));
        }
        else
        {
            uint8_t ppp[25]="{\"CurrentTemperature\":5}";
            ppp[22]=currenttemperature+48;
            char *property_payload;
            property_payload=(const char *)ppp;
            res = IOT_Linkkit_Report(user_example_ctx->master_devid, ITM_MSG_POST_PROPERTY,
                                (unsigned char *)property_payload, strlen(property_payload));
        }
        example_index++;
    }
    else if(example_index==3)
    {
        uint8_t diff;
        diff=heartbeatbuf01.Diff;
        
        if(diff>9)
        {
            uint8_t DF[12]="{\"diff\":15}";
            DF[8]=diff/10+48;
            DF[9]=diff%10+48;
            char *property_payload;
            property_payload=(const char *)DF;
            res = IOT_Linkkit_Report(user_example_ctx->master_devid, ITM_MSG_POST_PROPERTY,
                            (unsigned char *)property_payload, strlen(property_payload)); 
        }
        else
        {
            uint8_t DF[11]="{\"diff\":5}";
            DF[8]=diff+48; 
            char *property_payload;  
            property_payload=(const char *)DF;
            res = IOT_Linkkit_Report(user_example_ctx->master_devid, ITM_MSG_POST_PROPERTY,
                            (unsigned char *)property_payload, strlen(property_payload));
        } 
        example_index++;
    }      

    if(example_index==4)
    {
        uint8_t heaterstatus;
        heaterstatus=heartbeatbuf02.HeaterWorkStaus;
        uint8_t HWS[17]="{\"HeatStatus\":1}";
        HWS[14]=heaterstatus+48;
        char *property_payload;
        property_payload=(const char *)HWS;
        res = IOT_Linkkit_Report(user_example_ctx->master_devid, ITM_MSG_POST_PROPERTY,
                            (unsigned char *)property_payload, strlen(property_payload));
        example_index++;
    }
    else if(example_index==5)
    {
        uint8_t gear;
        gear=heartbeatbuf02.HeaterGear;
        uint8_t WG[11]="{\"gear\":1}";
        WG[8]=gear+48;
        char *property_payload;
        property_payload=(const char *)WG;
        res = IOT_Linkkit_Report(user_example_ctx->master_devid, ITM_MSG_POST_PROPERTY,
                            (unsigned char *)property_payload, strlen(property_payload));
        example_index++;
    }
    else if(example_index==6)
    {
        uint8_t pumptemper;
        pumptemper=heartbeatbuf02.WaterPumpTemperature;
        uint8_t PT[21]="{\"PumpTemperSet\":10}";
        PT[17]=pumptemper/10+48;
        PT[18]=pumptemper%10+48;
        char *property_payload;
        property_payload=(const char *)PT;
        res = IOT_Linkkit_Report(user_example_ctx->master_devid, ITM_MSG_POST_PROPERTY,
                            (unsigned char *)property_payload, strlen(property_payload));
        example_index++;
    }
    else if(example_index==7)
    {
        uint8_t pumpstatus;
        pumpstatus=heartbeatbuf02.WaterPumpWorkStatus;
        uint8_t PWS[17]="{\"PumpStatus\":1}";
        PWS[14]=pumpstatus+48;
        char *property_payload;
        property_payload=(const char *)PWS;
        res = IOT_Linkkit_Report(user_example_ctx->master_devid, ITM_MSG_POST_PROPERTY,
                            (unsigned char *)property_payload, strlen(property_payload));
        example_index++;
    } 
    else if(example_index==8)
    {
        uint8_t tankswitch;
        tankswitch=heartbeatbuf02.TimerSwitch;
        uint8_t TS[17]="{\"tankSwtich\":1}";
        TS[14]=tankswitch+48;
        char *property_payload;
        property_payload=(const char *)TS;
        res = IOT_Linkkit_Report(user_example_ctx->master_devid, ITM_MSG_POST_PROPERTY,
                            (unsigned char *)property_payload, strlen(property_payload));
        example_index++;
    }  
    else if(example_index==9)
    {
        uint8_t workmode;
        workmode=heartbeatbuf0B.Mode;
        uint8_t WM[18]="{\"workModeSet\":0}";
        WM[15]=workmode+48;
        char *property_payload;
        property_payload=(const char *)WM;
        res = IOT_Linkkit_Report(user_example_ctx->master_devid, ITM_MSG_POST_PROPERTY,
                            (unsigned char *)property_payload, strlen(property_payload));
        example_index++;
    }
    else if(example_index==10)
    {
        uint8_t forceoperation;
        forceoperation=heartbeatbuf0B.ForcedOperationMode;
        uint8_t FO[20]="{\"pumpModeForce\":0}";
        FO[17]=forceoperation+48;
        char *property_payload;
        property_payload=(const char *)FO;
        res = IOT_Linkkit_Report(user_example_ctx->master_devid, ITM_MSG_POST_PROPERTY,
                            (unsigned char *)property_payload, strlen(property_payload));
        example_index++;
    } 
    else if(example_index==11)
    {
        uint8_t freconvermode;
        freconvermode=heartbeatbuf0B.FreConverMode;
        uint8_t FCM[21]="{\"freqConverMode\":0}";
        FCM[18]=freconvermode+48;
        char *property_payload;
        property_payload=(const char *)FCM;
        res = IOT_Linkkit_Report(user_example_ctx->master_devid, ITM_MSG_POST_PROPERTY,
                            (unsigned char *)property_payload, strlen(property_payload));
        example_index++; 
    }
    else if(example_index==12)
    {
        uint8_t room1curtemper;
        room1curtemper=heartbeatbuf01.Room1RealTemperature;
        if(room1curtemper>9)
        {
            uint8_t R1T[22]="{\"room1CurTemper\":30}";
            R1T[18]=room1curtemper/10+48;
            R1T[19]=room1curtemper%10+48;
            char *property_payload;
            property_payload=(const char *)R1T;
            res = IOT_Linkkit_Report(user_example_ctx->master_devid, ITM_MSG_POST_PROPERTY,
                                (unsigned char *)property_payload, strlen(property_payload));
        }
        else
        {
            uint8_t R1T[21]="{\"room1CurTemper\":3}";
            R1T[18]=room1curtemper+48;
            char *property_payload;
            property_payload=(const char *)R1T;
            res = IOT_Linkkit_Report(user_example_ctx->master_devid, ITM_MSG_POST_PROPERTY,
                                (unsigned char *)property_payload, strlen(property_payload));
        }
        example_index++; 
    }
    else if(example_index==13)
    {
        uint8_t room2curtemper;
        room2curtemper=heartbeatbuf01.Room2RealTemperature;
        if(room2curtemper>9)
        {
            uint8_t R2T[22]="{\"room2CurTemper\":30}";
            R2T[18]=room2curtemper/10+48;
            R2T[19]=room2curtemper%10+48;
            char *property_payload;
            property_payload=(const char *)R2T;
            res = IOT_Linkkit_Report(user_example_ctx->master_devid, ITM_MSG_POST_PROPERTY,
                                (unsigned char *)property_payload, strlen(property_payload));
        }
        else
        {
            uint8_t R2T[21]="{\"room2CurTemper\":3}";
            R2T[18]=room2curtemper+48;
            char *property_payload;
            property_payload=(const char *)R2T;
            res = IOT_Linkkit_Report(user_example_ctx->master_devid, ITM_MSG_POST_PROPERTY,
                                (unsigned char *)property_payload, strlen(property_payload));
        }
        example_index++; 
    }
    else if(example_index==14)
    {
        uint8_t room3curtemper;
        room3curtemper=heartbeatbuf01.Room3RealTemperature;
        if(room3curtemper>9)
        {
            uint8_t R3T[22]="{\"room3CurTemper\":30}";
            R3T[18]=room3curtemper/10+48;
            R3T[19]=room3curtemper%10+48;
            char *property_payload;
            property_payload=(const char *)R3T;
            res = IOT_Linkkit_Report(user_example_ctx->master_devid, ITM_MSG_POST_PROPERTY,
                                (unsigned char *)property_payload, strlen(property_payload));
        }
        else
        {
            uint8_t R3T[21]="{\"room3CurTemper\":3}";
            R3T[18]=room3curtemper+48;
            char *property_payload;
            property_payload=(const char *)R3T;
            res = IOT_Linkkit_Report(user_example_ctx->master_devid, ITM_MSG_POST_PROPERTY,
                                (unsigned char *)property_payload, strlen(property_payload));
        }
        example_index=0; 
    }
}

void user_post_event(void)
{
    static int event_index = 0;
    int res = 0;
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();
    char *event_id = "EVENT";
    //char *event_payload = "NULL";


    if(ErrorCmd==1)//若无接收到相应数据包调用此函数则执行设备离线
    {
        char *event_payload;
        event_payload = "{\"ErrorCode\":1}";
        res = IOT_Linkkit_TriggerEvent(user_example_ctx->master_devid, event_id, strlen(event_id),
                                   event_payload, strlen(event_payload));
        IOT_Linkkit_Close(user_example_ctx->master_devid);
    }
    else
    {
        if(event_index==0)
        {
            uint8_t room1settemper=heartbeatbuf01.Room1SetTemperature;
            uint8_t room2settemper=heartbeatbuf01.Room2SetTemperature;
            uint8_t room3settemper=heartbeatbuf01.Room3SetTemperature;
            uint8_t RTS[62]="{\"room2TemperSet\":15,\"room1TemperSet\":15,\"room3TemperSet\":15}";
            RTS[18]=room2settemper/10+48;
            RTS[19]=room2settemper%10+48;
            RTS[38]=room1settemper/10+48;
            RTS[39]=room1settemper%10+48;
            RTS[58]=room3settemper/10+48;
            RTS[59]=room3settemper%10+48;
            char *event_payload;
            event_payload=(const char *)RTS;
            res = IOT_Linkkit_TriggerEvent(user_example_ctx->master_devid, event_id, strlen(event_id),
                                    event_payload, strlen(event_payload));
            event_index++;
        }
        else 
        {
            uint8_t onhour;
            uint8_t onmin;
            uint8_t offhour;
            uint8_t offmin;
            uint8_t targettemper;
            uint8_t room1temper;
            uint8_t room2temper;
            uint8_t room3temper;
            uint8_t tankindex;
            if(event_index==1)
            {
                onhour=heartbeatbuf03.OnHour;
                onmin=heartbeatbuf03.OnMin;
                offhour=heartbeatbuf03.OffHour;
                offmin=heartbeatbuf03.OffMin;
                targettemper=heartbeatbuf03.TargetTemper;
                room1temper=heartbeatbuf03.Room1Temper;
                room2temper=heartbeatbuf03.Room2Temper;
                room3temper=heartbeatbuf03.Room3Temper;
                tankindex=1;
                event_index++;
            }
            else if(event_index==2)
            {
                onhour=heartbeatbuf04.OnHour;
                onmin=heartbeatbuf04.OnMin;
                offhour=heartbeatbuf04.OffHour;
                offmin=heartbeatbuf04.OffMin;
                targettemper=heartbeatbuf04.TargetTemper;
                room1temper=heartbeatbuf04.Room1Temper;
                room2temper=heartbeatbuf04.Room2Temper;
                room3temper=heartbeatbuf04.Room3Temper;
                tankindex=2;
                event_index++;
            }
            else if(event_index==3)
            {
                onhour=heartbeatbuf05.OnHour;
                onmin=heartbeatbuf05.OnMin;
                offhour=heartbeatbuf05.OffHour;
                offmin=heartbeatbuf05.OffMin;
                targettemper=heartbeatbuf05.TargetTemper;
                room1temper=heartbeatbuf05.Room1Temper;
                room2temper=heartbeatbuf05.Room2Temper;
                room3temper=heartbeatbuf05.Room3Temper;
                tankindex=3;
                event_index++;
            }
            else if(event_index==4)
            {
                onhour=heartbeatbuf06.OnHour;
                onmin=heartbeatbuf06.OnMin;
                offhour=heartbeatbuf06.OffHour;
                offmin=heartbeatbuf06.OffMin;
                targettemper=heartbeatbuf06.TargetTemper;
                room1temper=heartbeatbuf06.Room1Temper;
                room2temper=heartbeatbuf06.Room2Temper;
                room3temper=heartbeatbuf06.Room3Temper;
                tankindex=4;
                event_index++;
            }
            else if(event_index==5)
            {
                onhour=heartbeatbuf07.OnHour;
                onmin=heartbeatbuf07.OnMin;
                offhour=heartbeatbuf07.OffHour;
                offmin=heartbeatbuf07.OffMin;
                targettemper=heartbeatbuf07.TargetTemper;
                room1temper=heartbeatbuf07.Room1Temper;
                room2temper=heartbeatbuf07.Room2Temper;
                room3temper=heartbeatbuf07.Room3Temper;
                tankindex=5;
                event_index++;
            }
            else if(event_index==6)
            {
                onhour=heartbeatbuf08.OnHour;
                onmin=heartbeatbuf08.OnMin;
                offhour=heartbeatbuf08.OffHour;
                offmin=heartbeatbuf08.OffMin;
                targettemper=heartbeatbuf08.TargetTemper;
                room1temper=heartbeatbuf08.Room1Temper;
                room2temper=heartbeatbuf08.Room2Temper;
                room3temper=heartbeatbuf08.Room3Temper;
                tankindex=6;
                event_index++;
            }
            else if(event_index==7)
            {
                onhour=heartbeatbuf09.OnHour;
                onmin=heartbeatbuf09.OnMin;
                offhour=heartbeatbuf09.OffHour;
                offmin=heartbeatbuf09.OffMin;
                targettemper=heartbeatbuf09.TargetTemper;
                room1temper=heartbeatbuf09.Room1Temper;
                room2temper=heartbeatbuf09.Room2Temper;
                room3temper=heartbeatbuf09.Room3Temper;
                tankindex=7;
                event_index++;
            }
            else if(event_index==8)
            {
                onhour=heartbeatbuf0A.OnHour;
                onmin=heartbeatbuf0A.OnMin;
                offhour=heartbeatbuf0A.OffHour;
                offmin=heartbeatbuf0A.OffMin;
                targettemper=heartbeatbuf0A.TargetTemper;
                room1temper=heartbeatbuf0A.Room1Temper;
                room2temper=heartbeatbuf0A.Room2Temper;
                room3temper=heartbeatbuf0A.Room3Temper;
                tankindex=8;
                event_index=0;
            }
            

            uint8_t TS[159]="{\"timePowerOff\":\"1554890111000\",\"room2TemperSet\":15,\"timePowerOn\":\"1554882910829\",\"room1TemperSet\":15,\"tankIndex\":1,\"room3TemperSet\":15,\"TankTargetTemper\":20}";
            //HAL_Printf("%s",TS);
            struct tm ontime; 
            ontime.tm_year=2019-1900;  
            ontime.tm_mon=4-1;  
            ontime.tm_mday=19;  
            ontime.tm_hour=onhour;  
            ontime.tm_min=onmin;  
            ontime.tm_sec=0;
            uint32_t ontm=mktime(&ontime)-(8*3600);

            TS[67]=ontm/1000000000+48;
            TS[68]=(ontm/100000000)%10+48;
            TS[69]=(ontm/10000000)%10+48;
            TS[70]=(ontm/1000000)%10+48;
            TS[71]=(ontm/100000)%10+48;
            TS[72]=(ontm/10000)%10+48;
            TS[73]=(ontm/1000)%10+48;
            TS[74]=(ontm/100)%10+48;
            TS[75]=(ontm/10)%10+48;
            TS[76]=ontm%10+48;
            TS[77]='0';
            TS[78]='0';
            TS[79]='0';

            struct tm offtime; 
            offtime.tm_year=2019-1900;  
            offtime.tm_mon=4-1;  
            offtime.tm_mday=19;  
            offtime.tm_hour=offhour;  
            offtime.tm_min=offmin;  
            offtime.tm_sec=0;
            uint32_t offtm=mktime(&offtime)-(8*3600);

            TS[17]=offtm/1000000000+48;
            TS[18]=(offtm/100000000)%10+48;
            TS[19]=(offtm/10000000)%10+48;
            TS[20]=(offtm/1000000)%10+48;
            TS[21]=(offtm/100000)%10+48;
            TS[22]=(offtm/10000)%10+48;
            TS[23]=(offtm/1000)%10+48;
            TS[24]=(offtm/100)%10+48;
            TS[25]=(offtm/10)%10+48;
            TS[26]=offtm%10+48;
            TS[27]='0';
            TS[28]='0';
            TS[29]='0';

            TS[49]=room2temper/10+48;
            TS[50]=room2temper%10+48;

            TS[99]=room1temper/10+48;
            TS[100]=room1temper%10+48;

            TS[114]=tankindex+48;

            TS[133]=room3temper/10+48;
            TS[134]=room3temper%10+48;

            TS[155]=targettemper/10+48;
            TS[156]=targettemper%10+48;
            char *event_payload;
            event_payload=(const char *)TS;
            res = IOT_Linkkit_TriggerEvent(user_example_ctx->master_devid, event_id, strlen(event_id),
                                    event_payload, strlen(event_payload));
        }
    }
}

void user_deviceinfo_update(void)
{
    int res = 0;
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();
    char *device_info_update = "[{\"attrKey\":\"abc\",\"attrValue\":\"hello,world\"}]";

    res = IOT_Linkkit_Report(user_example_ctx->master_devid, ITM_MSG_DEVICEINFO_UPDATE,
                             (unsigned char *)device_info_update, strlen(device_info_update));
    //EXAMPLE_TRACE("Device Info Update Message ID: %d", res);
}

void user_deviceinfo_delete(void)
{
    int res = 0;
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();
    char *device_info_delete = "[{\"attrKey\":\"abc\"}]";

    res = IOT_Linkkit_Report(user_example_ctx->master_devid, ITM_MSG_DEVICEINFO_DELETE,
                             (unsigned char *)device_info_delete, strlen(device_info_delete));
    //EXAMPLE_TRACE("Device Info Delete Message ID: %d", res);
}

void user_post_raw_data(void)
{
    int res = 0;
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();
    unsigned char raw_data[7] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};

    res = IOT_Linkkit_Report(user_example_ctx->master_devid, ITM_MSG_POST_RAW_DATA,
                             raw_data, 7);
    //EXAMPLE_TRACE("Post Raw Data Message ID: %d", res);
}

static int user_master_dev_available(void)
{
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();

    if (user_example_ctx->cloud_connected && user_example_ctx->master_initialized) {
        return 1;
    }

    return 0;
}

void set_iotx_info()
{
    memset(FlashData,0,sizeof(FlashData));
    spi_flash_read(0x1f7000, (uint32_t *)&FlashData, 4096);
    if(FlashData[1076]!=0xFF)
    {
        memcpy(device_name,FlashData+1024,20);
        memcpy(device_secrect,FlashData+1044,32);
        
    }
    HAL_SetProductKey(product_key);
    HAL_SetProductSecret(product_secret);
    HAL_SetDeviceName(device_name);
    HAL_SetDeviceSecret(device_secrect);
}

static int max_running_seconds = 0;

int linkkit_main(void *paras)
{

    uint64_t                        time_prev_sec = 0, time_now_sec = 0;
    uint64_t                        time_begin_sec = 0;
    int                             res = 0;
    iotx_linkkit_dev_meta_info_t    master_meta_info;
    user_example_ctx_t             *user_example_ctx = user_example_get_ctx();
#if defined(__UBUNTU_SDK_DEMO__)
    int                             argc = ((app_main_paras_t *)paras)->argc;
    char                          **argv = ((app_main_paras_t *)paras)->argv;

    if (argc > 1) {
        int     tmp = atoi(argv[1]);

        if (tmp >= 60) {
            max_running_seconds = tmp;
            EXAMPLE_TRACE("set [max_running_seconds] = %d seconds\n", max_running_seconds);
        }
    }
#endif
//Uart1_Init();
//Timer_Init();

//GPIO_INIT();
#if !defined(WIFI_PROVISION_ENABLED) || !defined(BUILD_AOS)
    set_iotx_info();
#endif

    memset(user_example_ctx, 0, sizeof(user_example_ctx_t));

    IOT_SetLogLevel(IOT_LOG_NONE);

    /* Register Callback */
    IOT_RegisterCallback(ITE_CONNECT_SUCC, user_connected_event_handler);
    IOT_RegisterCallback(ITE_DISCONNECTED, user_disconnected_event_handler);
    IOT_RegisterCallback(ITE_RAWDATA_ARRIVED, user_down_raw_data_arrived_event_handler);
    IOT_RegisterCallback(ITE_SERVICE_REQUST, user_service_request_event_handler);
    IOT_RegisterCallback(ITE_PROPERTY_SET, user_property_set_event_handler);
    IOT_RegisterCallback(ITE_PROPERTY_GET, user_property_get_event_handler);
    IOT_RegisterCallback(ITE_REPORT_REPLY, user_report_reply_event_handler);
    IOT_RegisterCallback(ITE_TRIGGER_EVENT_REPLY, user_trigger_event_reply_event_handler);
    IOT_RegisterCallback(ITE_TIMESTAMP_REPLY, user_timestamp_reply_event_handler);
    IOT_RegisterCallback(ITE_INITIALIZE_COMPLETED, user_initialized);
    IOT_RegisterCallback(ITE_FOTA, user_fota_event_handler);
    IOT_RegisterCallback(ITE_COTA, user_cota_event_handler);

    memset(&master_meta_info, 0, sizeof(iotx_linkkit_dev_meta_info_t));
    memcpy(master_meta_info.product_key, product_key, strlen(product_key));
    memcpy(master_meta_info.product_secret, product_secret, strlen(product_secret));
    memcpy(master_meta_info.device_name, device_name, strlen(device_name));
    memcpy(master_meta_info.device_secret, device_secrect, strlen(device_secrect));

    /* Choose Login Server, domain should be configured before IOT_Linkkit_Open() */
#if USE_CUSTOME_DOMAIN
    IOT_Ioctl(IOTX_IOCTL_SET_MQTT_DOMAIN, (void *)CUSTOME_DOMAIN_MQTT);
    IOT_Ioctl(IOTX_IOCTL_SET_HTTP_DOMAIN, (void *)CUSTOME_DOMAIN_HTTP);
#else
    int domain_type = IOTX_CLOUD_REGION_SHANGHAI;
    IOT_Ioctl(IOTX_IOCTL_SET_DOMAIN, (void *)&domain_type);
#endif

    /* Choose Login Method */
    int dynamic_register = 0;
    IOT_Ioctl(IOTX_IOCTL_SET_DYNAMIC_REGISTER, (void *)&dynamic_register);

    /* Choose Whether You Need Post Property/Event Reply */
    int post_event_reply = 0;
    IOT_Ioctl(IOTX_IOCTL_RECV_EVENT_REPLY, (void *)&post_event_reply);

    /* Create Master Device Resources */
    user_example_ctx->master_devid = IOT_Linkkit_Open(IOTX_LINKKIT_DEV_TYPE_MASTER, &master_meta_info);
    if (user_example_ctx->master_devid < 0) {
        //EXAMPLE_TRACE("IOT_Linkkit_Open Failed\n");
        return -1;
    }

    /* Start Connect Aliyun Server */
    res = IOT_Linkkit_Connect(user_example_ctx->master_devid);
    if (res < 0) {
        //EXAMPLE_TRACE("IOT_Linkkit_Connect Failed\n");
        return -1;
    }
    //HeartPoint=1;
    time_begin_sec = user_update_sec();
    

    while (1) {
        IOT_Linkkit_Yield(USER_EXAMPLE_YIELD_TIMEOUT_MS);

        time_now_sec = user_update_sec();
        //if (rdb.ReDistributionNetwork==0x00)//重新配网
        
        if (time_prev_sec == time_now_sec) {
            continue;
        }
        if (max_running_seconds && (time_now_sec - time_begin_sec > max_running_seconds)) {
            //EXAMPLE_TRACE("Example Run for Over %d Seconds, Break Loop!\n", max_running_seconds);
            break;
        }
        if(ErrorCmd==2)
        {
            //HAL_Printf("TESTTTTTTTTTTTTTTTTTT\n");
            user_example_ctx->master_devid = IOT_Linkkit_Open(IOTX_LINKKIT_DEV_TYPE_MASTER, &master_meta_info);
            res = IOT_Linkkit_Connect(user_example_ctx->master_devid);
            ErrorCmd=0;
        }
        
        
        //test();
        user_post_property();

        
        
        /* Post Event Example */
        if (time_now_sec % 61 == 0 && user_master_dev_available()) {
            user_post_event();
        }
        /*memset(rxbuff,0,20);
        res = hal_uart_recv_II(&uart1, rxbuff, 23,&rxsizes, 10);
        if(strstr(rxbuff,"read"))
        {
            spi_flash_read(0x1f6000, (uint32_t *)&read_data, LEN*4);
            res = hal_uart_send(&uart1, read_data, sizeof(read_data), 10);
        }
        if(strstr(rxbuff,"erase"))
        {
            haaa=hal_flash_erase(HAL_PARTITION_PARAMETER_1, haha, 1024);
            HAL_Printf("%d",haaa);
        }
        if(strstr(rxbuff,"set"))
        {
            memcpy(tttt,rxbuff+3,rxsizes);
            char *pp=tttt; 
            haaa=hal_flash_write(HAL_PARTITION_PARAMETER_1, &haha, pp, 20);
            HAL_Printf("%d",haaa);
            
        }*/

        /* Device Info Update Example */
        //if (time_now_sec % 23 == 0 && user_master_dev_available()) {
        //    user_deviceinfo_update();
        //}

        /* Device Info Delete Example */
        //if (time_now_sec % 29 == 0 && user_master_dev_available()) {
        //    user_deviceinfo_delete();
        //}

        /* Post Raw Example */
        //if (time_now_sec % 37 == 0 && user_master_dev_available()) {
        //    user_post_raw_data();
        //}

        time_prev_sec = time_now_sec;
    }

    IOT_Linkkit_Close(user_example_ctx->master_devid);

    IOT_DumpMemoryStats(IOT_LOG_NONE);
    IOT_SetLogLevel(IOT_LOG_NONE);

    return 0;
}
#endif

/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
//#include "stdint.h"

#include "aos/cli.h"
#include "aos/kernel.h"
#include "ulog/ulog.h"
#include "aos/yloop.h"

#include "netmgr.h"
#include "iot_export.h"
#include "iot_import.h"
#include "app_entry.h"
#include <aos/hal/gpio.h>
#include <aos/hal/uart.h>
#include <aos/hal/flash.h>
#include "agreement.h"

extern uint8_t HeartPoint;
extern uint8_t LedPoint;
extern gpio_dev_t led;
extern uart_dev_t uart1;
////////////////////////////////////////////
uint32_t haha=1024;
uint32_t haaa;
uint32_t aaaa;
uint32_t p;
int rxsizes;
uint8_t tttt[100];

 #define LEN 1024             //以读写4字节的数据为例


#define SEC 123           //读写的扇区（Sector）号


#define SEC_OFFSET 0      //扇区内偏移量（必须是4的倍数）

uint8_t read_data[LEN*4];
uint8_t rxbuff[1024];
////////////////////////////////////////////
//char haha;

#ifdef CSP_LINUXHOST
#include <signal.h>
#endif

#include <k_api.h>

#if defined(ENABLE_AOS_OTA) 
#include "ota/ota_service.h"
#endif

static char linkkit_started = 0;

#ifdef EN_COMBO_NET
char awss_running = 0;
extern int combo_net_init(void);
#else
static char awss_running    = 0;
#endif

void set_iotx_info();
void do_awss_active();

#ifdef CONFIG_PRINT_HEAP
void print_heap()
{
    extern k_mm_head *g_kmm_head;
    int               free = g_kmm_head->free_size;
    LOG("============free heap size =%d==========", free);
}
#endif

static void wifi_service_event(input_event_t *event, void *priv_data)
{
    if (event->type != EV_WIFI) {
        return;
    }

    if (event->code != CODE_WIFI_ON_GOT_IP) {
        return;
    }

    netmgr_ap_config_t config;
    memset(&config, 0, sizeof(netmgr_ap_config_t));
    netmgr_get_ap_config(&config);
    LOG("wifi_service_event config.ssid %s", config.ssid);
    if (strcmp(config.ssid, "adha") == 0 || strcmp(config.ssid, "aha") == 0) {
        // clear_wifi_ssid();
        return;
    }

 #ifdef EN_COMBO_NET
    if (awss_running) {
        awss_success_notify();
    }
#endif

    if (!linkkit_started) {
#ifdef CONFIG_PRINT_HEAP
        print_heap();
#endif
#ifdef MQTT_DIRECT
        aos_task_new("linkkit", (void (*)(void *))linkkit_main, NULL, 1024 * 6);
#else
        aos_task_new("linkkit", (void (*)(void *))linkkit_main, NULL, 1024 * 8);
#endif
        linkkit_started = 1;
    }
}


static void cloud_service_event(input_event_t *event, void *priv_data)
{
    if (event->type != EV_YUNIO) {
        return;
    }

    LOG("cloud_service_event %d", event->code);

    if (event->code == CODE_YUNIO_ON_CONNECTED) {
        LOG("user sub and pub here");
        return;
    }

    if (event->code == CODE_YUNIO_ON_DISCONNECTED) {
    }
}

/*
 * Note:
 * the linkkit_event_monitor must not block and should run to complete fast
 * if user wants to do complex operation with much time,
 * user should post one task to do this, not implement complex operation in
 * linkkit_event_monitor
 */

static void linkkit_event_monitor(int event)
{
    switch (event) {
        case IOTX_AWSS_START: // AWSS start without enbale, just supports device discover
            // operate led to indicate user
            LOG("IOTX_AWSS_START");
            break;
        case IOTX_AWSS_ENABLE: // AWSS enable, AWSS doesn't parse awss packet until AWSS is enabled.
            LOG("IOTX_AWSS_ENABLE");
            // operate led to indicate user
            break;
        case IOTX_AWSS_LOCK_CHAN: // AWSS lock channel(Got AWSS sync packet)
            LOG("IOTX_AWSS_LOCK_CHAN");
            HeartPoint=0;
            // operate led to indicate user
            break;
        case IOTX_AWSS_PASSWD_ERR: // AWSS decrypt passwd error
            LOG("IOTX_AWSS_PASSWD_ERR");
            // operate led to indicate user
            break;
        case IOTX_AWSS_GOT_SSID_PASSWD:
            LOG("IOTX_AWSS_GOT_SSID_PASSWD");
            // operate led to indicate user
            break;
        case IOTX_AWSS_CONNECT_ADHA: // AWSS try to connnect adha (device
                                     // discover, router solution)
            LOG("IOTX_AWSS_CONNECT_ADHA");
            // operate led to indicate user
            break;
        case IOTX_AWSS_CONNECT_ADHA_FAIL: // AWSS fails to connect adha
            LOG("IOTX_AWSS_CONNECT_ADHA_FAIL");
            // operate led to indicate user
            break;
        case IOTX_AWSS_CONNECT_AHA: // AWSS try to connect aha (AP solution)
            LOG("IOTX_AWSS_CONNECT_AHA");
            // operate led to indicate user
            break;
        case IOTX_AWSS_CONNECT_AHA_FAIL: // AWSS fails to connect aha
            LOG("IOTX_AWSS_CONNECT_AHA_FAIL");
            // operate led to indicate user
            break;
        case IOTX_AWSS_SETUP_NOTIFY: // AWSS sends out device setup information
                                     // (AP and router solution)
            LOG("IOTX_AWSS_SETUP_NOTIFY");
            // operate led to indicate user
            break;
        case IOTX_AWSS_CONNECT_ROUTER: // AWSS try to connect destination router
            LOG("IOTX_AWSS_CONNECT_ROUTER");
            // operate led to indicate user
            break;
        case IOTX_AWSS_CONNECT_ROUTER_FAIL: // AWSS fails to connect destination
                                            // router.
            LOG("IOTX_AWSS_CONNECT_ROUTER_FAIL");
            // operate led to indicate user
            break;
        case IOTX_AWSS_GOT_IP: // AWSS connects destination successfully and got
                               // ip address
            LOG("IOTX_AWSS_GOT_IP");
            // operate led to indicate user
            break;
        case IOTX_AWSS_SUC_NOTIFY: // AWSS sends out success notify (AWSS
                                   // sucess)
            LOG("IOTX_AWSS_SUC_NOTIFY");
            // operate led to indicate user
            break;
        case IOTX_AWSS_BIND_NOTIFY: // AWSS sends out bind notify information to
                                    // support bind between user and device
            LOG("IOTX_AWSS_BIND_NOTIFY");
            // operate led to indicate user
            break;
        case IOTX_AWSS_ENABLE_TIMEOUT: // AWSS enable timeout
                                       // user needs to enable awss again to support get ssid & passwd of router
            LOG("IOTX_AWSS_ENALBE_TIMEOUT");
            LedPoint=0;
            // operate led to indicate user
            break;
        case IOTX_CONN_CLOUD: // Device try to connect cloud
            LOG("IOTX_CONN_CLOUD");
            // operate led to indicate user
            break;
        case IOTX_CONN_CLOUD_FAIL: // Device fails to connect cloud, refer to
                                   // net_sockets.h for error code
            LOG("IOTX_CONN_CLOUD_FAIL");
            LedPoint=0;
            // operate led to indicate user
            break;
        case IOTX_CONN_CLOUD_SUC: // Device connects cloud successfully
            LOG("IOTX_CONN_CLOUD_SUC");
            HeartPoint=1;
            // operate led to indicate user
            break;
        case IOTX_RESET: // Linkkit reset success (just got reset response from
                         // cloud without any other operation)
            LOG("IOTX_RESET");
            // operate led to indicate user
            break;
        default:
            break;
    }
}

static void start_netmgr(void *p)
{
    iotx_event_regist_cb(linkkit_event_monitor);
    LOG("%s\n", __func__);
    aos_msleep(2000);
    netmgr_start(true);
    aos_task_exit(0);
}

void do_awss_active()
{
    LOG("do_awss_active %d\n", awss_running);
    awss_running = 1;
    #ifdef WIFI_PROVISION_ENABLED
    extern int awss_config_press();
    awss_config_press();

    #endif
}

#ifdef SUPPORT_DEV_AP
static void awss_close_dev_ap(void *p)
{
    awss_dev_ap_stop();
    LOG("%s exit\n", __func__);
    aos_task_exit(0);
}

void awss_open_dev_ap(void *p)
{
    iotx_event_regist_cb(linkkit_event_monitor);
    LOG("%s\n", __func__);
    if (netmgr_start(false) < 0) {
        aos_msleep(2000);
        awss_dev_ap_start();
    }
    aos_task_exit(0);
}

static void stop_netmgr(void *p)
{
    awss_stop();
    LOG("%s\n", __func__);
    aos_task_exit(0);
}

void do_awss_dev_ap()
{
    aos_task_new("netmgr_stop", stop_netmgr, NULL, 4096);
    netmgr_clear_ap_config();
    aos_task_new("dap_open", awss_open_dev_ap, NULL, 4096);
}

void do_awss()
{
    aos_task_new("dap_close", awss_close_dev_ap, NULL, 2048);
    netmgr_clear_ap_config();
    aos_task_new("netmgr_start", start_netmgr, NULL, 4096);
}
#endif

static void linkkit_reset(void *p)
{
    netmgr_clear_ap_config();
    HAL_Reboot();
}


extern int  awss_report_reset();
static void do_awss_reset()
{
#ifdef WIFI_PROVISION_ENABLED
    aos_task_new("reset", (void (*)(void *))awss_report_reset, NULL, 6144);
#endif
    aos_post_delayed_action(2000, linkkit_reset, NULL);
}


void linkkit_key_process(input_event_t *eventinfo, void *priv_data)
{
    if (eventinfo->type != EV_KEY) {
        return;
    }
    LOG("awss config press %u\n", eventinfo->value);

    if (eventinfo->code == CODE_BOOT) {
        if (eventinfo->value == VALUE_KEY_CLICK) {

            do_awss_active();
        }
         else if (eventinfo->value == VALUE_KEY_LTCLICK) {
            do_awss_reset();
         }
    }
}

#ifdef AOS_COMP_CLI
static void handle_reset_cmd(char *pwbuf, int blen, int argc, char **argv)
{
    aos_schedule_call(do_awss_reset, NULL);
}

static void handle_active_cmd(char *pwbuf, int blen, int argc, char **argv)
{
    aos_schedule_call(do_awss_active, NULL);
}
#ifdef SUPPORT_DEV_AP
static void handle_dev_ap_cmd(char *pwbuf, int blen, int argc, char **argv)
{
    aos_schedule_call(do_awss_dev_ap, NULL);
}

static void handle_awss_cmd(char *pwbuf, int blen, int argc, char **argv)
{
    aos_schedule_call(do_awss, NULL);
}

static struct cli_command awss_dev_ap_cmd = { .name     = "dev_ap",
                                              .help     = "awss_dev_ap [start]",
                                              .function = handle_dev_ap_cmd };
static struct cli_command awss_cmd = { .name     = "awss",
                                       .help     = "awss [start]",
                                       .function = handle_awss_cmd };
#endif
static struct cli_command resetcmd = { .name     = "reset",
                                       .help     = "factory reset",
                                       .function = handle_reset_cmd };

static struct cli_command awss_enable_cmd = { .name     = "active_awss",
                                              .help     = "active_awss [start]",
                                              .function = handle_active_cmd };
#endif

#ifdef CONFIG_PRINT_HEAP
static void duration_work(void *p)
{
    print_heap();
    aos_post_delayed_action(5000, duration_work, NULL);
}
#endif

static int mqtt_connected_event_handler(void)
{
    LOG("MQTT Construct  OTA start");
#if defined(ENABLE_AOS_OTA) 
    char product_key[PRODUCT_KEY_LEN + 1] = {0};
    char device_name[DEVICE_NAME_LEN + 1] = {0};
    char device_secret[DEVICE_SECRET_LEN + 1] = {0};
    HAL_GetProductKey(product_key);
    HAL_GetDeviceName(device_name);
    HAL_GetDeviceSecret(device_secret);
    static ota_service_t ctx = {0};
    memset(&ctx, 0, sizeof(ota_service_t));
    strncpy(ctx.pk, product_key, sizeof(ctx.pk)-1);
    strncpy(ctx.dn, device_name, sizeof(ctx.dn)-1);
    strncpy(ctx.ds, device_secret, sizeof(ctx.ds)-1);
    ctx.trans_protcol = 0;
    ctx.dl_protcol = 3;
    ota_service_init(&ctx);
#endif
    return 0;
}

int application_start(int argc, char **argv)
{
#ifdef CONFIG_PRINT_HEAP
    print_heap();
    aos_post_delayed_action(5000, duration_work, NULL);
#endif

#ifdef CSP_LINUXHOST
    signal(SIGPIPE, SIG_IGN);
#endif

#ifdef WITH_SAL
    sal_init();
#endif

#ifdef MDAL_MAL_ICA_TEST
    HAL_MDAL_MAL_Init();
#endif

    aos_set_log_level(AOS_LL_NONE);
    Uart1_Init();
    GPIO_INIT();
    spi_flash_read(0x1f7000, (uint32_t *)&read_data, LEN*4);
    while(read_data[1076]==0xff)
    {
        memset(rxbuff,0,sizeof(rxbuff));
        haaa = hal_uart_recv_II(&uart1, rxbuff, 23,&rxsizes, 10);
        if(strstr(rxbuff,"read"))
        {
            spi_flash_read(0x1f7000, (uint32_t *)&read_data, LEN*4);
            haaa = hal_uart_send(&uart1, read_data, 4096, 10);
        }
        if(strstr(rxbuff,"erase"))
        {
            haaa=hal_flash_erase(HAL_PARTITION_PARAMETER_2, haha, 1024);
            HAL_Printf("%d",haaa);
        }
        if(strstr(rxbuff,"set"))
        {
            memcpy(tttt,rxbuff+3,rxsizes-3);
            char *pp=tttt; 
            haaa=hal_flash_write(HAL_PARTITION_PARAMETER_2, &haha, pp, 20);
            HAL_Printf("%d",haaa);
        }
        if(strstr(rxbuff,"break"))
        {
            break;
        }
    }
    
    HeartPoint=1;
    Timer_Init();
    netmgr_init();
    //awss_report_reset();
    //awss_start();
    //netmgr_clear_ap_config();
    
    awss_config_press();
    
    //aos_register_event_filter(EV_KEY, linkkit_key_process, NULL);
    aos_register_event_filter(EV_WIFI, wifi_service_event, NULL);
    aos_register_event_filter(EV_YUNIO, cloud_service_event, NULL);
    IOT_RegisterCallback(ITE_MQTT_CONNECT_SUCC,mqtt_connected_event_handler);

#ifdef AOS_COMP_CLI
    aos_cli_register_command(&resetcmd);
    aos_cli_register_command(&awss_enable_cmd);
#ifdef SUPPORT_DEV_AP
    aos_cli_register_command(&awss_dev_ap_cmd);
    aos_cli_register_command(&awss_cmd);
#endif
#endif
    set_iotx_info();
    IOT_SetLogLevel(IOT_LOG_DEBUG);////////////////////////////////////////////////

#ifdef EN_COMBO_NET
    combo_net_init();
#else
#ifdef SUPPORT_DEV_AP
     aos_task_new("dap_open", awss_open_dev_ap, NULL, 4096);
#else
    aos_task_new("netmgr_start", start_netmgr, NULL, 4096);
#endif
#endif
    aos_loop_run();

    return 0;
}

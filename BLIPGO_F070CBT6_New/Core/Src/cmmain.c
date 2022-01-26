
/*
 ***************************************************************************
 *                            CAREMATIX, INC.
 *                           209 W JACKSON BLVD
 *                           CHICAGO, IL 60606
 *
 * Copyright (c) 2021 Carematix, Inc.  All rights reserved.
 *
 * This source code is an unpublished work of Carematix, Inc.
 * The source code contains confidential, trade secrets of Carematix.
 * Any attempt or participation in deciphering, decoding, reverse
 * engineering or in any way altering the source code is strictly
 * prohibited, unless the prior written consent of Carematix is obtained.
 *
 *
 * Module: cmmain.c
 * Author: J Davis
 * Date: November 11, 2021
 *
 ***************************************************************************
 */

 /*
  * This module contains the led handler.
  */

/***************************************************************************
 *                              INCLUDES
 **************************************************************************/

#include "main.h"
#include "usb_device.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"
#include "winbond_mem.h"
#include "config.h"
#include <cmglobals.h>

#if USB_ENABLED
#include "usbd_cdc_if.h"
#endif
/* USER CODE END Includes */

extern void SystemClock_Config(void);
extern void MX_SPI1_Init(void);
extern void MX_USB_DEVICE_Init(void);


/***************************************************************************
 *                               DEFINES
 **************************************************************************/

 #define USB_MAX_BUFLEN 64

 /***************************************************************************
 *                               TYPEDEFS
 **************************************************************************/

/***************************************************************************
 *                            GLOBAL VARIABLES
 **************************************************************************/


#if 0

uint8_t data_buf[10];
uint8_t gsm_signal_strength = 0;
uint8_t rx_esp = 0;
uint8_t at_timer = 0;
uint8_t gsm;
uint8_t loop_counter = 0;
uint8_t sequence_char[2];
uint8_t mqtt_server_connection_fail_counter = 0;

uint16_t timer_ms = 0;
uint16_t gsm_count = 0;
uint16_t timer_cmd = 0;
uint16_t timer_cmd_esp = 0;
uint16_t rx_esp_counter = 0;
uint16_t check_paired_device_timer = 0;
uint16_t total_records_meter = 0;


uint16_t json_count = 0;
uint16_t curly_brace_count = 0;
uint16_t softap_mode_timer = 0;

uint32_t file_size = 0;

#if INDIA != 0
unsigned char apn[40]="airtelgprs.com";
#elif USA != 0
unsigned char apn[40]="data641003";
#endif




uint8_t rx_string_esp[300];

uint8_t tx_string2[400];
uint8_t gsm_data[300];
uint8_t fota_fail_cause[40];
uint8_t char_total_record[2];
uint8_t wifi_ssid[60];
uint8_t wifi_pswd[60];
uint8_t ble_pairing_key[10];
uint8_t ble_paired_device[18];
uint8_t ble_available_device[18];

uint8_t json_response[600];
uint8_t http_token[17];

uint8_t hold_cv_value[10];



uint8_t meter_model[10];
uint8_t meter_serial_no[20];
uint8_t imei[16];
uint8_t imsi[16];
uint8_t ccid[21];
uint8_t signal_strength[4];
uint8_t module_firmware_version[40];

uint8_t ble_ssid_name[100];


uint8_t read_clock[11];
uint8_t read_meter_model[10];
uint8_t read_meter_serial_no[20];

uint8_t meter_records[100][17];
uint16_t row = 0;
uint16_t green_led_timer = 1;

uint32_t loc = 0;
uint32_t server_clock = 0;


uint8_t registered_to_gprs_network = 0;
uint8_t plus_enable = 1;
uint8_t esp_plus_enable = 1;
uint8_t watchdog_refill_flag = 1;
uint8_t connect_wifi_network_flag = 0;
uint8_t send_softap_response_flag = 0;
uint8_t ble_scan_meter_flag = 0;
uint8_t ble_start_pairing_flag = 0;
uint8_t ble_device_connected_flag = 0;
uint8_t ble_get_data_flag = 0;
uint8_t ble_connect_paired_meter_flag = 0;
uint8_t json_response_flag = 0;
uint8_t configuration_service_flag = 0;
uint8_t ca_certificate_flag = 0;
uint8_t client_certificate_flag = 0;
uint8_t client_key_flag = 0;
uint8_t update_device_twin_flag = 0;
uint8_t connected_to_mqtt_server_flag = 0;
uint8_t server_clock_flag = 0;
uint8_t ping_service_flag = 0;
uint8_t softap_mode_timeup_flag = 0;
uint8_t bg96_power_flag = 0;
uint8_t new_apn_flag = 0;
uint8_t server_clock_update = 0;
uint8_t sending_data_to_server_flag = 0;
uint8_t new_device_id_flag = 0;
uint8_t update_module_firmware_flag = 0;
uint8_t update_device_firmware_flag = 0;
uint8_t read_clock_flag = 0;
uint8_t bootup_complete_flag = 0;

uint8_t desired_reported_cv_matched_flag=0;



struct DataField
{
unsigned char dataId, dataType, dataLength, dataValue[50], decimalPlace;
};
#endif

uint8_t mqtt_server[100];
uint8_t mqtt_port[7];
uint8_t mqtt_username[100];
uint8_t mqtt_password[50];
uint8_t mqtt_client_id[100];

uint8_t protocol_version[5] = {"1.0"};
const uint8_t firmware_version[10] = {"3.0"};
uint8_t device_id[13] = {"H1922000001"};
uint8_t configuration_version[10] = {"3"};
const uint8_t blipgo_model[10] = {"600"};
static uint8_t apn[40]="data641003";




/***************************************************************************
 *                             LOCAL VARIABLES
 **************************************************************************/

static uint8_t twin_sb[100];
static uint8_t twin_pb[100];
static uint8_t twin_rp[100];
static uint8_t read_pb[100];

static uint8_t tx_string[400];
static uint8_t stored_settings[300];
static uint8_t stored_parameters[100];
static uint8_t meter_records[100][17];

static uint8_t temp_data[255];

static uint16_t meter_sequence_number = 0;
static uint16_t total_downloaded_records_meter = 0;

static uint32_t mem_write_address = LOGS_START_ADDRESS;
static uint32_t mem_read_address = LOGS_START_ADDRESS;

/***************************************************************************
 *                         LOCAL FUNCTION PROTOTYPES
 **************************************************************************/

static void explode_string(unsigned char*, uint16_t, uint8_t, uint8_t, uint8_t, uint16_t, uint8_t);
static void flush_tx_string(void);
static uint8_t read_runtime_parameters_from_flash(uint32_t);
static uint8_t read_settings_from_flash(uint32_t);
static void state_machine(SM_STRUC *);
static void store_runtime_parameters_to_flash(uint32_t);
static void store_settings_to_flash(uint32_t);

/***************************************************************************
 *                             GLOBAL FUNCTIONS
 **************************************************************************/


/***************************************************************************
 *                         cmmain
 *                         ------
 *
 * Set yellow led on/off as directed
 *
 * \param[in] - none
 *
 * \return - none
 */

void cmmain(void)
{
    uint8_t flash_data;

    __HAL_RCC_AHB_FORCE_RESET();
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_AHB_RELEASE_RESET();
    SystemClock_Config();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();
    MX_SPI1_Init();
    MX_USB_DEVICE_Init();
    HAL_NVIC_SetPriority(DMA1_Channel4_5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel4_5_IRQn);

#if USA
    sprintf((char*)tx_string, "\r\nBlipGo %s  USA\r\n", firmware_version);
    usb_write((uint8_t *)tx_string);
#elif INDIA
    sprintf((char*)tx_string, "\r\nBlipGo %s  INDIA\r\n", firmware_version);
    usb_write((uint8_t *)tx_string);
#endif

    led_init();
    quec_init();
    esp_init();
    quec_3v8enbl(1);
    beep_init();
    gsm_init();

    beep(2000);
    led_green_set(LED_MODE_ON);
    led_red_set(LED_MODE_ON);
    led_yellow_set(LED_MODE_ON);
    HAL_Delay(2000);
    led_green_set(LED_MODE_OFF);
    led_red_set(LED_MODE_OFF);
    led_yellow_set(LED_MODE_OFF);

    usb_write((uint8_t *)"\r\nESP Turning ON\r\n");
    esp_power_on();

    sFLASH_Init();

#if ERASE_FLASH != 0
    usb_write((uint8_t *)"\r\nErasing Flash...\r\n");
    sFLASH_EraseBulk(0);
    usb_write((uint8_t *)"\r\nFlash Erased !\r\n");

    while(1)
    {
    }
#endif

    flash_data = sFLASH_ReadByte1(0);

    if(flash_data == 0xff || flash_data == '#')
    {
        usb_write((uint8_t *)"\r\nMEMORY OK\r\n");
    }

    else
    {
        usb_write((uint8_t *)"\r\nMEMORY ERROR\r\n");
    }

////////// DEVICE SETTINGS FROM FLASH

    if(!read_settings_from_flash(DEVICE_SETTINGS_ADDRESS))
    {

        if(!read_settings_from_flash(DEVICE_SETTINGS_BACKUP_ADDRESS))
        {
            store_settings_to_flash(DEVICE_SETTINGS_ADDRESS);
            store_settings_to_flash(DEVICE_SETTINGS_BACKUP_ADDRESS);
            read_settings_from_flash(DEVICE_SETTINGS_ADDRESS);
            read_settings_from_flash(DEVICE_SETTINGS_BACKUP_ADDRESS);
        }

        else
        {
            store_settings_to_flash(DEVICE_SETTINGS_ADDRESS);
            read_settings_from_flash(DEVICE_SETTINGS_ADDRESS);
        }

    }

////////// RUNTIME PARAMETERS

    if(!read_runtime_parameters_from_flash(RUNTIME_PARAMETERS_ADDRESS))
    {

        if(!read_runtime_parameters_from_flash(RUNTIME_PARAMETERS_BACKUP_ADDRESS))
        {
            store_runtime_parameters_to_flash(RUNTIME_PARAMETERS_ADDRESS);
            store_runtime_parameters_to_flash(RUNTIME_PARAMETERS_BACKUP_ADDRESS);
            read_runtime_parameters_from_flash(RUNTIME_PARAMETERS_ADDRESS);
            read_runtime_parameters_from_flash(RUNTIME_PARAMETERS_BACKUP_ADDRESS);
        }

        else
        {
            store_runtime_parameters_to_flash(RUNTIME_PARAMETERS_ADDRESS);
            read_runtime_parameters_from_flash(RUNTIME_PARAMETERS_ADDRESS);
        }

    }

    led_green_set(LED_MODE_ON);
    usb_printf((uint8_t *)"INIT COMPLETE\r\n");

    while (1)
    {
    	quec_task();
    	state_machine(&gsm_stmachine);
    }

}

/***************************************************************************
 *                         dma_ch4_5_ih
 *                         ------------
 *
 * Set yellow led on/off as directed
 *
 * \param[in] - none
 *
 * \return - none
 */

void dma_ch4_5_ih(void)
{
    uint32_t isrflags;

    isrflags = DMA1->ISR;

    if (isrflags & DMA_ISR_TCIF4)
    {
        quec_dma_ih();
        return;
    }

    DMA1->IFCR = isrflags;
    return;
}

/***************************************************************************
 *                         crash
 *                         -----
 *
 * Set yellow led on/off as directed
 *
 * \param[in] sect_address
 *
 * \return - none
 */

void crash(void)
{
    NVIC_SystemReset();
    return;
}


/***************************************************************************
 *                         flush_array
 *                         -----------
 *
 * Set yellow led on/off as directed
 *
 * \param[in] sect_address
 *
 * \return - none
 */

void flush_array(uint8_t *str, uint16_t len)
{

    for(uint16_t i = 0; i < len; i++)
    {
        str[i] = 0;
    }

    return;
}




/***************************************************************************
 *                             LOCAL FUNCTIONS
 **************************************************************************/

/***************************************************************************
 *                         explose_string
 *                         --------------
 *
 * Set yellow led on/off as directed
 *
 * \param[in] sect_address
 *
 * \return - none
 */

//*************************************************************
// FUNCTION CAPTURE DATA FIELDS FROM A COMMA SEPERATED STRING
// STRING END CHAR *    STRING SEPERATOR ,
//*************************************************************
static void explode_string(unsigned char* buffer, uint16_t buffer_len, uint8_t sequence_number, uint8_t separator, uint8_t end_char_field, uint16_t field_len, uint8_t end_char_string)
{
    uint8_t comma_count=0;
    uint16_t i=0, j=0;

    flush_array(temp_data, 255);

    for(i=0; i< buffer_len; i++)
    {

        if(buffer[i]==separator)
        {
            comma_count++;
        }

        if(buffer[i] == end_char_string)
        {
            break;
        }

        if(comma_count==sequence_number)//// if sequence found. Note: sequence number of data fields starts from 0
        {

            for(j=0; j<field_len; j++)//// Copy data field, field_len length of field variable
            {

                if(buffer[i+1+j]==end_char_field || buffer[i+1+j]==end_char_string)/// stop copy when , or * reached
                {
                    break;
                }

                temp_data[j] = buffer[i+1+j];/////i+1+j  because already , at index i
            }

            break;
        }

    }

    return;
}

/***************************************************************************
 *                         flush_tx_string
 *                         ---------------
 *
 * Set yellow led on/off as directed
 *
 * \param[in] - none
 *
 * \return - none
 */

static void flush_tx_string(void)
{
    flush_array(tx_string, 400);
    return;
}

/***************************************************************************
 *                         read_runtime_parameters_from_flash
 *                         ----------------------------------
 *
 * Set yellow led on/off as directed
 *
 * \param[in] sect_address
 *
 * \return -
 */

static uint8_t read_runtime_parameters_from_flash(uint32_t sect_address)
{
    uint32_t loc = sect_address;
    uint16_t i=0;

    flush_array(stored_parameters, 100);

    for(i=0; i<100; i++)
    {
        stored_parameters[i] = sFLASH_ReadByte1(loc++);

        if(stored_parameters[i]=='*' || stored_parameters[i]==0xff)
        {
            break;
        }

    }

    //// TTL
    flush_tx_string();
    sprintf((char*)tx_string, "\r\n+READ: %s\r\n", stored_parameters);
    usb_write(tx_string);



    if(stored_parameters[0]=='#' && stored_parameters[i]=='*')
    {
        explode_string(stored_parameters, 100, 1, ',', ',', 5, '*'); //////  meter_sequence_number

        if(temp_data[0]!=0)
        {
            meter_sequence_number = atol((char*)temp_data);
        }


        explode_string(stored_parameters, 100, 2, ',', ',', 8, '*');//////  mem_write_address

        if(temp_data[0]!=0)
        {
            mem_write_address = atol((char*)temp_data);
        }


        explode_string(stored_parameters, 100, 3, ',', ',', 8, '*');//////  mem_read_address

        if(temp_data[0]!=0)
        {
            mem_read_address = atol((char*)temp_data);
        }

        explode_string(stored_parameters, 100, 4, ',', ',', 5, '*');//////  total_downloaded_records_meter

        if(temp_data[0]!=0)
        {
            total_downloaded_records_meter = atol((char*)temp_data);
        }


        //// TTL
        flush_tx_string();

        sprintf((char*)tx_string,
                "+PARSED: LSN:%u\nWA:%u  Rem:%u\nRA:%u  Rem:%u\nTDRM:%u\r\n\r\n",
                (unsigned int)meter_sequence_number,
                (unsigned int)mem_write_address,
                (unsigned int)(mem_write_address-LOGS_START_ADDRESS) % 17,
                (unsigned int)mem_read_address,
                (unsigned int)(mem_read_address-LOGS_START_ADDRESS) % 17,
                (unsigned int)total_downloaded_records_meter);

        usb_write(tx_string);
        return 1;
    }

    return 0;
}



/***************************************************************************
 *                         read_settings_from_flash
 *                         ------------------------
 *
 * Set yellow led on/off as directed
 *
 * \param[in] sect_address
 *
 * \return -
 */

static uint8_t read_settings_from_flash(uint32_t sect_address)
{
    uint32_t loc = sect_address;
    uint16_t i=0;

    flush_array(stored_settings, 300);

    for(i=0; i<300; i++)
    {
        stored_settings[i] = sFLASH_ReadByte1(loc++);

        if(stored_settings[i]=='*' || stored_settings[i]==0xff)
        break;
    }

//// TTL
    flush_tx_string();
    sprintf((char*)tx_string, "\r\n+READ: %s\r\n", stored_settings);
//    send_text_to_uart2(tx_string);
    usb_write(tx_string);



    if(stored_settings[0]=='#' && stored_settings[i]=='*')
    {
        explode_string(stored_settings, 300, 1, ',', ',', 40, '*');//////  APN

        if(temp_data[0]!=0)
        {
           flush_array(apn, 40);
           sprintf((char*)apn, "%s", temp_data);
        }


        explode_string(stored_settings, 300, 2, ',', ',', 10, '*');//////  Configuration Version

        if(temp_data[0]!=0)
        {
            flush_array(configuration_version, 10);
            sprintf((char*)configuration_version, "%s", temp_data);
        }


        explode_string(stored_settings, 300, 3, ',', ',', 100, '*');//////  mqtt_server

        if(temp_data[0]!=0)
        {
            flush_array(mqtt_server, 100);
            sprintf((char*)mqtt_server, "%s", temp_data);
        }


        explode_string(stored_settings, 300, 4, ',', ',', 7, '*');//////  mqtt_port

        if(temp_data[0]!=0)
        {
            flush_array(mqtt_port, 7);
            sprintf((char*)mqtt_port, "%s", temp_data);
        }


        explode_string(stored_settings, 300, 5, ',', ',', 100, '*');//////  mqtt_username

        if(temp_data[0]!=0)
        {
            flush_array(mqtt_username, 100);
            sprintf((char*)mqtt_username, "%s", temp_data);
        }

        explode_string(stored_settings, 300, 6, ',', ',', 50, '*');//////  mqtt_password

        if(temp_data[0]!=0)
        {
            flush_array(mqtt_password, 50);
            sprintf((char*)mqtt_password, "%s", temp_data);
        }

        explode_string(stored_settings, 300, 7, ',', ',', 100, '*');//////  mqtt_client_id

        if(temp_data[0]!=0)
        {
            flush_array(mqtt_client_id, 100);
            sprintf((char*)mqtt_client_id, "%s", temp_data);
        }

        explode_string(stored_settings, 300, 8, ',', ',', 100, '*');//////  twin_sb

        if(temp_data[0]!=0)
        {
            flush_array(twin_sb, 100);
            sprintf((char*)twin_sb, "%s", temp_data);
        }

        explode_string(stored_settings, 300, 9, ',', ',', 100, '*');//////  twin_pb

        if(temp_data[0]!=0)
        {
            flush_array(twin_pb, 100);
            sprintf((char*)twin_pb, "%s", temp_data);
        }

        explode_string(stored_settings, 300, 10, ',', ',', 100, '*');//////  twin_rp

        if(temp_data[0]!=0)
        {
            flush_array(twin_rp, 100);
            sprintf((char*)twin_rp, "%s", temp_data);
        }

        explode_string(stored_settings, 300, 11, ',', ',', 100, '*');//////  read_pb

        if(temp_data[0]!=0)
        {
            flush_array(read_pb, 100);
            sprintf((char*)read_pb, "%s", temp_data);
        }

        explode_string(stored_settings, 300, 12, ',', ',', 5, '*');//////  protocol_version

        if(temp_data[0]!=0)
        {
            flush_array(protocol_version, 5);
            sprintf((char*)protocol_version, "%s", temp_data);
        }

        explode_string(stored_settings, 300, 13, ',', ',', 12, '*');//////  device_id

        if(temp_data[0]!=0)
        {
            flush_array(device_id, 12);
            sprintf((char*)device_id, "%s", temp_data);
        }

        //// TTL
        flush_tx_string();

        sprintf((char*)tx_string, "\r\n+PARSED: APN:%s\nCV:%s\nSERVER:%s\nPORT:%s\nUSER:%s\nPASWD:%s\nCLIENT:%s\nTWIN_SB:%s\nTWIN_PB:%s\nTWIN_RP:%s\nREAD_PB:%s\nPV:%s\nDEVICE_ID:%s\r\n",
                apn,configuration_version,mqtt_server,mqtt_port,mqtt_username,mqtt_password,
                mqtt_client_id,twin_sb,twin_pb,twin_rp,read_pb,protocol_version,device_id);

 //       send_text_to_uart2(tx_string);
        usb_write(tx_string);

        return 1;
    }

    return 0;
}

/***************************************************************************
 *                         state_machine
 *                         -------------
 *
 * Set yellow led on/off as directed
 *
 * \param[in] - none
 *
 * \return - none
 */

static void state_machine(SM_STRUC *stinfo)
{
    uint32_t evindex;
    uint32_t evresult;
    S_TABLE **statearray;
    S_TABLE *curstateitem;
    void(*actsubr1)(void);
    void(*actsubr2)(void);
    uint32_t(*evsubr)(void);
    void(*dbgsubr)(uint32_t);

    statearray = stinfo->sms_stable;
    curstateitem = statearray[stinfo->sms_curstate];

    evindex = 0;
    evresult = 0;

    while (1)
    {
        evsubr = curstateitem->stbl_event;
        evresult = evsubr();

        if (evresult != 0)
        {
            actsubr1 = curstateitem->stbl_act1;
            actsubr2 = curstateitem->stbl_act2;
            dbgsubr = stinfo->sms_debug;

            stinfo->sms_curstate = curstateitem->stbl_newstate;
            dbgsubr(evindex);
            evindex = 0;
            curstateitem = statearray[stinfo->sms_curstate];

            if (actsubr1 == NULL)
            {
                break;
            }

            actsubr1();

            if (actsubr2 == NULL)
            {
                break;
            }

            actsubr2();
        }

        else
        {
            evindex++;
            curstateitem++;
        }

    }

    return;
}

/***************************************************************************
 *                         store_runtime_parameters_to_flash
 *                         ---------------------------------
 *
 * Set yellow led on/off as directed
 *
 * \param[in] sect_address
 *
 * \return -
 */


static void store_runtime_parameters_to_flash(uint32_t sect_address)
{
    uint32_t loc = sect_address;

    flush_array(stored_parameters, 100);

    sprintf((char*)stored_parameters,
            "#SET,%u,%u,%u,%u*",
            (unsigned int)meter_sequence_number,
            (unsigned int)mem_write_address,
            (unsigned int)mem_read_address,
            (unsigned int)total_downloaded_records_meter);

    ///// ERASE SECTOR IN FLASH
    sFLASH_EraseSector(sect_address);

    for(uint16_t i=0; i<strlen((char*)stored_parameters); i++)
    {

        if(stored_settings[i] == 0)
        {
            break;
        }

        sFLASH_WriteByte(loc++, stored_parameters[i]);
    }

    return;
}


/***************************************************************************
 *                         store_settings_to_flash
 *                         -----------------------
 *
 * Set yellow led on/off as directed
 *
 * \param[in] sect_address
 *
 * \return -
 */

static void store_settings_to_flash(uint32_t sect_address)
{
    uint32_t loc = sect_address;

    flush_array(stored_settings, 300);

    sprintf((char*)stored_settings, "#SET,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s*",
            apn,configuration_version,mqtt_server,mqtt_port,mqtt_username,mqtt_password,
            mqtt_client_id,twin_sb,twin_pb,twin_rp,read_pb,protocol_version,device_id);

    ///// ERASE SECTOR IN FLASH
    sFLASH_EraseSector(sect_address);

    for(uint16_t i=0; i<strlen((char*)stored_settings); i++)
    {

        if(stored_settings[i] == 0)
        {
            break;
        }

        sFLASH_WriteByte(loc++, stored_settings[i]);
    }

//// TTL
    flush_tx_string();
    sprintf((char*)tx_string, "\r\n+STORED: %s\r\n", stored_settings);
    usb_write(tx_string);
    return;
}


/*
 * End of module.
 */


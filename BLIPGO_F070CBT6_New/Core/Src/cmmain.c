
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



uint16_t curly_brace_count = 0;
uint16_t softap_mode_timer = 0;

uint32_t file_size = 0;





uint8_t rx_string_esp[300];

uint8_t tx_string2[400];
uint8_t gsm_data[300];
uint8_t fota_fail_cause[40];
uint8_t char_total_record[2];
uint8_t wifi_ssid[60];
uint8_t wifi_pswd[60];













uint8_t signal_strength[4];







uint16_t row = 0;
uint16_t green_led_timer = 1;

uint32_t loc = 0;



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



uint8_t connected_to_mqtt_server_flag = 0;
uint8_t server_clock_flag = 0;
uint8_t ping_service_flag = 0;
uint8_t softap_mode_timeup_flag = 0;
uint8_t bg96_power_flag = 0;
uint8_t new_apn_flag = 0;
uint8_t server_clock_update = 0;
uint8_t sending_data_to_server_flag = 0;
uint8_t new_device_id_flag = 0;

uint8_t read_clock_flag = 0;
uint8_t bootup_complete_flag = 0;






#endif


uint8_t ca_certificate_flag;
uint8_t client_certificate_flag;
uint8_t client_key_flag;
uint8_t configuration_service_flag;
uint8_t update_module_firmware_flag;
uint8_t update_device_firmware_flag;
uint8_t update_device_twin_flag;
uint8_t sending_readings;
uint8_t fetching_readings;
uint8_t mqtt_server[100];
uint8_t mqtt_port[7];
uint8_t mqtt_username[100];
uint8_t mqtt_password[50];
uint8_t mqtt_client_id[100];
uint8_t json_response[JRBUF_LEN];
uint16_t json_count;
uint8_t desired_reported_cv_matched_flag;
uint8_t http_token[17];

uint32_t server_clock;
uint8_t protocol_version[5] = {"1.0"};
const uint8_t firmware_version[10] = {"1.1"};
uint8_t device_id[13];
uint8_t configuration_version[10];
const uint8_t blipgo_model[10] = {"600"};
static uint8_t apn[40];
uint8_t ble_ssid_name[50];
uint8_t ble_pairing_key[PAIRINGKEY_MAXLEN + 1];
uint8_t ble_available_device[18];
uint8_t ble_paired_device[18];
uint8_t meter_model[METERMODEL_LEN + 1];
uint8_t meter_serial_no[METERSERIAL_LEN + 1];
uint8_t twin_sb[100];
uint8_t twin_pb[100];
uint8_t twin_rp[100];
uint8_t read_pb[100];
uint32_t mem_write_address = LOGS_START_ADDRESS;
uint32_t mem_read_address = LOGS_START_ADDRESS;
uint8_t imei[16];
uint8_t ccid[21];
uint8_t imsi[16];
uint8_t module_firmware_version[MFVLEN];
uint8_t meter_records[100][17];




/***************************************************************************
 *                             LOCAL VARIABLES
 **************************************************************************/

static uint8_t gsm_signal_strength;
static uint8_t stored_settings[300];
static uint8_t stored_parameters[100];
static uint8_t hold_cv_value[10];




static uint8_t temp_data[255];
static uint8_t read_clock[11];
static uint8_t read_meter_model[10];
static uint8_t read_meter_serial_no[20];


static uint16_t meter_sequence_number = 0;
static uint16_t total_downloaded_records_meter = 0;

struct DataField
{
    uint8_t dataId;
    uint8_t dataType;
    uint8_t dataLength;
    uint8_t dataValue[50];
    uint8_t decimalPlace;
};

/***************************************************************************
 *                         LOCAL FUNCTION PROTOTYPES
 **************************************************************************/

static void explode_string(unsigned char*, uint16_t, uint8_t, uint8_t, uint8_t, uint16_t, uint8_t);
static void read_clock_from_flash(void);
static void read_devinfo_from_flash(uint32_t);
static uint8_t read_runtime_parameters_from_flash(uint32_t);
static uint8_t read_settings_from_flash(uint32_t);
static void store_clock_to_flash(void);
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
    usb_init();
    HAL_NVIC_SetPriority(DMA1_Channel4_5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel4_5_IRQn);



    debug_printf(DBGLVL_MAX,(uint8_t *)"\r\nBlipGo %s USA\r\n", firmware_version);


	server_clock = 0;

    debuglevel = DBGLVL_MAX;
    mem_write_address = LOGS_START_ADDRESS;
    mem_read_address = LOGS_START_ADDRESS;
	desired_reported_cv_matched_flag = 0;
	sending_readings = 0;
	fetching_readings = 0;


    led_init();
    cons_init();
    quec_init();
    esp_init();

#if OLD_BOARD != 0
    quec_3v8enbl(1);
#endif

    beep_init();
    gsm_init();
    espat_init();
    master_init();

    beep(2000);
    led_green_set(LED_MODE_ON);
    led_red_set(LED_MODE_ON);
    led_yellow_set(LED_MODE_ON);
    HAL_Delay(2000);
    led_green_set(LED_MODE_OFF);
    led_red_set(LED_MODE_OFF);
    led_yellow_set(LED_MODE_OFF);

    debug_printf(DBGLVL_MAX,(uint8_t *)"\r\nESP Turning ON\r\n");
    esp_power_on();

    sFLASH_Init();

#if ERASE_FLASH != 0
    debug_printf(DBGLVL_MAX,(uint8_t *)"\r\nErasing Flash...\r\n");
    sFLASH_EraseBulk(0);
    debug_printf(DBGLVL_MAX,(uint8_t *)"\r\nFlash Erased !\r\n");

    while(1)
    {
    }
#endif

    flash_data = sFLASH_ReadByte1(0);

    if(flash_data == 0xff || flash_data == '#')
    {
        debug_printf(DBGLVL_MAX,(uint8_t *)"\r\nMEMORY OK\r\n");
    }

    else
    {
        debug_printf(DBGLVL_MAX,(uint8_t *)"\r\nMEMORY ERROR\r\n");
    }

    read_devinfo_from_flash(DEVICE_ID_ADDRESS);

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

	sprintf((char*)ble_ssid_name, "BlipGo+%s\0", device_id);
    led_green_set(LED_MODE_ON);
    debug_printf(DBGLVL_MAX, (uint8_t *)"INIT COMPLETE\r\n");

    while (1)
    {
        cons_task();
    	quec_task();
    	esp_task();
    	led_task();
    	state_machine(&gsm_stmachine);
    	state_machine(&esp_stmachine);
    	state_machine(&master_stmachine);
    }

}


/***************************************************************************
 *                         compare_cvs
 *                         ------------
 *
 * Set yellow led on/off as directed
 *
 * \param[in] - none
 *
 * \return - none
 *
 *
 *
 * 	+QMTRECV: 0,0,"$iothub/twin/res/200/?$rid=1","{
 *	"desired": {
 * 				"sd": "S2001C052080000000001620110Z",
 *				"cv": "4",
 *				"$version": 137
 *				},
 *	"reported": {
 *				"fv": "1.0",
 *				"pv": "1.0",
 *				"cv": "4",
 *				"imei": "864475049752735",
 *				"imsi": "404882990776241",
 *				"ccid": "89910271001167472368",
 *				"mv": "BG95M3LAR02A03_01.009.01.009",
 *				"sim": "89910273519000076474",
 *				"$version": 296
 *				}
 *					}"
 *
 */


void compare_cvs(void)
{
    uint16_t desired_cv;
    uint16_t reported_cv;

    desired_cv = 0;
    reported_cv = 0;

	if (json_get_value("sd", 2, 1, (char*)temp_data, 100))/////// GET SETUP DATA VALUE FROM JSON RESPONSE
	{
	    debug_printf(DBGLVL_MIN, (uint8_t *)"Setup Data: &s\r\n", temp_data);
	}

	if (json_get_value("cv", 2, 1, (char*)temp_data, 100))/////// GET SETUP DATA VALUE FROM JSON RESPONSE
	{
	    debug_printf(DBGLVL_MIN, (uint8_t *)"Desired Configuration Version: &s\r\n", temp_data);
    }

	desired_cv = atol((char*)temp_data);

	if (json_get_value("cv", 2, 2, (char*)temp_data, 100))/////// GET SETUP DATA VALUE FROM JSON RESPONSE
	{
	    debug_printf(DBGLVL_MIN, (uint8_t *)"Reported Configuration Version: &s\r\n", temp_data);
	}

	reported_cv = atol((char*)temp_data);

	if (desired_cv == reported_cv)
	{
		desired_reported_cv_matched_flag = 1;
	}

	else
	{
		desired_reported_cv_matched_flag = 0;
	}

    return;
}

/***************************************************************************
 *                         configuration_service
 *                         ------------
 *
 * Set yellow led on/off as directed
 *
 * \param[in] - none
 *
 * \return - none
 */

void configuration_service(void)
{



//	if(strstr((char*)gsm_data, "Unauthorized"))
    {

	}


	if (json_get_value("mqtt", 4, 1, (char*)temp_data, 100))/////// GET MQTT VERSION VALUE FROM JSON RESPONSE
	{

	}

	if(json_get_value("tls", 3, 1, (char*)temp_data, 100))/////// GET TLS VERSION VALUE FROM JSON RESPONSE
    {

	}

	if(json_get_value("did", 3, 1, (char*)temp_data, 100))/////// GET DEVICE ID VALUE FROM JSON RESPONSE
	{

	}

	if(json_get_value("svr", 3, 1, (char*)mqtt_server, 100))/////// GET SERVER HOST NAME VALUE FROM JSON RESPONSE
	{

	}

	if(json_get_value("prt", 3, 1, (char*)mqtt_port, 7))/////// GET PORT VALUE FROM JSON RESPONSE
	{

	}

	if(json_get_value("un", 2, 1, (char*)mqtt_username, 100))/////// GET USERNAME VALUE FROM JSON RESPONSE
	{

	}

	if(json_get_value("pd", 2, 1, (char*)mqtt_password, 50))/////// GET PASSWORD VALUE FROM JSON RESPONSE
	{

	}

	if(json_get_value("cid", 3, 1, (char*)mqtt_client_id, 100))/////// GET CLIENT ID VALUE FROM JSON RESPONSE
    {

	}

	if(json_get_value("qos", 3, 1, (char*)temp_data, 100))/////// GET QOS VALUE FROM JSON RESPONSE
	{

	}

	if(json_get_value("twin_sb", 7, 1, (char*)twin_sb, 100))/////// GET SUBSCRIBE TOPIC VALUE FROM JSON RESPONSE
	{

	}

	if(json_get_value("twin_pb", 7, 1, (char*)twin_pb, 100))/////// GET PUBLISH TOPIC FOR NULL DATA VALUE FROM JSON RESPONSE
	{

	}

	if(json_get_value("twin_rp", 7, 1, (char*)twin_rp, 100))/////// GET PUBLISH TOPIC TO REPORT META DATA VALUE FROM JSON RESPONSE
	{

	}

	if(json_get_value("read_pb", 7, 1, (char*)read_pb, 100))/////// GET PUBLISH TOPIC FOR MEASUREMENT DATA VALUE FROM JSON RESPONSE
	{

	}



	return;
}




/***************************************************************************
 *                         convert_to_epoch
 *                         ------------
 *
 * Set yellow led on/off as directed
 *
 * \param[in] - none
 *
 * \return - none
 */


uint32_t convert_to_epoch(uint8_t *ascdate, uint8_t *asctime)
{
	struct tm t;

	uint32_t t_of_day;

	unsigned char yr[5]={0}, mon[3]={0}, day[3]={0}, hr[3]={0}, min[3]={0}, sec[3]={0};

	 yr[0] = '2';
	 yr[1] = '0';
	 yr[2] = ascdate[2];
	 yr[3] = ascdate[3];

	 mon[0] = ascdate[5];
	 mon[1] = ascdate[6];

	 day[0] = ascdate[8];
	 day[1] = ascdate[9];

	 hr[0] = asctime[0];
	 hr[1] = asctime[1];

	 min[0] = asctime[3];
	 min[1] = asctime[4];

	 sec[0] = asctime[6];
	 sec[1] = asctime[7];

	 t.tm_year = atoi((char*)yr)-1900;  // Year - 1970
		t.tm_mon = atoi((char*)mon)-1;           // Month, where 0 = jan
		t.tm_mday = atoi((char*)day);          // Day of the month
		t.tm_hour = atoi((char*)hr);
		t.tm_min = atoi((char*)min);
		t.tm_sec = atoi((char*)sec);
		t.tm_isdst = 0;        // Is DST on? 1 = yes, 0 = no, -1 = unknown
		t_of_day = mktime(&t);

		return t_of_day;

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
 *                         erase_all_records
 *                         --------------
 *
 * Set yellow led on/off as directed
 *
 * \param[in] sect_address
 *
 * \return - none
 */

void erase_all_records(void)
{
	sFLASH_EraseBulk(LOGS_START_ADDRESS);

	mem_write_address=LOGS_START_ADDRESS;
	mem_read_address=LOGS_START_ADDRESS;

	store_runtime_parameters_to_flash(RUNTIME_PARAMETERS_ADDRESS);
	store_runtime_parameters_to_flash(RUNTIME_PARAMETERS_BACKUP_ADDRESS);
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
 *                         json_get_value
 *                         -----------
 *
 * Set yellow led on/off as directed
 *
 * \param[in] sect_address
 *
 * \return - none
 *
 * ///{"st":1,"ca":1260,"ck":1678,"cc":1479,"sc":398,"cv":"1","tk":"wCCgqF6gDTWC3qWg"}
 * //// occurance is added to get value in case of multiple occurance
 */

uint8_t json_get_value(uint8_t *parameter, uint8_t parameter_length, uint8_t occurance, uint8_t *variable, uint8_t variable_size)
{
	uint8_t parameter_index;
	uint8_t variable_index;
	uint8_t occurance_count;
	uint8_t bracket_flag;
	uint8_t j;
	uint16_t i;

	parameter_index=0;
	variable_index=0;
	occurance_count=0;
	bracket_flag=0;

	for(i=0; i <= json_count; i++)
	{
		if(json_response[i] == parameter[parameter_index])
		{
			if(parameter_index == parameter_length-1)////// PARAMETER FOUND CONDITION
			{
				occurance_count++;

				if(occurance_count==occurance)
				{
					for(j = 0; j < variable_size; j++)/////// FLUSH VARIABLE
					{
						variable[j] = 0;
					}

					if(json_response[i+3]==' ')//////////// CONDITION FOR space after semicolon "sd": "123123"
						i = i+1;

					if(json_response[i+3]=='"')//////////// CONDITION FOR NUMERIC OR STRING VALUE
						i = i+4;
					else if(json_response[i+3]=='{')//////////// CONDITION FOR "desired": { }
						bracket_flag = 1;
					else
						i = i+3;

					for(i=i; i <= json_count; i++)/////// COPY PARAMETER VALUE TO VARIABLE
					{
						if(bracket_flag && json_response[i]=='}')////// COPY COMPLETE CONDITION IN CASE OF "desired": { }
						{
							if(variable_index < variable_size)////////  TO AVOID BUFFER OVERFLOW
								variable[variable_index++] = json_response[i];
							return 1;
						}
						else if(!bracket_flag && (json_response[i]=='"' || json_response[i]==',' || json_response[i]=='}'))////// COPY COMPLETE CONDITION
						{
							return 1;
						}
						else
						{
							if(variable_index < variable_size)////////  TO AVOID BUFFER OVERFLOW
								variable[variable_index++] = json_response[i];
						}
					}
				}
				else
				{
					goto SEARCH_AGAIN;
				}
			}

			parameter_index++;
		}
		else
		{
SEARCH_AGAIN:
			parameter_index = 0;
		}
	}

	return 0;
}


/***************************************************************************
 *                         ping_response
 *                         -----------
 *
 * Set yellow led on/off as directed
 *
 * \param[in] sect_address
 *
 * \return - none
 */

uint8_t ping_response(void)
{
    uint8_t retstat;
    uint32_t i;
    uint8_t tempcv[10];

    ca_certificate_flag = 0;
    client_certificate_flag = 0;
    client_key_flag = 0;
    configuration_service_flag = 0;
    update_module_firmware_flag = 0;
    update_device_firmware_flag = 0;
    update_device_twin_flag = 0;

	if(json_response[0] == 0)
	{
		json_response[0] = '0';
	}

	if(json_get_value("st", 2, 1, (char*)temp_data, 100))/////// GET STATUS VALUE FROM JSON RESPONSE
	{

	}

	else
	{
		return 0;
	}

    retstat = 0;

	if(temp_data[0] == '1')////////////// IF STATUS is 1
	{

        if(json_get_value("cv", 2, 1, (char*)tempcv, 10))/////// GET SERVER CONFIGURATION VERSION VALUE FROM JSON RESPONSE
		{

		}

		if(configuration_version[0]!=tempcv[0] || configuration_version[1]!=tempcv[1] || configuration_version[2]!=tempcv[2])
		{
			sprintf((char*)hold_cv_value, "%s", (char*)temp_data);///// HOLD CV VALUE IN TEMP VARIABLE

			if(json_get_value("tk", 2, 1, (char*)http_token, 17))/////// GET HTTP TOKEN VALUE FROM JSON RESPONSE
			{

			}

			if(json_get_value("ca", 2, 1, (char*)temp_data, 100))/////// GET ROOT CA VALUE FROM JSON RESPONSE
			{
			    ca_certificate_flag = 1;
			}

			if(json_get_value("cc", 2, 1, (char*)temp_data, 100))/////// GET CLIENT CERTIFICATE VALUE FROM JSON RESPONSE
			{
                client_certificate_flag = 1;
			}

			if(json_get_value("ck", 2, 1, (char*)temp_data, 100))/////// GET CLIENT KEY VALUE FROM JSON RESPONSE
			{
                client_key_flag = 1;
			}

			if(json_get_value("sc", 2, 1, (char*)temp_data, 100))/////// GET SERVER CONFIGURATION VALUE FROM JSON RESPONSE
			{
				configuration_service_flag = 1;
			}

			if(json_get_value("fv", 2, 1, (char*)temp_data, 100))/////// GET Firmware Version VALUE FROM JSON RESPONSE
			{
				update_device_firmware_flag = 1;
			}

			if(json_get_value("mv", 2, 1, (char*)temp_data, 100))/////// GET Module Version VALUE FROM JSON RESPONSE
			{
				update_module_firmware_flag = 1;
			}

            for (i = 0; i < sizeof(configuration_version); i++)
            {
                configuration_version[i] = tempcv[i];
            }

		}

		update_device_twin_flag=1;
	}

	else if(temp_data[0] == '0')////////////// IF STATUS is 0 ///// Added 25.04.2021
	{
		ca_certificate_flag = 0;
		client_certificate_flag = 0;
		client_key_flag = 0;
		configuration_service_flag = 0;
		update_device_firmware_flag = 0;
		update_module_firmware_flag = 0;
		retstat = 1;

		update_device_twin_flag=1;
	}

	return retstat;
}





/***************************************************************************
 *                         publish_meter_readings
 *                         ---------------------------------
 *
 * Set yellow led on/off as directed
 *
 * \param[in] sect_address
 *
 * \return -
 */

uint16_t publish_meter_readings(uint8_t *tx_string)
{
	uint8_t data_byte;
    uint8_t zz;

	uint16_t total_length=0;

	uint32_t t;

	struct DataField serial_number;
	struct DataField timestamp;
	struct DataField model_number;
	struct DataField meter_id;
	struct DataField raw_data;
	struct DataField signal_strength;

	uint8_t string1[40];
	uint8_t string2[40];
	uint8_t string3[40];
	uint8_t string4[40];
	uint8_t string5[40];
	uint8_t string6[50];
	uint8_t packet_length[2];
	uint8_t string7[100];
	uint8_t bytes[4];




    total_length=0;

	////// BLIPGO SERIAL NUMBER
	serial_number.dataId=0x1A;
	serial_number.dataType='1';
	serial_number.decimalPlace='0';
	sprintf((char*)serial_number.dataValue, "%s\0", device_id);
	serial_number.dataLength=strlen((char*)serial_number.dataValue);

	sprintf((char*)string2, "%02X%c%02X%s%c\0",
			serial_number.dataId, serial_number.dataType, serial_number.dataLength, serial_number.dataValue,
			serial_number.decimalPlace);

	total_length=strlen((char*)string2);///// length of serial number

			////////////////////////// Read stored clock value record, if it is clock record
	read_clock_from_flash();

	////// TIMESTAMP
	timestamp.dataId=0x1C;
	timestamp.dataType='2';
	timestamp.decimalPlace='0';

	///convert timestamp to hex
	t = atol((char*)read_clock);
	bytes[0] = (t >> 24) & 0xFF;
	bytes[1] = (t >> 16) & 0xFF;
	bytes[2] = (t >> 8) & 0xFF;
	bytes[3] = t & 0xFF;

	sprintf((char*)timestamp.dataValue, "%02X%02X%02X%02X\0", bytes[0], bytes[1], bytes[2], bytes[3]);
			timestamp.dataLength=strlen((char*)timestamp.dataValue);

	sprintf((char*)string3, "%02X%c%02X%s%c\0",
				timestamp.dataId, timestamp.dataType, timestamp.dataLength, timestamp.dataValue, timestamp.decimalPlace);

	total_length=total_length+strlen((char*)string3);///// length of timestamp


	////// model_number
	model_number.dataId=0x02;
	model_number.dataType='2';
	model_number.decimalPlace='0';

//	if(atol((char*)read_meter_model)==923 || atol((char*)read_meter_model)==897)//// if accu check meter
	{
		sprintf((char*)model_number.dataValue, "262\0");
	}

//	else
	{
//		sprintf((char*)model_number.dataValue, "%s\0", read_meter_model);/// if other meter
	}

	model_number.dataLength=strlen((char*)model_number.dataValue);

	sprintf((char*)string4, "%02X%c%02X%s%c\0",
			model_number.dataId, model_number.dataType, model_number.dataLength, model_number.dataValue,
			model_number.decimalPlace);

	total_length=total_length+strlen((char*)string4);///// length of model_number


	////// meter_id or serial no
	meter_id.dataId=0x01;
	meter_id.dataType='1';//ascii
	meter_id.decimalPlace='0';
	sprintf((char*)meter_id.dataValue, "%s\0", meter_serial_no);
	meter_id.dataLength=strlen((char*)meter_id.dataValue);

	sprintf((char*)string5, "%02X%c%02X%s%c\0",
			meter_id.dataId, meter_id.dataType, meter_id.dataLength, meter_id.dataValue,
			meter_id.decimalPlace);

	total_length=total_length+strlen((char*)string5);///// length of meter_id


	////////////// raw_data
	raw_data.dataId=0x1B;
	raw_data.dataType='2';///ascii hex
	raw_data.decimalPlace='0';

	/////////////////////////////// For sending raw data in ascii hex   length 34 bytes 0x22
	data_byte=0;

	for(zz = 0; zz < 17; zz++)
	{
		data_byte = sFLASH_ReadByte1(mem_read_address);////// read data from flash
		sprintf((char*)(raw_data.dataValue+zz*2), "%02X", data_byte);
        mem_read_address++;
	}

	raw_data.dataLength=strlen((char*)raw_data.dataValue);

	sprintf((char*)string6, "%02X%c%02X%s%c\0",
			raw_data.dataId, raw_data.dataType, raw_data.dataLength, raw_data.dataValue,
			raw_data.decimalPlace);

	total_length=total_length+strlen((char*)string6);///// length of raw_data



	////////////////// signal strength
	signal_strength.dataId=0x15;
	signal_strength.dataType='2';
	signal_strength.decimalPlace='0';

	sprintf((char*)signal_strength.dataValue, "%02X\0", gsm_signal_strength);

	signal_strength.dataLength=strlen((char*)signal_strength.dataValue);

	sprintf((char*)string7, "%02X%c%02X%s%c\0",
			signal_strength.dataId, signal_strength.dataType, signal_strength.dataLength, signal_strength.dataValue,
			signal_strength.decimalPlace);

	total_length=total_length+strlen((char*)string7);///// length of signal_strength


	total_length=total_length+6+1;

	///// covert length into 16 bit hex
	packet_length[0] = total_length & 0xFF;
	packet_length[1] = total_length >> 8;

	//// convert length into ascii hex
	sprintf((char*)string1, "{'data':'S3%02X%02X\0", packet_length[1], packet_length[0]);////// length 6 bytes

	//// send string1+string2

	total_length = sprintf((char*)tx_string, "%s%s%s%s%s%s%sZ'}\r\n%c\0", string1, string2, string3, string4, string5, string6, string7, 0x1A);


	store_runtime_parameters_to_flash(RUNTIME_PARAMETERS_ADDRESS);
	store_runtime_parameters_to_flash(RUNTIME_PARAMETERS_BACKUP_ADDRESS);


	return total_length;
}







/***************************************************************************
 *                         store_devinfo_to_flash
 *                         ---------------------------------
 *
 * Set yellow led on/off as directed
 *
 * \param[in] sect_address
 *
 * \return -
 */


void store_devinfo_to_flash(uint32_t sect_address)
{
    uint32_t loc = sect_address;
    uint32_t i;
    uint32_t reclen;

    flush_array(stored_parameters, 100);

    reclen = sprintf((char*)stored_parameters,
                     "#SET,%s,%s,%s*",
                     ble_paired_device,
                     meter_model,
                     meter_serial_no);

    ///// ERASE SECTOR IN FLASH
    sFLASH_EraseSector(sect_address);

    for(i = 0; i < reclen; i++)
    {
        sFLASH_WriteByte(loc++, stored_parameters[i]);
    }

    debug_printf(DBGLVL_MAX, "WRITE DEVINFO %s\r\n", stored_parameters);
    return;
}









/***************************************************************************
 *                         store_records_to_flash
 *                         ---------------------------------
 *
 * Set yellow led on/off as directed
 *
 * \param[in] sect_address
 *
 * \return -
 */


void store_records_to_flash(uint16_t reccnt)
{
    uint16_t cc;
    uint16_t zz;

    debug_printf(DBGLVL_MAX, "NO OF RECORDS : %u\r\n", reccnt);

    if ((mem_write_address + (reccnt * 17)) > FLASH_LAST_ADDRESS)
    {
        erase_all_records();
    }

	store_clock_to_flash();

    for(cc = 0; cc < reccnt; cc++)
	{
	    total_downloaded_records_meter++;

		for(zz = 0; zz < 17; zz++)
		{
			sFLASH_WriteByte(mem_write_address, meter_records[cc][zz]);///////// write data to flash
			mem_write_address++;
		}

        debug_printf(DBGLVL_MAX, "RECORD: \r\n");
	}

	store_runtime_parameters_to_flash(RUNTIME_PARAMETERS_ADDRESS);
	store_runtime_parameters_to_flash(RUNTIME_PARAMETERS_BACKUP_ADDRESS);
    return;
}

/***************************************************************************
 *                             LOCAL FUNCTIONS
 **************************************************************************/






/***************************************************************************
 *                         explode_string
 *                         --------------
 *
 * Set yellow led on/off as directed
 *
 * \param[in] sect_address
 *
 * \return - none
 */


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
 *                         read_clock_from_flash
 *                         ---------------
 *
 * Set yellow led on/off as directed
 *
 * \param[in] - none
 *
 * \return - none
 */

static void read_clock_from_flash(void)
{
    uint8_t i;
	uint8_t server_clock_format[18];

	server_clock_format[0] = sFLASH_ReadByte1(mem_read_address);

	if(server_clock_format[0]=='#')//////////////////// read clock
	{
		for(i = 1; i < 17; i++)
		{
			server_clock_format[i] = sFLASH_ReadByte1(mem_read_address+i);
		}

		if(server_clock_format[1]=='c' && server_clock_format[2]=='l' && server_clock_format[3]=='o' && server_clock_format[4]=='c' && server_clock_format[5]=='k' && server_clock_format[16]=='*')
		{
			flush_array((char*)read_clock, 11);

			for(i = 0; i < 10; i++)
			{
				read_clock[i] = server_clock_format[i+6];
			}

			mem_read_address=mem_read_address+17;///// total 17 bytes stri

			server_clock_format[0] = sFLASH_ReadByte1(mem_read_address);

			if(server_clock_format[0]=='#')////////////// read meter serial no.
			{
				for(i = 1; i < 17; i++)
				{
					server_clock_format[i] = sFLASH_ReadByte1(mem_read_address+i);
				}

				if(server_clock_format[1]=='m' && server_clock_format[2]=='s' && server_clock_format[3]=='r' && server_clock_format[4]=='l' && server_clock_format[16]=='*')
				{
					flush_array((char*)read_meter_serial_no, 20);

					for(i = 0; i < 11; i++)
					{
						read_meter_serial_no[i] = server_clock_format[i+5];
					}

					read_meter_model[0] = read_meter_serial_no[0];
					read_meter_model[1] = read_meter_serial_no[1];
					read_meter_model[2] = read_meter_serial_no[2];

					mem_read_address=mem_read_address+17;///// total 17 bytes string
				}
			}
		}
	}

    return;
}


/***************************************************************************
 *                         read_devinfo_from_flash
 *                         ----------------------------------
 *
 * Set yellow led on/off as directed
 *
 * \param[in] sect_address
 *
 * \return -
 */

static void read_devinfo_from_flash(uint32_t sect_address)
{
    uint32_t loc;
    uint16_t i;

    loc = sect_address;
    flush_array(stored_parameters, 100);

    for(i = 0; i < 100; i++)
    {
        stored_parameters[i] = sFLASH_ReadByte1(loc++);

        if ((stored_parameters[i] == '*') || (stored_parameters[i] == 0xFF))
        {
            break;
        }

    }

    debug_printf(DBGLVL_MAX,"READ DEV INFO: %s\r\n", stored_parameters);



    if(stored_parameters[0]=='#' && stored_parameters[i]=='*')
    {


        explode_string(stored_parameters, 100, 1, ',', ',', 18, '*');

        i = 0;

        do
        {
            ble_paired_device[i] = temp_data[i];
            i++;
        } while (temp_data[i] != 0x00);



        explode_string(stored_parameters, 100, 2, ',', ',', METERMODEL_LEN, '*');


        i = 0;

        do
        {
            meter_model[i] = temp_data[i];
            i++;
        } while (temp_data[i] != 0x00);


        explode_string(stored_parameters, 100, 3, ',', ',', METERSERIAL_LEN, '*');

        i = 0;

        do
        {
            meter_serial_no[i] = temp_data[i];
            i++;
        } while (temp_data[i] != 0x00);

    }

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

//    sprintf((char*)tx_string, "\r\n+READ: %s\r\n", stored_parameters);
//    debug_printf(DBGLVL_MAX,tx_string);



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


/*
        sprintf((char*)tx_string,
                "+PARSED: LSN:%u\r\nWA:%u  Rem:%u\r\nRA:%u  Rem:%u\r\nTDRM:%u\r\n\r\n",
                (unsigned int)meter_sequence_number,
                (unsigned int)mem_write_address,
                (unsigned int)(mem_write_address-LOGS_START_ADDRESS) % 17,
                (unsigned int)mem_read_address,
                (unsigned int)(mem_read_address-LOGS_START_ADDRESS) % 17,
                (unsigned int)total_downloaded_records_meter);

        debug_printf(DBGLVL_MAX,tx_string);
*/
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

    debug_printf(DBGLVL_MAX, "\r\n+READ: %s\r\n", stored_settings);



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

        explode_string(stored_settings, 300, 13, ',', ',', 11, '*');//////  device_id

        if(temp_data[0]!=0)
        {
            flush_array(device_id, 12);
            sprintf((char*)device_id, "%s", temp_data);
        }

        //// TTL


        debug_printf(DBGLVL_MAX, (uint8_t *)"\r\n+PARSED: APN:%s\r\nCV:%s\r\nSERVER:%s\r\nPORT:%s\r\nUSER:%s\r\nPASWD:%s\r\n",
                apn,configuration_version,mqtt_server,mqtt_port,mqtt_username,mqtt_password);

        debug_printf(DBGLVL_MAX, (uint8_t *)"CLIENT:%s\r\nTWIN_SB:%s\r\nTWIN_PB:%s\r\nTWIN_RP:%s\r\nREAD_PB:%s\r\nPV:%s\r\nDEVICE_ID:%s\r\n",
                mqtt_client_id,twin_sb,twin_pb,twin_rp,read_pb,protocol_version,device_id);

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
 *                         store_clock_to_flash
 *                         ---------------------------------
 *
 * Set yellow led on/off as directed
 *
 * \param[in] sect_address
 *
 * \return -
 */

static void store_clock_to_flash(void)
{
	unsigned char server_clock_format[18]={0};
	uint8_t i;

	if(server_clock>1000000000)
	{
		sprintf((char*)server_clock_format, "#clock%u*\0", server_clock);
//		send_text_to_uart2(server_clock_format);// Debug TTL

		for(i = 0; i < 17; i++)
		{
			sFLASH_WriteByte(mem_write_address++, server_clock_format[i]);
		}

		flush_array((char*)server_clock_format, 18);
		sprintf((char*)server_clock_format, "#msrl%s*\0", meter_serial_no);
//		send_text_to_uart2(server_clock_format);// Debug TTL

		for(uint8_t i=0; i<17; i++)
		{
			sFLASH_WriteByte(mem_write_address++, server_clock_format[i]);
		}

	}
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
            "#SET,%u,%u,%u,%u,*",
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

    sprintf((char*)stored_settings, "#SET,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s*",
            apn,configuration_version,mqtt_server,mqtt_port,mqtt_username,mqtt_password,
            mqtt_client_id,twin_sb,twin_pb,twin_rp,read_pb,protocol_version,device_id, ble_paired_device);

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

    debug_printf(DBGLVL_MAX, "n+STORED: %s\r\n", stored_settings);
    return;
}


/*
 * End of module.
 */


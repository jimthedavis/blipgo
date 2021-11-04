/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#define USB_ENABLED 1///////////////    TO ENABLE OR DISABLE DEBUG USB CODE

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"
#include "winbond_mem.h"

#if USB_ENABLED
#include "usbd_cdc_if.h"
#endif
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

///////////////////// SELECT MODULE ///////////////
#define BG96 0
#define BG95 1
//////////////////////
//////////////////////  SETTINGS FOR INDIA/USA
#define INDIA 0
#if !INDIA
#define USA 1
#endif
/////////////////////
/////////////////////  ERASE FLASH ON STARTUP
#define ERASE_FLASH 0


#define DEVICE_ID_ADDRESS 0
#define DEVICE_SETTINGS_ADDRESS 4096
#define DEVICE_SETTINGS_BACKUP_ADDRESS 8192
#define RUNTIME_PARAMETERS_ADDRESS 12288
#define RUNTIME_PARAMETERS_BACKUP_ADDRESS 16384
#define LOGS_START_ADDRESS 20480
#define FLASH_LAST_ADDRESS 2097341

#define led_red_on   HAL_GPIO_WritePin(LED2_GPIO_Port,LED2_Pin,GPIO_PIN_SET);////////// RED LED
#define led_red_off  HAL_GPIO_WritePin(LED2_GPIO_Port,LED2_Pin,GPIO_PIN_RESET);

#define led_green_on   HAL_GPIO_WritePin(LED1_GPIO_Port,LED1_Pin,GPIO_PIN_SET);///////// green LED
#define led_green_off  HAL_GPIO_WritePin(LED1_GPIO_Port,LED1_Pin,GPIO_PIN_RESET);

#define led_yellow_on   HAL_GPIO_WritePin(LED3_GPIO_Port,LED3_Pin,GPIO_PIN_SET);///////// Yellow LED
#define led_yellow_off  HAL_GPIO_WritePin(LED3_GPIO_Port,LED3_Pin,GPIO_PIN_RESET);

//// TO RESET SEND HIGH PULSE OF 150ms to 460ms,   KEEP LOW FOR NORMAL FUNCTION
#define GSM_RESET_HIGH  HAL_GPIO_WritePin(GSM_RESET_GPIO_Port,GSM_RESET_Pin,GPIO_PIN_SET);
#define GSM_RESET_LOW   HAL_GPIO_WritePin(GSM_RESET_GPIO_Port,GSM_RESET_Pin,GPIO_PIN_RESET);


#define GSM_VCC_ON   HAL_GPIO_WritePin(GSM_VCC_GPIO_Port,GSM_VCC_Pin,GPIO_PIN_SET);
#define GSM_VCC_OFF  HAL_GPIO_WritePin(GSM_VCC_GPIO_Port,GSM_VCC_Pin,GPIO_PIN_RESET);

//// ESP Enable pin is active low from mcu, Low from mcu will give high to esp. To enable esp we have to give high to enable pin
#define ESP_ENABLED   HAL_GPIO_WritePin(ESP_PWR_GPIO_Port,ESP_PWR_Pin,GPIO_PIN_RESET);
#define ESP_DISABLED    HAL_GPIO_WritePin(ESP_PWR_GPIO_Port,ESP_PWR_Pin,GPIO_PIN_SET);

#define ESP_POWER_ON     HAL_GPIO_WritePin(ESP_PWR_GPIO_Port,ESP_PWR_Pin,GPIO_PIN_RESET);
#define ESP_POWER_OFF    HAL_GPIO_WritePin(ESP_PWR_GPIO_Port,ESP_PWR_Pin,GPIO_PIN_SET);


#define AP_MODE_TIMEOUT_DURATION 300


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
uint8_t flash_data = 0, data_buf[10]={0}, gsm_signal_strength=0;
uint8_t rx_esp=0, at_timer=0, gsm, loop_counter=0, sequence_char[2] = {0}, mqtt_server_connection_fail_counter=0;

uint16_t timer_ms=0, gsm_count=0, timer_cmd=0, timer_cmd_esp=0, rx_esp_counter=0, check_paired_device_timer=0, total_records_meter=0,
meter_sequence_number = 0, total_downloaded_records_meter=0, json_count=0, curly_brace_count=0, softap_mode_timer=0;

uint32_t file_size=0;

#if INDIA
unsigned char apn[40]="airtelgprs.com\0";
#elif USA
unsigned char apn[40]="data641003\0";
#endif


unsigned char rx_string_esp[300]={0}, tx_string[400]={0}, gsm_data[300]={0}, fota_fail_cause[40]={0}, char_total_record[2]={0},
wifi_ssid[60]={0}, wifi_pswd[60]={0}, ble_pairing_key[10]={0}, ble_paired_device[18]={0},
ble_available_device[18]={0}, device_id[13]="H1922000001\0", json_response[600]={0}, http_token[17]={0}, temp_data[255]={0},
hold_cv_value[10]={0}, configuration_version[10]="3\0", mqtt_server[100]={0}, mqtt_port[7]={0}, mqtt_username[100]={0}, mqtt_password[50]={0},
mqtt_client_id[100]={0}, twin_sb[100]={0}, twin_pb[100]={0}, twin_rp[100]={0}, read_pb[100]={0}, protocol_version[5]="1.0\0",
firmware_version[10]="1.1\0", meter_model[10]={0}, meter_serial_no[20]={0}, imei[16]={0}, imsi[16]={0}, ccid[21]={0},
signal_strength[4]={0}, module_firmware_version[40]={0}, blipgo_model[10]="600\0", ble_ssid_name[100]={0};;

unsigned char stored_settings[300]={0}, stored_parameters[100]={0};
unsigned char read_clock[11]={0}, read_meter_model[10]={0}, read_meter_serial_no[20]={0};

unsigned char meter_records[100][17]={0};
uint16_t row = 0, green_led_timer=1;

uint32_t loc = 0, server_clock=0, mem_write_address=LOGS_START_ADDRESS, mem_read_address=LOGS_START_ADDRESS;

_Bool registered_to_gprs_network=0,plus_enable=1,esp_plus_enable=1,watchdog_refill_flag=1, connect_wifi_network_flag = 0, send_softap_response_flag=0,
ble_scan_meter_flag = 0, ble_start_pairing_flag = 0, ble_device_connected_flag = 0, ble_get_data_flag = 0, ble_connect_paired_meter_flag=0, json_response_flag=0,
configuration_service_flag=0, ca_certificate_flag=0, client_certificate_flag=0, client_key_flag=0, update_device_twin_flag=0,
connected_to_mqtt_server_flag=0, server_clock_flag=0, ping_service_flag=0, softap_mode_timeup_flag=0, bg96_power_flag=0, new_apn_flag=0,
server_clock_update=0,sending_data_to_server_flag=0,led3_on_flag=0,new_device_id_flag=0,update_module_firmware_flag=0,
update_device_firmware_flag=0, green_led_blink_flag=0, green_led_on_flag=0, read_clock_flag=0, bootup_complete_flag=0,
enable_green_led_handler=1, enable_red_led_handler=1, enable_yellow_led_handler=1, desired_reported_cv_matched_flag=0;

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */


//#define APPLICATION_ADDRESS     (uint32_t)0x08003000

//#if   (defined ( __CC_ARM ))
//__IO uint32_t VectorTable[48] __attribute__((at(0x20000000)));
//#elif (defined (__ICCARM__))
//#pragma location = 0x20000000
//__no_init __IO uint32_t VectorTable[48];
//#elif defined   (  __GNUC__  )
//__IO uint32_t VectorTable[48] __attribute__((section(".RAMVectorTable")));
//#endif


///////////////////////// STRUCTURES

struct DataField
{
	unsigned char dataId, dataType, dataLength, dataValue[50], decimalPlace;
};


/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;
TIM_HandleTypeDef htim16;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_RTC_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM6_Init(void);
static void MX_TIM7_Init(void);
static void MX_TIM16_Init(void);
/* USER CODE BEGIN PFP */

void update_cv_value(void);
void esp_disable_softap_mode_handler(void);
void wifi_command_handler(void);
_Bool read_device_twin(void);
void send_text_to_uart1(uint8_t *str);
void wait_ms(uint16_t t);
_Bool at_cmd_send_esp(char *at_command, char *success_response,char *error_response, uint16_t delay, _Bool high_prioity_task);
_Bool at_cmd_send(char *at_command, char *success_response,char *error_response, uint16_t delay, _Bool receive_sms_flag);
_Bool ble_initialize_gatt_client(char* str);
void ble_get_paired_device(void);
void ble_scan_available_device(void);
_Bool ble_is_available_device_paired(void);
void beep(uint8_t duration);
_Bool send_softap_response(char* str, uint16_t len);
void send_text_to_usb(char *str);
_Bool ping_service(void);
_Bool json_get_value(char* parameter, uint8_t parameter_length, uint8_t occurance, char* variable, uint8_t variable_size);
_Bool configuration_service(void);
_Bool ping_service(void);
_Bool get_root_ca_certificate(void);
_Bool get_client_certificate(void);
_Bool get_client_key(void);
_Bool get_server_clock(void);
_Bool connect_mqtt_server(void);
_Bool update_device_twin(void);
_Bool read_settings_from_flash(uint32_t sect_address);
void explode_string(unsigned char* buffer, uint16_t buffer_len, uint8_t sequence_number, uint8_t separator, uint8_t end_char_field, uint16_t field_len, uint8_t end_char_string);
void store_settings_to_flash(uint32_t sect_address);
void server_clock_handler(void);
void ping_service_handler(void);
void configuration_service_handler(void);
void ca_certificate_handler(void);
void client_certificate_handler(void);
void client_key_handler(void);
void device_twin_handler(void);
void erase_all_records(void);
_Bool read_runtime_parameters_from_flash(uint32_t sect_address);
void store_runtime_parameters_to_flash(uint32_t sect_address);
_Bool publish_meter_readings(void);
void publish_meter_readings_handler(void);
void power_on_bg96(void);
void power_down_bg96(void);
void disconnect_mqtt_connection(void);
void power_down_bg96_handler(void);
void read_clock_from_flash(void);
void store_clock_to_flash(void);
void yellow_led_handler(void);
void red_led_handler(void);
void green_led_handler(void);
_Bool get_gsm_signal_strength(void);
_Bool get_module_firmware_version(void);
_Bool update_module_firmware(void);
uint16_t getIndexOf(char *searchStr, uint16_t searchStrLength, char *mainStr, uint16_t mainStrLength);
time_t convert_to_epoch(unsigned char* __date, unsigned char* __time);
_Bool get_ntp_clock(void);
_Bool activate_pdp(void);
_Bool deactivate_pdp(void);


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART1)
	{
		HAL_UART_Receive_IT(&huart1, &rx_esp, 1);

//		if(rx_esp == '+')
//			rx_esp_counter = 0;

		rx_string_esp[rx_esp_counter] = rx_esp;

		///+NOTIFY:0,2,1,17,[0B]L[00]�[07][04][12][11][0B]8F;u��[00][00]
		if(ble_get_data_flag && rx_string_esp[1]=='N' && rx_string_esp[2]=='O' && rx_string_esp[3]=='T' && rx_string_esp[4]=='I' && rx_string_esp[5]=='F' && rx_string_esp[6]=='Y' && rx_string_esp[7]==':' && rx_string_esp[10]=='2' && rx_string_esp[12]=='1' && rx_string_esp[14]=='1' && rx_string_esp[15]=='7' && rx_string_esp[16]==',' && rx_esp_counter > 16)
		{
			if(rx_esp_counter==17)
				esp_plus_enable = 0;

			if(rx_esp_counter > 33)
			{
				for(uint8_t cc = 0; cc < 17; cc++)
				{
					meter_records[row][cc] = rx_string_esp[cc+17];
				}

				row++;

				sequence_char[0] = rx_string_esp[18];
				sequence_char[1] = rx_string_esp[19];
				rx_string_esp[16] = 0;

				esp_plus_enable = 1;
			}
		}
		////+INDICATE:0,2,4,4,[06][00][01][01] //// model 923
		else if(ble_get_data_flag && meter_model[0]=='9' && meter_model[1]=='2' && meter_model[2]=='3' && rx_string_esp[1]=='I' && rx_string_esp[2]=='N' && rx_string_esp[3]=='D' && rx_string_esp[4]=='I' && rx_string_esp[5]=='C' && rx_string_esp[6]=='A' && rx_string_esp[7]=='T' && rx_string_esp[8]=='E' && rx_string_esp[12]=='2' && rx_string_esp[14]=='4' && rx_string_esp[16]=='4' && rx_string_esp[18]==0x06 && rx_string_esp[19]==0x00 && rx_string_esp[rx_esp_counter]==0x0D)
		{
			rx_string_esp[rx_esp_counter] = 0;
		}
		////+INDICATE:0,2,4,4,[05][00]M[00]//// model 923
		else if(ble_get_data_flag && meter_model[0]=='9' && meter_model[1]=='2' && meter_model[2]=='3' && rx_string_esp[1]=='I' && rx_string_esp[2]=='N' && rx_string_esp[3]=='D' && rx_string_esp[4]=='I' && rx_string_esp[5]=='C' && rx_string_esp[6]=='A' && rx_string_esp[7]=='T' && rx_string_esp[8]=='E' && rx_string_esp[12]=='2' && rx_string_esp[14]=='4' && rx_string_esp[16]=='4' && rx_string_esp[18]==0x05 && rx_string_esp[19]==0x00 && rx_string_esp[rx_esp_counter]==0x0D)
		{
			char_total_record[0] = rx_string_esp[20];
			char_total_record[1] = rx_string_esp[21];
			rx_string_esp[rx_esp_counter] = 0;
		}
		////+INDICATE:0,2,3,4,[06][00][01][01] //// model 897
		else if(ble_get_data_flag && meter_model[0]=='8' && meter_model[1]=='9' && meter_model[2]=='7' && rx_string_esp[1]=='I' && rx_string_esp[2]=='N' && rx_string_esp[3]=='D' && rx_string_esp[4]=='I' && rx_string_esp[5]=='C' && rx_string_esp[6]=='A' && rx_string_esp[7]=='T' && rx_string_esp[8]=='E' && rx_string_esp[12]=='2' && rx_string_esp[14]=='3' && rx_string_esp[16]=='4' && rx_string_esp[18]==0x06 && rx_string_esp[19]==0x00 && rx_string_esp[rx_esp_counter]==0x0D)
		{
			rx_string_esp[rx_esp_counter] = 0;
		}
		////+INDICATE:0,2,3,4,[05][00][0E][00]//// model 897
		else if(ble_get_data_flag && meter_model[0]=='8' && meter_model[1]=='9' && meter_model[2]=='7' && rx_string_esp[1]=='I' && rx_string_esp[2]=='N' && rx_string_esp[3]=='D' && rx_string_esp[4]=='I' && rx_string_esp[5]=='C' && rx_string_esp[6]=='A' && rx_string_esp[7]=='T' && rx_string_esp[8]=='E' && rx_string_esp[12]=='2' && rx_string_esp[14]=='3' && rx_string_esp[16]=='4' && rx_string_esp[18]==0x05 && rx_string_esp[19]==0x00 && rx_string_esp[rx_esp_counter]==0x0D)
		{
			char_total_record[0] = rx_string_esp[20];
			char_total_record[1] = rx_string_esp[21];
			rx_string_esp[rx_esp_counter] = 0;
		}
		//+#WIFI#ssid,pswd;
		else if(rx_string_esp[1]=='#' && rx_string_esp[2]=='W' && rx_string_esp[3]=='I' && rx_string_esp[4]=='F' && rx_string_esp[5]=='I' && rx_string_esp[6]=='#' && rx_string_esp[rx_esp_counter]==';')
		{
			for(uint8_t i = 0; i < 60; i++)
			{
				wifi_ssid[i] = 0;
				wifi_pswd[i] = 0;
			}

			loop_counter = 7;

			for(uint8_t i = 0; rx_string_esp[loop_counter]!=',' && rx_string_esp[loop_counter]!=';'; i++)
			{
				wifi_ssid[i] = rx_string_esp[loop_counter++];
			}

			if(rx_string_esp[loop_counter]!=';')
			{
				loop_counter++;

				for(uint8_t i = 0; rx_string_esp[loop_counter]!=';'; i++)
				{
					wifi_pswd[i] = rx_string_esp[loop_counter++];
				}
			}

			connect_wifi_network_flag = 1;
			send_softap_response_flag = 1;
		}

		//+#PAIR#897364;
		else if(rx_string_esp[1]=='#' && rx_string_esp[2]=='P' && rx_string_esp[3]=='A' && rx_string_esp[4]=='I' && rx_string_esp[5]=='R' && rx_string_esp[6]=='#' && rx_string_esp[rx_esp_counter]==';')
		{
			for(uint8_t i = 0; i < 10; i++)
			{
				ble_pairing_key[i] = 0;
			}

			loop_counter = 7;

			for(uint8_t i = 0; i < 10 && rx_string_esp[loop_counter]!=';'; i++)
			{
				ble_pairing_key[i] = rx_string_esp[loop_counter++];
			}

			ble_start_pairing_flag = 1;

		}
		//+#APN#airtelgprs.com;
		else if(rx_string_esp[1]=='#' && rx_string_esp[2]=='A' && rx_string_esp[3]=='P' && rx_string_esp[4]=='N' && rx_string_esp[5]=='#' && rx_string_esp[rx_esp_counter]==';')
		{
			for(uint8_t i = 0; i < 40; i++)
			{
				apn[i] = 0;
			}

			loop_counter = 6;

			for(uint8_t i = 0; i < 40 && rx_string_esp[loop_counter]!=';'; i++)
			{
				apn[i] = rx_string_esp[loop_counter++];
			}

			new_apn_flag = 1;

		}
		//+#SN#HH0202000001;
		else if(rx_string_esp[1]=='#' && rx_string_esp[2]=='S' && rx_string_esp[3]=='N' && rx_string_esp[4]=='#' && rx_string_esp[rx_esp_counter]==';')
		{
			for(uint8_t i = 0; i < 13; i++)
			{
				device_id[i] = 0;
			}

			loop_counter = 5;

			for(uint8_t i = 0; i < 12 && rx_string_esp[loop_counter]!=';'; i++)
			{
				device_id[i] = rx_string_esp[loop_counter++];
			}

			new_device_id_flag = 1;

		}
		//// +BLEENCDEV:0,9c:1d:58:9c:06:1e
		else if(rx_string_esp[1]=='B' && rx_string_esp[2]=='L' && rx_string_esp[3]=='E' && rx_string_esp[4]=='E' && rx_string_esp[5]=='N' && rx_string_esp[6]=='C' && rx_string_esp[7]=='D' && rx_string_esp[8]=='E' && rx_string_esp[9]=='V' && rx_string_esp[10]==':' && rx_string_esp[11]=='0' && rx_string_esp[12]==',' && rx_string_esp[rx_esp_counter]==0x0D)
		{
//			for(uint8_t i = 0; i < 18; i++)
//			{
//				ble_paired_device[i] = 0;
//			}

			loop_counter = 13;

			for(uint8_t i = 0; rx_string_esp[loop_counter]!=0x0D; i++)
			{
				ble_paired_device[i] = rx_string_esp[loop_counter++];
			}

		}
		//+BLESCAN:9c:1d:58:9c:06:1e,-55,020104030208180f096d657465722b3032333535303335,,0
		//+BLESCAN:9c:1d:58:9c:06:1e,-55,020104030208180f09 6d65746572 2b3032333535303335,,0
		//0123456789012345678901234567890123456789012345678 9012345678 9012345678901234567890

		else if(ble_scan_meter_flag && rx_esp_counter > 70 && rx_string_esp[1]=='B' && rx_string_esp[2]=='L' && rx_string_esp[3]=='E' && rx_string_esp[4]=='S' && rx_string_esp[5]=='C' && rx_string_esp[6]=='A' && rx_string_esp[7]=='N' && rx_string_esp[8]==':' && rx_string_esp[rx_esp_counter]==',' && rx_string_esp[49]=='6' && rx_string_esp[50]=='d' && rx_string_esp[51]=='6' && rx_string_esp[52]=='5' && rx_string_esp[53]=='7' && rx_string_esp[54]=='4' && rx_string_esp[55]=='6' && rx_string_esp[56]=='5' && rx_string_esp[57]=='7' && rx_string_esp[58]=='2')
		{
//			for(uint8_t i = 0; i < 18; i++)
//			{
//				ble_available_device[i] = 0;
//			}

			loop_counter = 9;

			for(uint8_t i = 0; rx_string_esp[loop_counter]!=','; i++)
			{
				ble_available_device[i] = rx_string_esp[loop_counter++];
			}

			ble_scan_meter_flag = 0;

		}

		///+BLEDISCONN:0,"9c:1d:58:9c:06:1e"
		else if(rx_string_esp[1]=='B' && rx_string_esp[2]=='L' && rx_string_esp[3]=='E' && rx_string_esp[4]=='D' && rx_string_esp[5]=='I' && rx_string_esp[6]=='S' && rx_string_esp[7]=='C' && rx_string_esp[8]=='O' && rx_string_esp[9]=='N' && rx_string_esp[10]=='N' && rx_string_esp[11]==':' && rx_string_esp[12]=='0' && rx_string_esp[rx_esp_counter]==0x0D)
		{
			ble_device_connected_flag = 0;

		}

		if(rx_string_esp[rx_esp_counter]=='+' && esp_plus_enable)
			{
				rx_esp_counter=0;
				rx_string_esp[rx_esp_counter]='+';
			}

	 if(rx_esp_counter>=290)
		 rx_esp_counter=0;

		rx_esp_counter++;


	}

	if (huart->Instance == USART2)
	{
		HAL_UART_Receive_IT(&huart2, &gsm, 1);

		gsm_data[gsm_count]=gsm;


		if(json_response_flag && gsm_data[gsm_count] == '{')///////////// JSON DATA START CONDITION
		{
			curly_brace_count++;

			if(curly_brace_count==1)///// { START CONDITION
			{
				gsm_data[gsm_count] = 0;

				gsm_count = 0;
				json_count = 0;
				plus_enable = 0;

				gsm_data[0] = '{';
			}

			json_response[json_count++] = gsm_data[gsm_count];
		}
		else if(json_response_flag && gsm_data[0] == '{')/////////// JSON DATA CAPTURE AND END CONDITION
		{
			if(json_count < 599)
			{
				json_response[json_count] = gsm_data[gsm_count];

				if(gsm_data[gsm_count] == '}')////// } END CONDITION
				{
					curly_brace_count--;

					if(curly_brace_count==0)
					{
						json_response_flag = 0;
						plus_enable = 1;
					}
					else
						json_count++;
				}
				else
					json_count++;
			}
			else
				json_response_flag = 0;
		}




		if(plus_enable)
   {
      if(gsm_data[gsm_count]=='+')
			{
				gsm_count=0;
				gsm_data[gsm_count]='+';
			}
   }

	 if(gsm_count>=290)
		 gsm_count=0;

   gsm_count++;

	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance==TIM6) // 1ms
	{
		if(watchdog_refill_flag)
		{
//			HAL_WWDG_Refresh(&hwwdg);
		}

		timer_ms++;

		if(timer_ms>9000)
			timer_ms=9000;

		if(green_led_timer>0)
			green_led_timer--;

		if(enable_green_led_handler)
		{
			green_led_handler();
		}

//		ms_counter++;
//		if(ms_counter>9000)
//			ms_counter=9000;

	}

	else if(htim->Instance==TIM7)  //1 sec
	{
		if(watchdog_refill_flag)
		{
//			HAL_IWDG_Refresh(&hiwdg);
		}

		if(!server_clock_update)
			server_clock++;

		if(enable_yellow_led_handler)
		{
			yellow_led_handler();
		}

		if(enable_red_led_handler)
		{
			red_led_handler();
		}



		if(timer_cmd < 400)
			timer_cmd++;

		if(timer_cmd_esp < 400)
			timer_cmd_esp++;

		if(softap_mode_timer<320)
			softap_mode_timer++;

		if(softap_mode_timer==AP_MODE_TIMEOUT_DURATION)
			softap_mode_timeup_flag=1;

		if(check_paired_device_timer > 2)
		{
			check_paired_device_timer = 0;
			ble_connect_paired_meter_flag = 1;
		}
		else
			check_paired_device_timer++;

	}
}

/*
time_t epoch_time=0;
epoch_time = convert_to_epoch(date, time1);
*/

time_t convert_to_epoch(unsigned char* __date, unsigned char* __time)
{
	struct tm t;

	time_t t_of_day;

	unsigned char yr[5]={0}, mon[3]={0}, day[3]={0}, hr[3]={0}, min[3]={0}, sec[3]={0};

	 yr[0] = '2';
	 yr[1] = '0';
	 yr[2] = __date[4];
	 yr[3] = __date[5];

	 mon[0] = __date[2];
	 mon[1] = __date[3];

	 day[0] = __date[0];
	 day[1] = __date[1];

	 hr[0] = __time[0];
	 hr[1] = __time[1];

	 min[0] = __time[2];
	 min[1] = __time[3];

	 sec[0] = __time[4];
	 sec[1] = __time[5];

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

void restart_esp(void)
{
	//ESP EN
	ESP_DISABLED;
	HAL_Delay(1000);
	ESP_ENABLED;
	HAL_Delay(1000);

	at_cmd_send_esp("ATE0\r\n\0","OK\r\n","ERROR",5,0);
}

void send_text_to_uart1(uint8_t *str)
{
	uint16_t i=0;
	for(i=0; i<400 && str[i]!=0; i++);

	HAL_UART_Transmit_IT(&huart1, str, i);

	HAL_Delay(i*3);

}

void send_text_to_uart2(uint8_t *str)
{
	uint16_t i=0;
	for(i=0; i<400 && str[i]!=0; i++);

	HAL_UART_Transmit_IT(&huart2, str, i);

	HAL_Delay(i*3);

}

void send_text_to_usb(char *str)
{
	#if USB_ENABLED
	uint16_t i=0;
	for(i=0; i<400 && str[i]!=0; i++);

	CDC_Transmit_FS((uint8_t*) str, i);

	HAL_Delay(i*3);
	#endif

}

void flush_array(char *str, uint16_t len)
{
	for(uint16_t i = 0; i < len; i++)
	{
		str[i] = 0;
	}
}

void flush_tx_string(void)
{
	flush_array((char*)tx_string, 400);
}

void high_priority_tasks(void)
{

}

void wait_ms(uint16_t t)
{
	timer_ms = 0;

	while(timer_ms < t)
	{
		HAL_Delay(1);
		high_priority_tasks(); // HIGH PRIORITY TASKS
	}
}

//this function clears previously stored data of gsm commands from variable array
void flush_gsm_array(void)
{
	uint16_t i=0;
  for(i=0; i<295; i++)
	gsm_data[i] = 0;
  gsm_count = 0;
}

_Bool ble_initialize_gatt_client(char* str)
{
	if(at_cmd_send_esp("AT+BLEINIT=0\r\n\0","OK\r\n","ERROR",5,0))
		if(at_cmd_send_esp("AT+BLEINIT=1\r\n\0","OK\r\n","ERROR",5,0))
		{
			flush_array((char*)tx_string,400);
			sprintf((char*)tx_string,"AT+BLENAME=\"%s\"\r\n\0", str);
			if(at_cmd_send_esp((char*)tx_string,"OK\r\n","ERROR",5,0))
				return 1;
		}
	return 0;
}

_Bool activate_pdp(void)
{
	if(at_cmd_send("AT+QIACT=1\r\n\0","OK\r\n","ERROR",160,0))
	{
		return 1;
	}

	return 0;
}

_Bool deactivate_pdp(void)
{
	if(at_cmd_send("AT+QIDEACT=1\r\n\0","OK\r\n","ERROR",50,0))
	{
		return 1;
	}

	return 0;
}


void esp_power_on(void)
{
	if(!at_cmd_send_esp("AT\r\n\0","OK\r\n","ERROR",5,0))
	{

//	send_text_to_uart1("\r\nESP Power Pin High\r\n");
		ESP_POWER_OFF;

		HAL_Delay(3000);

//		send_text_to_uart1("\r\nESP EN Pin High\r\n");
		//ESP PWR ON
		ESP_DISABLED;
		HAL_Delay(1000);

//		send_text_to_uart1("\r\nESP Power Pin Low\r\n");
		ESP_POWER_ON;

		HAL_Delay(1000);

		timer_cmd_esp=0;
		uint16_t ret=0;
		flush_array((char*)rx_string_esp, 300);
		rx_esp_counter = 0;

		char *searchStr = {0};
		searchStr = "ready";

//		send_text_to_uart1("\r\nESP EN Pin Low\r\n");
		ESP_ENABLED;
//		HAL_Delay(1200);

		while(timer_cmd_esp<5)
		{
			ret = getIndexOf(searchStr, strlen(searchStr), (char*)rx_string_esp, 299);

			if(ret > 0)
			{
				break;
			}
		}

//		send_text_to_uart1("\r\nESP EN Pin High\r\n");
		//ESP EN
//		HAL_GPIO_WritePin(ESP_EN_GPIO_Port,ESP_EN_Pin,GPIO_PIN_SET);
//		HAL_Delay(3000);
	}


	at_cmd_send_esp("AT\r\n\0","OK\r\n","ERROR",5,0);
	at_cmd_send_esp("ATE0\r\n\0","OK\r\n","ERROR",5,0);
}


void store_runtime_parameters_to_flash(uint32_t sect_address)
{
	uint32_t loc = sect_address;

	flush_array((char*)stored_parameters, 100);

	sprintf((char*)stored_parameters, "#SET,%u,%u,%u,%u*\0",
	meter_sequence_number, mem_write_address, mem_read_address, total_downloaded_records_meter);

	///// ERASE SECTOR IN FLASH
	sFLASH_EraseSector(sect_address);

	for(uint16_t i=0; i<strlen((char*)stored_parameters); i++)
	{
		if(stored_settings[i]==0)
			break;

		sFLASH_WriteByte(loc++, stored_parameters[i]);
	}

	//// TTL
	flush_tx_string();
	sprintf((char*)tx_string, "\r\n+STORED: %s\r\n\0", stored_parameters);
//	send_text_to_uart2(tx_string);
	send_text_to_usb((char*)tx_string);

}


_Bool read_runtime_parameters_from_flash(uint32_t sect_address)
{
	uint32_t loc = sect_address;
	uint16_t i=0;

	flush_array((char*)stored_parameters, 100);

	for(i=0; i<100; i++)
	{
		stored_parameters[i] = sFLASH_ReadByte1(loc++);

		if(stored_parameters[i]=='*' || stored_parameters[i]==0xff)
			break;
	}

	//// TTL
	flush_tx_string();
	sprintf((char*)tx_string, "\r\n+READ: %s\r\n\0", stored_parameters);
	send_text_to_uart2(tx_string);
	send_text_to_usb((char*)tx_string);



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

		sprintf((char*)tx_string, "+PARSED: LSN:%u\nWA:%u  Rem:%u\nRA:%u  Rem:%u\nTDRM:%u\r\n\r\n\0",
			meter_sequence_number, mem_write_address,(mem_write_address-LOGS_START_ADDRESS)%17, mem_read_address, (mem_read_address-LOGS_START_ADDRESS)%17, total_downloaded_records_meter);

		send_text_to_uart2(tx_string);
		send_text_to_usb((char*)tx_string);

		return 1;
	}

	return 0;
}


void store_settings_to_flash(uint32_t sect_address)
{
	uint32_t loc = sect_address;

	flush_array((char*)stored_settings, 300);

	sprintf((char*)stored_settings, "#SET,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s*\0",
	apn,configuration_version,mqtt_server,mqtt_port,mqtt_username,mqtt_password,
	mqtt_client_id,twin_sb,twin_pb,twin_rp,read_pb,protocol_version,device_id);

	///// ERASE SECTOR IN FLASH
	sFLASH_EraseSector(sect_address);

	for(uint16_t i=0; i<strlen((char*)stored_settings); i++)
	{
		if(stored_settings[i]==0)
			break;

		sFLASH_WriteByte(loc++, stored_settings[i]);
	}

	//// TTL
	flush_tx_string();
	sprintf((char*)tx_string, "\r\n+STORED: %s\r\n\0", stored_settings);
	send_text_to_uart2(tx_string);
	send_text_to_usb((char*)tx_string);

}

//*************************************************************
// FUNCTION CAPTURE DATA FIELDS FROM A COMMA SEPERATED STRING
// STRING END CHAR *    STRING SEPERATOR ,
//*************************************************************
void explode_string(unsigned char* buffer, uint16_t buffer_len, uint8_t sequence_number, uint8_t separator, uint8_t end_char_field, uint16_t field_len, uint8_t end_char_string)
{
	uint8_t comma_count=0;
	uint16_t i=0, j=0;

	flush_array((char*)temp_data,255);

	for(i=0; i< buffer_len; i++)
	{
		if(buffer[i]==separator)
			comma_count++;

		if(buffer[i]==end_char_string)
			break;

		if(comma_count==sequence_number)//// if sequence found. Note: sequence number of data fields starts from 0
		{
			for(j=0; j<field_len; j++)//// Copy data field, field_len length of field variable
			{
				if(buffer[i+1+j]==end_char_field || buffer[i+1+j]==end_char_string)/// stop copy when , or * reached
					break;

				temp_data[j] = buffer[i+1+j];/////i+1+j  because already , at index i
			}

			break;
		}
	}
}


_Bool read_settings_from_flash(uint32_t sect_address)
{
	uint32_t loc = sect_address;
	uint16_t i=0;

	flush_array((char*)stored_settings, 300);

	for(i=0; i<300; i++)
	{
		stored_settings[i] = sFLASH_ReadByte1(loc++);

		if(stored_settings[i]=='*' || stored_settings[i]==0xff)
			break;
	}

	//// TTL
	flush_tx_string();
	sprintf((char*)tx_string, "\r\n+READ: %s\r\n\0", stored_settings);
	send_text_to_uart2(tx_string);
	send_text_to_usb((char*)tx_string);



	if(stored_settings[0]=='#' && stored_settings[i]=='*')
	{
		explode_string(stored_settings, 300, 1, ',', ',', 40, '*');//////  APN

		if(temp_data[0]!=0)
		{
			flush_array((char*)apn, 40);
			sprintf((char*)apn, "%s", temp_data);
		}


		explode_string(stored_settings, 300, 2, ',', ',', 10, '*');//////  Configuration Version

		if(temp_data[0]!=0)
		{
			flush_array((char*)configuration_version, 10);
			sprintf((char*)configuration_version, "%s", temp_data);
		}


		explode_string(stored_settings, 300, 3, ',', ',', 100, '*');//////  mqtt_server

		if(temp_data[0]!=0)
		{
			flush_array((char*)mqtt_server, 100);
			sprintf((char*)mqtt_server, "%s", temp_data);
		}


		explode_string(stored_settings, 300, 4, ',', ',', 7, '*');//////  mqtt_port

		if(temp_data[0]!=0)
		{
			flush_array((char*)mqtt_port, 7);
			sprintf((char*)mqtt_port, "%s", temp_data);
		}


		explode_string(stored_settings, 300, 5, ',', ',', 100, '*');//////  mqtt_username

		if(temp_data[0]!=0)
		{
			flush_array((char*)mqtt_username, 100);
			sprintf((char*)mqtt_username, "%s", temp_data);
		}

		explode_string(stored_settings, 300, 6, ',', ',', 50, '*');//////  mqtt_password

		if(temp_data[0]!=0)
		{
			flush_array((char*)mqtt_password, 50);
			sprintf((char*)mqtt_password, "%s", temp_data);
		}

		explode_string(stored_settings, 300, 7, ',', ',', 100, '*');//////  mqtt_client_id

		if(temp_data[0]!=0)
		{
			flush_array((char*)mqtt_client_id, 100);
			sprintf((char*)mqtt_client_id, "%s", temp_data);
		}

		explode_string(stored_settings, 300, 8, ',', ',', 100, '*');//////  twin_sb

		if(temp_data[0]!=0)
		{
			flush_array((char*)twin_sb, 100);
			sprintf((char*)twin_sb, "%s", temp_data);
		}

		explode_string(stored_settings, 300, 9, ',', ',', 100, '*');//////  twin_pb

		if(temp_data[0]!=0)
		{
			flush_array((char*)twin_pb, 100);
			sprintf((char*)twin_pb, "%s", temp_data);
		}

		explode_string(stored_settings, 300, 10, ',', ',', 100, '*');//////  twin_rp

		if(temp_data[0]!=0)
		{
			flush_array((char*)twin_rp, 100);
			sprintf((char*)twin_rp, "%s", temp_data);
		}

		explode_string(stored_settings, 300, 11, ',', ',', 100, '*');//////  read_pb

		if(temp_data[0]!=0)
		{
			flush_array((char*)read_pb, 100);
			sprintf((char*)read_pb, "%s", temp_data);
		}

		explode_string(stored_settings, 300, 12, ',', ',', 5, '*');//////  protocol_version

		if(temp_data[0]!=0)
		{
			flush_array((char*)protocol_version, 5);
			sprintf((char*)protocol_version, "%s", temp_data);
		}

		explode_string(stored_settings, 300, 13, ',', ',', 12, '*');//////  device_id

		if(temp_data[0]!=0)
		{
			flush_array((char*)device_id, 12);
			sprintf((char*)device_id, "%s", temp_data);
		}

		//// TTL
		flush_tx_string();

		sprintf((char*)tx_string, "\r\n+PARSED: APN:%s\nCV:%s\nSERVER:%s\nPORT:%s\nUSER:%s\nPASWD:%s\nCLIENT:%s\nTWIN_SB:%s\nTWIN_PB:%s\nTWIN_RP:%s\nREAD_PB:%s\nPV:%s\nDEVICE_ID:%s\r\n\0",
	apn,configuration_version,mqtt_server,mqtt_port,mqtt_username,mqtt_password,
	mqtt_client_id,twin_sb,twin_pb,twin_rp,read_pb,protocol_version,device_id);

		send_text_to_uart2(tx_string);
		send_text_to_usb((char*)tx_string);

		return 1;
	}

	return 0;
}


void ble_get_paired_device(void)
{
	flush_array((char*)ble_paired_device,18);
	at_cmd_send_esp("AT+BLEENCDEV?\r\n\0","OK\r\n","ERROR",5,0);
}

void ble_scan_available_device(void)
{
	flush_array((char*)ble_available_device,18);
	at_cmd_send_esp("AT+BLESCANPARAM=0,0,0,100,50\r\n\0", "OK\r\n", "ERROR", 5, 0);

	ble_scan_meter_flag = 1;
	at_cmd_send_esp("AT+BLESCAN=1,1\r\n\0","OK\r\n", "ERROR", 3, 0);
	ble_scan_meter_flag = 0;
}



///FUNCTION TO SEND AT COMMANDS AND LISTEN TO RESPONSE
_Bool at_cmd_send(char *at_command, char *success_response,char *error_response, uint16_t delay, _Bool receive_sms_flag)
{
//	wait_ms(300);
	HAL_UART_Receive_IT(&huart2, &gsm, 1);
		flush_gsm_array();
	send_text_to_uart2((uint8_t*)at_command);

	timer_cmd=0;

	while(timer_cmd < delay)
		{
			if(strstr((char*)gsm_data, (char*)success_response))
				{
					bg96_power_flag=1;
//					gsm_proper_run = 1;
					return 1;
				}

			if(strstr((char*)gsm_data, (char*)"ERROR") || strstr((char*)gsm_data, (char*)error_response))
				{
					bg96_power_flag=1;
//					gsm_proper_run = 1;
					return 0;
				}

			if(receive_sms_flag)
				high_priority_tasks(); /////High Priority Task

		}
	return 0;
}

///FUNCTION TO SEND AT COMMANDS AND LISTEN TO RESPONSE FROM ESP MODULE
_Bool at_cmd_send_esp(char *at_command, char *success_response,char *error_response, uint16_t delay, _Bool high_prioity_task)
{
//	wait_ms(300);
	HAL_UART_Receive_IT(&huart1, &rx_esp, 1);
	flush_array((char*)rx_string_esp, 300);
	rx_esp_counter = 0;
	send_text_to_uart1((uint8_t*)at_command);

	timer_cmd_esp=0;

	while(timer_cmd_esp < delay)
		{
			if(strstr((char*)rx_string_esp, (char*)success_response))
				{
//					gsm_proper_run = 1;
					return 1;
				}

			if(strstr((char*)rx_string_esp, (char*)"ERROR") || strstr((char*)rx_string_esp, (char*)error_response))
				{
//					gsm_proper_run = 1;
					return 0;
				}

			if(high_prioity_task)
				high_priority_tasks(); /////High Priority Task

		}
	return 0;
}

void check_gprs_network_registration(void)
{


//	flush_gsm_array();
//	wait_ms(300);

	at_cmd_send("AT+CGREG?\r\n\0","OK\r\n","ERROR",5,0);
	wait_ms(300);
	at_cmd_send("AT+CEREG?\r\n\0","OK\r\n","ERROR",5,0);
	wait_ms(300);
	at_cmd_send("AT+QCSQ\r\n\0","OK\r\n","ERROR",5,0);
	wait_ms(300);

	//+CGREG: 0,1
	//+CGREG: 0,5

	//if(gsm_data[10] == '1' || gsm_data[10] == '5')
	if(at_cmd_send("AT+COPS?\r\n\0","COPS: 0,0","ERROR",5,0))//+COPS: 0,0,"airtel airtel",0
	{
		if(!registered_to_gprs_network)
			send_text_to_usb("REG 1\n\r\0");

		registered_to_gprs_network=1;
		wait_ms(3000);
	}
	else
	{
		if(registered_to_gprs_network)
			send_text_to_usb("REG 0\n\r\0");

		registered_to_gprs_network = 0;
		wait_ms(4000);
	}

}

_Bool __ROM_Page_Erase_(uint32_t page_address, uint32_t number_of_pages)
{

	uint32_t page_error=0;

	FLASH_EraseInitTypeDef settings_flash;
	settings_flash.NbPages = number_of_pages;
	settings_flash.PageAddress = page_address;
	settings_flash.TypeErase = FLASH_TYPEERASE_PAGES;


	HAL_FLASH_Unlock();

	if(HAL_FLASHEx_Erase(&settings_flash, &page_error) == HAL_OK)
	{
		HAL_FLASH_Lock();
		return 1;
	}

	HAL_FLASH_Lock();
	return 0;
}

void __ROM_Write_32bit_(uint32_t Address, uint64_t Data)
{
	HAL_FLASH_Unlock();

	HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, Address, Data);

	HAL_FLASH_Lock();
}

void __Call_BootLoader_(uint64_t file_id, uint64_t file_size1)
{
	send_text_to_uart2((uint8_t*)"Call Bootloader\r\n\0");
	HAL_UART_DeInit(&huart2);

	if(__ROM_Page_Erase_((uint32_t)0x08002800,1))//6th page of settings
	{
		__ROM_Write_32bit_((uint32_t)0x08002800, file_id);
		__ROM_Write_32bit_((uint32_t)0x08002808, file_size1);

//		__ROM_Write_32bit_((uint32_t)0x08002008, (uint64_t)134072);

		NVIC_SystemReset();
		HAL_Delay(5000);
	}

//*((__IO uint32_t*)0x2000049Cu)=99;

}

_Bool ftp_download(char* ftp_server, char* ftp_user, char* ftp_pswd, uint16_t ftp_port, char* ftp_filename)
{
//	store_settings_to_flash_flag = 1;
//	wait_ms(10);
//
//	gsm_gprs_shut();
//
//	//RESTART MODULE TO CLEAR RAM
//	gsm_power_rst();
//
//	gsm_proper_run = 0;
//	gsm_run_check();
//
//	gsm_proper_run = 0;
//	gsm_run_check();

	power_on_bg96();

	uint8_t cc=0;

	do
	{
		check_gprs_network_registration();
		cc++;
	} while(!registered_to_gprs_network && cc < 5);


	flush_array((char*)fota_fail_cause, 40);

	if(registered_to_gprs_network)
	{
		HAL_Delay(100);


		if(deactivate_pdp())
		{
			flush_tx_string();
			sprintf((char*)tx_string, "AT+QICSGP=1,1,\"%s\",\"\",\"\",1\r\n\0", apn);///////////SET APN
			if(at_cmd_send((char*)tx_string,"OK\r\n","ERROR",5,0))
			{

				if(activate_pdp())
				{
					if(at_cmd_send("AT+QIACT?\r\n\0","QIACT:","ERROR",5,0))
					{
						HAL_Delay(100);

						at_cmd_send("AT+QFTPCFG=\"contextid\",1\r\n\0","OK\r\n","ERROR",5,0);

						flush_tx_string();
						sprintf((char*)tx_string, "AT+QFTPCFG=\"account\",\"%s\",\"%s\"\r\n\0", ftp_user, ftp_pswd);//////////SET USER & PASSWORD
						if(at_cmd_send((char*)tx_string,"OK\r\n","ERROR",5,0))
						{
							at_cmd_send("AT+QFTPCFG=\"filetype\",1\r\n\0","OK\r\n","ERROR",5,0);

							at_cmd_send("AT+QFTPCFG=\"transmode\",1\r\n\0","OK\r\n","ERROR",5,0);

							at_cmd_send("AT+QFTPCFG=\"rsptimeout\",90\r\n\0","OK\r\n","ERROR",5,0);

							flush_tx_string();
							sprintf((char*)tx_string, "AT+QFTPOPEN=\"%s\",%u\r\n\0", ftp_server,ftp_port);/////OPEN FTP CONNECTION
							if(at_cmd_send((char*)tx_string,"QFTPOPEN:","ERROR",95,0))
							{
								HAL_Delay(20);
								if(gsm_data[11]=='0')//if connection success///QFTPOPEN: 0,0
								{
									at_cmd_send("AT+QFTPLIST=\".\"\r\n\0","QFTPLIST:","ERROR",60,0);//+QFTPLIST: 0
									HAL_Delay(1000);


									flush_tx_string();
									sprintf((char*)tx_string,"AT+QFDEL=\"%s\"\r\n\0", ftp_filename);
									at_cmd_send((char*)tx_string,"OK\r\n","ERROR",5,0); ///////DELETE OLD FILE

									at_cmd_send("AT+QFLDS=\"UFS\"\r\n\0","OK\r\n","ERROR",5,0);


									HAL_Delay(300);
										flush_tx_string();
										sprintf((char*)tx_string, "AT+QFTPGET=\"%s\",\"%s\"\r\n\0", ftp_filename, ftp_filename);////////DOWNLOAD FILE
										if(at_cmd_send((char*)tx_string,"QFTPGET:","ERROR",180,0))
										{
											HAL_Delay(100);

											//+QFTPGET: 0,25369
											if(gsm_data[2]=='F' && gsm_data[5]=='G' && gsm_data[7]=='T' && gsm_data[8]==':' && gsm_data[10]=='0')//if no error
											{

												HAL_Delay(100);
												flush_tx_string();
												for(uint8_t z=12; gsm_data[z]!=0x0D && z < 20; z++)///// COPY FILE SIZE
												{
													tx_string[z-12] = gsm_data[z];
												}

												file_size = atol((char*)tx_string);
												send_text_to_uart1(tx_string);

//												send_sms_data(53);

												///AT+QFLST="*"
//												at_cmd_send("AT+QFLST=\"*\"\r\n\0","OK","ERROR",5,0);////////Get File List USF
//												HAL_Delay(100);

												///+QFTPCLOSE: 0,0
												at_cmd_send("AT+QFTPCLOSE\r\n\0","QFTPCLOSE: 0","ERROR",120,0);////////CLOSE FTP CONNECTION
												HAL_Delay(100);

													at_cmd_send("AT+CFUN=0\r\n\0","OK\r\n","ERROR",10,0);/////////////MINIMUM FUNCTIONALITY MODE

												__Call_BootLoader_(1 , (uint64_t)file_size);
												return 1;

//													flush_tx_string();
//												sprintf((char*)tx_string, "AT+QFOPEN=\"%s\"\r\n\0", ftp_filename);/////OPEN FILE FROM RAM
//													if(at_cmd_send((char*)tx_string,"QFOPEN:","ERROR",120,0))
//													{
//														uint32_t read_value=0;
//
//														HAL_Delay(100);
//														flush_tx_string();
//														for(uint8_t z=9; gsm_data[z]!=0x0D && z < 20; z++)///// COPY FILE ID
//														{
//															tx_string[z-9] = gsm_data[z];
//														}
//
//														read_value = atol((char*)tx_string);
//
////														__Call_BootLoader_((uint64_t) file_size);
//														__Call_BootLoader_((uint64_t) read_value, (uint64_t)file_size);
//
//														return 1;
//													}


											}
											else
												sprintf((char*)fota_fail_cause,"FILE-DOWNLOAD-ERR");
										}
										else
											sprintf((char*)fota_fail_cause,"FILE-DOWNLOAD-ERR");

								}
								else
									sprintf((char*)fota_fail_cause,"FTP-CONN-ERR");
							}
							else
								sprintf((char*)fota_fail_cause,"FTP-CONN-ERR");
						}
					}
				}
			}
		}
	}
	else
		sprintf((char*)fota_fail_cause,"NO-GPRS");

	return 0;
}



void Be_SecondFW(void)
{
	  __IO uint32_t *VectorTable = (volatile uint32_t *)0x20000000U;
    uint32_t ui32_VectorIndex = 0;
    for(ui32_VectorIndex = 0; ui32_VectorIndex < 48; ui32_VectorIndex++)
    {
        VectorTable[ui32_VectorIndex] = *(__IO uint32_t*)((uint32_t)0x08003000U + (ui32_VectorIndex << 2));
    }
    __HAL_RCC_AHB_FORCE_RESET();
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_AHB_RELEASE_RESET();
    __HAL_SYSCFG_REMAPMEMORY_SRAM();

	/* Relocate by software the vector table to the internal SRAM at 0x20000000 ***/

  /* Copy the vector table from the Flash (mapped at the base of the application
     load address 0x08004000) to the base address of the SRAM at 0x20000000. */

//	uint32_t i = 0;
//
//  for(i = 0; i < 48; i++)
//  {
//    VectorTable[i] = *(__IO uint32_t*)(APPLICATION_ADDRESS + (i<<2));
//  }

//  /* Enable the SYSCFG peripheral clock*/
//  __HAL_RCC_SYSCFG_CLK_ENABLE();
//  /* Remap SRAM at 0x00000000 */
//  __HAL_SYSCFG_REMAPMEMORY_SRAM();

}

_Bool esp_enable_softap_mode(void)
{
	//Set ssid and password
	if(at_cmd_send_esp("AT\r\n\0","OK\r\n","ERROR",5,0))
	{
		sprintf((char*)ble_ssid_name, "BlipGo+%s\0", device_id);

		sprintf((char*)tx_string, "AT+CWSAP=\"%s\",\"1234567890\",5,3\r\n\0", ble_ssid_name);
		//set wifi mode to SoftAP+Station
		if(at_cmd_send_esp((char*)tx_string,"OK\r\n","ERROR",5,0))
		{
			//Set SoftAP IP and default gateway
			if(at_cmd_send_esp("AT+CIPAP=\"192.168.50.1\",\"192.168.50.1\",\"255.255.255.0\"\r\n\0","OK\r\n","ERROR",5,0))
			{
				//Check IP settings
				if(at_cmd_send_esp("AT+CIPAP?\r\n\0","OK\r\n","ERROR",5,0))
				{
					//Turn on multiplexing, it is required
					if(at_cmd_send_esp("AT+CIPMUX=1\r\n\0","OK\r\n","ERROR",5,0))
					{
						//Start server on port 9001 or any port
						if(at_cmd_send_esp("AT+CIPSERVER=1,9001\r\n\0","OK\r\n","ERROR",5,0))
						{
							return 1;
						}
					}
				}
			}
		}
	}

	return 0;

}


_Bool ble_is_available_device_paired(void)
{
	for(uint8_t i=0; i < 18; i++)
	{
		if(ble_available_device[i] != ble_paired_device[i] || ble_available_device[0]==0 || ble_paired_device[0]==0)
			return 0;
	}

	return 1;
}

void beep(uint8_t duration)
{
	//BUZZOR ON
	HAL_GPIO_WritePin(BUZZOR_GPIO_Port,BUZZOR_Pin,GPIO_PIN_SET);
	HAL_TIM_PWM_Start(&htim16, TIM_CHANNEL_1);//// PWM Buzzer ON
	htim16.Instance->CCR1 = 500;/// 50% duty cycle

	HAL_Delay(duration);

	//BUZZOR OFF
	HAL_GPIO_WritePin(BUZZOR_GPIO_Port,BUZZOR_Pin,GPIO_PIN_RESET);
	HAL_TIM_PWM_Stop(&htim16, TIM_CHANNEL_1);/// PWM Buzzer OFF


	////PWM Buzzer Settings
//		HAL_TIM_PWM_Start(&htim16, TIM_CHANNEL_1);
//		htim16.Instance->CCR1 = 500;/// 50% duty cycle
//			HAL_TIM_PWM_Stop(&htim16, TIM_CHANNEL_1);
//			HAL_Delay(500);
//			HAL_TIM_PWM_Start(&htim16, TIM_CHANNEL_1);
//			HAL_Delay(500);

}

_Bool send_softap_response(char* str, uint16_t len)
{
	unsigned char ap_tx_string[400]={0};

	sprintf((char*)ap_tx_string, "AT+CIPSEND=0,%u\r\n\0", len);

	if(at_cmd_send_esp((char*)ap_tx_string,">","ERROR",5,0))
	{
		if(at_cmd_send_esp(str ,"SEND OK\r\n","SEND FAIL",5,0))
		{
			return 1;
		}
	}

	return 0;
}

/** TO SEARCH FOR A STRING IN A GIVEN MAIN STRING
	* @PARAM			searchStr - SUB STRING TO SEARCH
	*							searchStrLength - LENGTH OF STRING TO SEARCH
	*							mainStr - MAIN STRING TO SEARCH IN
	*							mainStrLength - LENGTH OF MAIN SRTING
	*
	*	@RETURN
	*							Found - returns INDEX OF LAST CHAR OF SEARCH STRING IN A GIVEN MAIN STRING
	*							Not Found - returns 0
	*/
uint16_t getIndexOf(char *searchStr, uint16_t searchStrLength, char *mainStr, uint16_t mainStrLength)
{
	for(uint16_t i=0; i<(mainStrLength-searchStrLength+1); i++)
	{
		if(searchStr[0]==mainStr[i])/////1st char matched
		{
			for(uint16_t j=1, k=i+1; j<searchStrLength; j++, k++)
			{
				if(searchStr[j]==mainStr[k])
				{
					if(j==(searchStrLength-1))
					{
						return k;///i is start index k is end index
					}
				}
				else
				{
					break;
				}
			}
		}
	}

	return 0;
}

_Bool update_module_firmware(void)
{
	//////////   Module Firmware Update ////////
	deactivate_pdp();

	at_cmd_send("AT+QHTTPCFG=\"contextid\",1\r\n\0","OK\r\n","ERROR",5,0);

	at_cmd_send("at+qhttpcfg=\"requestheader\",0\r\n\0","OK\r\n","ERROR",5,0);

	flush_tx_string();
	sprintf((char*)tx_string, "AT+QICSGP=1,1,\"%s\",\"\",\"\",1\r\n\0", apn);///////////SET APN
	at_cmd_send((char*)tx_string,"OK\r\n","ERROR",5,0);

	if(activate_pdp())
	{

		at_cmd_send("AT+QIACT?\r\n\0","OK\r\n","ERROR",5,0);


		if(at_cmd_send("at+qhttpurl=68,80\r\n\0","CONNECT\r\n","ERROR",70,0))
		{

			flush_tx_string();
			sprintf((char*)tx_string, "https://shark.carematix.com/cs/%s/mu?token=%s", device_id, http_token);

			at_cmd_send((char*)tx_string,"OK\r\n","ERROR",70,0);

			at_cmd_send("at+qhttpurl?\r\n\0","OK\r\n","ERROR",5,0);

			if(at_cmd_send("at+qhttpget=80\r\n\0","QHTTPGET: 0","ERROR",70,0))
			{
				HAL_Delay(10);


				plus_enable=0;


				if(at_cmd_send("AT+QHTTPREAD=80\r\n\0","QHTTPREAD: 0\r\n","ERROR", 20, 0))//+QHTTPREAD: 0
				{
					/////fetch url
					/*
					AT+QHTTPREAD=80

					CONNECT
					https://bg96dfota.s3-us-west-2.amazonaws.com/BG96MAR02A07M1G_01.014.01.014-01.017.01.017.bin
					OK

					+QHTTPREAD: 0
					*/

					//fetch url code tested online:  https://onlinegdb.com/HkRttS0Ow

					if(strstr((char*)gsm_data, "Unauthorized"))
					{
						ping_service_flag=1;
						plus_enable=1;
						return 0;
					}

					unsigned char update_url[150]={0};
					char *searchStr = {0};
					uint16_t startIndex = 0, endIndex = 0;

					searchStr = "CONNECT";
					startIndex = getIndexOf(searchStr, (uint16_t)strlen(searchStr), (char*)gsm_data, (uint16_t)strlen((char*)gsm_data));

					searchStr = "OK";
					endIndex = getIndexOf(searchStr, (uint16_t)strlen(searchStr), (char*)gsm_data, (uint16_t)strlen((char*)gsm_data));


					if(startIndex > 0)
					{

						////////FETCH UPDATE URL
						for(uint16_t j=0, index=(startIndex+3); index<(endIndex-3); j++, index++)
						{
							update_url[j] = gsm_data[index];
						}

						send_text_to_usb("\r\nUpdating BG96 Firmware Please Wait!\r\n\0");

						flush_tx_string();
						sprintf((char*)tx_string, "AT+QFOTADL=\"%s\"\r\n\0", update_url);



						plus_enable=0;

						if(at_cmd_send((char*)tx_string,"HTTPSTART","ERROR", 600, 0))
						{
							uint16_t searchIndex=0;

							plus_enable=1;

							searchStr = "\"HTTPEND\"";

							while(1)
							{
								searchIndex = getIndexOf(searchStr, (uint16_t)strlen(searchStr), (char*)gsm_data, gsm_count+1);

								if(searchIndex!=0)
								{
									plus_enable=0;
									break;
								}
							}

							HAL_Delay(10);

							searchStr = "\"HTTPEND\",0";
							searchIndex = getIndexOf(searchStr, (uint16_t)strlen(searchStr), (char*)gsm_data, gsm_count+1);



							if(searchIndex==0)
							{
								flush_array((char*)temp_data, sizeof(temp_data)/sizeof(temp_data[0]));
								sprintf((char*)temp_data, "\r\nFail\r\nSearch:%s  Index:%u  gsmdata:%s\r\n\0", searchStr, searchIndex, gsm_data);
								send_text_to_usb((char*)temp_data);

								return 0;
							}

							plus_enable=1;

							searchStr = "\"END\"";

							while(1)
							{
								searchIndex = getIndexOf(searchStr, (uint16_t)strlen(searchStr), (char*)gsm_data, gsm_count+1);

//								flush_array((char*)temp_data, sizeof(temp_data)/sizeof(temp_data[0]));
//								sprintf((char*)temp_data, "\r\nwhile\r\nSearch:%s  Index:%u  gsmdata:%s\r\n\0", searchStr, searchIndex, gsm_data);
//								send_text_to_usb((char*)temp_data);

								if(searchIndex!=0)
								{
									break;
								}

//								HAL_Delay(100);
							}



							HAL_Delay(10);

							searchStr = "\"END\",0";
							searchIndex = getIndexOf(searchStr, (uint16_t)strlen(searchStr), (char*)gsm_data, gsm_count+1);

							if(searchIndex==0)
							{
								flush_array((char*)temp_data, sizeof(temp_data)/sizeof(temp_data[0]));
								sprintf((char*)temp_data, "\r\nFail\r\nSearch:%s  Index:%u  gsmdata:%s\r\n\0", searchStr, searchIndex, gsm_data);
								send_text_to_usb((char*)temp_data);

								return 0;
							}

							flush_gsm_array();

							plus_enable=0;

							searchStr = "APP RDY";

							while(1)
							{
								searchIndex = getIndexOf(searchStr, (uint16_t)strlen(searchStr), (char*)gsm_data, gsm_count+1);

								flush_array((char*)temp_data, sizeof(temp_data)/sizeof(temp_data[0]));
								sprintf((char*)temp_data, "\r\nwhile\r\nSearch:%s  Index:%u  gsmdata:%s\r\n\0", searchStr, searchIndex, gsm_data);
								send_text_to_usb((char*)temp_data);

								if(searchIndex!=0)
								{
									break;
								}

//								HAL_Delay(100);
							}

							plus_enable=1;
							return 1;
						}

						plus_enable=1;
					}
				}

				plus_enable=1;
			}
		}

	}

	return 0;

}


_Bool update_device_firmware(void)
{
	//////////   Device Firmware Update ////////
	deactivate_pdp();

	at_cmd_send("AT+QHTTPCFG=\"contextid\",1\r\n\0","OK\r\n","ERROR",5,0);

	at_cmd_send("at+qhttpcfg=\"requestheader\",0\r\n\0","OK\r\n","ERROR",5,0);

	flush_tx_string();
	sprintf((char*)tx_string, "AT+QICSGP=1,1,\"%s\",\"\",\"\",1\r\n\0", apn);///////////SET APN
	at_cmd_send((char*)tx_string,"OK\r\n","ERROR",5,0);

	if(activate_pdp())
	{

		at_cmd_send("AT+QIACT?\r\n\0","OK\r\n","ERROR",5,0);


		if(at_cmd_send("at+qhttpurl=68,80\r\n\0","CONNECT\r\n","ERROR",70,0))
		{

			flush_tx_string();
			sprintf((char*)tx_string, "https://shark.carematix.com/cs/%s/df?token=%s", device_id, http_token);

			at_cmd_send((char*)tx_string,"OK\r\n","ERROR",70,0);

			at_cmd_send("at+qhttpurl?\r\n\0","OK\r\n","ERROR",5,0);

			plus_enable=1;

			if(at_cmd_send("at+qhttpget=80\r\n\0","QHTTPGET: 0,200","ERROR",70,0))
			{
				HAL_Delay(20);

				// To fetch file sizeof from response +QHTTPGET: 0,200,45268
				unsigned char response_size[8]={0};
					char *searchStr = {0};
					uint16_t startIndex = 0, endIndex = 0;
					_Bool valid_file_size=0;

					searchStr = "0,200,";
					startIndex = getIndexOf(searchStr, (uint16_t)strlen(searchStr), (char*)gsm_data, (uint16_t)strlen((char*)gsm_data));

					searchStr = "\r\n";
					endIndex = getIndexOf(searchStr, (uint16_t)strlen(searchStr), (char*)gsm_data, (uint16_t)strlen((char*)gsm_data));


					if(startIndex > 0)
					{

						////////FETCH UPDATE URL
						for(uint16_t j=0, index=(startIndex+1); index<(endIndex-1); j++, index++)
						{
							response_size[j] = gsm_data[index];
						}

						file_size = atol((char*)response_size);

						//// if file size is valid  i.e. > 30000 bytes
						if(file_size > 30000)
						{
							valid_file_size = 1;
						}
						else
						{
							update_device_firmware_flag = 0;
						}
					}

				plus_enable=0;

				if(valid_file_size)
				{
					///// DELETE OLD FILE
					at_cmd_send("AT+QFDEL=\"blipgo.bin\"\r\n\0","OK\r\n","ERROR",5,0); ///////DELETE OLD FILE

					////// download the bin file content to a file directly
					if(at_cmd_send("AT+QHTTPREADFILE=\"blipgo.bin\",80\r\n\0","QHTTPREADFILE: 0\r\n","ERROR", 120, 0))//+QHTTPREAD: 0
					{
						/// add cfun 0
						at_cmd_send("AT+CFUN=0\r\n\0","OK\r\n","ERROR",10,0);/////////////MINIMUM FUNCTIONALITY MODE

						//// IF FILE DOWNLOAD COMPLETE THEN FIRMWARE WILL BE UPDATED
						update_device_firmware_flag=0;

						// UPDATE CV IF ALL CHANGES DONE
						update_cv_value();

						__Call_BootLoader_(1 , (uint64_t)file_size);
						return 1;
					}
				}
				plus_enable=1;



				////// debug data TTL
//				flush_tx_string();
//				sprintf((char*)tx_string, "\r\ngsm_data:%s\r\nresponse_size:%s\r\nstartIndex:%u\r\nfile_size:%u\r\n\0",
//					gsm_data, response_size, startIndex, file_size);
//				send_text_to_uart2(tx_string);
//				while(1);
			}
		}
	}

	return 0;
}


_Bool ping_service(void)
{
	//////////   PING SERVICE ////////
	deactivate_pdp();

	at_cmd_send("AT+QHTTPCFG=\"contextid\",1\r\n\0","OK\r\n","ERROR",5,0);

	at_cmd_send("at+qhttpcfg=\"requestheader\",1\r\n\0","OK\r\n","ERROR",5,0);

	flush_tx_string();
	sprintf((char*)tx_string, "AT+QICSGP=1,1,\"%s\",\"\",\"\",1\r\n\0", apn);///////////SET APN
	at_cmd_send((char*)tx_string,"OK\r\n","ERROR",5,0);

	if(activate_pdp())
	{

		at_cmd_send("AT+QIACT?\r\n\0","OK\r\n","ERROR",5,0);


		if(at_cmd_send("at+qhttpurl=27,80\r\n\0","CONNECT\r\n","ERROR",70,0))
		{
			at_cmd_send("https://shark.carematix.com\r\n\0","OK\r\n","ERROR",70,0);

			at_cmd_send("at+qhttpurl?\r\n\0","OK\r\n","ERROR",5,0);


				flush_tx_string();
				sprintf((char*)tx_string, "POST /cs/%s HTTP/1.1\r\n\0", device_id);///////////// DEVICE ID
				sprintf((char*)tx_string, "%sHost: shark.carematix.com\r\n\0", tx_string);
				sprintf((char*)tx_string, "%sContent-Type: application/json\r\n\0", tx_string);
				sprintf((char*)tx_string, "%sContent-Length: 160\r\n\0", tx_string);
				sprintf((char*)tx_string, "%sAuthorization: Basic Y2FyZW1hdGl4OnBhc3N3b3Jk\r\n\r\n\0", tx_string);

				/*
				{�model�:�600�,"sim":"898604051918C0024877","imei":"866425038410462","imsi":"460042513904877",�cv�:1, �fv�:�1.0�,�pv�:�1.1�,�mv�:�01.017�}
				*/
				sprintf((char*)tx_string, "%s{\"model\":\"%s\",\"sim\":\"%s\",\"imei\":\"%s\",\"imsi\":\"%s\",\"cv\":%s,\"fv\":\"%s\",\"pv\":\"%s\",\"mv\":\"%s\"}\r\n\0",
					tx_string, blipgo_model, ccid, imei, imsi, configuration_version, firmware_version, protocol_version, module_firmware_version);


				unsigned char at_command[30]={0};

//				sprintf((char*)at_command, "at+qhttpget=80,%u\r\n\0", strlen((char*)tx_string));
				sprintf((char*)at_command, "at+qhttppost=%u,80,80\r\n\0", strlen((char*)tx_string));

			if(at_cmd_send((char*)at_command,"CONNECT\r\n","ERROR",70,0))
			{
				HAL_Delay(10);

				if(at_cmd_send((char*)tx_string, "QHTTPPOST: 0", "ERROR", 70, 0))//+QHTTPGET: 0,200,80
				{

					flush_array((char*)json_response, 600);
					json_response_flag = 1;

					if(at_cmd_send("AT+QHTTPREAD=80\r\n\0","QHTTPREAD: 0\r\n","ERROR",70,0))///+QHTTPREAD: 0
					{
						json_response_flag = 0;

						if(json_response[0] == 0)
						{
							json_response[0] = '0';
						}

						send_text_to_usb("\r\nJSON Response: \0");
						send_text_to_usb((char*)json_response);
						send_text_to_usb("\r\n\0");

						if(json_get_value("st", 2, 1, (char*)temp_data, 100))/////// GET STATUS VALUE FROM JSON RESPONSE
						{
							send_text_to_usb("\r\nStatus: \0");
							send_text_to_usb((char*)temp_data);
							send_text_to_usb("\r\n\0");
						}
						else
						{
							return 0;
						}

						if(temp_data[0] == '1')////////////// IF STATUS is 1
						{
							if(json_get_value("cv", 2, 1, (char*)temp_data, 10))/////// GET SERVER CONFIGURATION VERSION VALUE FROM JSON RESPONSE
							{
								send_text_to_usb("Server Configuration Version: \0");
								send_text_to_usb((char*)temp_data);
								send_text_to_usb("\r\n\0");
							}

							if(configuration_version[0]!=temp_data[0] || configuration_version[1]!=temp_data[1] || configuration_version[2]!=temp_data[2])
							{
								sprintf((char*)hold_cv_value, "%s\0", (char*)temp_data);///// HOLD CV VALUE IN TEMP VARIABLE

								if(json_get_value("tk", 2, 1, (char*)http_token, 17))/////// GET HTTP TOKEN VALUE FROM JSON RESPONSE
								{
									send_text_to_usb("HTTP Token: \0");
									send_text_to_usb((char*)http_token);
									send_text_to_usb("\r\n\0");
								}

								if(json_get_value("ca", 2, 1, (char*)temp_data, 100))/////// GET ROOT CA VALUE FROM JSON RESPONSE
								{
									send_text_to_usb("Root CA: \0");
									send_text_to_usb((char*)temp_data);
									send_text_to_usb("\r\n\0");

									ca_certificate_flag = 1;
								}

								if(json_get_value("cc", 2, 1, (char*)temp_data, 100))/////// GET CLIENT CERTIFICATE VALUE FROM JSON RESPONSE
								{
									send_text_to_usb("Client Certificate: \0");
									send_text_to_usb((char*)temp_data);
									send_text_to_usb("\r\n\0");

									client_certificate_flag = 1;
								}

								if(json_get_value("ck", 2, 1, (char*)temp_data, 100))/////// GET CLIENT KEY VALUE FROM JSON RESPONSE
								{
									send_text_to_usb("Client Key: \0");
									send_text_to_usb((char*)temp_data);
									send_text_to_usb("\r\n\0");

									client_key_flag = 1;
								}

								if(json_get_value("sc", 2, 1, (char*)temp_data, 100))/////// GET SERVER CONFIGURATION VALUE FROM JSON RESPONSE
								{
									send_text_to_usb("Server Configuration: \0");
									send_text_to_usb((char*)temp_data);
									send_text_to_usb("\r\n\0");

									configuration_service_flag = 1;
								}

								if(json_get_value("fv", 2, 1, (char*)temp_data, 100))/////// GET Firmware Version VALUE FROM JSON RESPONSE
								{
									send_text_to_usb("Device Firmware Version: \0");
									send_text_to_usb((char*)temp_data);
									send_text_to_usb("\r\n\0");

									update_device_firmware_flag = 1;
								}

								if(json_get_value("mv", 2, 1, (char*)temp_data, 100))/////// GET Module Version VALUE FROM JSON RESPONSE
								{
									send_text_to_usb("Module Firmware Version: \0");
									send_text_to_usb((char*)temp_data);
									send_text_to_usb("\r\n\0");

									update_module_firmware_flag = 1;
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
							update_device_firmware_flag = 0;

							update_device_twin_flag=1;
						}

						return 1;
					}

					json_response_flag = 0;
				}
			}
		}

	}

	return 0;
	////////// PING SERVICE END ////////
}


_Bool configuration_service(void)
{
	//////////   CONFIGURATION SERVICE ////////
	deactivate_pdp();

	at_cmd_send("AT+QHTTPCFG=\"contextid\",1\r\n\0","OK\r\n","ERROR",5,0);

	at_cmd_send("at+qhttpcfg=\"requestheader\",0\r\n\0","OK\r\n","ERROR",5,0);

	flush_tx_string();
	sprintf((char*)tx_string, "AT+QICSGP=1,1,\"%s\",\"\",\"\",1\r\n\0", apn);///////////SET APN
	at_cmd_send((char*)tx_string,"OK\r\n","ERROR",5,0);

	if(activate_pdp())
	{

		at_cmd_send("AT+QIACT?\r\n\0","OK\r\n","ERROR",5,0);


		if(at_cmd_send("at+qhttpurl=68,80\r\n\0","CONNECT\r\n","ERROR",70,0))
		{

			flush_tx_string();
			sprintf((char*)tx_string, "https://shark.carematix.com/cs/%s/sc?token=%s", device_id, http_token);

			at_cmd_send((char*)tx_string,"OK\r\n","ERROR",70,0);

			at_cmd_send("at+qhttpurl?\r\n\0","OK\r\n","ERROR",5,0);

			if(at_cmd_send("at+qhttpget=80\r\n\0","QHTTPGET: 0","ERROR",70,0))
			{
				HAL_Delay(10);


					flush_array((char*)json_response, 600);
					json_response_flag = 1;

					if(at_cmd_send("AT+QHTTPREAD=80\r\n\0","QHTTPREAD: 0\r\n","ERROR",70,0))///+QHTTPREAD: 0
					{

						if(strstr((char*)gsm_data, "Unauthorized"))
						{
							ping_service_flag=1;
							return 0;
						}

						json_response_flag = 0;

						send_text_to_usb("\r\nJSON Response: \0");
						send_text_to_usb((char*)json_response);
						send_text_to_usb("\r\n\0");

						if(json_get_value("mqtt", 4, 1, (char*)temp_data, 100))/////// GET MQTT VERSION VALUE FROM JSON RESPONSE
						{
							send_text_to_usb("\r\nMQTT Protocol Version: \0");
							send_text_to_usb((char*)temp_data);
							send_text_to_usb("\r\n\0");
						}

						if(json_get_value("tls", 3, 1, (char*)temp_data, 100))/////// GET TLS VERSION VALUE FROM JSON RESPONSE
						{
							send_text_to_usb("TLS Version: \0");
							send_text_to_usb((char*)temp_data);
							send_text_to_usb("\r\n\0");
						}

						if(json_get_value("did", 3, 1, (char*)temp_data, 100))/////// GET DEVICE ID VALUE FROM JSON RESPONSE
						{
							send_text_to_usb("Device ID: \0");
							send_text_to_usb((char*)temp_data);
							send_text_to_usb("\r\n\0");
						}

						if(json_get_value("svr", 3, 1, (char*)mqtt_server, 100))/////// GET SERVER HOST NAME VALUE FROM JSON RESPONSE
						{
							send_text_to_usb("Server Host Name: \0");
							send_text_to_usb((char*)mqtt_server);
							send_text_to_usb("\r\n\0");
						}

						if(json_get_value("prt", 3, 1, (char*)mqtt_port, 7))/////// GET PORT VALUE FROM JSON RESPONSE
						{
							send_text_to_usb("Port Number: \0");
							send_text_to_usb((char*)mqtt_port);
							send_text_to_usb("\r\n\0");
						}

						if(json_get_value("un", 2, 1, (char*)mqtt_username, 100))/////// GET USERNAME VALUE FROM JSON RESPONSE
						{
							send_text_to_usb("User Name: \0");
							send_text_to_usb((char*)mqtt_username);
							send_text_to_usb("\r\n\0");
						}

						if(json_get_value("pd", 2, 1, (char*)mqtt_password, 50))/////// GET PASSWORD VALUE FROM JSON RESPONSE
						{
							send_text_to_usb("Password: \0");
							send_text_to_usb((char*)mqtt_password);
							send_text_to_usb("\r\n\0");
						}

						if(json_get_value("cid", 3, 1, (char*)mqtt_client_id, 100))/////// GET CLIENT ID VALUE FROM JSON RESPONSE
						{
							send_text_to_usb("Client ID: \0");
							send_text_to_usb((char*)mqtt_client_id);
							send_text_to_usb("\r\n\0");
						}

						if(json_get_value("qos", 3, 1, (char*)temp_data, 100))/////// GET QOS VALUE FROM JSON RESPONSE
						{
							send_text_to_usb("QoS in MQTT: \0");
							send_text_to_usb((char*)temp_data);
							send_text_to_usb("\r\n\0");
						}

						if(json_get_value("twin_sb", 7, 1, (char*)twin_sb, 100))/////// GET SUBSCRIBE TOPIC VALUE FROM JSON RESPONSE
						{
							send_text_to_usb("twin_sb: \0");
							send_text_to_usb((char*)twin_sb);
							send_text_to_usb("\r\n\0");
						}

						if(json_get_value("twin_pb", 7, 1, (char*)twin_pb, 100))/////// GET PUBLISH TOPIC FOR NULL DATA VALUE FROM JSON RESPONSE
						{
							send_text_to_usb("twin_pb: \0");
							send_text_to_usb((char*)twin_pb);
							send_text_to_usb("\r\n\0");
						}

						if(json_get_value("twin_rp", 7, 1, (char*)twin_rp, 100))/////// GET PUBLISH TOPIC TO REPORT META DATA VALUE FROM JSON RESPONSE
						{
							send_text_to_usb("twin_rp: \0");
							send_text_to_usb((char*)twin_rp);
							send_text_to_usb("\r\n\0");
						}

						if(json_get_value("read_pb", 7, 1, (char*)read_pb, 100))/////// GET PUBLISH TOPIC FOR MEASUREMENT DATA VALUE FROM JSON RESPONSE
						{
							send_text_to_usb("read_pb: \0");
							send_text_to_usb((char*)read_pb);
							send_text_to_usb("\r\n\0");
						}

						update_device_twin_flag = 1;/////// IF NEW CONFIGURATION THEN UPDATE DEVICE TWIN

						return 1;
					}

					json_response_flag = 0;

			}
		}

	}

	return 0;
	////////// CONFIGURATION SERVICE END ////////
}


_Bool get_root_ca_certificate(void)
{
	//////////   ROOT CA CERTIFICATE SERVICE ////////
	deactivate_pdp();

	at_cmd_send("AT+QHTTPCFG=\"contextid\",1\r\n\0","OK\r\n","ERROR",5,0);

	at_cmd_send("at+qhttpcfg=\"requestheader\",0\r\n\0","OK\r\n","ERROR",5,0);

	flush_tx_string();
	sprintf((char*)tx_string, "AT+QICSGP=1,1,\"%s\",\"\",\"\",1\r\n\0", apn);///////////SET APN
	at_cmd_send((char*)tx_string,"OK\r\n","ERROR",5,0);

	if(activate_pdp())
	{

		at_cmd_send("AT+QIACT?\r\n\0","OK\r\n","ERROR",5,0);


		if(at_cmd_send("at+qhttpurl=68,80\r\n\0","CONNECT\r\n","ERROR",70,0))
		{

			flush_tx_string();
			sprintf((char*)tx_string, "https://shark.carematix.com/cs/%s/ca?token=%s", device_id, http_token);

			at_cmd_send((char*)tx_string,"OK\r\n","ERROR",70,0);

			at_cmd_send("at+qhttpurl?\r\n\0","OK\r\n","ERROR",5,0);

			if(at_cmd_send("at+qhttpget=80\r\n\0","QHTTPGET: 0","ERROR",70,0))
			{
				HAL_Delay(10);

//						at_cmd_send("AT+QFDEL=\"UFS:security/CaCert.crt\"\r\n\0","OK\r\n","ERROR", 10, 0);

						if(at_cmd_send("at+qhttpreadfile=\"UFS:security/CaCert.crt\",80\r\n\0","QHTTPREADFILE: 0\r\n","ERROR", 20, 0))//+QHTTPREADFILE: 0
						{
							return 1;
						}
			}
		}

	}

	return 0;
	////////// ROOT CA CERTIFICATE SERVICE END ////////
}

_Bool get_client_certificate(void)
{
	//////////   CLIENT CERTIFICATE SERVICE ////////
	deactivate_pdp();

	at_cmd_send("AT+QHTTPCFG=\"contextid\",1\r\n\0","OK\r\n","ERROR",5,0);

	at_cmd_send("at+qhttpcfg=\"requestheader\",0\r\n\0","OK\r\n","ERROR",5,0);

	flush_tx_string();
	sprintf((char*)tx_string, "AT+QICSGP=1,1,\"%s\",\"\",\"\",1\r\n\0", apn);///////////SET APN
	at_cmd_send((char*)tx_string,"OK\r\n","ERROR",5,0);

	if(activate_pdp())
	{

		at_cmd_send("AT+QIACT?\r\n\0","OK\r\n","ERROR",5,0);


		if(at_cmd_send("at+qhttpurl=68,80\r\n\0","CONNECT\r\n","ERROR",70,0))
		{

			flush_tx_string();
			sprintf((char*)tx_string, "https://shark.carematix.com/cs/%s/cc?token=%s", device_id, http_token);

			at_cmd_send((char*)tx_string,"OK\r\n","ERROR",70,0);

			at_cmd_send("at+qhttpurl?\r\n\0","OK\r\n","ERROR",5,0);

			if(at_cmd_send("at+qhttpget=80\r\n\0","QHTTPGET: 0","ERROR",70,0))
			{
				HAL_Delay(10);

//						at_cmd_send("AT+QFDEL=\"UFS:security/Client.crt\"\r\n\0","OK\r\n","ERROR", 10, 0);

						if(at_cmd_send("at+qhttpreadfile=\"UFS:security/Client.crt\",80\r\n\0","QHTTPREADFILE: 0\r\n","ERROR", 20, 0))//+QHTTPREADFILE: 0
						{
							return 1;
						}
			}
		}

	}

	return 0;
	////////// CLIENT CERTIFICATE SERVICE END ////////
}

_Bool get_client_key(void)
{
	//////////   CLIENT KEY SERVICE ////////
	deactivate_pdp();

	at_cmd_send("AT+QHTTPCFG=\"contextid\",1\r\n\0","OK\r\n","ERROR",5,0);

	at_cmd_send("at+qhttpcfg=\"requestheader\",0\r\n\0","OK\r\n","ERROR",5,0);

	flush_tx_string();
	sprintf((char*)tx_string, "AT+QICSGP=1,1,\"%s\",\"\",\"\",1\r\n\0", apn);///////////SET APN
	at_cmd_send((char*)tx_string,"OK\r\n","ERROR",5,0);

	if(activate_pdp())
	{

		at_cmd_send("AT+QIACT?\r\n\0","OK\r\n","ERROR",5,0);


		if(at_cmd_send("at+qhttpurl=68,80\r\n\0","CONNECT\r\n","ERROR",70,0))
		{

			flush_tx_string();
			sprintf((char*)tx_string, "https://shark.carematix.com/cs/%s/ck?token=%s", device_id, http_token);

			at_cmd_send((char*)tx_string,"OK\r\n","ERROR",70,0);

			at_cmd_send("at+qhttpurl?\r\n\0","OK\r\n","ERROR",5,0);

			if(at_cmd_send("at+qhttpget=80\r\n\0","QHTTPGET: 0","ERROR",70,0))
			{
				HAL_Delay(10);

//						at_cmd_send("AT+QFDEL=\"UFS:security/key.pem\"\r\n\0","OK\r\n","ERROR", 10, 0);

						if(at_cmd_send("at+qhttpreadfile=\"UFS:security/key.pem\",80\r\n\0","QHTTPREADFILE: 0","ERROR", 20, 0))//+QHTTPREADFILE: 0
						{
							return 1;
						}
			}
		}

	}

	return 0;
	////////// CLIENT KEY SERVICE END ////////
}


_Bool get_server_clock(void)
{
	//////////   SERVER CLOCK SERVICE ////////
	deactivate_pdp();

	at_cmd_send("AT+QHTTPCFG=\"contextid\",1\r\n\0","OK\r\n","ERROR",5,0);

	at_cmd_send("at+qhttpcfg=\"requestheader\",0\r\n\0","OK\r\n","ERROR",5,0);

	flush_tx_string();
	sprintf((char*)tx_string, "AT+QICSGP=1,1,\"%s\",\"\",\"\",1\r\n\0", apn);///////////SET APN
	at_cmd_send((char*)tx_string,"OK\r\n","ERROR",5,0);

	if(activate_pdp())
	{

		at_cmd_send("AT+QIACT?\r\n\0","OK\r\n","ERROR",5,0);


		if(at_cmd_send("at+qhttpurl=33,80\r\n\0","CONNECT\r\n","ERROR",70,0))
		{
			at_cmd_send("https://shark.carematix.com/cs/dt","OK\r\n","ERROR",70,0);

			at_cmd_send("at+qhttpurl?\r\n\0","OK\r\n","ERROR",5,0);

			if(at_cmd_send("at+qhttpget=80\r\n\0","QHTTPGET: 0","ERROR",70,0))
			{
				HAL_Delay(10);

				plus_enable = 0;

				if(at_cmd_send("AT+QHTTPREAD=80\r\n\0","QHTTPREAD: 0\r\n","ERROR",70,0))///+QHTTPREAD: 0
				{
					flush_array((char*)temp_data,100);
					uint8_t z=0,j=0;

					for(z=0; z < 50; z++)
					{
						if(gsm_data[z] == 'T')////CONNECT search T
							break;
					}

					for(z=z+3; z < 50; z++)
					{
						if(gsm_data[z] == 0x0D)
							break;

						temp_data[j++] = gsm_data[z];
					}

					plus_enable = 1;
					server_clock_update=1;

					send_text_to_usb("\r\nServer Clock: \0");
					send_text_to_usb((char*)temp_data);
					send_text_to_usb("\r\n\0");

					server_clock = atol((char*)temp_data);

					server_clock_update=0;

					flush_tx_string();
					sprintf((char*)tx_string, "\r\nServer Clock: %u\r\n\0", server_clock);
					send_text_to_usb((char*)tx_string);

					return 1;
				}

				plus_enable = 1;
			}
		}

	}

	return 0;
	////////// SERVER CLOCK SERVICE END ////////
}

_Bool get_ntp_clock(void)
{
	/* Code tested in online editor
			https://onlinegdb.com/d_Vrn2Fpx
	*/
	//////////   NTP CLOCK SERVICE ////////
	deactivate_pdp();

	flush_tx_string();
	sprintf((char*)tx_string, "AT+QICSGP=1,1,\"%s\",\"\",\"\",1\r\n\0", apn);///////////SET APN
	at_cmd_send((char*)tx_string,"OK\r\n","ERROR",5,0);

	if(activate_pdp())
	{
		HAL_Delay(300);

		plus_enable=1;

		//at+qntp=1,�0.us.pool.ntp.org�
		if(at_cmd_send("at+qntp=1,\"0.us.pool.ntp.org\"\r\n\0","QNTP:","ERROR",130,0))
		{
			plus_enable=0;
			HAL_Delay(100);
			plus_enable=1;

			if(strstr((char*)gsm_data, "QNTP: 0"))
			{

				unsigned char ntp_format[50]={0}, ntp_date[7]={0}, ntp_time[7]={0};

				explode_string(gsm_data, strlen((char*)gsm_data), 1, ',', '\r', 25, '\r');

				sprintf((char*)ntp_format, "%s\0", temp_data);

				ntp_date[0] = ntp_format[9];
				ntp_date[1] = ntp_format[10];
				ntp_date[2] = ntp_format[6];
				ntp_date[3] = ntp_format[7];
				ntp_date[4] = ntp_format[3];
				ntp_date[5] = ntp_format[4];


				ntp_time[0] = ntp_format[12];
				ntp_time[1] = ntp_format[13];
				ntp_time[2] = ntp_format[15];
				ntp_time[3] = ntp_format[16];
				ntp_time[4] = ntp_format[18];
				ntp_time[5] = ntp_format[19];

				server_clock_update = 1;
				server_clock = convert_to_epoch(ntp_date, ntp_time);
				server_clock_update = 0;

	//			send_text_to_usb("\r\nGSM Data: \0");
	//			send_text_to_usb((char*)gsm_data);
	//			send_text_to_usb("\r\n\0");

				send_text_to_usb("\r\nNTP Clock: \0");
				send_text_to_usb((char*)ntp_format);
				send_text_to_usb("\r\n\0");

				send_text_to_usb("\r\nEpoch Time: \0");
				sprintf((char*)temp_data, "%ld\0", (long) server_clock);
				send_text_to_usb((char*)temp_data);
				send_text_to_usb("\r\n\0");

				return 1;
			}
		}
	}

	return 0;
	////////// NTP CLOCK SERVICE END ////////
}



///////// JSON PARSER  /////////

///{"st":1,"ca":1260,"ck":1678,"cc":1479,"sc":398,"cv":"1","tk":"wCCgqF6gDTWC3qWg"}
//// occurance is added to get value in case of multiple occurance
_Bool json_get_value(char* parameter, uint8_t parameter_length, uint8_t occurance, char* variable, uint8_t variable_size)
{
	uint8_t parameter_index=0, variable_index=0, occurance_count=0;
	_Bool bracket_flag=0;

	for(uint16_t i=0; i <= json_count; i++)
	{
		if(json_response[i]==parameter[parameter_index])
		{
			if(parameter_index == parameter_length-1)////// PARAMETER FOUND CONDITION
			{
				occurance_count++;

				if(occurance_count==occurance)
				{
					for(uint8_t j=0; j<variable_size; j++)/////// FLUSH VARIABLE
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
//////// JSON PARSER END ////////


_Bool connect_mqtt_server(void)
{
	deactivate_pdp();

	flush_tx_string();
	sprintf((char*)tx_string, "AT+QICSGP=1,1,\"%s\",\"\",\"\",1\r\n\0", apn);///////////SET APN
	at_cmd_send((char*)tx_string,"OK\r\n","ERROR",5,0);

	if(activate_pdp())
	{
		at_cmd_send("AT+QIACT?\r\n\0","OK\r\n","ERROR",5,0);

		at_cmd_send("at+qmtcfg=\"SSL\",0,1,2\r\n\0","OK\r\n","ERROR",5,0);

		at_cmd_send("at+qmtcfg=\"version\",0,4\r\n\0","OK\r\n","ERROR",5,0);

		at_cmd_send("at+qsslcfg=\"seclevel\",2,2\r\n\0","OK\r\n","ERROR",5,0);

		at_cmd_send("at+qsslcfg=\"sslversion\",2,4\r\n\0","OK\r\n","ERROR",5,0);

		at_cmd_send("at+qsslcfg=\"ciphersuite\",2,0xFFFF\r\n\0","OK\r\n","ERROR",5,0);

		at_cmd_send("AT+QSSLCFG=\"cacert\",2,\"security/CaCert.crt\"\r\n\0","OK\r\n","ERROR",5,0);

		at_cmd_send("AT+QSSLCFG=\"clientcert\",2,\"security/Client.crt\"\r\n\0","OK\r\n","ERROR",5,0);

		at_cmd_send("AT+QSSLCFG=\"clientkey\",2,\"security/key.pem\"\r\n\0","OK\r\n","ERROR",5,0);

		flush_tx_string();
		sprintf((char*)tx_string, "at+qmtopen=0,\"%s\",%s\r\n\0", mqtt_server, mqtt_port);/////OPEN NETWORK FOR MQTT CLIENT
		if(at_cmd_send((char*)tx_string,"QMTOPEN: 0,0\r\n","ERROR",80,0))/// +QMTOPEN: 0,0
		{
			flush_tx_string();
			sprintf((char*)tx_string, "at+qmtconn=0,\"%s\",\"%s\",\"%s\"\r\n\0", mqtt_client_id, mqtt_username, mqtt_password);/////CONNECT CLIENT TO MQTT SERVER
			if(at_cmd_send((char*)tx_string,"QMTCONN: 0,0,0\r\n","ERROR",20,0))///+QMTCONN: 0,0,0
			{
				return 1;
			}
		}

	}

	return 0;
}

void disconnect_mqtt_connection(void)
{
	at_cmd_send("AT+QMTDISC=0\r\n\0","QMTDISC: 0,0\r\n","ERROR",20,0);//+QMTDISC: 0,0
	connected_to_mqtt_server_flag=0;
}

void power_down_bg96(void)
{
	#if BG96
	send_text_to_usb((char*)"\r\nTURNING OFF BG96\r\n\0");
	#elif BG95
	send_text_to_usb((char*)"\r\nTURNING OFF BG95\r\n\0");
	#endif

	at_cmd_send("AT+QPOWD\r\n\0","pdpdeact","ERROR",10,0);
}

void power_down_bg96_handler(void)
{
	if(bg96_power_flag && (mem_write_address==mem_read_address || mqtt_server_connection_fail_counter>=2) && !server_clock_flag && !ping_service_flag && !configuration_service_flag && !update_device_twin_flag && !ca_certificate_flag && !client_certificate_flag && !client_key_flag)
	{
		power_down_bg96();
		bg96_power_flag=0;
	}
}

void power_on_bg96(void)
{
	if(!at_cmd_send("AT\r\n\0","OK\r\n","ERROR",2,0))
	{
		#if BG96
		send_text_to_usb((char*)"\r\nTURNING ON BG96\r\n\0");
		#elif BG95
		send_text_to_usb((char*)"\r\nTURNING ON BG95\r\n\0");
		#endif


		//GSM  PWRKEY
		HAL_GPIO_WritePin(GSM_PWRKEY_GPIO_Port,GSM_PWRKEY_Pin,GPIO_PIN_SET);
		HAL_Delay(1500);
		HAL_GPIO_WritePin(GSM_PWRKEY_GPIO_Port,GSM_PWRKEY_Pin,GPIO_PIN_RESET);

		timer_cmd = 0;

		while(timer_cmd <= 30)
		{
			if(strstr((char*)gsm_data, (char*)"CPIN: NOT INSERTED"))
			{
				send_text_to_usb((char*)"\r\nSIM NOT INSERTED\r\n\0");
				break;
			}

			#if BG96
			if(strstr((char*)gsm_data, (char*)"SMS DONE"))
			{
				break;
			}
			#elif BG95
			if(strstr((char*)gsm_data, (char*)"APP RDY"))
			{
				break;
			}
			#endif

		}

		at_cmd_send("ATE0\r\n\0","OK\r\n","ERROR",2,0);
	}
}

void erase_all_records(void)
{
	sFLASH_EraseBulk(LOGS_START_ADDRESS);

	mem_write_address=LOGS_START_ADDRESS;
	mem_read_address=LOGS_START_ADDRESS;

	store_runtime_parameters_to_flash(RUNTIME_PARAMETERS_ADDRESS);
	store_runtime_parameters_to_flash(RUNTIME_PARAMETERS_BACKUP_ADDRESS);
}

_Bool get_gsm_signal_strength(void)
{
	if(at_cmd_send("AT+CSQ\r\n\0", "OK\r\n", "ERROR\r\n", 5, 0))
		{
			//+CSQ: 28,99

			unsigned char temp_signal[3]={0};
			uint8_t start = 0;
			for(; start<15; start++)
			{
				if(gsm_data[start]=='C')
				{
					if(gsm_data[start+2]=='Q')
					{
						temp_signal[0] = gsm_data[start+5];

						if(gsm_data[start+6]!=',')
						{
							temp_signal[1] = gsm_data[start+6];
						}

						gsm_signal_strength = atoi((char*)temp_signal);

						flush_tx_string();
						sprintf((char*)tx_string, "\r\nSignal Strength : %u\r\n\0", gsm_signal_strength);
						send_text_to_usb((char*)tx_string);

						return 1;
					}
				}
			}
		}

		return 0;
}

_Bool publish_meter_readings(void)
{
	///////////// PUBLISH READING DATA

	while(mem_write_address>mem_read_address)
	{


		flush_tx_string();

		sprintf((char*)tx_string, "at+qmtpub=0,1,1,0,\"%s\"\r\n\0", read_pb);/////PUBLISH TO READ_PB TOPIC

		if(at_cmd_send((char*)tx_string,">","ERROR",20,0))/// >
		{
			struct DataField serial_number, timestamp, model_number, meter_id, raw_data, signal_strength;
			unsigned char string1[100]={0}, string2[100]={0}, string3[100]={0}, string4[100]={0}, string5[100]={0}, string6[100]={0},
				packet_length[2]={0}, string7[100]={0};
			uint16_t total_length=0;

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
			uint32_t t = atol((char*)read_clock);
			unsigned char bytes[4]={0};
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

			if(atol((char*)read_meter_model)==923 || atol((char*)read_meter_model)==897)//// if accu check meter
			{
				sprintf((char*)model_number.dataValue, "262\0");
			}
			else
			{
				sprintf((char*)model_number.dataValue, "%s\0", read_meter_model);/// if other meter
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
			sprintf((char*)meter_id.dataValue, "%s\0", read_meter_serial_no);
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
			uint8_t data_byte=0;
			for(uint8_t zz=0; zz<17; zz++)
			{
				data_byte = sFLASH_ReadByte1(mem_read_address++);////// read data from flash

				sprintf((char*)(raw_data.dataValue+zz*2), "%02X", data_byte);

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
			flush_tx_string();
			sprintf((char*)tx_string, "%s%s%s%s%s%s%sZ'}\r\n%c\0", string1, string2, string3, string4, string5, string6, string7, 0x1A);


//											////////////////////////////// For sending raw data in Hex   lenght 17 bytes
//											for(uint8_t zz=0; zz<17; zz++)
//											{
//												meter_records[cc][zz] = sFLASH_ReadByte1(mem_read_address++);
////												CDC_Transmit_FS(&meter_records[cc][zz], 1);
//												HAL_UART_Transmit_IT(&huart2, &meter_records[cc][zz], 1);
//												HAL_Delay(1);
//
//											}


			if(at_cmd_send((char*)tx_string,"QMTPUB: 0,1,0\r\n","ERROR",30,0))///+QMTPUB: 0,1,0
			{
				store_runtime_parameters_to_flash(RUNTIME_PARAMETERS_ADDRESS);
				store_runtime_parameters_to_flash(RUNTIME_PARAMETERS_BACKUP_ADDRESS);
				beep(1);
//												HAL_Delay(1000);
			}
			else
			{
				return 0;
			}
		}
		else
		{
			return 0;
		}

		////Debug TTL
		flush_tx_string();
		sprintf((char*)tx_string, "\r\nWA:%u    Rem:%u    RA:%u    Rem:%u\r\n\0", mem_write_address, (mem_write_address-LOGS_START_ADDRESS)%17,
		mem_read_address, (mem_read_address-LOGS_START_ADDRESS)%17);
		send_text_to_usb((char*)tx_string);


		if(mem_write_address==mem_read_address)
		{
			erase_all_records();

			beep(20);
			HAL_Delay(100);
			beep(30);
			return 1;
		}

	}

	return 0;
}

_Bool read_device_twin(void)
{
	flush_tx_string();
	sprintf((char*)tx_string, "at+qmtsub=0,1,\"%s\",0\r\n\0", twin_sb);/////SUBSCRIBE TO TWIN_SB TOPIC
	if(at_cmd_send((char*)tx_string,"QMTSUB: 0,1,0,0\r\n","ERROR",20,0))/// +QMTSUB: 0,1,0,0
	{
		flush_tx_string();
		sprintf((char*)tx_string, "at+qmtpub=0,1,1,0,\"%s\"\r\n\0", twin_pb);/////PUBLISH TO TWIN_PB TOPIC
		if(at_cmd_send((char*)tx_string,">","ERROR",20,0))/// >
		{
			HAL_UART_Transmit_IT(&huart2, 0x00, 1);/////// SEND NULL
			HAL_Delay(2);
			send_text_to_uart2((uint8_t*)"\r\n\0");/////// PRESS ENTER
			flush_tx_string();
			sprintf((char*)tx_string, "%c\0", 0x1A);/////// CTRL+Z

			flush_array((char*)json_response, 600);

			if(at_cmd_send((char*)tx_string,"QMTPUB: 0,1,0\r\n","ERROR",20,0))///+QMTPUB: 0,1,0
			{
				json_response_flag = 1;

				timer_cmd=0;

				while(timer_cmd < 20)
				{
					if(strstr((char*)gsm_data, (char*)"QMTRECV:"))
					{
						HAL_Delay(100);
						json_response_flag = 0;

						/*
							+QMTRECV: 0,0,"$iothub/twin/res/200/?$rid=1","{
								"desired": {
									"sd": "S2001C052080000000001620110Z",
									"cv": "4",
									"$version": 137
								},
								"reported": {
									"fv": "1.0",
									"pv": "1.0",
									"cv": "4",
									"imei": "864475049752735",
									"imsi": "404882990776241",
									"ccid": "89910271001167472368",
									"mv": "BG95M3LAR02A03_01.009.01.009",
									"sim": "89910273519000076474",
									"$version": 296
								}
							}"
						*/

						uint16_t desired_cv=0, reported_cv=0;

						send_text_to_usb("\r\nTwin Response: \0");
						send_text_to_usb((char*)json_response);
						send_text_to_usb("\r\n\0");

						if(json_get_value("sd", 2, 1, (char*)temp_data, 100))/////// GET SETUP DATA VALUE FROM JSON RESPONSE
						{
							send_text_to_usb("\r\nSetup Data: \0");
							send_text_to_usb((char*)temp_data);
							send_text_to_usb("\r\n\0");
						}

						if(json_get_value("cv", 2, 1, (char*)temp_data, 100))/////// GET SETUP DATA VALUE FROM JSON RESPONSE
						{
							send_text_to_usb("\r\nDesired Configuration Version: \0");
							send_text_to_usb((char*)temp_data);
							send_text_to_usb("\r\n\0");
						}

						desired_cv = atol((char*)temp_data);

						if(json_get_value("cv", 2, 2, (char*)temp_data, 100))/////// GET SETUP DATA VALUE FROM JSON RESPONSE
						{
							send_text_to_usb("\r\nReported Configuration Version: \0");
							send_text_to_usb((char*)temp_data);
							send_text_to_usb("\r\n\0");
						}

						reported_cv = atol((char*)temp_data);

						if(desired_cv==reported_cv)
						{
							send_text_to_usb("\r\nDesired Reported CV Matched\r\n\0");
							desired_reported_cv_matched_flag=1;
						}
						else
						{
							send_text_to_usb("\r\nDesired Reported CV NOT Matched\r\n\0");
							desired_reported_cv_matched_flag=0;
						}


						return 1;
					}
				}

				json_response_flag = 0;

			}
		}
	}

	return 0;
}

_Bool update_device_twin(void)
{
	flush_tx_string();
	sprintf((char*)tx_string, "at+qmtpub=0,1,1,0,\"%s\"\r\n\0", twin_rp);/////PUBLISH TO TWIN_RP TOPIC
	if(at_cmd_send((char*)tx_string,">","ERROR",20,0))/// >
	{
		flush_tx_string();
		sprintf((char*)tx_string, "{\"fv\":\"%s\",\"pv\":\"%s\",\"cv\":\"%s\",\"mv\":\"%s\",\"sim\":\"%s\",\"imei\":\"%s\",\"imsi\":\"%s\"}\r\n%c\0",
			firmware_version, protocol_version, configuration_version, module_firmware_version, ccid, imei, imsi, 0x1A);/////PUBLISH TO TWIN_RP TOPIC
		if(at_cmd_send((char*)tx_string,"QMTPUB: 0,1,0","ERROR",20,0))///+QMTPUB: 0,1,0
		{
			HAL_Delay(1000);

			send_text_to_usb("\r\nPublish Data: \0");
			send_text_to_usb((char*)tx_string);
			send_text_to_usb("\r\n\0");

			return 1;
		}
	}

	return 0;
}

void server_clock_handler(void)
{
	if(server_clock_flag)
	{
		power_on_bg96();

		while(1)
		{
			wifi_command_handler();

			check_gprs_network_registration();

			if(registered_to_gprs_network)
			{
				if(get_ntp_clock())//// get clock from NTP server
				{
					server_clock_flag=0;
					break;
				}
				else
				{
					if(get_server_clock())///// if NTP clock request failed then get clock from carematix server
					{
						server_clock_flag=0;
						break;
					}
				}

			}
			else
				HAL_Delay(10000);
		}
	}
}

void update_module_firmware_handler(void)
{
	if(update_module_firmware_flag)
	{
		power_on_bg96();

		while(1)
		{
			wifi_command_handler();

			check_gprs_network_registration();

			if(registered_to_gprs_network && update_module_firmware())
			{
				update_module_firmware_flag=0;
				send_text_to_usb("\r\nModule Firmware Update Success\r\n\0");

				get_module_firmware_version();

				// UPDATE CV IF ALL CHANGES DONE
				update_cv_value();

				break;
			}
			else if(registered_to_gprs_network)
			{
				send_text_to_usb("\r\nModule Firmware Update Fail\r\n\0");
				break;
			}
			else
				HAL_Delay(10000);
		}
	}
}


void update_device_firmware_handler(void)
{
	if(update_device_firmware_flag)
	{
		power_on_bg96();

		while(1)
		{
			wifi_command_handler();

			check_gprs_network_registration();

			if(registered_to_gprs_network && update_device_firmware())
			{
				update_device_firmware_flag=0;
				send_text_to_usb("\r\nDevice Firmware Update Success\r\n\0");

				break;
			}
			else if(registered_to_gprs_network)
			{
				send_text_to_usb("\r\nDevice Firmware Update Fail\r\n\0");
				update_device_firmware_flag = 0;
				break;
			}
			else
				HAL_Delay(10000);
		}
	}
}


void ping_service_handler(void)
{
	if(ping_service_flag)
	{
		power_on_bg96();

		while(1)
		{
			wifi_command_handler();

			check_gprs_network_registration();

			if(registered_to_gprs_network)
			{
				if(ping_service())
				{
					ping_service_flag=0;
					break;
				}
				else
				{
					/*
					IF PING RESPONSE 0 THEN POWER OFF BG95,
					TURN ON RED LED,
					STOP FIRMWARE,
					KEEP WIFI ACTIVE FOR 5 MIN,
					WIFI COMMANDS WILL WORK
					*/

					power_down_bg96();
					enable_green_led_handler=0;
					enable_red_led_handler=0;
					enable_yellow_led_handler=0;
					led_green_off;
					led_yellow_off;
					led_red_on;

					while(1)
					{
						wifi_command_handler();
						esp_disable_softap_mode_handler();
					}
				}
			}

			HAL_Delay(10000);
		}
	}
}

void configuration_service_handler(void)
{
	if(configuration_service_flag)
	{
		power_on_bg96();

		while(1)
		{
			wifi_command_handler();

			check_gprs_network_registration();

			if(registered_to_gprs_network && configuration_service())
			{
				configuration_service_flag=0;
				store_settings_to_flash(DEVICE_SETTINGS_ADDRESS);
				store_settings_to_flash(DEVICE_SETTINGS_BACKUP_ADDRESS);
				read_settings_from_flash(DEVICE_SETTINGS_ADDRESS);

				// UPDATE CV IF ALL CHANGES DONE
				update_cv_value();
				break;
			}
			else
				HAL_Delay(10000);
		}
	}
}

void ca_certificate_handler(void)
{
	if(ca_certificate_flag)
	{
		power_on_bg96();

		while(1)
		{
			wifi_command_handler();

			check_gprs_network_registration();

			if(registered_to_gprs_network && get_root_ca_certificate())
			{
				ca_certificate_flag = 0;
				send_text_to_usb("\r\nCA Certificate Updated\r\n\0");

				// UPDATE CV IF ALL CHANGES DONE
				update_cv_value();
				break;
			}
			else
				HAL_Delay(10000);
		}
	}
}

void client_certificate_handler(void)
{
	if(client_certificate_flag)
	{
		power_on_bg96();

		while(1)
		{
			wifi_command_handler();

			check_gprs_network_registration();

			if(registered_to_gprs_network && get_client_certificate())
			{
				client_certificate_flag = 0;
				send_text_to_usb("\r\nClient Certificate Updated\r\n\0");

				// UPDATE CV IF ALL CHANGES DONE
				update_cv_value();
				break;
			}
			else
				HAL_Delay(10000);
		}
	}
}

void client_key_handler(void)
{
	if(client_key_flag)
	{
		power_on_bg96();

		while(1)
		{
			wifi_command_handler();

			check_gprs_network_registration();

			if(registered_to_gprs_network && get_client_key())
			{
				client_key_flag = 0;
				send_text_to_usb("\r\nClient Key Updated\r\n\0");

				// UPDATE CV IF ALL CHANGES DONE
				update_cv_value();
				break;
			}
			else
				HAL_Delay(10000);
		}
	}
}

void device_twin_handler(void)
{
	if(update_device_twin_flag)
	{
		power_on_bg96();

		while(1)
		{
			wifi_command_handler();

			check_gprs_network_registration();

			if(registered_to_gprs_network && connect_mqtt_server())
			{
				send_text_to_usb("\r\nConnected to MQTT Server\r\n\0");

				connected_to_mqtt_server_flag = 1;

				if(read_device_twin())
				{
					if(update_device_twin())
					{
						update_device_twin_flag = 0;
						get_gsm_signal_strength();

						if(publish_meter_readings())
						{

						}

						disconnect_mqtt_connection();
						break;
					}
				}

				disconnect_mqtt_connection();
			}
			else
				HAL_Delay(10000);
		}
	}
}

void publish_meter_readings_handler(void)
{
	if(mem_write_address>mem_read_address && !ping_service_flag && mqtt_server_connection_fail_counter<2)
	{
		power_on_bg96();
		sending_data_to_server_flag=1;

		while(1)
		{
			// ICD 2 flow for data transmission
			if(bootup_complete_flag)
			{
				server_clock_flag = 1;
				server_clock_handler();
			}

			check_gprs_network_registration();

			if(registered_to_gprs_network && connect_mqtt_server())
			{
				mqtt_server_connection_fail_counter=0;

				send_text_to_usb("\r\nConnected to MQTT Server\r\n\0");

				connected_to_mqtt_server_flag = 1;
				get_gsm_signal_strength();

				// ICD 2 flow for data transmission
				if(bootup_complete_flag)
				{
					read_device_twin();//// READ TWIN

					if(desired_reported_cv_matched_flag)///// IF DESIRED AND REPORTED CV IS SAME THEN DO NOT UPDATE TWIN
					{
						desired_reported_cv_matched_flag=0;
					}
					else///// IF DESIRED AND REPORTED CV IS NOT SAME THEN CALL PING SERVICE
					{
						ping_service_flag=1;
						disconnect_mqtt_connection();
						break;
					}

				}

				if(publish_meter_readings())
				{

//					if(bootup_complete_flag)
//					{
//						update_device_twin();
//					}

					disconnect_mqtt_connection();
//					server_clock_flag=1;
					sending_data_to_server_flag=0;
					break;
				}
				else//30.10.2020
				{
					sending_data_to_server_flag=0;
					disconnect_mqtt_connection();
					break;
				}
			}
			else if(registered_to_gprs_network)//mqtt connection failure condition
			{
				mqtt_server_connection_fail_counter++;

				if(mqtt_server_connection_fail_counter==2)
				{
					sending_data_to_server_flag=0;
					break;
				}

				if(mqtt_server_connection_fail_counter==1)
				{
					ping_service_flag=1;
					sending_data_to_server_flag=0;
					break;
				}
			}
			else
				HAL_Delay(10000);
		}
	}
}


void store_clock_to_flash(void)
{
	unsigned char server_clock_format[18]={0};

	if(server_clock>1000000000)
	{
		sprintf((char*)server_clock_format, "#clock%u*\0", server_clock);
//		send_text_to_uart2(server_clock_format);// Debug TTL

		for(uint8_t i=0; i<17; i++)
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

void read_clock_from_flash(void)
{
	unsigned char server_clock_format[18]={0};

	server_clock_format[0] = sFLASH_ReadByte1(mem_read_address);

	if(server_clock_format[0]=='#')//////////////////// read clock
	{
		for(uint8_t i=1; i<17; i++)
		{
			server_clock_format[i] = sFLASH_ReadByte1(mem_read_address+i);
		}

		if(server_clock_format[1]=='c' && server_clock_format[2]=='l' && server_clock_format[3]=='o' && server_clock_format[4]=='c' && server_clock_format[5]=='k' && server_clock_format[16]=='*')
		{
			flush_array((char*)read_clock, 11);

			for(uint8_t i=0; i<10; i++)
			{
				read_clock[i] = server_clock_format[i+6];
			}

			mem_read_address=mem_read_address+17;///// total 17 bytes string
			read_clock_flag=1;///////// flag set once will never reset

			server_clock_format[0] = sFLASH_ReadByte1(mem_read_address);

			if(server_clock_format[0]=='#')////////////// read meter serial no.
			{
				for(uint8_t i=1; i<17; i++)
				{
					server_clock_format[i] = sFLASH_ReadByte1(mem_read_address+i);
				}

				if(server_clock_format[1]=='m' && server_clock_format[2]=='s' && server_clock_format[3]=='r' && server_clock_format[4]=='l' && server_clock_format[16]=='*')
				{
					flush_array((char*)read_meter_serial_no, 20);

					for(uint8_t i=0; i<11; i++)
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


	///// Read clock data if clock not read even once and current address is not clock data
	if(!read_clock_flag)
	{
		_Bool clockDataFoundFlag = 0;
		uint32_t addr = 0;

		/// search for clock data
		for(addr = mem_read_address; addr>=LOGS_START_ADDRESS; addr=addr-17)
		{
			server_clock_format[0] = sFLASH_ReadByte1(addr);

			if(server_clock_format[0]=='#')//// search for hash #
			{
				server_clock_format[0] = sFLASH_ReadByte1(addr+1);

				if(server_clock_format[0]=='c')////// search for c
				{
					clockDataFoundFlag = 1;
					break;
				}
			}

		}


		/// read clock data and meter serial
		if(clockDataFoundFlag)
		{
			uint8_t i=0;
			server_clock_format[0] = sFLASH_ReadByte1(addr);

			if(server_clock_format[0]=='#')//////////////////// read clock
			{
				for(i=1; i<17; i++)
				{
					server_clock_format[i] = sFLASH_ReadByte1(addr+i);
				}

				if(server_clock_format[1]=='c' && server_clock_format[2]=='l' && server_clock_format[3]=='o' && server_clock_format[4]=='c' && server_clock_format[5]=='k' && server_clock_format[16]=='*')
				{
					addr = addr+i;//// addr incremented to check meter serial number

					flush_array((char*)read_clock, 11);

					for(i=0; i<10; i++)
					{
						read_clock[i] = server_clock_format[i+6];
					}

					read_clock_flag=1;///////// flag set once will never reset

					server_clock_format[0] = sFLASH_ReadByte1(addr);

					if(server_clock_format[0]=='#')////////////// read meter serial no.
					{
						for(i=1; i<17; i++)
						{
							server_clock_format[i] = sFLASH_ReadByte1(addr+i);
						}

						if(server_clock_format[1]=='m' && server_clock_format[2]=='s' && server_clock_format[3]=='r' && server_clock_format[4]=='l' && server_clock_format[16]=='*')
						{
							flush_array((char*)read_meter_serial_no, 20);

							for(i=0; i<11; i++)
							{
								read_meter_serial_no[i] = server_clock_format[i+5];
							}

							read_meter_model[0] = read_meter_serial_no[0];
							read_meter_model[1] = read_meter_serial_no[1];
							read_meter_model[2] = read_meter_serial_no[2];

						}
					}
				}
			}


		}//if(clockDataFoundFlag)

	}///if(!read_clock_flag)


}

void green_led_handler(void)
{
	if(green_led_blink_flag)
	{
		if(green_led_timer==0)
		{
			if(green_led_on_flag)
			{
				led_green_on;
				green_led_on_flag=0;
				green_led_timer=100;
			}
			else
			{
				led_green_off;
				green_led_on_flag=1;
				green_led_timer=200;
			}
		}
	}
	else
	{
		if(server_clock > 1000000000 && mem_write_address==mem_read_address)
		{
			led_green_on;
		}
		else
		{
			led_green_off;
		}
	}
}

void red_led_handler(void)
{
	if(server_clock > 1000000000)
	{
		led_red_off;
	}
	else
	{
		led_red_on;
	}
}

///yellow led
void yellow_led_handler(void)
{
	if(sending_data_to_server_flag)
	{
		if(led3_on_flag)
		{
			led3_on_flag=0;
			led_yellow_off;
		}
		else
		{
			led3_on_flag=1;
			led_yellow_on;
		}
	}
	else
	{
		if(mem_write_address>mem_read_address)
		{
			led_yellow_on;
		}
		else
		{
			led_yellow_off;
		}
	}
}

_Bool get_module_firmware_version(void)
{
	if(at_cmd_send("AT+QGMR\r\n\0","OK\r\n","ERROR",5,0))
	{
		//\r\nBG96MAR02A07M1G_01.014.01.014\r\nOK\r\n
		explode_string(gsm_data, 300, 1, '\n', '\r', 29, 'O');

		if(temp_data[0]!=0)
		{
			flush_array((char*)module_firmware_version, 40);
			sprintf((char*)module_firmware_version, "%s\0", temp_data);

			flush_tx_string();
			sprintf((char*)tx_string, "\r\nModule Firmware : %s\r\n\0", module_firmware_version);
			send_text_to_usb((char*)tx_string);

			return 1;
		}
	}

	return 0;
}


void wifi_command_handler(void)
{
	if(new_apn_flag)
		{
			new_apn_flag=0;

			store_settings_to_flash(DEVICE_SETTINGS_ADDRESS);
			store_settings_to_flash(DEVICE_SETTINGS_BACKUP_ADDRESS);

			flush_tx_string();
			sprintf((char*)tx_string, "AT+QICSGP=1,1,\"%s\",\"\",\"\",1\r\n\0", apn);///////////SET APN
			at_cmd_send((char*)tx_string,"OK\r\n","ERROR",5,0);

			flush_tx_string();
			sprintf((char*)tx_string, "\r\nNew APN : %s\r\n\0", apn);
			send_softap_response((char*)tx_string, strlen((char*)tx_string));
			send_text_to_usb((char*)tx_string);
		}
		else if(new_device_id_flag)
		{
			new_device_id_flag=0;

			store_settings_to_flash(DEVICE_SETTINGS_ADDRESS);
			store_settings_to_flash(DEVICE_SETTINGS_BACKUP_ADDRESS);

			flush_tx_string();
			sprintf((char*)tx_string, "\r\nNew Serial Number : %s\r\n\0", device_id);
			send_softap_response((char*)tx_string, strlen((char*)tx_string));
			send_text_to_usb((char*)tx_string);
		}
}

void esp_disable_softap_mode_handler(void)
{
	if(softap_mode_timeup_flag)
	{
		softap_mode_timeup_flag=0;
		at_cmd_send_esp("AT+CWMODE=0\r\n\0","OK\r\n","ERROR",5,0);
	}
}

//// MAIN PURPOSE IS TO UPDATE CV ONLY IF ALL CHANGES DONE IN CONFIGURATION (AS PER PING RESPONSE)
void update_cv_value(void)
{
	if(!configuration_service_flag && !ca_certificate_flag && !client_certificate_flag && !client_key_flag && !update_device_firmware_flag && !update_module_firmware_flag)
	{
		sprintf((char*)configuration_version, "%s\0", (char*)hold_cv_value);
		store_settings_to_flash(DEVICE_SETTINGS_ADDRESS);
		store_settings_to_flash(DEVICE_SETTINGS_BACKUP_ADDRESS);
		read_settings_from_flash(DEVICE_SETTINGS_ADDRESS);
	}
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
	Be_SecondFW();
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

//	HAL_Delay(10000);

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_RTC_Init();
  MX_SPI1_Init();
  MX_USB_DEVICE_Init();
  MX_TIM6_Init();
  MX_TIM7_Init();
  MX_TIM16_Init();
  /* USER CODE BEGIN 2 */

	HAL_UART_Receive_IT(&huart1, &rx_esp, 1);
	HAL_UART_Receive_IT(&huart2, &gsm, 1);

	HAL_TIM_Base_Start_IT(&htim6);
	HAL_TIM_Base_Start_IT(&htim7);



	GSM_RESET_LOW;//// NORMAL FUNCTION
	GSM_VCC_ON;///// GSM VCC ENABLED 4.2V SUPPLY ENABLE FOR LEDs

	enable_green_led_handler=0;
	enable_red_led_handler=0;
	enable_yellow_led_handler=0;

	led_green_on;
	led_red_on;
	led_yellow_on;

	HAL_Delay(2000);

	led_green_off;
	led_red_off;
	led_yellow_off;

	HAL_Delay(2000);

	enable_green_led_handler=1;
	enable_red_led_handler=1;
	enable_yellow_led_handler=1;

	#if USA
	sprintf((char*)tx_string, "\r\nBlipGo %s  USA\r\n\0", firmware_version);
	send_text_to_usb((char*)tx_string);
	#elif INDIA
	sprintf((char*)tx_string, "\r\nBlipGo %s  INDIA\r\n\0", firmware_version);
	send_text_to_usb((char*)tx_string);
	#endif


//	HAL_TIM_PWM_Start(&htim16, TIM_CHANNEL_1);//// PWM Buzzer ON
//	htim16.Instance->CCR1 = 500;/// 50% duty cycle

//	while(1);

	beep(100);/// PWM Beep

	//BUZZOR ON
	HAL_GPIO_WritePin(BUZZOR_GPIO_Port,BUZZOR_Pin,GPIO_PIN_SET);

	HAL_Delay(1);

	//BUZZOR OFF
	HAL_GPIO_WritePin(BUZZOR_GPIO_Port,BUZZOR_Pin,GPIO_PIN_RESET);


	send_text_to_usb("\r\nESP Turning ON\r\n\0");


	esp_power_on();


	#if BG96
	send_text_to_usb("\r\nBG96 Turning ON\r\n\0");
	#elif BG95
	send_text_to_usb("\r\nBG95 Turning ON\r\n\0");
	#endif

	power_on_bg96();


	sFLASH_Init();


	#if ERASE_FLASH
	send_text_to_usb("\r\nErasing Flash...\r\n\0");
	send_text_to_uart2((uint8_t*)"\r\nErasing Flash...\r\n\0");
	sFLASH_EraseBulk(0);
	send_text_to_usb("\r\nFlash Erased !\r\n\0");
	send_text_to_uart2((uint8_t*)"\r\nFlash Erased !\r\n\0");

	while(1);
	#endif

	flash_data = sFLASH_ReadByte1(0);

	if(flash_data == 0xff || flash_data == '#')
		send_text_to_usb("\r\nMEMORY OK\r\n\0");
	else
		send_text_to_usb("\r\nMEMORY ERROR\r\n\0");


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


//	timer_cmd = 0;
//
//	while(timer_cmd <= 30)
//	{
//		if(strstr((char*)gsm_data, (char*)"CPIN: NOT INSERTED"))
//		{
//			send_text_to_usb((char*)"\r\nSIM NOT INSERTED\r\n\0");
//			break;
//		}
//
//		#if BG96
//		if(strstr((char*)gsm_data, (char*)"SMS DONE"))
//		{
//			break;
//		}
//		#elif BG95
//		if(strstr((char*)gsm_data, (char*)"APP RDY"))
//		{
//			break;
//		}
//		#endif
//	}


	HAL_Delay(1000);

	power_on_bg96();


	send_text_to_uart2("ATE0\r\n\0");
	send_text_to_uart1("ATE0\r\n\0");
	HAL_Delay(300);

	if(at_cmd_send_esp("AT\r\n\0","OK\r\n","ERROR",5,0))
	{
		send_text_to_usb("\r\nESP Communication OK\r\n\0");
		send_text_to_uart1((uint8_t*)"\r\nESP Communication OK\r\n\0");
	}
	else
	{
		send_text_to_usb("\r\nESP Communication ERROR\r\n\0");
		send_text_to_uart1((uint8_t*)"\r\nESP Communication ERROR\r\n\0");
	}

	#if BG96
	if(at_cmd_send("AT\r\n\0","OK\r\n","ERROR",5,0))
		send_text_to_usb("\r\nBG96 Communication OK\r\n\0");
	else
		send_text_to_usb("\r\nBG96 Communication ERROR\r\n\0");
	#elif BG95
	if(at_cmd_send("AT\r\n\0","OK\r\n","ERROR",5,0))
		send_text_to_usb("\r\nBG95 Communication OK\r\n\0");
	else
		send_text_to_usb("\r\nBG95 Communication ERROR\r\n\0");
	#endif

//	flush_tx_string();
//	sprintf((char*)tx_string, "BG96 response: %s\r\n\0", gsm_data);
//	send_text_to_usb((char*)tx_string);


	at_cmd_send("AT+CFUN=1\r\n\0","OK\r\n","ERROR",10,0);/////////////FULL FUNCTIONALITY MODE

	flush_tx_string();
	sprintf((char*)tx_string, "AT+QICSGP=1,1,\"%s\",\"\",\"\",1\r\n\0", apn);///////////SET APN
	at_cmd_send((char*)tx_string,"OK\r\n","ERROR",5,0);

	at_cmd_send_esp("AT+CWMODE=3\r\n\0","OK\r\n","ERROR",5,0);
	esp_enable_softap_mode();

	send_text_to_uart2("AT+QURCCFG=\"urcport\",\"uart1\"\r\n\0");
	HAL_Delay(300);



	#if INDIA
	//for 2G only
	at_cmd_send("AT+QCFG=\"nwscanseq\",010203\r\n\0","OK\r\n","ERROR",5,0);
	at_cmd_send("AT+QCFG=\"nwscanmode\",1,1\r\n\0","OK\r\n","ERROR",5,0);
	at_cmd_send("AT+QCFG=\"iotopmode\",2\r\n\0","OK\r\n","ERROR",5,0);
	#elif USA
	// for usa
	//AT+QCFG="nwscanmode",0,1
	//at+qcfg="nwscanseq",020103,1
	//(eMTC -> GSM -> NB-IoT)

	at_cmd_send("AT+QCFG=\"nwscanseq\"\r\n\0","OK\r\n","ERROR",5,0);

//	at_cmd_send("AT+QCFG=\"nwscanmode\",0\r\n\0","OK\r\n","ERROR",5,0);
//	at_cmd_send("AT+QCFG=\"iotopmode\",2\r\n\0","OK\r\n","ERROR",5,0);
	#endif



	at_cmd_send("AT+CREG=0\r\n\0","OK\r\n","ERROR",5,0);

	at_cmd_send("AT+CGREG=0\r\n\0","OK\r\n","ERROR",5,0);

	at_cmd_send("AT+GSN\r\n\0","OK\r\n","ERROR",5,0);

	for(uint8_t i=0; i<15; i++)//\r\n868446033000023\r\n
	{
		imei[i] = gsm_data[i+2];
	}

	flush_tx_string();
	sprintf((char*)tx_string, "\r\nIMEI : %s\r\n\0", imei);
	send_text_to_usb((char*)tx_string);


	at_cmd_send("AT+CIMI\r\n\0","OK\r\n","ERROR",5,0);
	for(uint8_t i=0; i<15; i++)
	{
		imsi[i] = gsm_data[i+2];
	}

	flush_tx_string();
	sprintf((char*)tx_string, "\r\nIMSI : %s\r\n\0", imsi);
	send_text_to_usb((char*)tx_string);


	at_cmd_send("AT+QCCID\r\n\0","OK\r\n","ERROR",5,0);
	for(uint8_t i=0; i<20; i++)
	{
		ccid[i] = gsm_data[i+8];
	}

	flush_tx_string();
	sprintf((char*)tx_string, "\r\nCCID : %s\r\n\0", ccid);
	send_text_to_usb((char*)tx_string);


	#if USA
	// for usa
	//AT+QCFG="nwscanmode",0,1
	//at+qcfg="nwscanseq",020103,1
	//(eMTC -> GSM -> NB-IoT)

//	at_cmd_send("AT+QCFG=\"nwscanseq\"\r\n\0","OK\r\n","ERROR",5,0);

	at_cmd_send("AT+QCFG=\"nwscanmode\",0\r\n\0","OK\r\n","ERROR",5,0);
//	at_cmd_send("AT+QCFG=\"iotopmode\",2\r\n\0","OK\r\n","ERROR",5,0);
	#endif


//	at_cmd_send("AT+CSQ\r\n\0","+CSQ: ","ERROR",5,0);
//
//	HAL_Delay(50);
//
//	signal_strength[0] = gsm_data[6];
//
//	if(gsm_data[7]!=',')
//	{
//		signal_strength[1] = gsm_data[7];
//	}
//
////	for(uint8_t i=0; i<2; i++)//+CSQ: 28,99
////	{
////		signal_strength[i] = gsm_data[i+6];
////	}
//
//	flush_tx_string();
//	sprintf((char*)tx_string, "\r\nSignal Strength : %s\r\n\0", signal_strength);
//	send_text_to_usb((char*)tx_string);


	get_module_firmware_version();


	get_gsm_signal_strength();


	at_cmd_send("AT+COPS?\r\n\0","OK\r\n","ERROR",5,0);

	at_cmd_send("AT&W\r\n\0","OK\r\n","ERROR",5,0);
	HAL_Delay(500);

	////////firmware update bg96

//	at_cmd_send("AT+QFOTADL=\"https://bg96dfota.s3-us-west-2.amazonaws.com/01.014.01.014-11.018.11.018.bin\"\r\n","HTTP", "ERROR", 10,0);
//	while(1);



//	//// RESTART GSM MODULE
//	///// POWER OFF
//	HAL_GPIO_WritePin(GSM_PWRKEY_GPIO_Port,GSM_PWRKEY_Pin,GPIO_PIN_SET);
//	HAL_Delay(700);
//	HAL_GPIO_WritePin(GSM_PWRKEY_GPIO_Port,GSM_PWRKEY_Pin,GPIO_PIN_RESET);
//	HAL_Delay(5000);
//	///// POWER ON
//	HAL_GPIO_WritePin(GSM_PWRKEY_GPIO_Port,GSM_PWRKEY_Pin,GPIO_PIN_SET);
//	HAL_Delay(550);
//	HAL_GPIO_WritePin(GSM_PWRKEY_GPIO_Port,GSM_PWRKEY_Pin,GPIO_PIN_RESET);
//	HAL_Delay(5000);

//	send_text_to_usb("Usb test Data blipgo\r\n\0");


//	erase_all_records();

	HAL_Delay(500);
	server_clock_flag=1;
	ping_service_flag=1;


	server_clock_handler();

	ping_service_handler();

	configuration_service_handler();

	ca_certificate_handler();

	client_certificate_handler();

	client_key_handler();

	update_module_firmware_handler();

	update_device_firmware_handler();

	device_twin_handler();

	power_down_bg96_handler();



//	while(1){
////	if(!ftp_download("ftp.i3indyatechnologies.com","gpsftp@i3indyatechnologies.com","Gpsfota@123", 21, "blipgo.bin"))
////	if(!ftp_download("ftp.i3indyatechnologies.com","testftp@i3indyatechnologies.com","testftp!@#", 21, "blipgo.bin"))
////	if(!ftp_download("ftp.quectel.com","test","test", 21, "blipgo.bin"))
//	if(!ftp_download("act.gpsbox.co.in","ftp_user","A#$%876()*&h", 21, "blipgo.bin"))
//	{
//		HAL_Delay(1000);
//	}
//  }

	esp_enable_softap_mode();

	HAL_Delay(1000);

	sprintf((char*)ble_ssid_name, "BlipGo+%s\0", device_id);

	ble_initialize_gatt_client((char*)ble_ssid_name);

	ble_get_paired_device();

	send_text_to_uart1("ATE0\r\n\0");


	beep(20);
	HAL_Delay(100);
	beep(30);

	softap_mode_timer=0;

	bootup_complete_flag = 1;


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

//		send_text_to_uart1("AT\r\n\0");
//
//		send_text_to_uart2("AT\r\n\0");
//
//		flash_data = sFLASH_ReadByte1(0);
//
//		flush_array((char*)tx_string, strlen((char*)tx_string));
//		sprintf((char*)tx_string, "\r\nbyte: %x\r\n\0", flash_data);
//		send_text_to_uart1(tx_string);
//
//		if(flash_data == 'A')
//			send_text_to_uart1("MEMORY OK\r\n\0");
//		else
//			send_text_to_uart1("MEMORY ERROR\r\n\0");
//
//		data_buf[0] = sFLASH_ReadByte1(1);
//		data_buf[1] = sFLASH_ReadByte1(2);
//		data_buf[2] = sFLASH_ReadByte1(3);
//		data_buf[3] = sFLASH_ReadByte1(4);
//		data_buf[4] = sFLASH_ReadByte1(5);
//
//		send_text_to_uart1("Data:\0");
//		send_text_to_uart1(data_buf);
//		send_text_to_uart1("\r\n\0");

//		check_gprs_network_registration();

		power_down_bg96_handler();

		wifi_command_handler();



		if(connect_wifi_network_flag)
		{
//			restart_esp();

			HAL_Delay(1000);
//			at_cmd_send_esp("AT+CWMODE=1\r\n\0", "OK\r\n", "ERROR", 10, 0);

			flush_array((char*)tx_string, 400);

			sprintf((char*)tx_string, "AT+CWJAP=\"%s\",\"%s\"\r\n\0", wifi_ssid, wifi_pswd);
			at_cmd_send_esp((char*)tx_string,"OK\r\n", "ERROR", 10, 0);
			connect_wifi_network_flag = 0;
		}

		if(send_softap_response_flag)
		{
			if(at_cmd_send_esp("AT+CIPSEND=0,4\r\n\0",">","ERROR",5,0))
			{
				if(at_cmd_send_esp("OK\r\n\0","SEND OK\r\n","SEND FAIL",5,0))
				{
					send_softap_response_flag = 0;
				}
			}
		}

		//// SCAN AND PAIR WITH METER
		if(ble_start_pairing_flag)
		{
			green_led_blink_flag=1;//// blink when communiating with glucometer

			at_cmd_send_esp("AT+BLEENCCLEAR\r\n\0", "OK\r\n", "ERROR", 5, 0);

			sprintf((char*)ble_ssid_name, "BlipGo+%s\0", device_id);

			if(ble_initialize_gatt_client((char*)ble_ssid_name))
			{
				ble_scan_available_device();

				at_cmd_send_esp("AT+BLESECPARAM=1,4,16,3,3\r\n\0", "OK\r\n", "ERROR", 5, 0);

				flush_tx_string();
				sprintf((char*)tx_string, "AT+BLECONN=0,\"%s\"\r\n\0", ble_available_device);

				///+BLECONN:0,"9c:1d:58:9c:06:1e"
				unsigned char connection_success_resposne[30] = {0};
				sprintf((char*)connection_success_resposne, "BLECONN:0,\"%s\"", ble_available_device);

				at_cmd_send_esp((char*)tx_string, (char*)connection_success_resposne, "BLECONN:0,-1\r\n", 20, 0);


				//////   GET MODEL AND SERIAL NUMBER OF METER
				at_cmd_send_esp("AT+BLEGATTCPRIMSRV=0\r\n\0", "OK\r\n", "ERROR", 5, 0);///////primary services

				at_cmd_send_esp("AT+BLEGATTCCHAR=0,3\r\n\0", "OK\r\n", "ERROR", 5, 0);//// device information service

				at_cmd_send_esp("AT+BLEGATTCRD=0,3,2\r\n\0", "OK\r\n", "ERROR", 2, 0);///// model number
				explode_string(rx_string_esp,300,2,',','\r',10,'\n');
				if(temp_data[0]!=0)
				{
					flush_array((char*)meter_model, 10);
					sprintf((char*)meter_model, "%s\0", temp_data);
				}
				send_text_to_usb((char*)"\r\nModel: \0");
				send_text_to_usb((char*)meter_model);
				send_text_to_usb((char*)"\r\n\0");


				at_cmd_send_esp("AT+BLEGATTCRD=0,3,3\r\n\0", "OK\r\n", "ERROR", 2, 0);//// serial number
				explode_string(rx_string_esp,300,2,',','\r',20,'\n');
				if(temp_data[0]!=0)
				{
					flush_array((char*)meter_serial_no, 20);
					sprintf((char*)meter_serial_no, "%s\0", temp_data);
				}
				send_text_to_usb((char*)"\r\nMeter SN: \0");
				send_text_to_usb((char*)meter_serial_no);
				send_text_to_usb((char*)"\r\n\0");





				at_cmd_send_esp("AT+BLEENC=0,3\r\n\0", "BLESECKEYREQ:0", "ERROR", 5, 0);
				HAL_Delay(100);

				flush_tx_string();
				sprintf((char*)tx_string, "AT+BLEKEYREPLY=0,%s\r\n\0", ble_pairing_key);

				_Bool pairing_success_flag=0;

				if(strstr((char*)meter_model, "923\0"))
				{
					if(at_cmd_send_esp((char*)tx_string, "BLECONNPARAM:0,16", "ERROR", 5, 0))///BLECONNPARAM:0,16
						pairing_success_flag=1;
				}
				else/// model 897
				{
					if(at_cmd_send_esp((char*)tx_string, "BLEAUTHCMPL:0,0", "ERROR", 5, 0))
						pairing_success_flag=1;
				}

				if(pairing_success_flag)
				{
					HAL_Delay(100);
					send_softap_response("+#Device Paired;\r\n\0", 18);
					send_text_to_usb((char*)"\r\nDevice Paired\r\n\0");
					ble_device_connected_flag = 1;
					ble_get_data_flag = 1;

					meter_sequence_number=0;//// reset sequence number to read records from 0
					total_downloaded_records_meter=0;///// reset
					store_runtime_parameters_to_flash(RUNTIME_PARAMETERS_ADDRESS);
					store_runtime_parameters_to_flash(RUNTIME_PARAMETERS_BACKUP_ADDRESS);

					beep(1);
				}
				else
				{
					send_softap_response("+#Pairing Failed;\r\n\0", 19);
					send_text_to_usb((char*)"\r\nPairing Failed!\nTry Again\r\n\0");
				}
			}

			ble_start_pairing_flag = 0;

			green_led_blink_flag=0;

		}


		/////SEARCH AND CONNECT TO PAIRED METER
		if(ble_connect_paired_meter_flag)
		{
			sprintf((char*)ble_ssid_name, "BlipGo+%s\0", device_id);

			if(ble_initialize_gatt_client((char*)ble_ssid_name))
			{
				ble_get_paired_device();

				if(ble_paired_device[0]!=0)
				{
					ble_scan_available_device();

					///// IS AVAILABLE DEVICE PAIRED
					if(ble_is_available_device_paired())
					{
						green_led_blink_flag=1;

						at_cmd_send_esp("AT+BLESECPARAM=13,4,16,3,3\r\n\0", "OK\r\n", "ERROR", 5, 0);

						flush_tx_string();
						sprintf((char*)tx_string, "AT+BLECONN=0,\"%s\"\r\n\0", ble_available_device);

						///+BLECONN:0,"9c:1d:58:9c:06:1e"
						unsigned char connection_success_resposne[30] = {0};
						sprintf((char*)connection_success_resposne, "BLECONN:0,\"%s\"", ble_available_device);

						at_cmd_send_esp((char*)tx_string, (char*)connection_success_resposne, "BLECONN:0,-1\r\n", 20, 0);
						HAL_Delay(100);


						at_cmd_send_esp("AT+BLEGATTCPRIMSRV=0\r\n\0", "OK\r\n", "ERROR", 5, 0);///////primary services

						at_cmd_send_esp("AT+BLEGATTCCHAR=0,3\r\n\0", "OK\r\n", "ERROR", 5, 0);//// device information service

						at_cmd_send_esp("AT+BLEGATTCRD=0,3,2\r\n\0", "OK\r\n", "ERROR", 2, 0);///// model number

						explode_string(rx_string_esp,300,2,',','\r',10,'\n');
						if(temp_data[0]!=0)
						{
							flush_array((char*)meter_model, 10);
							sprintf((char*)meter_model, "%s\0", temp_data);
						}
						send_text_to_usb((char*)"\r\nModel: \0");
						send_text_to_usb((char*)meter_model);
						send_text_to_usb((char*)"\r\n\0");


						at_cmd_send_esp("AT+BLEGATTCRD=0,3,3\r\n\0", "OK\r\n", "ERROR", 2, 0);//// serial number

						explode_string(rx_string_esp,300,2,',','\r',20,'\n');
						if(temp_data[0]!=0)
						{
							flush_array((char*)meter_serial_no, 20);
							sprintf((char*)meter_serial_no, "%s\0", temp_data);
						}
						send_text_to_usb((char*)"\r\nMeter SN: \0");
						send_text_to_usb((char*)meter_serial_no);
						send_text_to_usb((char*)"\r\n\0");



						_Bool connected_flag=0;

						if(strstr((char*)meter_model, "923\0"))
						{
							if(at_cmd_send_esp("AT+BLEENC=0,3\r\n\0", "BLECONNPARAM:0,16", "ERROR", 5, 0))
								connected_flag=1;
						}
						else/// model 897
						{
							if(at_cmd_send_esp("AT+BLEENC=0,3\r\n\0", "OK\r\n", "ERROR", 5, 0))
								connected_flag=1;
						}


						if(connected_flag)//BLECONNPARAM:0,16
						{
							HAL_Delay(100);
							ble_device_connected_flag = 1;
							ble_get_data_flag = 1;
							beep(1);
						}

						green_led_blink_flag=0;
					}
				}
			}

			check_paired_device_timer = 0;
			ble_connect_paired_meter_flag = 0;

		}


		//// GET DATA FROM METER
		if(ble_get_data_flag && ble_device_connected_flag)
		{

			green_led_blink_flag=1;

			at_cmd_send_esp("AT+BLEGATTCPRIMSRV=0\r\n\0", "OK\r\n", "ERROR", 5, 0);///////primary services


			at_cmd_send_esp("AT+BLEGATTCCHAR=0,2\r\n\0", "OK\r\n", "ERROR", 5, 0);

			at_cmd_send_esp("AT+BLEGATTCWR=0,2,1,1,2\r\n\0", ">", "ERROR", 5, 0);
			at_cmd_send_esp("1\r\n\0", "OK\r\n", "ERROR", 2, 0);

			if(strstr((char*)meter_model, "923\0"))
			{
				at_cmd_send_esp("AT+BLEGATTCWR=0,2,2,1,2\r\n\0", ">", "ERROR", 5, 0);
				at_cmd_send_esp("1\r\n\0", "OK\r\n", "ERROR", 2, 0);

				at_cmd_send_esp("AT+BLEGATTCWR=0,2,4,1,2\r\n\0", ">", "ERROR", 5, 0);
				at_cmd_send_esp("2\r\n\0", "OK\r\n", "ERROR", 2, 0);
			}
			else//// model 897
			{
				at_cmd_send_esp("AT+BLEGATTCWR=0,2,3,1,2\r\n\0", ">", "ERROR", 5, 0);
				at_cmd_send_esp("2\r\n\0", "OK\r\n", "ERROR", 2, 0);
			}


//			at_cmd_send_esp("AT+BLEGATTCRD=0,2,1,1\r\n\0", "OK\r\n", "ERROR", 2, 0);
//			HAL_Delay(500);

//			at_cmd_send_esp("AT+BLEGATTCRD=0,2,2,1\r\n\0", "OK\r\n", "ERROR", 2, 0);
//			HAL_Delay(500);

//			at_cmd_send_esp("AT+BLEGATTCRD=0,2,4,1\r\n\0", "OK\r\n", "ERROR", 2, 0);
//			HAL_Delay(500);

			char_total_record[0] = 0;//M
			char_total_record[1] = 0;//00

			if(strstr((char*)meter_model, "923\0"))
			{
				at_cmd_send_esp("AT+BLEGATTCWR=0,2,4,,2\r\n\0", ">", "ERROR", 2, 0);
			}
			else//model 897
			{
				at_cmd_send_esp("AT+BLEGATTCWR=0,2,3,,2\r\n\0", ">", "ERROR", 2, 0);
			}


			flush_tx_string();
			sprintf((char*)tx_string, "%c%c\0",4, 1);
			send_text_to_uart1(tx_string);
			HAL_Delay(500);

			total_records_meter = char_total_record[0] | (char_total_record[1] << 8);

			flush_tx_string();
			sprintf((char*)tx_string, "Total records : %u\r\n\0", total_records_meter);
			send_text_to_usb((char*)tx_string);

			/////// LABEL
			FETCH_METER_RECORDS:

			row = 0;

			//// ERASE LOGS VARIABLE
			for(uint8_t i=0; i<100; i++)
			{
				for(uint8_t j=0; j<17; j++)
				{
					meter_records[i][j] = 0;
				}
			}

			/////Sequence number conversion into char[2]
			uint16_t meter_starting_sequence_number = 0, meter_ending_sequence_number = 0;

			if(meter_sequence_number!=0)
			{
				meter_starting_sequence_number = meter_sequence_number+1;
			}

			meter_ending_sequence_number = meter_starting_sequence_number + 99;///// 100 records max


			unsigned char meter_sequence_char[2] = {0}, meter_sequence_char2[2] = {0};
			meter_sequence_char[0] = meter_starting_sequence_number & 0xFF;
			meter_sequence_char[1] = meter_starting_sequence_number >> 8;

			meter_sequence_char2[0] = meter_ending_sequence_number & 0xFF;
			meter_sequence_char2[1] = meter_ending_sequence_number >> 8;

			uint8_t racp_opcode = 0x01, racp_operator = 0x04, racp_filter = 0x01;
			uint16_t number_of_bytes = 7;///// number of bytes of racp command

			flush_tx_string();
			if(strstr((char*)meter_model, "923\0"))
			{
				sprintf((char*)tx_string, "AT+BLEGATTCWR=0,2,4,,%u\r\n\0" , number_of_bytes);
			}
			else///model 897
			{
				sprintf((char*)tx_string, "AT+BLEGATTCWR=0,2,3,,%u\r\n\0" , number_of_bytes);
			}

			at_cmd_send_esp((char*)tx_string, ">", "ERROR", 2, 0);////// 7 bytes to send

			flush_tx_string();

			//// 01-01  for all records, 01-03-01-28-00 for record greater than or equal to (03) sequence number (01) 40 ( 28-00 )
			///// 01-02-01-28-00   for record less than or equal to (02) sequence number (01) 40 (28-00)

			//sprintf((char*)tx_string, "%c%c%c%c%c\0",racp_opcode, racp_operator, racp_filter, meter_sequence_char[0], meter_sequence_char[1]);//// last char is 0x00 so functions will not work, need to write code in this case

			sprintf((char*)tx_string, "%c%c%c%c%c%c%c\0",racp_opcode, racp_operator, racp_filter, meter_sequence_char[0], meter_sequence_char[1],meter_sequence_char2[0], meter_sequence_char2[1]);


			//////AT function
			wait_ms(300);
			flush_array((char*)rx_string_esp, 300);
			rx_esp_counter = 0;

			HAL_UART_Transmit_IT(&huart1, tx_string, number_of_bytes);
			HAL_Delay(15);


			timer_cmd_esp=0;

			while(timer_cmd_esp < 10)
			{
				if(strstr((char*)rx_string_esp, "INDICATE:0,2,4,4,"))
				{
					break;
				}

				if(strstr((char*)rx_string_esp, "ERROR"))
				{
					break;
				}
			}
			///////AT function end

			//+INDICATE:0,2,4,4,[06][00][01][01]
//			at_cmd_send_esp((char*)tx_string, "INDICATE:0,2,4,4,", "ERROR", 10, 0);
//			HAL_Delay(100);

			at_cmd_send_esp("AT+BLEDISCONN=0\r\n\0", "BLEDISCONN:0,", "ERROR", 10, 0);
			beep(1);
			HAL_Delay(100);

			if(row>0)
				meter_sequence_number = sequence_char[0] | (sequence_char[1] << 8);

			flush_tx_string();
			sprintf((char*)tx_string, "Last Sequence Number : %u\r\n\0", meter_sequence_number);
			send_text_to_usb((char*)tx_string);

			//////// PRINT METER RECORDS
			_Bool write_server_clock_flag=1;


			for(uint8_t cc = 0; cc < 100; cc++)
			{
				if(meter_records[cc][0]!=0)
				{
					total_downloaded_records_meter++;

					flush_tx_string();
					sprintf((char*)tx_string, "Record No. %u: \0", total_downloaded_records_meter);
					send_text_to_usb((char*)tx_string);


					uint8_t data_byte=0;

					while(1)////// search for blank address to start write
					{
						data_byte=sFLASH_ReadByte1(mem_write_address);

						if(data_byte==0xFF && (mem_write_address-LOGS_START_ADDRESS)%17==0)///// if empty address
							break;
						else
							mem_write_address++;

						if(mem_write_address>=FLASH_LAST_ADDRESS)
						{
							erase_all_records();
							break;
						}
					}

					///// write meter data download time
					if(write_server_clock_flag)
					{
						store_clock_to_flash();
						write_server_clock_flag=0;
					}

					for(uint8_t zz=0; zz<17; zz++)
					{
						#if USB_ENABLED
						CDC_Transmit_FS(&meter_records[cc][zz], 1);
						HAL_Delay(2);
						#endif

						sFLASH_WriteByte(mem_write_address++, meter_records[cc][zz]);///////// write data to flash
					}


					store_runtime_parameters_to_flash(RUNTIME_PARAMETERS_ADDRESS);
					store_runtime_parameters_to_flash(RUNTIME_PARAMETERS_BACKUP_ADDRESS);

					send_text_to_usb("\r\n\0");
				}
			}

//			total_downloaded_records_meter = total_downloaded_records_meter + row;
//
//			store_runtime_parameters_to_flash(RUNTIME_PARAMETERS_ADDRESS);
//			store_runtime_parameters_to_flash(RUNTIME_PARAMETERS_BACKUP_ADDRESS);

			if(total_downloaded_records_meter < total_records_meter && ble_device_connected_flag)
				goto FETCH_METER_RECORDS;
			else if(total_downloaded_records_meter == total_records_meter)
			{

				beep(20);
				HAL_Delay(100);
				beep(30);

				mqtt_server_connection_fail_counter=0;

//				if(row>0)
//					server_clock_flag=1;

			}

			ble_get_data_flag = 0;

			green_led_blink_flag=0;
		}

		HAL_Delay(1000);

//		check_gprs_network_registration();



		server_clock_handler();

		ping_service_handler();

		configuration_service_handler();

		ca_certificate_handler();

		client_certificate_handler();

		client_key_handler();

		update_module_firmware_handler();

		update_device_firmware_handler();

		device_twin_handler();

		publish_meter_readings_handler();



		// ESP SOFT AP MODE POWER OFF HANDLER
		esp_disable_softap_mode_handler();

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL3;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB|RCC_PERIPHCLK_USART1
                              |RCC_PERIPHCLK_RTC;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 48;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 1000;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 48000;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 1000;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}

/**
  * @brief TIM16 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM16_Init(void)
{

  /* USER CODE BEGIN TIM16_Init 0 */

  /* USER CODE END TIM16_Init 0 */

  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM16_Init 1 */

  /* USER CODE END TIM16_Init 1 */
  htim16.Instance = TIM16;
  htim16.Init.Prescaler = 12-1;
  htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim16.Init.Period = 1000-1;
  htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim16.Init.RepetitionCounter = 0;
  htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim16) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim16) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 500;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim16, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim16, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM16_Init 2 */

  /* USER CODE END TIM16_Init 2 */
  HAL_TIM_MspPostInit(&htim16);

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GSM_PWRKEY_Pin|ESP_PWR_Pin|GSM_DTR_Pin|ESP_EN_Pin
                          |LED1_Pin|LED2_Pin|BUZZOR_Pin|LED3_Pin
                          |GSM_RESET_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GSM_VCC_GPIO_Port, GSM_VCC_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : PC13 PC14 PC15 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 PA8 PA13
                           PA14 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_8|GPIO_PIN_13
                          |GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : FLASH_CS_Pin GSM_VCC_Pin */
  GPIO_InitStruct.Pin = FLASH_CS_Pin|GSM_VCC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : GSM_PWRKEY_Pin ESP_PWR_Pin GSM_DTR_Pin ESP_EN_Pin
                           LED1_Pin LED2_Pin BUZZOR_Pin LED3_Pin
                           GSM_RESET_Pin */
  GPIO_InitStruct.Pin = GSM_PWRKEY_Pin|ESP_PWR_Pin|GSM_DTR_Pin|ESP_EN_Pin
                          |LED1_Pin|LED2_Pin|BUZZOR_Pin|LED3_Pin
                          |GSM_RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB1 PB12 PB13 PB14
                           PB15 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_15|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

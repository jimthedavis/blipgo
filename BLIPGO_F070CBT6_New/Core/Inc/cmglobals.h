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
 * Module: cmexterns.h
 * Author: J Davis
 * Date: December 15, 2021
 *
 ***************************************************************************
 */

/***************************************************************************
 * This module contains externs and global defines used by the
 * Carematix code.
 */

/***************************************************************************
 *                               DEFINES
 **************************************************************************/

#define DBGLVL_NONE 0
#define DBGLVL_MIN 1
#define DBGLVL_MED 2
#define DBGLVL_STATE 3
#define DBGLVL_MAX  4

#define LED_MODE_OFF   0
#define LED_MODE_ON    1
#define LED_MODE_BLINK 2

#define QS_OK 0
#define QS_TXBUSY 1
#define QS_RXBUSY 2
#define QS_OVERFLOW 3
#define QS_TIMEOUT 4
#define QS_STREAMING 5
#define QS_INPROGRESS 6

#define CA_NONE 0
#define CA_NOTPOWERED 1
#define CA_INPROGRESS 2
#define CA_POWERED 3
#define CA_CONNECTED 4
#define CA_APMODE 5
#define CA_PAIRED 6
#define CA_CLOCK 7
#define CA_SENDRECORDS 8
#define CA_BADCONFIG 9
#define CA_CVMATCH 10
#define CA_NOMETER 11
#define CA_FAILED 12

#define CR_NONE 0
#define CR_POWERUP 1
#define CR_POWERDOWN 2
#define CR_CONNECT 3
#define CR_APMODE 4
#define CR_PING 5
#define CR_CLOCK 6
#define CR_SENDRECORDS 7
#define CR_RDTWIN 8
#define CR_UPDCONFIG 9
#define CR_UPDTWIN 10
#define CR_DISCONNECT 11

#define GSM_ENBL_PORT GPIOA
#define GSM_ENBL_PIN GPIO_PIN_15

#define GSM_RESET_PORT GPIOB
#define GSM_RESET_PIN GPIO_PIN_7

#define GSM_PWR_PORT GPIOB
#define GSM_PWR_PIN GPIO_PIN_0

#define GSM_DTR_PORT GPIOB
#define GSM_DTR_PIN GPIO_PIN_10

#define GSM_UART USART2
#define GSM_UART_TX_PORT GPIOA
#define GSM_UART_TX_PIN GPIO_PIN_3
#define GSM_UART_RX_PORT GPIOA
#define GSM_UART_RX_PIN GPIO_PIN_2
#define GSM_UART_TX_AF GPIO_AF1_USART2
#define GSM_UART_RX_AF GPIO_AF1_USART2


#define ESP_ENBL_PORT GPIOB
#define ESP_ENBL_PIN GPIO_PIN_11

#define ESP_PWR_PORT GPIOB
#define ESP_PWR_PIN GPIO_PIN_2

#define ESP_UART USART1
#define ESP_UART_TX_PORT GPIOA
#define ESP_UART_TX_PIN GPIO_PIN_10
#define ESP_UART_RX_PORT GPIOA
#define ESP_UART_RX_PIN GPIO_PIN_9
#define ESP_UART_TX_AF GPIO_AF1_USART1
#define ESP_UART_RX_AF GPIO_AF1_USART1

#define PAIRINGKEY_LEN 9
#define METERMODEL_LEN 10
#define METERSERIAL_LEN 20
#define JRBUF_LEN 600
#define MFV_LEN 40
#define METERRECORDS_LEN 25
#define APN_LEN 40
#define DEVICEID_LEN 13
#define MQTTSERVER_LEN 100
#define MQTTPORT_LEN 7
#define MQTTUNAME_LEN 100
#define MQTTPWORD_LEN 50
#define MQTTCLIENTID_LEN 100
#define PV_LEN 5
#define FV_LEN 10
#define TWIN_LEN 100
#define SSIDNAME_LEN 50
#define CV_LEN 10

#define DEVICE_ID_ADDRESS 0
#define DEVICE_SETTINGS_ADDRESS 4096
#define DEVICE_SETTINGS_BACKUP_ADDRESS 8192
#define RUNTIME_PARAMETERS_ADDRESS 12288
#define RUNTIME_PARAMETERS_BACKUP_ADDRESS 16384
#define LOGS_START_ADDRESS 20480
#define FLASH_LAST_ADDRESS 2097341

#define METER_TYPE_NONE '0'
#define METER_TYPE_GLUCOSE 'G'
#define METER_TYPE_PULSEOX 'P'

#define ESP_TXBUFLEN 128
#define ESP_MAX_RECEIVE_LEN 256


 ///////////////////// SELECT MODULE ///////////////
#define BG96 0
#define BG95 1
//////////////////////


#define OLD_BOARD 0

#define AP_MODE_TIMEOUT_DURATION 300



/***************************************************************************
 *                               TYPEDEFS
 **************************************************************************/

typedef struct stbl
{
    uint32_t(*stbl_event)(void);
    void(*stbl_act1)(void);
    void(*stbl_act2)(void);
    uint32_t stbl_newstate;
} S_TABLE;

typedef struct sms
{
    S_TABLE **sms_stable;
    uint32_t sms_curstate;
    void(*sms_debug)(uint32_t);
} SM_STRUC;


/***************************************************************************
 *                            GLOBAL VARIABLE EXTERNS
 **************************************************************************/

extern uint32_t debuglevel;
extern SM_STRUC gsm_stmachine;
extern uint8_t debug_print_ready;
extern uint8_t esp_ready_flag;
extern SM_STRUC esp_stmachine;
extern SM_STRUC master_stmachine;
extern SM_STRUC roche_stmachine;
extern uint8_t desired_reported_cv_matched_flag;
extern uint8_t ca_certificate_flag;
extern uint8_t client_certificate_flag;
extern uint8_t client_key_flag;
extern uint8_t configuration_service_flag;
extern uint8_t new_configuration_version[];
extern uint8_t espver;
extern uint8_t meter_erased_flag;
extern uint32_t meter_model_bin;
extern uint8_t meter_type;
extern uint8_t update_module_firmware_flag;
extern uint8_t update_device_firmware_flag;
extern uint8_t update_device_twin_flag;
extern uint8_t sending_readings;
extern uint8_t fetching_readings;
extern uint8_t pair_failed;
extern uint8_t ping_failed_flag;
extern uint8_t bg95_rssi;
extern uint8_t scanning_red;

extern uint8_t ble_available_device[];
extern uint8_t ble_paired_device[];
extern uint8_t ble_pairing_key[];
extern uint8_t ble_ssid_name[];
extern uint16_t json_count;
extern uint8_t json_response[];
extern uint8_t meter_model[];
extern uint8_t meter_serial_no[];
extern uint8_t mqtt_server[];
extern uint8_t mqtt_port[];
extern uint8_t mqtt_username[];
extern uint8_t mqtt_password[];
extern uint8_t mqtt_client_id[];
extern uint32_t server_clock;
extern uint8_t twin_sb[];
extern uint8_t twin_pb[];
extern uint8_t twin_rp[];
extern uint8_t read_pb[];
extern uint32_t mem_write_address;
extern uint32_t mem_read_address;
extern uint8_t device_id[];
extern uint8_t imei[16];
extern const uint8_t blipgo_model[];
extern uint8_t ccid[];
extern uint8_t imsi[];
extern uint8_t configuration_version[];
extern const uint8_t firmware_version[];
extern uint8_t protocol_version[];
extern uint8_t module_firmware_version[];
extern uint8_t meter_records[][17];
extern uint8_t http_token[];
extern uint16_t meter_sequence_number;
extern uint16_t total_downloaded_records_meter;
extern uint8_t apn[];
extern uint32_t testword;

extern uint32_t bgretrycount;

extern uint8_t esp_rcv_status;
extern uint32_t esp_rcv_count;
extern uint8_t esp_rcv_buffer[];
extern uint8_t esp_txbuf[];



/***************************************************************************
 *                         GLOBAL FUNCTION PROTOTYPES
 **************************************************************************/

extern void compare_cvs(void);
extern uint32_t convert_to_epoch(uint8_t *, uint8_t *);
extern void crash(void);
void erase_all_records(void);
extern uint8_t json_get_value(uint8_t *, uint8_t, uint8_t, uint8_t *, uint8_t);
extern uint8_t ping_response(void);
extern uint16_t publish_meter_readings(uint8_t *);
extern uint8_t read_settings_from_flash(uint32_t sect_address);
extern void store_devinfo_to_flash(uint32_t);
extern void store_records_to_flash(uint16_t);
extern void store_settings_to_flash(uint32_t);
extern void configuration_service(void);

extern void beep(uint32_t);
extern void beep_init(void);
extern void beep_set(uint8_t, uint8_t, uint8_t);
extern void beep_task(void);
extern void beep_timer_ih(void);

extern void cons_init(void);
extern void cons_input(uint8_t *, uint32_t);
extern void cons_task(void);

extern void debug_dumpab(uint32_t, uint8_t *, uint32_t, uint8_t);
extern void debug_printf(uint32_t, uint8_t *, ... );
extern void debug_write(uint32_t, uint8_t *);
extern void debug_write_fixed(uint32_t, uint8_t *, uint32_t);

extern void dma_ch4_5_ih(void);

extern uint32_t esp_compare(uint32_t, uint8_t *, uint32_t);
extern void esp_init(void);
extern void esp_power_on(void);
extern void esp_reset(void);
extern uint8_t esp_receive(uint8_t *, uint16_t, uint32_t, void(*)(uint8_t, uint32_t));
extern void esp_rxflush(void);
extern void esp_task(void);
extern uint8_t esp_transmit(uint8_t *, uint16_t);

extern uint8_t espat_answer(void);
extern void espat_init(void);
extern void espat_request(uint8_t);

extern uint8_t gsm_answer(void);
extern void gsm_init(void);
extern void gsm_request(uint8_t);

extern uint8_t jpd5_answer(void);
extern void jpd5_init(void);
extern void jpd5_request(uint8_t req);

extern void led_green_set(uint8_t);
extern void led_init(void);
extern void led_red_set(uint8_t);
extern void led_task(void);
extern void led_yellow_set(uint8_t);

extern void master_init(void);
extern void master_timer_ih(void);

extern void quec_3v8enbl(uint8_t);
extern void quec_dma_ih(void);
extern uint32_t quec_outcompl(void);
extern void quec_rxflush(void);
extern void quec_init(void);
extern uint8_t quec_receive(uint8_t *, uint16_t, uint32_t, void(*)(uint8_t, uint32_t));
extern void quec_task(void);
extern uint8_t quec_transmit(uint8_t *, uint16_t);

extern uint8_t roche_answer(void);
extern void roche_init(void);
extern void roche_request(uint8_t req);

extern void usb_init(void);
extern void usb_write(uint8_t *);
extern void usb_write_fixed(uint8_t *, uint32_t);



/*
 * End of module.
 */

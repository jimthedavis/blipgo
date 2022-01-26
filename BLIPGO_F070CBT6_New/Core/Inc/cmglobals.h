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
 * This module contains externs used by the Carematix code.
 */

 /***************************************************************************
 *                               DEFINES
 **************************************************************************/

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

extern SM_STRUC gsm_stmachine;
extern uint8_t quec_ready_flag;

/***************************************************************************
 *                         GLOBAL FUNCTION PROTOTYPES
 **************************************************************************/

extern void crash(void);

extern void beep(uint32_t);
extern void beep_init(void);

extern void dma_ch4_5_ih(void);

extern void esp_init(void);
extern void esp_power_on(void);

extern void gsm_init(void);

extern void led_green_set(uint8_t);
extern void led_init(void);
extern void led_red_set(uint8_t);
extern void led_yellow_set(uint8_t);

extern void quec_3v8enbl(uint8_t);
extern void quec_dma_ih(void);
extern void quec_reset(void);
extern void quec_rxflush(void);
extern void quec_init(void);
extern void quec_power_on(void);
extern uint8_t quec_receive(uint8_t *, uint16_t, uint32_t, void(*)(uint8_t, uint32_t));
extern void quec_task(void);
extern uint8_t quec_transmit(uint8_t *, uint16_t);

extern void usb_printf(uint8_t *, ... );
extern void usb_write(uint8_t *);
extern void usb_write_fixed(uint8_t *, uint32_t);



/*
 * End of module.
 */

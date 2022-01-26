
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
 * Module: led.c
 * Author: J Davis
 * Date: December 15, 2021
 *
 ***************************************************************************
 */

 /*
  * This module contains the led handler.
  */

/***************************************************************************
 *                              INCLUDES
 **************************************************************************/

#include "stdio.h"
#include "stdlib.h"
#include "main.h"
#include "config.h"
#include <cmglobals.h>

/***************************************************************************
 *                               DEFINES
 **************************************************************************/

#define GREEN_LED_PORT GPIOB
#define GREEN_LED_PIN GPIO_PIN_3

#define RED_LED_PORT GPIOB
#define RED_LED_PIN GPIO_PIN_4

#define YELLOW_LED_PORT GPIOB
#define YELLOW_LED_PIN GPIO_PIN_6

#define BLINK_PERIOD_MS 1000

/***************************************************************************
 *                               TYPEDEFS
 **************************************************************************/

 typedef struct LHS
 {
     uint8_t lhs_mode;
     uint8_t lhs_state;
 } LED_HANDLER_STRUC;

/***************************************************************************
 *                            GLOBAL VARIABLES
 **************************************************************************/

/***************************************************************************
 *                             LOCAL VARIABLES
 **************************************************************************/

 static uint32_t led_timer;

 static LED_HANDLER_STRUC green_led;
 static LED_HANDLER_STRUC red_led;
 static LED_HANDLER_STRUC yellow_led;

/***************************************************************************
 *                         LOCAL FUNCTION PROTOTYPES
 **************************************************************************/

/***************************************************************************
 *                             GLOBAL FUNCTIONS
 **************************************************************************/



/***************************************************************************
 *                         led_init
 *                         --------
 *
 * Initializes the led pins and handler.
 *
 * param[in] - none
 *
 * return - none
 */

void led_init(void)
{
    GPIO_InitTypeDef initstruc;

    led_timer = 0xFFFFFFFF;
    green_led.lhs_mode = LED_MODE_OFF;
    green_led.lhs_state = 0;
    red_led.lhs_mode = LED_MODE_OFF;
    red_led.lhs_state = 0;
    yellow_led.lhs_mode = LED_MODE_OFF;
    yellow_led.lhs_state = 0;

    initstruc.Pin = GREEN_LED_PIN;
    initstruc.Mode = GPIO_MODE_OUTPUT_PP;
    initstruc.Pull = GPIO_NOPULL;
    initstruc.Speed = GPIO_SPEED_FREQ_LOW;
    initstruc.Alternate = 0;
    HAL_GPIO_Init(GREEN_LED_PORT, &initstruc);

    initstruc.Pin = RED_LED_PIN;
    HAL_GPIO_Init(RED_LED_PORT, &initstruc);

    initstruc.Pin = YELLOW_LED_PIN;
    HAL_GPIO_Init(YELLOW_LED_PORT, &initstruc);

    HAL_GPIO_WritePin(GREEN_LED_PORT, GREEN_LED_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RED_LED_PORT, RED_LED_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(YELLOW_LED_PORT, YELLOW_LED_PIN, GPIO_PIN_RESET);

    led_timer = BLINK_PERIOD_MS / 2;

    return;
}

/***************************************************************************
 *                         led_green_set
 *                         -------------
 *
 * Set green led on/off as directed
 *
 * param[in] - led mode
 *
 * return - none
 */

void led_green_set(uint8_t mode)
{
    led_timer = 0xFFFFFFFF;
	green_led.lhs_mode = mode;

    if (green_led.lhs_mode == LED_MODE_ON)
        {
            HAL_GPIO_WritePin(GREEN_LED_PORT, GREEN_LED_PIN, GPIO_PIN_SET);
            green_led.lhs_state = 1;
        }

    else if (green_led.lhs_mode == LED_MODE_OFF)
        {
            HAL_GPIO_WritePin(GREEN_LED_PORT, GREEN_LED_PIN, GPIO_PIN_RESET);
            green_led.lhs_state = 0;
        }

    led_timer = BLINK_PERIOD_MS / 2;
    return;
}

/***************************************************************************
 *                         led_red_set
 *                         -------------
 *
 * Set red led on/off as directed
 *
 * param[in] - ledstate
 *
 * return - none
 */

void led_red_set(uint8_t mode)
{
    led_timer = 0xFFFFFFFF;
	red_led.lhs_mode = mode;

    if (red_led.lhs_mode == LED_MODE_ON)
        {
            HAL_GPIO_WritePin(RED_LED_PORT, RED_LED_PIN, GPIO_PIN_SET);
            red_led.lhs_state = 1;
        }

    else if (red_led.lhs_mode == LED_MODE_OFF)
        {
            HAL_GPIO_WritePin(RED_LED_PORT, RED_LED_PIN, GPIO_PIN_RESET);
            red_led.lhs_state = 0;
        }

    led_timer = BLINK_PERIOD_MS;

    return;
}

/***************************************************************************
 *                         led_yellow_set
 *                         --------------
 *
 * Set yellow led on/off as directed
 *
 * param[in] - ledstate
 *
 * return - none
 */

void led_yellow_set(uint8_t mode)
{
    led_timer = 0xFFFFFFFF;
    yellow_led.lhs_mode = mode;

    if (yellow_led.lhs_mode == LED_MODE_ON)
        {
            HAL_GPIO_WritePin(YELLOW_LED_PORT, YELLOW_LED_PIN, GPIO_PIN_SET);
            yellow_led.lhs_state = 1;
        }

    else if (yellow_led.lhs_mode == LED_MODE_OFF)
        {
            HAL_GPIO_WritePin(YELLOW_LED_PORT, YELLOW_LED_PIN, GPIO_PIN_RESET);
            yellow_led.lhs_state = 0;
        }

    led_timer = BLINK_PERIOD_MS;
    return;
}

/***************************************************************************
 *                         led_timer_ih
 *                         --------------
 *
 * Set yellow led on/off as directed
 *
 * param[in] - ledstate
 *
 * return - none
 */

void led_timer_ih()
{

    if (led_timer & 0x80000000)
    {
    	return;
    }

    led_timer--;

    if (led_timer == 0)
    {
    	led_timer = BLINK_PERIOD_MS;

        if (green_led.lhs_mode == LED_MODE_BLINK)
        {
    	    green_led.lhs_state ^= 0x01;

            if (green_led.lhs_state)
            {
                HAL_GPIO_WritePin(GREEN_LED_PORT, GREEN_LED_PIN, GPIO_PIN_SET);
            }

            else
            {
                HAL_GPIO_WritePin(GREEN_LED_PORT, GREEN_LED_PIN, GPIO_PIN_RESET);
            }

        }

        if (red_led.lhs_mode == LED_MODE_BLINK)
        {
    	    red_led.lhs_state ^= 0x01;

            if (red_led.lhs_state)
            {
                HAL_GPIO_WritePin(RED_LED_PORT, RED_LED_PIN, GPIO_PIN_SET);
            }

            else
            {
                HAL_GPIO_WritePin(RED_LED_PORT, RED_LED_PIN, GPIO_PIN_RESET);
            }

        }

        if (yellow_led.lhs_mode == LED_MODE_BLINK)
        {
    	    yellow_led.lhs_state ^= 0x01;

            if (yellow_led.lhs_state)
            {
                HAL_GPIO_WritePin(YELLOW_LED_PORT, YELLOW_LED_PIN, GPIO_PIN_SET);
            }

            else
            {
                HAL_GPIO_WritePin(YELLOW_LED_PORT, YELLOW_LED_PIN, GPIO_PIN_RESET);
            }

        }

    }

    return;
}
/***************************************************************************
 *                             LOCAL FUNCTIONS
 **************************************************************************/

/*
 * End of module.
 */

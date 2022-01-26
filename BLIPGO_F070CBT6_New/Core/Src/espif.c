
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
 * Module: espif.c
 * Author: J Davis
 * Date: December 17, 2021
 *
 ***************************************************************************
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

/***************************************************************************
 *                               TYPEDEFS
 **************************************************************************/

/***************************************************************************
 *                            GLOBAL VARIABLES
 **************************************************************************/

/***************************************************************************
 *                             LOCAL VARIABLES
 **************************************************************************/

/***************************************************************************
 *                         LOCAL FUNCTION PROTOTYPES
 **************************************************************************/

/***************************************************************************
 *                             GLOBAL FUNCTIONS
 **************************************************************************/

/***************************************************************************
 *                         esp_init
 *                         --------
 *
 *
 */

void esp_init(void)
{
    GPIO_InitTypeDef initstruc;
    UART_HandleTypeDef huart;
    HAL_StatusTypeDef halstat;

    initstruc.Pin = ESP_PWR_PIN;
    initstruc.Mode = GPIO_MODE_OUTPUT_OD;
    initstruc.Pull = GPIO_NOPULL;
    initstruc.Speed = GPIO_SPEED_FREQ_LOW;
    initstruc.Alternate = 0;
    HAL_GPIO_Init(ESP_PWR_PORT, &initstruc);

    initstruc.Pin = ESP_ENBL_PIN;
    HAL_GPIO_Init(ESP_ENBL_PORT, &initstruc);

    HAL_GPIO_WritePin(ESP_PWR_PORT, ESP_PWR_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(ESP_ENBL_PORT, ESP_ENBL_PIN, GPIO_PIN_SET);

    initstruc.Pin = ESP_UART_TX_PIN;
    initstruc.Mode = GPIO_MODE_AF_PP;
    initstruc.Alternate = ESP_UART_TX_AF;
    HAL_GPIO_Init(ESP_UART_TX_PORT, &initstruc);

    initstruc.Pin = ESP_UART_RX_PIN;
    initstruc.Mode = GPIO_MODE_AF_PP;
    initstruc.Alternate = ESP_UART_RX_AF;
    HAL_GPIO_Init(ESP_UART_RX_PORT, &initstruc);

    __HAL_RCC_USART1_CLK_ENABLE();

    huart.Instance = ESP_UART;
    huart.Init.BaudRate = 115200;
    huart.Init.WordLength = UART_WORDLENGTH_8B;
    huart.Init.StopBits = UART_STOPBITS_1;
    huart.Init.Parity = UART_PARITY_NONE;
    huart.Init.Mode = UART_MODE_TX_RX;
    huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart.Init.OverSampling = UART_OVERSAMPLING_16;
    huart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    halstat = HAL_UART_Init(&huart);

    if (halstat != HAL_OK)
    {

//        crash();
    }



    return;
}

/***************************************************************************
 *                         esp_power_on
 *                         -----------
 *
 *
 */

void esp_power_on(void)
{

    return;
}

/***************************************************************************
 *                         esp_uart_ih
 *                         -----------
 *
 *
 */

void esp_uart_ih(void)
{

    return;
}



/***************************************************************************
 *                             LOCAL FUNCTIONS
 **************************************************************************/

/*
 * End of module.
 */


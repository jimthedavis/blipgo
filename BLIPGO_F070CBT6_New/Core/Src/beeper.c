
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
 * Module: beeper.c
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

extern TIM_HandleTypeDef htim16;

/***************************************************************************
 *                               DEFINES
 **************************************************************************/

#define BEEPER_PORT GPIOB
#define BEEPER_PIN GPIO_PIN_8

/***************************************************************************
 *                               TYPEDEFS
 **************************************************************************/

/***************************************************************************
 *                            GLOBAL VARIABLES
 **************************************************************************/

/***************************************************************************
 *                             LOCAL VARIABLES
 **************************************************************************/

 static TIM_TypeDef *timer;

/***************************************************************************
 *                         LOCAL FUNCTION PROTOTYPES
 **************************************************************************/

/***************************************************************************
 *                             GLOBAL FUNCTIONS
 **************************************************************************/

/***************************************************************************
 *                         beep
 *                         ----
 *
 * Initializes the led pins and handler.
 *
 * param[in] - time - #msecs to beep
 *
 * return - none
 */

void beep(uint32_t time)
{
    HAL_GPIO_WritePin(BUZZOR_GPIO_Port,BUZZOR_Pin,GPIO_PIN_SET);
    timer->BDTR = TIM_BDTR_MOE;
    timer->CR1 = TIM_CR1_CEN;
    timer->EGR = TIM_EGR_UG;
    timer->CCR1 = 500;
    HAL_Delay(time);
    timer->BDTR = 0;
    timer->DIER = 0;
    HAL_GPIO_WritePin(BUZZOR_GPIO_Port,BUZZOR_Pin,GPIO_PIN_RESET);
    return;
}

/***************************************************************************
 *                         beep_init
 *                         ---------
 *
 * Initializes the led pins and handler.
 *
 * param[in] - none
 *
 * return - none
 */

void beep_init(void)
{
    GPIO_InitTypeDef initstruc;

    timer = TIM16;
    __HAL_RCC_TIM16_CLK_ENABLE();

    initstruc.Pin = BEEPER_PIN;
    initstruc.Mode = GPIO_MODE_AF_PP;
//    initstruc.Mode = GPIO_MODE_OUTPUT_PP; //zzz
    initstruc.Pull = GPIO_NOPULL;
    initstruc.Speed = GPIO_SPEED_FREQ_LOW;
    initstruc.Alternate = GPIO_AF2_TIM16;
    HAL_GPIO_Init(BEEPER_PORT, &initstruc);

//    HAL_GPIO_WritePin(BEEPER_PORT, BEEPER_PIN, GPIO_PIN_SET);



    timer->PSC = 47;
    timer->ARR = 999;
    timer->CCR1 = 500;
    timer->RCR = 0;
    timer->CCMR1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1PE;
    timer->CCER = TIM_CCER_CC1E;
    timer->BDTR = TIM_BDTR_MOE;
    timer->CR1 = TIM_CR1_CEN;
    timer->EGR = TIM_EGR_UG;

    return;
}


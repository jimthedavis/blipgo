
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
 * This mocule contains the beeper handler.  It contains a couple of APIs
 * so that outside modules can manipulate the beeper.  They are documented
 * as they appear.
 */

/***************************************************************************
 *                              INCLUDES
 **************************************************************************/

#include "stdio.h"
#include "stdlib.h"
#include "main.h"

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

static uint8_t beepison;
static uint8_t numcycles;

static uint32_t offperiod;
static uint32_t onperiod;
static uint32_t beeptimer;
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
 * This subroutine turns on the beeper for a set time then turns it off.
 * It is meant to be used before the task loop is executed in cmmain.  The
 * caller supplies the no of msecs to have the beeper on.  This subroutine
 * does not return until that time expires.
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
 * This subroutine is called during power up initialization to initialize
 * the beeper hardware and variables.
 *
 * param[in] - none
 *
 * return - none
 */

void beep_init(void)
{
    GPIO_InitTypeDef initstruc;

    timer = TIM16;
    beepison = 0;
    numcycles = 0;
    beeptimer = 0;
    __HAL_RCC_TIM16_CLK_ENABLE();

    initstruc.Pin = BEEPER_PIN;
    initstruc.Mode = GPIO_MODE_AF_PP;
    initstruc.Pull = GPIO_NOPULL;
    initstruc.Speed = GPIO_SPEED_FREQ_LOW;
    initstruc.Alternate = GPIO_AF2_TIM16;
    HAL_GPIO_Init(BEEPER_PORT, &initstruc);

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

/***************************************************************************
 *                         beep_set
 *                         --------
 *
 * The purpose of this subroutine is to provide a means for outside
 * code to manipulate the beeper.  It is called with the desired
 * parameters.  We then use them to set parameters for the beeper task
 * which will then control the beeper.
 *
 * param[in] - numcycs - # of beep cycles to execute, 0x80 means cycle forever
 * param[in] - ontim - the on period in 1/10 secs
 * param[in] - offtim - the off period in 1/10 secs
 *
 * return - none
 *
 * When we exit, the beeper task will be set up to turn the beeper on and
 * off using the periods inputted.  It will do this numcycs times.  If
 * this subroutine is called while a previous beeper cycle is running the
 * previous syscle will be overridden and it starts over again with the
 * new parameters supplied by this subroutine.
 */

void beep_set(uint8_t numcycs, uint8_t ontim, uint8_t offtim)
{
    timer->BDTR = 0;
    timer->DIER = 0;
    HAL_GPIO_WritePin(BUZZOR_GPIO_Port,BUZZOR_Pin,GPIO_PIN_RESET);
    beepison = 0;
    beeptimer = 0;
    offperiod = (uint32_t)offtim * 100;
    onperiod = (uint32_t)ontim * 100;
    numcycles = numcycs;
    return;
}

/***************************************************************************
 *                         beep_task
 *                         ---------
 *
 * This subroutine is called from the task loop in cmmain.  It turns the
 * beeper on and off in accordance with the parameters set by the
 * beep_set subroutine.
 *
 * param[in] - none
 *
 * return - none
 */

void beep_task(void)
{

    /*
     * If the cycles counter is expired just return.
     */

    if (numcycles)
    {

        /*
         * The beeper timer is decremented by the timer interrupt
         * handler.  When it goes to zero we then switch the beeper
         * on or off depending on its last state.
         */

        if (beeptimer == 0)
        {

            /*
             * If the beeper was/is on set the
             * beeper timer to the off msecs that was determined
             * by the last cal to beep_set.  We also decrement the cycle
             * counter when the beeper is turned off.  The timr int
             * handler turns the beeper off when the timer expires.
             */

            if (beepison)
            {
                beeptimer = offperiod;
                beepison = 0;

                if ((numcycles & 0x80) == 0)
                {
                    numcycles--;
                }

            }

            /*
             * If the beeper was off turn it on.
             */

            else
            {
                HAL_GPIO_WritePin(BUZZOR_GPIO_Port,BUZZOR_Pin,GPIO_PIN_SET);
                timer->BDTR = TIM_BDTR_MOE;
                timer->CR1 = TIM_CR1_CEN;
                timer->EGR = TIM_EGR_UG;
                timer->CCR1 = 500;
                beeptimer = onperiod;
                beepison = 1;
            }

        }

    }

    return;
}

/***************************************************************************
 *                         beep_timer_ih
 *                         -------------
 *
 * This subroutine is called by the timer interrupt handler every 1 ms.
 * We decrement the local timer variable until it is 0.  The timer
 * is used to time beeper on & off times..
 *
 * \param[in] - none
 *
 * \return - none
 */

void beep_timer_ih(void)
{

    if (beeptimer)
    {
        beeptimer--;

        if (beeptimer == 0)
        {

            if (beepison)
            {
                timer->BDTR = 0;
                timer->DIER = 0;
                HAL_GPIO_WritePin(BUZZOR_GPIO_Port,BUZZOR_Pin,GPIO_PIN_RESET);
            }

        }

    }

    return;
}

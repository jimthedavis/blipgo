
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
 * Module: quectel.c
 * Author: J Davis
 * Date: November 11, 2021
 *
 ***************************************************************************
 */

/***************************************************************************
 *                              INCLUDES
 **************************************************************************/

#include "stdio.h"
#include "stdlib.h"
#include "main.h"
#include <cmglobals.h>

/***************************************************************************
 *                               DEFINES
 **************************************************************************/



#define GSM_DMA DMA1
#define GSM_DMA_CHAN DMA1_Channel4

#define RXINTBUFLEN 512

#define TS_INIT 0
#define TS_IDLE 1
#define TS_CR1 2
#define TS_MESSAGE 3
#define TS_RXOVFLOW 4
#define TS_COMPL 5
#define TS_TIMEOUT 6

/***************************************************************************
 *                               TYPEDEFS
 **************************************************************************/

/***************************************************************************
 *                            GLOBAL VARIABLES
 **************************************************************************/

/***************************************************************************
 *                             LOCAL VARIABLES
 **************************************************************************/

static uint8_t output_busy;
static uint8_t taskstate;
static uint8_t taskoldstate;
static uint8_t input_busy;
static uint8_t input_status;

static uint32_t rxget;
static uint32_t rxput;
static uint32_t timer1;
static uint32_t timer2;
static uint32_t txbuflen;
static uint32_t rxbufindex;
static uint32_t rxbuflen;
static uint32_t rxtimeout;
static void(*rxcomplih)(uint8_t, uint32_t);

static uint8_t *rxbufaddr;
static uint8_t *txbufaddr;

static uint8_t rxintbuf[RXINTBUFLEN];

/***************************************************************************
 *                         LOCAL FUNCTION PROTOTYPES
 **************************************************************************/

static void set_timer(uint32_t);

/***************************************************************************
 *                             GLOBAL FUNCTIONS
 **************************************************************************/

/***************************************************************************
 *                         quec_3v8enbl
 *                         ------------
 *
 *
 */

void quec_3v8enbl(uint8_t onoff)
{

    if (onoff)
    {
        HAL_GPIO_WritePin(GSM_ENBL_PORT, GSM_ENBL_PIN, GPIO_PIN_SET);
    }

    else
    {
        HAL_GPIO_WritePin(GSM_ENBL_PORT, GSM_ENBL_PIN, GPIO_PIN_RESET);
    }

    return;
}

/***************************************************************************
 *                         quec_init
 *                         ---------
 *
 *
 */

void quec_init(void)
{
    GPIO_InitTypeDef initstruc;
    UART_HandleTypeDef huart;
    HAL_StatusTypeDef halstat;

    initstruc.Pin = GSM_PWR_PIN;
    initstruc.Mode = GPIO_MODE_OUTPUT_PP;
    initstruc.Pull = GPIO_NOPULL;
    initstruc.Speed = GPIO_SPEED_FREQ_LOW;
    initstruc.Alternate = 0;
    HAL_GPIO_Init(GSM_PWR_PORT, &initstruc);

    initstruc.Pin = GSM_RESET_PIN;
    HAL_GPIO_Init(GSM_RESET_PORT, &initstruc);

    initstruc.Pin = GSM_ENBL_PIN;
    HAL_GPIO_Init(GSM_ENBL_PORT, &initstruc);

    initstruc.Pin = GSM_DTR_PIN;
    initstruc.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(GSM_DTR_PORT, &initstruc);

    HAL_GPIO_WritePin(GSM_PWR_PORT, GSM_PWR_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GSM_ENBL_PORT, GSM_ENBL_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GSM_RESET_PORT, GSM_RESET_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GSM_DTR_PORT, GSM_DTR_PIN, GPIO_PIN_RESET);

    initstruc.Pin = GSM_UART_TX_PIN;
    initstruc.Mode = GPIO_MODE_AF_PP;
    initstruc.Speed = GPIO_SPEED_FREQ_HIGH;
    initstruc.Alternate = GSM_UART_TX_AF;
    HAL_GPIO_Init(GSM_UART_TX_PORT, &initstruc);           /* COMMENT OUT FOR STEVE */

    initstruc.Pin = GSM_UART_RX_PIN;
    initstruc.Mode = GPIO_MODE_AF_PP;
    initstruc.Alternate = GSM_UART_RX_AF;
    HAL_GPIO_Init(GSM_UART_RX_PORT, &initstruc);           /* COMMENT OUT FOR STEVE */

    __HAL_RCC_USART2_CLK_ENABLE();                         /* COMMENT OUT FOR STEVE */

    huart.Instance = GSM_UART;
    huart.Init.BaudRate = 115200;
    huart.Init.WordLength = UART_WORDLENGTH_8B;
    huart.Init.StopBits = UART_STOPBITS_1;
    huart.Init.Parity = UART_PARITY_NONE;
    huart.Init.Mode = UART_MODE_TX_RX;
    huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart.Init.OverSampling = UART_OVERSAMPLING_16;
    huart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    halstat = HAL_UART_Init(&huart);                       /* COMMENT OUT FOR STEVE */

    if (halstat != HAL_OK)
    {
//        crash();
    }

    GSM_DMA_CHAN->CPAR = (uint32_t)&GSM_UART->TDR;             /* COMMENT OUT FOR STEVE */

    taskstate = TS_INIT;
    taskoldstate = 255;
    output_busy = 0;
    set_timer(0);
    rxget = 0;
    rxput = 0;
    HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);                     /* COMMENT OUT FOR STEVE */
    HAL_NVIC_EnableIRQ(USART2_IRQn);                            /* COMMENT OUT FOR STEVE */
    GSM_UART->CR1 |= USART_CR1_RXNEIE;                          /* COMMENT OUT FOR STEVE */
    return;
}

/***************************************************************************
 *                         quec_task
 *                         ---------
 *
 *
 */

void quec_task(void)
{
    uint8_t tempchar;

    if (taskstate != taskoldstate)
    {
//        debug_printf(DBGLVL_MAX, (uint8_t *)"QUEC STATE: %d -> %d\r\n", taskoldstate, taskstate);
    }

    taskoldstate = taskstate;

//            HAL_GPIO_WritePin(GSM_PWR_PORT, GSM_PWR_PIN, GPIO_PIN_SET);
//            HAL_Delay(1500);
//            HAL_GPIO_WritePin(GSM_PWR_PORT, GSM_PWR_PIN, GPIO_PIN_RESET);

    switch(taskstate)
    {

        case TS_INIT:
        {

            rxget = 0;
            rxput = 0;
            rxbufindex = 0;
            taskstate = TS_IDLE;
            break;
        }

        case TS_IDLE:
        {

//        break;                      /* UNCOMMENT OUT FOR STEVE */
            tempchar = gsm_answer();

			if ((tempchar == CA_NONE) || (tempchar == CA_NOTPOWERED))
		    {
			    break;
			}

            if (input_busy == 0)
            {
                break;
            }

            if (rxget != rxput)
            {

                set_timer(rxtimeout);
                tempchar = rxintbuf[rxget];
                rxget++;
                rxget &= RXINTBUFLEN - 1;
                rxbufindex = 0;

                if (tempchar == 0x0D)
                {
                    taskstate = TS_CR1;
                }

                else
                {
                    rxbufaddr[rxbufindex] = tempchar;
                    rxbufindex++;
                }

                if (rxbufindex == rxbuflen)
                {
                    taskstate = TS_RXOVFLOW;
                }

                else
                {
                    taskstate = TS_MESSAGE;
                }

            }

            else
            {

                if (timer1 == 0)
                {
                    taskstate = TS_TIMEOUT;
                }

            }

            break;
        }

        case TS_CR1:
        {

            if (rxget != rxput)
            {
                set_timer(rxtimeout);
                tempchar = rxintbuf[rxget];
                rxget++;
                rxget &= RXINTBUFLEN - 1;

                if (tempchar == 0x0D)
                {
                	break;
                }

                rxbufaddr[rxbufindex] = tempchar;
                rxbufindex++;

                if (rxbufindex == rxbuflen)
                {
                    taskstate = TS_RXOVFLOW;
                }

                else
                {
                    taskstate = TS_MESSAGE;
                }

            }

            else
            {

                if (timer1 == 0)
                {
                    taskstate = TS_TIMEOUT;
                }

            }

            break;
        }

        case TS_MESSAGE:
        {

            if (rxget != rxput)
            {
                set_timer(rxtimeout);

                while (rxget != rxput)
                {
                    tempchar = rxintbuf[rxget];
                    rxget++;
                    rxget &= RXINTBUFLEN - 1;
                    rxbufaddr[rxbufindex] = tempchar;
                    rxbufindex++;

                    if (tempchar == 0x0D)
                    {
                        taskstate = TS_COMPL;
                	    break;
                    }


                    if (rxbufindex == rxbuflen)
                    {
                        timer2 = 1000;
                        taskstate = TS_RXOVFLOW;
                        break;
                    }

                }

                break;
            }

            else
            {

                if (timer1 == 0)
                {
                    taskstate = TS_TIMEOUT;
                }

            }

            break;
        }

        case TS_COMPL:
        {
            input_busy = 0;
            input_status = QS_OK;
            taskstate = TS_IDLE;
            rxbufaddr[rxbufindex] = 0x00;
            debug_printf(DBGLVL_MAX, (uint8_t *)"QUEC IN: %s\r\n", rxbufaddr);

            if (rxcomplih != NULL)
            {
                rxcomplih(input_status, rxbufindex);
            }

            break;
        }

        case TS_RXOVFLOW:
        {

            if (rxget != rxput)
            {
                set_timer(rxtimeout);
                tempchar = rxintbuf[rxget];
                rxget++;
                rxget &= RXINTBUFLEN - 1;

                if (tempchar == 0x0D)
                {
                    input_status = QS_OVERFLOW;
                    rxbufaddr[rxbufindex] = 0x00;
                    debug_printf(DBGLVL_MAX, (uint8_t *)"QUEC IN: %s\r\n", rxbufaddr);

                    if (rxcomplih != NULL)
                    {
                        rxcomplih(input_status, rxbufindex);
                    }

                    input_busy = 0;
                    taskstate = TS_IDLE;
                    break;
                }

                if (timer2 == 0)
                {
                    input_status = QS_STREAMING;

                    if (rxcomplih != NULL)
                    {
                        rxcomplih(input_status, rxbufindex);
                    }

                    input_busy = 0;
                    taskstate = TS_IDLE;
                    break;
                }

            }

            else
            {

            	if (timer1 == 0)
            	{
                    input_status = QS_OVERFLOW;

                    if (rxcomplih != NULL)
                    {
                        rxcomplih(input_status, rxbufindex);
                    }

                    input_busy = 0;
                    taskstate = TS_IDLE;
                }

            }

            break;
        }


        case TS_TIMEOUT:
        {
            input_busy = 0;
            input_status = QS_TIMEOUT;
            rxbufaddr[rxbufindex] = 0x00;
            debug_printf(DBGLVL_MAX, (uint8_t *)"QUEC IN/TO: %s\r\n", rxbufaddr);

            if (rxcomplih != NULL)
            {
                rxcomplih(input_status, rxbufindex);
            }

            taskstate = TS_IDLE;
            break;
        }

        default:
        {
            crash();
        }

    }

    return;
}

/***************************************************************************
 *                         quec_outcompl
 *                         ------------
 *
 *
 */

uint32_t quec_outcompl(void)
{
    uint32_t stat;

    if (output_busy)
    {
        stat = 0;
    }

    else
    {
        stat = 1;
    }

    return stat;
}

/***************************************************************************
 *                         quec_receive
 *                         ------------
 *
 *
 */

uint8_t quec_receive(uint8_t *ibuf, uint16_t ilen, uint32_t timeout, void(*complih)(uint8_t, uint32_t))
{
    uint8_t retstat;

    if (input_busy)
    {
        retstat = QS_RXBUSY;
    }

    if (ilen == 0)
    {
        rxbuflen = 0;
        retstat = QS_OK;
    }

    else if (ibuf == 0)
    {
        crash();
    }

    else
    {
        rxbufaddr = ibuf;
        rxbuflen = ilen;
        rxbufindex = 0;
        rxtimeout = timeout;
        rxcomplih = complih;
        set_timer(rxtimeout);
        input_busy = 1;
        retstat = QS_INPROGRESS;
    }

    debug_printf(DBGLVL_MAX, (uint8_t *)"QUEC RECEIVE: %d\r\n", retstat);
    return retstat;
}



/***************************************************************************
 *                         quec_rxflush
 *                         ------------
 *
 *
 */

void quec_rxflush(void)
{
    rxget = rxput;
    input_busy = 0;
    return;
}

/***************************************************************************
 *                         quec_transmit
 *                         -------------
 *
 *
 */

uint8_t quec_transmit(uint8_t *obuf, uint16_t olen)
{
    uint8_t retstat;

    debug_printf(DBGLVL_MAX, (uint8_t *)"QUEC OUT(%d): ", olen);
    debug_write_fixed(DBGLVL_MAX, obuf, olen);
    debug_printf(DBGLVL_MAX, (uint8_t *)"\r\n");

    if (olen == 0)
    {
           retstat = QS_OK;
    }

    else if (output_busy)
    {
           retstat = QS_TXBUSY;
    }

    else if (obuf == 0)
    {
        crash();
    }

    else
    {
        output_busy = 1;
        txbufaddr = obuf;
        txbuflen = olen;
        GSM_DMA_CHAN->CMAR = (uint32_t)obuf;
        GSM_DMA_CHAN->CNDTR = olen;
        GSM_UART->CR3 |= USART_CR3_DMAT;
        GSM_DMA_CHAN->CCR =  DMA_CCR_EN | DMA_CCR_TCIE | DMA_CCR_DIR | DMA_CCR_MINC | DMA_CCR_PSIZE_0;
        retstat = QS_INPROGRESS;
    }

    return retstat;
}

/***************************************************************************
 *                         quec_dma_ih
 *                         -----------
 *
 *
 */

void quec_dma_ih(void)
{
    GSM_UART->CR3 &= ~USART_CR3_DMAT;
    GSM_DMA_CHAN->CCR = 0;
    GSM_DMA->IFCR = DMA_IFCR_CTCIF4;
    output_busy = 0;
    return;
}

/***************************************************************************
 *                         quec_timer_ih
 *                         -------------
 *
 *
 */

void quec_timer_ih(void)
{

    if (timer1)
    {
        timer1--;
    }

    if (timer2)
    {
        timer2--;
    }

    return;
}

/***************************************************************************
 *                         quec_uart_ih
 *                         ------------
 *
 *
 */

void quec_uart_ih(void)
{
    uint16_t inchar;
    uint32_t intstat;

    intstat = GSM_UART->ISR;

    if (intstat & USART_ISR_RXNE)
    {
        inchar = GSM_UART->RDR;

        if (inchar != 0x000A)
        {
            rxintbuf[rxput] = (uint8_t)inchar;
            rxput++;
            rxput &= RXINTBUFLEN - 1;
        }

        return;
    }

    GSM_UART->ICR = 0x0002055F;
    GSM_UART->CR1 &= ~(USART_CR1_IDLEIE | USART_CR1_TCIE | USART_CR1_PEIE | USART_CR1_CMIE | USART_CR1_RTOIE);
    return;
}

/***************************************************************************
 *                             LOCAL FUNCTIONS
 **************************************************************************/

/***************************************************************************
 *                         set_timer
 *                         ---------
 *
 *
 */

static void set_timer(uint32_t newtime)
{
    __disable_irq();
    timer1 = newtime;
    __enable_irq();
    return;
}

/*
 * End of module.
 */

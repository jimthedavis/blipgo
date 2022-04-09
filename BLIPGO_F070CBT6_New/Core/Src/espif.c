
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



#define RXINTBUFLEN 512

#define TS_INIT 0
#define TS_IDLE 1
#define TS_CR1 2
#define TS_MESSAGE 3
#define TS_RXOVFLOW 4
#define TS_COMPL 5
#define TS_TIMEOUT 6
#define TS_NOTIND 7
#define TS_NOTINDCOUNT 8
#define TS_NOTINDDATA 9
#define TS_NOTINDEND 10

/***************************************************************************
 *                               TYPEDEFS
 **************************************************************************/

/***************************************************************************
 *                            GLOBAL VARIABLES
 **************************************************************************/

uint8_t esp_ready_flag;

/***************************************************************************
 *                             LOCAL VARIABLES
 **************************************************************************/

static uint8_t output_busy;
static uint8_t taskstate;
static uint8_t taskoldstate;
static uint8_t input_busy;
static uint8_t input_status;
static uint8_t commacount;

static uint16_t notindcount;


static uint32_t rxget;
static uint32_t rxput;
static uint32_t timer1;
static uint32_t timer2;
static uint32_t txbuflen;
static uint32_t txbufdex;
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

    esp_ready_flag = 0;
    initstruc.Pin = ESP_PWR_PIN;
    initstruc.Mode = GPIO_MODE_OUTPUT_PP;
    initstruc.Pull = GPIO_NOPULL;
    initstruc.Speed = GPIO_SPEED_FREQ_LOW;
    initstruc.Alternate = 0;
    HAL_GPIO_Init(ESP_PWR_PORT, &initstruc);

    initstruc.Pin = ESP_ENBL_PIN;
    HAL_GPIO_Init(ESP_ENBL_PORT, &initstruc);

    HAL_GPIO_WritePin(ESP_ENBL_PORT, ESP_ENBL_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(ESP_PWR_PORT, ESP_PWR_PIN, GPIO_PIN_SET);


    initstruc.Pin = ESP_UART_TX_PIN;
    initstruc.Mode = GPIO_MODE_AF_PP;
    initstruc.Speed = GPIO_SPEED_FREQ_HIGH;
    initstruc.Alternate = ESP_UART_TX_AF;
    HAL_GPIO_Init(ESP_UART_TX_PORT, &initstruc); /* comment out for Steve */

    initstruc.Pin = ESP_UART_RX_PIN;
    initstruc.Mode = GPIO_MODE_AF_PP;
    initstruc.Alternate = ESP_UART_RX_AF;
    HAL_GPIO_Init(ESP_UART_RX_PORT, &initstruc);  /* comment out for Steve */

    __HAL_RCC_USART1_CLK_ENABLE();  /* comment out for Steve */

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
    halstat = HAL_UART_Init(&huart);  /* comment out for Steve */

    if (halstat != HAL_OK)
    {

//        crash();
    }

    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);                     /* COMMENT OUT FOR STEVE */
    HAL_NVIC_EnableIRQ(USART1_IRQn);

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
 *                         esp_receive
 *                         -----------
 *
 *
 */

uint8_t esp_receive(uint8_t *ibuf, uint16_t ilen, uint32_t timeout, void(*complih)(uint8_t, uint32_t))
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

//    debug_printf(DBGLVL_MAX, (uint8_t *)"ESP RECEIVE: %d\r\n", retstat);
    return retstat;
}







/***************************************************************************
 *                         esp_reset
 *                         ----------
 *
 *
 */

void esp_reset(void)
{
    esp_ready_flag = 0;
    HAL_GPIO_WritePin(ESP_ENBL_PORT, ESP_ENBL_PIN, GPIO_PIN_SET);
    HAL_Delay(1000);
    rxget = 0;
    rxput = 0;
    rxbufindex = 0;
    HAL_GPIO_WritePin(ESP_ENBL_PORT, ESP_ENBL_PIN, GPIO_PIN_RESET);
    HAL_Delay(1000);
    return;
}

/***************************************************************************
 *                         esp_rxflush
 *                         ------------
 *
 *
 */

void esp_rxflush(void)
{
    rxget = rxput;
    input_busy = 0;
    rxbufindex = 0;
    return;
}



/***************************************************************************
 *                         esp_task
 *                         ---------
 *
 *
 */

void esp_task(void)
{
    uint8_t tempchar;

    if (taskstate != taskoldstate)
    {
//        debug_printf(DBGLVL_MAX, (uint8_t *)"ESP STATE: %d -> %d\r\n", taskoldstate, taskstate);
    }

    taskoldstate = taskstate;



    switch(taskstate)
    {

        case TS_INIT:
        {

// break;
            rxget = 0;
            rxput = 0;
            rxbufindex = 0;
            taskstate = TS_IDLE;
            break;
        }

        case TS_IDLE:
        {

//        break;                      /* UNCOMMENT OUT FOR STEVE */
            tempchar = espat_answer();

			if ((tempchar == CA_NONE) || (tempchar == CA_NOTPOWERED))
			{
			    taskstate = TS_IDLE;
			    break;
			}

            if (input_busy == 0)
            {
                break;
            }

            if (rxget != rxput)
            {
                tempchar = rxintbuf[rxget];
                rxget++;
                rxget &= RXINTBUFLEN - 1;

                if (tempchar < 0x21 )
                {
                    break;
                }

                set_timer(rxtimeout);
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

                if ((tempchar == 0x0D) || (tempchar == 0x0A))
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

                    if (tempchar == 0x0A)
                    {
                        continue;
                    }



                    rxbufaddr[rxbufindex] = tempchar;
                    rxbufindex++;

                    if ( rxbufaddr[0] == 'Y')
                    {
                        tempchar = tempchar;
                    }

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

                    if ((rxbufaddr[0] == '+') && (tempchar == ':'))
                    {

                        if ((rxbufaddr[1] == 'N') && (rxbufaddr[2] == 'O'))
                        {
                            commacount = 0;
                            taskstate = TS_NOTIND;
                            break;
                        }

                        if ((rxbufaddr[1] == 'I') && (rxbufaddr[3] == 'D'))
                        {
                            commacount = 0;
                            taskstate = TS_NOTIND;
                            break;
                        }

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

        case TS_NOTIND:
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

                    if (rxbufindex == rxbuflen)
                    {
                        timer2 = 1000;
                        taskstate = TS_RXOVFLOW;
                        break;
                    }

                    if (tempchar == ',')
                    {
                        commacount++;

                        if (commacount == 3)
                        {
                            notindcount = 0;
                            taskstate = TS_NOTINDCOUNT;
                            break;
                        }

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


        case TS_NOTINDCOUNT:
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

                    if (rxbufindex == rxbuflen)
                    {
                        timer2 = 1000;
                        taskstate = TS_RXOVFLOW;
                        break;
                    }

                    if (tempchar == ',')
                    {
                        taskstate = TS_NOTINDDATA;
                        break;
                    }

                    if ((tempchar < '0') || (tempchar > '9'))
                    {
                        taskstate = TS_MESSAGE;
                        break;
                    }

                    notindcount *= 10;
                    notindcount += tempchar - 0x30;
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



        case TS_NOTINDDATA:
        {

            if (rxget != rxput)
            {
                set_timer(rxtimeout);

                while (rxget != rxput)
                {

                    if (notindcount == 0)
                    {
                        taskstate = TS_NOTINDEND;
                        break;
                    }

                    tempchar = rxintbuf[rxget];
                    rxget++;
                    rxget &= RXINTBUFLEN - 1;
                    rxbufaddr[rxbufindex] = tempchar;
                    rxbufindex++;
                    notindcount--;

                    if (rxbufindex == rxbuflen)
                    {
                        taskstate = TS_NOTINDEND;
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


        case TS_NOTINDEND:
        {

taskstate = TS_COMPL;
break;
            if (rxget != rxput)
            {
                set_timer(rxtimeout);

                while (rxget != rxput)
                {
                    tempchar = rxintbuf[rxget];
                    rxget++;
                    rxget &= RXINTBUFLEN - 1;


                    if (tempchar == 0x0D)
                    {
                        taskstate = TS_COMPL;
                        break;
                    }

                }

                break;
            }

            else
            {

                if (timer1 == 0)
                {
                    taskstate = TS_IDLE;
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
//            debug_printf(DBGLVL_MAX, (uint8_t *)"ESP IN: %d, %s\r\n", rxbufindex, rxbufaddr);

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
            rxbufaddr[rxbufindex] = 0;
            debug_printf(DBGLVL_MAX, (uint8_t *)"ESP IN: %d, %s\r\n", rxbufindex, rxbufaddr);

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
 *                         esp_transmit
 *                         ------------
 *
 *
 */

uint8_t esp_transmit(uint8_t *obuf, uint16_t olen)
{
    uint8_t retstat;
    uint32_t uartreg;

    debug_printf(DBGLVL_MAX, (uint8_t *)"ESP OUT: ");
    usb_write_fixed(obuf, olen);
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
        txbufdex = 0;
        uartreg = ESP_UART->CR1;
        uartreg = uartreg | USART_CR1_TXEIE;
        ESP_UART->CR1 = uartreg;
        retstat = QS_INPROGRESS;
    }

    return retstat;
}





/***************************************************************************
 *                         esp_timer_ih
 *                         ------------
 *
 *
 */

void esp_timer_ih(void)
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
 *                         esp_uart_ih
 *                         -----------
 *
 *
 */

void esp_uart_ih(void)
{

    uint16_t inchar;
    uint32_t intstat;
    uint32_t cr1reg;

    cr1reg = ESP_UART->CR1;
    intstat = ESP_UART->ISR;

    if (intstat & USART_ISR_RXNE)
    {
        inchar = ESP_UART->RDR;
        rxintbuf[rxput] = (uint8_t)inchar;
        rxput++;
        rxput &= RXINTBUFLEN - 1;

        if (rxput == rxget)
        {
            rxput = rxget;
        }

        return;
    }


    if ((intstat & USART_ISR_TXE) && (cr1reg & USART_CR1_TXEIE))
    {

        if (txbufdex == (txbuflen - 1))
        {
            cr1reg = ESP_UART->CR1;
            cr1reg &= ~USART_CR1_TXEIE;
            cr1reg |= USART_CR1_TCIE;
            ESP_UART->CR1 = cr1reg;
        }

        ESP_UART->TDR = txbufaddr[txbufdex];
        txbufdex++;



        return;
    }

    if ((intstat & USART_ISR_TC) && (cr1reg & USART_CR1_TCIE))
    {
        output_busy = 0;
        cr1reg = ESP_UART->CR1;
        cr1reg &= ~USART_CR1_TCIE;
        ESP_UART->CR1 = cr1reg;
        ESP_UART->ICR = USART_ICR_TCCF;
        return;
    }

    cr1reg = ESP_UART->CR1;
    cr1reg &= ~(USART_CR1_IDLEIE | USART_CR1_PEIE | USART_CR1_CMIE | USART_CR1_RTOIE);
    ESP_UART->CR1 = cr1reg;
    ESP_UART->ICR = 0x0002055F;
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


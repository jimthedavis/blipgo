
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
 * Module: usbdebug.c
 * Author: J Davis
 * Date: December 20, 2021
 *
 ***************************************************************************
 */

 /*
  * This module contains the led handler.
  */

/***************************************************************************
 *                              INCLUDES
 **************************************************************************/

#include "stdarg.h"
#include "stdio.h"
#include "stdlib.h"
#include "main.h"
#include "usb_device.h"
#include "usbd_desc.h"
#include "config.h"
#include "usbd_cdc_if.h"
#include <cmglobals.h>

extern PCD_HandleTypeDef hpcd_USB_FS;
extern HAL_StatusTypeDef PCD_EP_ISR_Handler(PCD_HandleTypeDef *);

/***************************************************************************
 *                               DEFINES
 **************************************************************************/

#define USB_MAX_BUF_LEN 64
#define MAX_FSPEC_LEN 256

#define PRS_INIT 0
#define PRS_DONE 1
#define PRS_PERCENT 3
#define PRS_WIDTH 4
#define PRS_PFIELD 5
#define PRS_CSPEC 6
#define PRS_DSPEC 7
#define PRS_SSPEC 8
#define PRS_USPEC 9
#define PRS_XSPEC 10
#define PRS_TYPE 11


/***************************************************************************
 *                               TYPEDEFS
 **************************************************************************/

/***************************************************************************
 *                            GLOBAL VARIABLES
 **************************************************************************/

uint8_t debug_print_ready;
USBD_HandleTypeDef hUsbDeviceFS;

/***************************************************************************
 *                             LOCAL VARIABLES
 **************************************************************************/

static uint8_t zflag;
static uint8_t pfield;
static uint8_t negflag;
static uint8_t width;
static uint32_t prbufdex;
static uint8_t prbuf[USB_MAX_BUF_LEN];
static const uint8_t bin2hex[] = {"0123456789ABCDEF"};

/***************************************************************************
 *                         LOCAL FUNCTION PROTOTYPES
 **************************************************************************/

static void format_du(uint32_t);
static void format_x(uint32_t);
static void store_curchar(uint8_t);

/***************************************************************************
 *                             GLOBAL FUNCTIONS
 **************************************************************************/





/***************************************************************************
 *                         usb_init
 *                         ----------
 *
 * Set yellow led on/off as directedvoid MX_USB_DEVICE_Init(void)
 *
 * \param[in] - ledstate
 *
 * \return - none
 */


void usb_init(void)
{
    USBD_StatusTypeDef stat;

    debug_print_ready = 0;

    stat = USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS);

    if (stat == USBD_OK)
    {
        stat = USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC);

        if (stat == USBD_OK)
        {
            debug_print_ready = 1;
            USBD_Start(&hUsbDeviceFS);
        }

    }

    return;
}

/***************************************************************************
 *                         usb_printf
 *                         ----------
 *
 * Set yellow led on/off as directed
 *
 * \param[in] - ledstate
 *
 * \return - none
 */

void usb_printf(uint8_t *fspec, ... )
{
    uint8_t curchar;
    uint8_t run;
    uint8_t u8spec;
    uint8_t nibble;

    uint32_t fspecdex;
    uint32_t state;
    uint32_t u32spec;
    uint32_t i;

    uint8_t *u8ptrspec;
    va_list args;

    va_start(args, fspec);
    prbufdex = 0;
    fspecdex = 0;
    state = PRS_INIT;
    curchar = 0xFF;

    if (debug_print_ready)
    {
        run = 1;
    }

    else
    {
        run = 0;
    }

    while (run)
    {

        switch (state)
        {

            case PRS_INIT:
            {
                curchar = fspec[fspecdex];
                fspecdex++;

                if ((curchar == 0x00) || (curchar > 0x7E) || (fspecdex > MAX_FSPEC_LEN))
                {
                	state = PRS_DONE;
                }

                else if (curchar == '%')
                {
                    state = PRS_PERCENT;
                }

                else
                {
                    store_curchar(curchar);
                }

                break;
            }

            case PRS_DONE:
            {

                if (prbufdex > 0)
                {
                    usb_write_fixed(prbuf, prbufdex);
                }

                run = 0;
                break;
            }

            case PRS_PERCENT:
            {
                curchar = fspec[fspecdex];
                fspecdex++;
                zflag = 0x00;
                width = 0;
                pfield = 255;
                negflag = 0;

                if ((curchar == 0x00) || (curchar > 0x7E) || (fspecdex > MAX_FSPEC_LEN))
                {
                	state = PRS_DONE;
                }

                else if (curchar == '%')
                {
                    store_curchar('%');
                    state = PRS_INIT;
                }

                else if ((curchar == '0') || (curchar == '+') || (curchar == '-') || (curchar == 0x20) || (curchar == '#'))
                {
                    zflag = curchar;
                    state = PRS_WIDTH;
                }

                else if ((curchar >= '1') && (curchar <= '9'))
                {
                    width = curchar - 0x30;
                    state = PRS_WIDTH;
                }

                else if (curchar == '.')
                {
                	state = PRS_PFIELD;
                }

                else
                {
                    state = PRS_TYPE;
                }

                break;
            }

            case PRS_WIDTH:
            {
                curchar = fspec[fspecdex];
                fspecdex++;

                if ((curchar == 0x00) || (curchar > 0x7E) || (fspecdex > MAX_FSPEC_LEN))
                {
                	state = PRS_DONE;
                }

  				else if ((curchar >= '0') && (curchar <= '9'))
  				{
  				    width *= 10;
  				    width = width + (curchar - 0x30);
  				}

  				else if (curchar == '.')
  				{
  				    pfield = 0;
  				    state = PRS_PFIELD;
  				}

  				else
  				{
  				    state = PRS_TYPE;
  				}

  				break;
            }

            case PRS_PFIELD:
            {
                curchar = fspec[fspecdex];
                fspecdex++;

                if ((curchar == 0x00) || (curchar > 0x7E) || (fspecdex > MAX_FSPEC_LEN))
                {
                	state = PRS_DONE;
                }

                else if ((curchar >= '0') && (curchar <= '9'))
                {
  				    pfield *= 10;
  				    pfield = pfield + (curchar - 0x30);
  				    pfield &= 0x7F;
                }

                else
                {
                    store_curchar(curchar);
                    state = PRS_DONE;
                }

                break;
            }


            case PRS_TYPE:
            {

  				if (curchar == 'c')
  				{
                    state = PRS_CSPEC;
                }

				else if (curchar == 'd')
				{
				    state = PRS_DSPEC;
				}

				else if (curchar == 'u')
				{
				    state = PRS_USPEC;
				}

				else if (curchar == 's')
				{
				    state = PRS_SSPEC;
				}

				else if (curchar == 'X')
				{
				    state = PRS_XSPEC;
				}

				else
				{
				    store_curchar(curchar);
				    state = PRS_INIT;
				}

				break;
            }

            case PRS_CSPEC:
            {
                u8spec = (uint8_t)(va_arg(args, uint32_t));
                store_curchar(u8spec);
                state = PRS_INIT;
                break;
            }

            case PRS_DSPEC:
            {
                u32spec = va_arg(args, uint32_t);

                if (u32spec & 0x80000000)
                {
                    u32spec = -u32spec;
                    negflag = 1;
                }

                format_du(u32spec);
                state = PRS_INIT;
                break;
            }

            case PRS_USPEC:
            {
                u32spec = va_arg(args, uint32_t);
                format_du(u32spec);
                state = PRS_INIT;
                break;
            }

            case PRS_SSPEC:
            {
                u8ptrspec = (uint8_t *)(va_arg(args, uint32_t));
                i = 0;

                while (1)
                {

                    if ((u8ptrspec[i] == 0x00) || ((width > 0) && (i >= width)))
                    {
                        break;
                    }

                    if ((u8ptrspec[i] >= 0x20) && (u8ptrspec[i] < 0x7F))
                    {
                        store_curchar(u8ptrspec[i]);
                    }

                    else
                    {
                        store_curchar('<');
                        nibble = u8ptrspec[i] >> 4;
                        store_curchar(bin2hex[nibble]);
                        nibble = u8ptrspec[i] & 0x0F;
                        store_curchar(bin2hex[nibble]);
                        store_curchar('>');
                    }

                    i++;
                }

                state = PRS_INIT;
                break;
            }

            case PRS_XSPEC:
            {
                u32spec = va_arg(args, uint32_t);
                format_x(u32spec);
                state = PRS_INIT;
                break;
            }

            default:
            {
                crash();
            }

        }

    }

    va_end(args);
    return;
}


/***************************************************************************
 *                         usb_print_ih
 *                         ------------
 *
 * Set yellow led on/off as directedvoid HAL_PCD_IRQHandler(PCD_HandleTypeDef *hpcd)
 *
 * \param[in] - nonevoid USB_IRQHandler(void)
 *
 * \return - none
 */

void usb_print_ih(void)
{
    uint16_t intstat;

    intstat = USB->ISTR;

    /* servicing of the endpoint correct transfer interrupt */
    /* clear of the CTR flag into the sub */

    if (intstat & USB_ISTR_CTR)
    {
        PCD_EP_ISR_Handler(&hpcd_USB_FS);
    }

    if (intstat & USB_ISTR_RESET)
    {
        HAL_PCD_ResetCallback(&hpcd_USB_FS);
        HAL_PCD_SetAddress(&hpcd_USB_FS, 0U);
    }

    if (intstat & USB_ISTR_ERR)
    {
        debug_print_ready = 0;
        USB->CNTR &= 0x007F;
    }

    if (intstat & USB_ISTR_WKUP)
    {
        USB->CNTR &= (uint16_t) ~(USB_CNTR_LPMODE);
        USB->CNTR &= (uint16_t) ~(USB_CNTR_FSUSP);

        if (hpcd_USB_FS.LPM_State == LPM_L1)
        {
            hpcd_USB_FS.LPM_State = LPM_L0;

            HAL_PCDEx_LPM_Callback(&hpcd_USB_FS, PCD_LPM_L0_ACTIVE);
            HAL_PCD_ResumeCallback(&hpcd_USB_FS);
        }

    }

    if (intstat & USB_ISTR_SUSP)
    {
        USB->CNTR |= (uint16_t)USB_CNTR_FSUSP;
        USB->CNTR |= (uint16_t)USB_CNTR_LPMODE;
        HAL_PCD_SuspendCallback(&hpcd_USB_FS);
    }

  /* Handle LPM Interrupt */
    if (intstat & USB_ISTR_L1REQ)
    {

        /* Force suspend and low-power mode before going to L1 state*/

        if (&hpcd_USB_FS.LPM_State == LPM_L0)
        {
            USB->CNTR |= (uint16_t)USB_CNTR_LPMODE;
            USB->CNTR |= (uint16_t)USB_CNTR_FSUSP;
            hpcd_USB_FS.LPM_State = LPM_L1;
            hpcd_USB_FS.BESL = ((uint32_t)hpcd_USB_FS.Instance->LPMCSR & USB_LPMCSR_BESL) >> 2;
            HAL_PCDEx_LPM_Callback(&hpcd_USB_FS, PCD_LPM_L1_ACTIVE);
        }

        else
        {
            HAL_PCD_SuspendCallback(&hpcd_USB_FS);
        }

    }

    if (intstat & USB_ISTR_SOF)
    {
        HAL_PCD_SOFCallback(&hpcd_USB_FS);
    }

    USB->ISTR = ~0x7F80;
    return;
}



/***************************************************************************
 *                         usb_task
 *                         --------
 *
 * Set yellow led on/off as directed
 *
 * \param[in] - none
 *
 * \return - none
 */

void usb_task(void)
{

#if USB_ENABLED != 0

#endif

    return;
}

/***************************************************************************
 *                         usb_write
 *                         ---------
 *
 * Set yellow led on/off as directed
 *
 * \param[in] - ledstate
 *
 * \return - none
 */

void usb_write(uint8_t *obuf)
{
#if USB_ENABLED != 0
    uint32_t i;

    for (i = 0; i < USB_MAX_BUF_LEN; i++)
    {

    	if (obuf[i] == 0x00)
        {
            break;
        }

    }

    usb_write_fixed(obuf, i);
#endif

    return;
}

/***************************************************************************
 *                         usb_write_fixed
 *                         ---------------
 *
 * Set yellow led on/off as directed
 *
 * \param[in] - ledstate
 *
 * \return - none
 */

void usb_write_fixed(uint8_t *obuf, uint32_t len)
{
#if USB_ENABLED != 0
    uint8_t stat;
    uint32_t wrtlen;
    uint32_t i;
#endif

#if USB_ENABLED != 0
    wrtlen = len;

    if (wrtlen > USB_MAX_BUF_LEN)
    {
        wrtlen = USB_MAX_BUF_LEN;
    }

    for (i = 0; i < 10; i++)
    {
        stat = CDC_Transmit_FS(obuf, wrtlen);

        if (stat != USBD_BUSY)
        {
        	break;
        }

        HAL_Delay(20);
    }

#endif

    return;
}

/***************************************************************************
 *                             LOCAL FUNCTIONS
 **************************************************************************/

/***************************************************************************
 *                         format_du
 *                         ---------
 *
 * Set yellow led on/off as directed
 *
 * \param[in] - ledstate
 *
 * \return - none
 */


static void format_du(uint32_t binval)
{
    uint8_t signchar;
    uint32_t i;
    uint32_t numdigits;
    uint32_t remain;
    uint32_t outdex;
    uint32_t signcount;
    uint8_t ascdec[10];

    signcount = 0;
    outdex = 10;
    remain = binval;

    if (remain == 0)
    {
        numdigits = 1;
        ascdec[9] = '0';
        outdex = 9;
    }

    else
    {

        while (remain != 0)
        {
            outdex--;
            ascdec[outdex] = (remain % 10) + '0';
            remain = remain / 10;
        }

        numdigits = 10 - outdex;
    }

    if (negflag)
    {
        signcount = 1;
        signchar = '-';
    }

    else if (zflag == '+')
    {
        signcount = 1;
        signchar = '+';
    }

    if ((zflag == '0') && (width > (numdigits + signcount)))
    {

        if (signcount)
        {
            store_curchar(signchar);
        }

        for (i = 0; i < (width - (numdigits + signcount)); i++)
        {
            store_curchar('0');
        }

    }

    if ((zflag != '0') && (width > (numdigits + signcount)))
    {

        for (i = 0; i < (width - (numdigits + signcount)); i++)
        {
            store_curchar(' ');
        }

        if (signcount)
        {
            store_curchar(signchar);
        }

    }

    for (i = 0; i < numdigits; i++)
    {
        store_curchar(ascdec[outdex + i]);
    }

    return;
}

/***************************************************************************
 *                         format_x
 *                         --------
 *
 * Set yellow led on/off as directed
 *
 * \param[in] - ledstate
 *
 * \return - none
 */


static void format_x(uint32_t binval)
{
    uint32_t i;
    uint32_t numdigits;
    uint32_t remain;
    uint32_t outdex;
    uint8_t aschex[8];

    outdex = 8;
    remain = binval;

    if (remain == 0)
    {
        numdigits = 1;
        aschex[7] = '0';
        outdex = 7;
    }

    else
    {

        while (remain != 0)
        {
            outdex--;
            aschex[outdex] = (remain & 0x0F);
            remain = remain >> 4;

            if (aschex[outdex] < 10)
            {
                aschex[outdex] += '0';
            }
            else
            {
                aschex[outdex] += ('A' - 10);
            }

        }

        numdigits = 8 - outdex;
    }

    if ((zflag == '0') && (width > numdigits))
    {

        for (i = 0; i < (width - numdigits); i++)
        {
            store_curchar('0');
        }

    }

    if ((zflag != '0') && (width > numdigits))
    {

        for (i = 0; i < (width - numdigits); i++)
        {
            store_curchar(' ');
        }

    }

    for (i = 0; i < numdigits; i++)
    {
        store_curchar(aschex[outdex + i]);
    }

    return;
}

/***************************************************************************
 *                         store_curchar
 *                         -------------
 *
 * Set yellow led on/off as directed
 *
 * \param[in] - ledstate
 *
 * \return - none
 */

static void store_curchar(uint8_t nxtchar)
{
    prbuf[prbufdex] = nxtchar;
    prbufdex++;

    if (prbufdex >= USB_MAX_BUF_LEN)
    {
        usb_write_fixed(prbuf, USB_MAX_BUF_LEN);
        prbufdex = 0;
    }

    return;
}

/*
 * End of module.
 */


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
 * Module: gsm.c
 * Author: J Davis
 * Date: January 6, 2022
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

#define S_INIT 0
#define S_WTAT 1
#define S_CHKRSP1 2
#define S_IDLE 3
#define S_WTRDY 4
#define S_CHKRSP1A 5
#define S_WTRDYOK 6
#define S_CHKRXRDY 7
#define S_INIT2 8
#define S_INIT3 9
#define S_INIT3WT 10
#define S_CHKINITMSG 11
#define S_WTAPN 12
#define S_CHKRSPAPN 13
#define S_WTQURCCFG 14
#define S_CHKRSPQURCCFG 15
#define S_WTQCFG 16
#define S_CHKRSPQCFG 17
#define S_WTQCFG2 18
#define S_CHKRSPQCFG2 19
#define S_WTQCFG3 20
#define S_CHKRSPQCFG3 21
#define S_WTGSN 22
#define S_CHKRSPGSN 23
#define S_WTCIMI 24
#define S_CHKRSPCIMI 25
#define S_WTQCCID 26
#define S_CHKRSPQCCID 27





#define MAX_RECEIVE_LEN 31

/***************************************************************************
 *                               TYPEDEFS
 **************************************************************************/

/***************************************************************************
 *                         LOCAL FUNCTION PROTOTYPES
 **************************************************************************/

static uint32_t e_always(void);
static uint32_t e_quecready(void);
static uint32_t e_rcverr(void);
static uint32_t e_rcvok(void);
static uint32_t e_rcvtimout(void);
static uint32_t e_rxat(void);
static uint32_t e_rxccid(void);
static uint32_t e_rxcpinr(void);
static uint32_t e_rximei(void);
static uint32_t e_rximsi(void);
static uint32_t e_rxok(void);
static uint32_t e_rxrdy(void);
static uint32_t e_timeout(void);


static void a_flushrx(void);
static void a_gotcpinr(void);
static void a_gotrdy(void);
static void a_initvars(void);
static void a_nop(void);
static void a_rcvdata(void);
static void a_rcvinitmsgs(void);
static void a_reset(void);
static void a_saveccid(void);
static void a_saveimei(void);
static void a_saveimsi(void);
static void a_trapn(void);
static void a_trcimi(void);
static void a_trgsn(void);
static void a_trmsg1(void);
static void a_trqccid(void);
static void a_trqcfg(void);
static void a_txate0(void);

#if INDIA != 0
static void a_trqcfg2(void);
static void a_trqcfg3(void);
#endif

static void a_trqurccfg(void);
static void a_tmr5sec(void);


static uint32_t compare(uint32_t, uint8_t *);
static void gsm_rcv_ih(uint8_t, uint32_t);
static void state_trace(uint32_t);




/***************************************************************************
 *                            GLOBAL VARIABLES
 **************************************************************************/

SM_STRUC gsm_stmachine;

/***************************************************************************
 *                             LOCAL VARIABLES
 **************************************************************************/


static const S_TABLE st_init[] = {{&e_quecready, &a_tmr5sec, &a_initvars, S_INIT2},
                                  {&e_always, NULL, &a_nop, S_INIT}};

static const S_TABLE st_init2[] = {{&e_timeout, &a_nop, &a_nop, S_INIT3},
                                   {&e_always, NULL, &a_nop, S_INIT2}};

static const S_TABLE st_init3[] = {{&e_always, &a_rcvinitmsgs, &a_nop, S_INIT3WT}};

static const S_TABLE st_init3wt[] = {{&e_rcvok, &a_nop, &a_nop, S_INIT3}, /* chkinitmsg */
                                     {&e_rcvtimout, &a_txate0, NULL, S_INIT3},  /* zzz */
                                     {&e_rcvtimout, &a_trmsg1, &a_nop, S_WTAT},
                                     {&e_rcverr, &a_flushrx, &a_reset, S_INIT},
                                     {&e_always, NULL, &a_nop, S_INIT3WT}};

static const S_TABLE st_chkinitmsg[] = {{&e_rxrdy, &a_gotrdy, &a_nop, S_INIT3},
                                        {&e_rxcpinr, &a_gotcpinr, &a_nop, S_INIT3},
                                        {&e_always, &a_nop, &a_nop, S_INIT3}};

static const S_TABLE st_wtrdy[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRXRDY},
                                   {&e_rcvtimout, &a_trapn, &a_nop, S_WTAT},   /* zz */
                                   {&e_rcvtimout, &a_trmsg1, &a_nop, S_WTAT},
                                   {&e_rcverr, &a_flushrx, &a_reset, S_INIT},
                                   {&e_always, NULL, &a_nop, S_WTRDY}};

/*
static const S_TABLE st_chkrxrdy[] = {{&e_rxrdy, &a_trmsg1, &a_nop, S_WTAT},
                                      {&e_always, &a_flushrx, &a_trmsg1, S_WTAT}};

  */

static const S_TABLE st_chkrxrdy[] = {{&e_rxrdy, &a_trapn, &a_nop, S_WTAPN},
                                      {&e_always, &a_flushrx, &a_trapn, S_WTAPN}};

static const S_TABLE st_wtat[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSP1},
                                  {&e_rcverr, &a_reset, &a_nop, S_INIT},
                                  {&e_always, NULL, &a_nop, S_WTAT}};

static const S_TABLE st_chkrsp1[] = {{&e_rxrdy, &a_flushrx, &a_trapn, S_WTAPN},
                                     {&e_rxok, &a_flushrx, &a_trapn, S_WTAPN},
                                     {&e_always, &a_flushrx, &a_trmsg1, S_WTAT}};

static const S_TABLE st_wtrdyok[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSP1A},
                                     {&e_rcverr, &a_flushrx, &a_trmsg1, S_WTAT},
                                     {&e_always, NULL, &a_nop, S_WTRDYOK}};

static const S_TABLE st_chkrsp1a[] = {{&e_rxok, &a_trapn, &a_nop, S_WTAPN},
                                      {&e_always, &a_flushrx, &a_trmsg1, S_WTAT}};

static const S_TABLE st_wtapn[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPAPN},
                                   {&e_rcverr, &a_rcvdata, &a_nop, S_WTAPN},   /* zzz */
                                   {&e_rcverr, &a_reset, &a_nop, S_INIT},
                                   {&e_always, NULL, &a_nop, S_WTAPN}};

static const S_TABLE st_chkrspapn[] = {{&e_rximei, &a_trqurccfg, &a_nop, S_WTQURCCFG},
                                       {&e_always, &a_flushrx, &a_trapn, S_WTAPN}};

static const S_TABLE st_wtqurccfg[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPQURCCFG},
                                       {&e_rcverr, &a_reset, &a_nop, S_INIT},
                                       {&e_always, NULL, &a_nop, S_WTQURCCFG}};

static const S_TABLE st_chkrspqurccfg[] = {{&e_rxok, &a_trqcfg, &a_nop, S_WTQCFG},
                                           {&e_always, &a_flushrx, &a_trqurccfg, S_WTQURCCFG}};


#if INDIA != 0
static const S_TABLE st_wtqcfg[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPQCFG},
                                    {&e_rcverr, &a_reset, &a_nop, S_INIT},
                                    {&e_always, NULL, &a_nop, S_WTQCFG}};

static const S_TABLE st_chkrspqcfg[] = {{&e_rxok, &a_trqcfg2, &a_nop, S_WTQCFG2},
                                        {&e_always, &a_flushrx, &a_trqcfg, S_WTQCFG}};

static const S_TABLE st_wtqcfg2[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPQCFG2},
                                     {&e_rcverr, &a_reset, &a_nop, S_INIT},
                                     {&e_always, NULL, &a_nop, S_WTQCFG2}};

static const S_TABLE st_chkrspqcfg2[] = {{&e_rxok, &a_trqcfg3, &a_nop, S_WTQCFG3},
                                         {&e_always, &a_flushrx, &a_trqcfg2, S_WTQCFG2}};

static const S_TABLE st_wtqcfg3[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPQCFG3},
                                     {&e_rcverr, &a_reset, &a_nop, S_INIT},
                                     {&e_always, NULL, &a_nop, S_WTQCFG3}};

static const S_TABLE st_chkrspqcfg3[] = {{&e_rxok, &a_trgsn, &a_nop, S_WTGSN},
                                         {&e_always, &a_flushrx, &a_trqcfg2, S_WTQCFG3}};

#elif USA != 0

static const S_TABLE st_wtqcfg[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPQCFG},
                                    {&e_rcverr, &a_reset, &a_nop, S_INIT},
                                    {&e_always, NULL, &a_nop, S_WTQCFG}};

static const S_TABLE st_chkrspqcfg[] = {{&e_rxok, &a_trgsn, &a_nop, S_WTGSN},
                                        {&e_always, &a_flushrx, &a_trqcfg, S_WTQCFG}};
#endif


static const S_TABLE st_wtgsn[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPGSN},
                                   {&e_rcverr, &a_reset, &a_nop, S_INIT},
                                   {&e_always, NULL, &a_nop, S_WTGSN}};

static const S_TABLE st_chkrspgsn[] = {{&e_rximei, &a_saveimei, &a_trcimi, S_WTCIMI},
                                       {&e_always, &a_flushrx, &a_trcimi, S_WTCIMI}};

static const S_TABLE st_wtcimi[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPCIMI},
                                    {&e_rcverr, &a_reset, &a_nop, S_INIT},
                                    {&e_always, NULL, &a_nop, S_WTCIMI}};

static const S_TABLE st_chkrspcimi[] = {{&e_rximsi, &a_saveimsi, &a_trqccid, S_WTQCCID},
                                        {&e_always, &a_flushrx, &a_trqccid, S_WTQCCID}};

static const S_TABLE st_wtqccid[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPQCCID},
                                     {&e_rcverr, &a_reset, &a_nop, S_INIT},
                                     {&e_always, NULL, &a_nop, S_WTQCCID}};

static const S_TABLE st_chkrspqccid[] = {{&e_rxccid, &a_saveccid, &a_flushrx, S_IDLE},
                                         {&e_always, &a_flushrx, &a_nop, S_IDLE}};

static const S_TABLE st_idle[] = {{&e_always, NULL, &a_nop, S_IDLE}};




static const S_TABLE *state_table[] =
{
    st_init,
    st_wtat,
    st_chkrsp1,
    st_idle,
    st_wtrdy,
    st_chkrsp1a,
    st_wtrdyok,
    st_chkrxrdy,
    st_init2,
    st_init3,
    st_init3wt,
    st_chkinitmsg,
    st_wtapn,
    st_chkrspapn,
    st_wtqurccfg,
    st_chkrspqurccfg,
    st_wtqcfg,
    st_chkrspqcfg,

#if INDIA != 0
    st_wtqcfg2,
    st_chkrspqcfg2,
    st_wtqcfg3,
    st_chkrspqcfg3,
#elif USA != 0
    NULL,
    NULL,
    NULL,
    NULL,
#endif

    st_wtgsn,
    st_chkrspgsn,
    st_wtcimi,
    st_chkrspcimi,
    st_wtqccid,
    st_chkrspqccid
};

static uint8_t rcv_status;

static uint8_t gotrdyflag;
static uint8_t gotcpinrflag;

static uint32_t myiccidlen;
static uint32_t rcv_count;
static uint32_t timer;
static uint32_t oldstate;
static uint8_t rcv_buffer[MAX_RECEIVE_LEN + 1];
static uint8_t myimei[15];
static uint8_t myiccid[22];
static uint8_t myimsi[15];


#if INDIA == 1
static const uint8_t apnmsg[] = {"AT+QICSGP=1,1,\"airtelgprs.com\",,,1"};
static const uint8_t qcfg1msg[] = {"\"nwscanseq\",010203"};
static const uint8_t qcfg2msg[] = {"\"nwscanmode\",1,1"};
static const uint8_t qcfg3msg[] = {"\"iotopmode\",2"};
#elif USA == 1
static const uint8_t apnmsg[] = {"AT+QICSGP=1,1,\"data641003\",\"\",\"\",1"};
static const uint8_t qcfg1msg[] = {"\"nwscanseq\""};
#endif

static const uint8_t qurccfgmsg[] = {"\"urcport\",\"uart1\""};



/***************************************************************************
 *                             GLOBAL FUNCTIONS
 **************************************************************************/


/***************************************************************************
 *                         gsm_init
 *                         --------
 *
 * Initializes the led pins and handler.
 *
 * param[in] - none
 *
 * return - none
 */

void gsm_init(void)
{
    gsm_stmachine.sms_stable = (S_TABLE **)&state_table;
    gsm_stmachine.sms_curstate = S_INIT;
    gsm_stmachine.sms_debug = &state_trace;
    oldstate = S_INIT;
    return;
}



/***************************************************************************
 *                         gsm_timer_ih
 *                         --------------
 *
 * Set yellow led on/off as directed
 *
 * param[in] - ledstate
 *
 * return - none
 */

void gsm_timer_ih()
{

    if (timer)
    {
        timer--;
    }

    return;
}

/***************************************************************************
 *                               EVENTS
 **************************************************************************/

/**************************************************************************
 *                            e_always
 *                            --------
 *
 */

static uint32_t e_always(void)
{
    return 1;
}


/**************************************************************************
 *                            e_quecready
 *                            -----------
 *
 */

static uint32_t e_quecready(void)
{
    return (uint32_t)quec_ready_flag;
}

/**************************************************************************
 *                            e_rcverr
 *                            --------
 *
 */

static uint32_t e_rcverr(void)
{

   if ((rcv_status != QS_INPROGRESS) && (rcv_status != QS_OK))
   {
       usb_printf((uint8_t *)"GSM RCV STAT: %d\r\n", rcv_status);
       return 1;
   }

   return 0;
}

/**************************************************************************
 *                            e_rcvok
 *                            -------
 *
 */

static uint32_t e_rcvok(void)
{
   return rcv_status == QS_OK;
}

/**************************************************************************
 *                            e_rcvtimout
 *                            -----------
 *
 */

static uint32_t e_rcvtimout(void)
{
   return rcv_status == QS_TIMEOUT;
}

/**************************************************************************
 *                            e_rxat
 *                            ------
 *
 */


static uint32_t e_rxat(void)
{

    if ((rcv_count >= 3) && (rcv_buffer[0] == 'A') && (rcv_buffer[1] == 'T'))
    {
        return 1;
    }

    return 0;
}

/**************************************************************************
 *                            e_rxccid
 *                            --------
 *
 */

static uint32_t e_rxccid(void)
{
    uint32_t stat;

    stat = compare(7, (uint8_t *)"+QCCID:");
    return stat;
}

/**************************************************************************
 *                            e_rxcpinr
 *                            ---------
 *
 */

static uint32_t e_rxcpinr(void)
{
    uint32_t stat;

    stat = compare(12, (uint8_t *)"+CPIN: READY");
    return stat;
}

/**************************************************************************
 *                            e_rximei
 *                            ---------
 *
 */

static uint32_t e_rximei(void)
{
    uint32_t stat;

    if (rcv_count >= 16)
    {
        stat = 1;
    }

    else
    {
        stat = 0;
    }

    return stat;
}

/**************************************************************************
 *                            e_rximsi
 *                            ---------
 *
 */

static uint32_t e_rximsi(void)
{
    uint32_t stat;

    if (rcv_count >= 16)
    {
        stat = 1;
    }

    else
    {
        stat = 0;
    }

    return stat;
}

/**************************************************************************
 *                            e_rxok
 *                            ------
 *
 */

static uint32_t e_rxok(void)
{

    if ((rcv_count == 3) && (rcv_buffer[0] == 'O') && (rcv_buffer[1] == 'K'))
    {
        return 1;
    }

    return 0;
}


/**************************************************************************
 *                            e_rxrdy
 *                            -------
 *
 */

static uint32_t e_rxrdy(void)
{

    if ((rcv_count == 4) && (rcv_buffer[0] == 'R') && (rcv_buffer[1] == 'D') && (rcv_buffer[2] == 'Y'))
    {
        return 1;
    }

    return 0;
}

/**************************************************************************
 *                            e_timeout
 *                            ---------
 *
 */

static uint32_t e_timeout(void)
{

    if (timer == 0)
    {
        return 1;
    }

    return 0;
}

/***************************************************************************
 *                               ACTIONS
 **************************************************************************/

/**************************************************************************
 *                               a_flushrx
 *                               ---------
 *
 */

static void a_flushrx(void)
{
    quec_rxflush();
    return;
}

/**************************************************************************
 *                               a_gotcpinr
 *                               ----------
 *
 */

static void a_gotcpinr(void)
{
    gotcpinrflag = 1;
    return;
}

/**************************************************************************
 *                               a_gotrdy
 *                               --------
 *
 */

static void a_gotrdy(void)
{
    gotrdyflag = 1;
    return;
}

/**************************************************************************
 *                               a_initvars
 *                               ----------
 *
 */

static void a_initvars(void)
{
    gotcpinrflag = 0;
    gotrdyflag = 0;
    return;
}

/**************************************************************************
 *                               a_nop
 *                               -----
 *
 */

static void a_nop(void)
{
    return;
}

/**************************************************************************
 *                               a_rcvdata
 *                               ---------
 *
 */

static void a_rcvdata(void)
{
    rcv_status = quec_receive(rcv_buffer, MAX_RECEIVE_LEN, 5000, &gsm_rcv_ih);
    return;
}

/**************************************************************************
 *                               a_rcvinitmsgs
 *                               -------------
 *
 */

static void a_rcvinitmsgs(void)
{
    rcv_status = quec_receive(rcv_buffer, MAX_RECEIVE_LEN, 1000, &gsm_rcv_ih);
    return;
}

/**************************************************************************
 *                               a_reset
 *                               -------
 *
 */

static void a_reset(void)
{
    quec_reset();
    return;
}

/**************************************************************************
 *                               a_saveccid
 *                               ----------
 *
 */

static void a_saveccid(void)
{
    uint32_t i;

    for (i = 0; i < 22; i++)
    {

        if (rcv_buffer[i + 8] == 0x0D)
        {
            break;
        }

        myiccid[i] = rcv_buffer[i + 8];
    }

    myiccidlen = i;
    return;
}

/**************************************************************************
 *                               a_saveimei
 *                               ----------
 *
 */

static void a_saveimei(void)
{
    uint32_t i;

    for (i = 0; i < 15; i++)
    {
        myimei[i] = rcv_buffer[i];
    }

    return;
}

/**************************************************************************
 *                               a_saveimsi
 *                               ----------
 *
 */

static void a_saveimsi(void)
{
    uint32_t i;

    for (i = 0; i < 15; i++)
    {
        myimsi[i] = rcv_buffer[i];
    }

    return;
}

/**************************************************************************
 *                               a_tmr5sec
 *                               ---------
 *
 */

static void a_tmr5sec(void)
{
    __disable_irq();
    timer = 5000;
    __enable_irq();
    return;
}

/**************************************************************************
 *                               a_trapn
 *                               -------
 *
 */

static void a_trapn(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 16, 5000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit((uint8_t *)apnmsg, sizeof(apnmsg) - 1);
    }

    return;
}

/**************************************************************************
 *                               a_trcimi
 *                               --------
 *
 */

static void a_trcimi(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 16, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit((uint8_t *)"AT+CIMI\r\n", 9);
    }

    return;
}

/**************************************************************************
 *                               a_trgsn
 *                               -------
 *
 */

static void a_trgsn(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 32, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit((uint8_t *)"AT+GSN\r\n", 8);
    }

    return;
}




/**************************************************************************
 *                               a_trmsg1
 *                               --------
 *
 */

static void a_trmsg1(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 16, 5000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit((uint8_t *)"AT\r\n", 4);
    }

    return;
}

/**************************************************************************
 *                               a_trqccid
 *                               ---------
 *
 */

static void a_trqccid(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 32, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit((uint8_t *)"AT+QCCID\r\n", 10);
    }

    return;
}

/**************************************************************************
 *                               a_trqcfg
 *                               --------
 *
 */

static void a_trqcfg(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 32, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit((uint8_t *)qcfg1msg, sizeof(qcfg1msg) - 1);
    }

    return;
}

#if INDIA != 0
/**************************************************************************
 *                               a_trqcfg2
 *                               ---------
 *
 */

static void a_trqcfg2(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 32, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit((uint8_t *)qcfg2msg, sizeof(qcgf1msg) - 1);
    }

    return;
}

/**************************************************************************
 *                               a_trqcfg3
 *                               ---------
 *
 */

static void a_trqcfg3(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 32, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit((uint8_t *)qcfg3msg, sizeof(qcgf1msg) - 1);
    }

    return;
}
#endif

/**************************************************************************
 *                               a_trqurccfg
 *                               -----------
 *
 */

static void a_trqurccfg(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 32, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {

        quec_transmit((uint8_t *)qurccfgmsg, sizeof(qurccfgmsg) - 1);
    }

    return;
}

/**************************************************************************
 *                               a_txate0
 *                               --------
 *
 */

static void a_txate0(void)
{
    quec_transmit((uint8_t *)"ATE0\r\n", 6);
    return;
}



/***************************************************************************
 *                             LOCAL FUNCTIONS
 **************************************************************************/

/***************************************************************************
 *                         compare
 *                         -------
 *
 * Set yellow led on/off as directed
 *
 * param[in] - ledstate
 *
 * return - none
 */

static uint32_t compare(uint32_t count, uint8_t *response)
{
    uint32_t i;
    uint32_t stat;

    stat = 0;

    for (i = 0; i < count; i++)
    {

        if (response[i] != rcv_buffer[i])
        {
            stat = 0;
            break;
        }

    }

    return stat;
}

/***************************************************************************
 *                         gsm_timer_ih
 *                         ------------
 *
 * Set yellow led on/off as directed
 *
 * param[in] - ledstate
 *
 * return - none
 */

static void gsm_rcv_ih(uint8_t stat, uint32_t count)
{
    rcv_status = stat;
    rcv_count = count;
    return;
}

static void state_trace(uint32_t evnum)
{

    if (gsm_stmachine.sms_curstate != oldstate)
    {
        usb_printf((uint8_t *)"GSM STATE: %u -> %u, EVENT %u\r\n", oldstate, gsm_stmachine.sms_curstate, evnum);
    }

    oldstate = gsm_stmachine.sms_curstate;
    return;
}
/*
 * End of module.
 */

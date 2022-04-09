
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
#define S_WTAT2 4
#define S_CHKRSP1A 5
#define S_WTCSQ 6
#define S_CHKRSPCSQ 7
#define S_INIT2 8
#define S_WTOKAPN 9
#define S_INIT3WT 10
#define S_CHKOKAPN 11
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
#define S_WTCREG 28
#define S_CHKRSPCREG 29
#define S_WTCGREG 30
#define S_CHKRSPCGREG 31
#define S_CHKOKGSN 32
#define S_WTOKGSN 33
#define S_WTOKCIMI 34
#define S_CHKOKCIMI 35
#define S_WTOKQCCID 36
#define S_CHKOKQCCID 37
#define S_WTGMR 38
#define S_CHKRSPGMR 39
#define S_WTOKGMR 40
#define S_CHKOKGMR 41
#define S_CONNECT 42
#define S_WTQIACT 43
#define S_CHKRSPQIACT 44
#define S_WTQMTCFG1 45
#define S_CHKRSPQMTCFG1 46
#define S_WTQMTCFG2 47
#define S_CHKRSPQMTCFG2 48
#define S_WTQSSL1 49
#define S_CHKRSPQSSL1 50
#define S_WTQSSL2 51
#define S_CHKRSPQSSL2 52
#define S_WTQSSL3 53
#define S_CHKRSPQSSL3 54
#define S_WTQSSL4 55
#define S_CHKRSPQSSL4 56
#define S_WTQSSL5 57
#define S_CHKRSPQSSL5 58
#define S_WTQSSL6 59
#define S_CHKRSPQSSL6 60
#define S_WTQMTCON 61
#define S_CHKRSPQMTCON 62
#define S_WTQMTOPEN 63
#define S_CHKRSPQMTOPEN 64
#define S_CONNECTED 65
#define S_WTOKQMTOPEN 66
#define S_CHKOKQMTOPEN 67
#define S_WTOKQMTCON 68
#define S_CHKOKQMTCON 69
#define S_CLOCK 70
#define S_CLOCKWT 71
#define S_CLOCKRSP 72
#define S_CLOCKDELAY 73
#define S_SENDRECS 74
#define S_VCSUBWT 75
#define S_VCSUBRSP 76
#define S_VCPUBWT 77
#define S_VCPUBRSP 78
#define S_VCPUBWTOUT 79
#define S_VCPUBDATAWT 80
#define S_VCPUBTMOUT 81
#define S_VERIFYCV 82
#define S_VCPUBDATA1 83
#define S_VCJSON 84
#define S_VCJSONWT 85
#define S_VCJSONRSP 86
#define S_PING 87
#define S_PHTCFGCWT 88
#define S_PHTCFGCRSP 89
#define S_PHTCFGRQHWT 90
#define S_PHTCFGRQHRSP 91
#define S_PHTURL1WT 92
#define S_PHTURL1RSP 93
#define S_PHTURL2WT 94
#define S_PHTURL2RSP 95
#define S_PHTPOST1WT 96
#define S_PHTPOST1RSP 97
#define S_PHTPOST2WT 98
#define S_PHTPOST2RSP 99
#define S_PHTREADWT 100
#define S_PHTREADRSP 101
#define S_PINGCHK 102
#define S_SRSUBWT 103
#define S_SRSUBRSP 104
#define S_SRPUBWT 105
#define S_SRPUBTMOUT 106
#define S_SRPUBRSP 107
#define S_SRPUBWTOUT 108
#define S_SRPUBDATAWT 109
#define S_SRPUBDATA1 110
#define S_SRJSON 111
#define S_SRJSONWT 112
#define S_SRPUBDATA2 113
#define S_SRPUBRECWT 114
#define S_SRPUBRECRSP 115
#define S_SRRECOUTWT 116
#define S_SRRECSUBWT 117
#define S_SRRECRSP 118
#define S_MQTDISC 119
#define S_MQTDISCWT 120
#define S_MQTDISCRSP 121
#define S_MQTCLOSEWT 122
#define S_MQTCLOSERSP 123
#define S_UPDATECFG 124

#define S_CAHTCFGCWT 125
#define S_CAHTCFGCRSP 126
#define S_CAHTCFGRQHWT 127
#define S_CAHTCFGRQHRSP 128
#define S_CAHTURL1WT 129
#define S_CAHTURL1RSP 130
#define S_CAHTURL2WT 131
#define S_CAHTURL2RSP 132
#define S_CAHTGETWT 133
#define S_CAHTGETRSP 134
#define S_CAHTRDFILEWT 135
#define S_CAHTRDFILERSP 136
//#define S_UPDCFG 137

#define S_CCHTCFGCWT 138
#define S_CCHTCFGCRSP 139
#define S_CCHTCFGRQHWT 140
#define S_CCHTCFGRQHRSP 141
#define S_CCHTURL1WT 142
#define S_CCHTURL1RSP 143
#define S_CCHTURL2WT 144
#define S_CCHTURL2RSP 145
#define S_CCHTGETWT 146
#define S_CCHTGETRSP 147
#define S_CCHTRDFILEWT 148
#define S_CCHTRDFILERSP 149

#define S_CKHTCFGCWT 150
#define S_CKHTCFGCRSP 151
#define S_CKHTCFGRQHWT 152
#define S_CKHTCFGRQHRSP 153
#define S_CKHTURL1WT 154
#define S_CKHTURL1RSP 155
#define S_CKHTURL2WT 156
#define S_CKHTURL2RSP 157
#define S_CKHTGETWT 158
#define S_CKHTGETRSP 159
#define S_CKHTRDFILEWT 160
#define S_CKHTRDFILERSP 161
#define S_UPDTWIN 162
#define S_UTPUBWT 163
#define S_UTPUBTMOUT 164
#define S_UTPUBWTOUT 165
#define S_UTPUBDATAWT 166
#define S_UTPUBRSP 167

#define S_SCHTCFGCWT 168
#define S_SCHTCFGCRSP 169
#define S_SCHTCFGRQHWT 170
#define S_SCHTCFGRQHRSP 171
#define S_SCHTURL1WT 172
#define S_SCHTURL1RSP 173
#define S_SCHTURL2WT 174
#define S_SCHTURL2RSP 175
#define S_SCHTGETWT 176
#define S_SCHTGETRSP 177
#define S_SCHTREADWT 178
#define S_SCHTREADRSP 179





#define POSTBUFLEN 400
#define TXBUFLEN 64
#define MAX_RECEIVE_LEN 400

/***************************************************************************
 *                               TYPEDEFS
 **************************************************************************/

/***************************************************************************
 *                         LOCAL FUNCTION PROTOTYPES
 **************************************************************************/

static uint32_t e_always(void);
static uint32_t e_eof(void);
static uint32_t e_equalcvs(void);
static uint32_t e_outidle(void);
static uint32_t e_pingok(void);
static uint32_t e_rcverr(void);
static uint32_t e_rcvok(void);
static uint32_t e_rcvovflow(void);
static uint32_t e_rcvtimout(void);
static uint32_t e_reqclock(void);
static uint32_t e_reqconnect(void);
static uint32_t e_reqdisconn(void);
static uint32_t e_reqnone(void);
static uint32_t e_reqping(void);
static uint32_t e_reqpower(void);
static uint32_t e_reqpwrdown(void);
static uint32_t e_reqrdtwin(void);
static uint32_t e_reqrecords(void);
static uint32_t e_requpdcfg(void);
static uint32_t e_requpdtwin(void);



static uint32_t e_rxat(void);
static uint32_t e_rxccid(void);
static uint32_t e_rxconnect(void);
static uint32_t e_rxconnok(void);
static uint32_t e_rxcpinr(void);
static uint32_t e_rxcr(void);
static uint32_t e_rxcsq(void);
static uint32_t e_rxgt(void);
static uint32_t e_rximei(void);
static uint32_t e_rximsi(void);
static uint32_t e_rxjsonend(void);
static uint32_t e_rxok(void);
static uint32_t e_rxqcfg(void);
static uint32_t e_rxqhtget(void);
static uint32_t e_rxqhtpost(void);
static uint32_t e_rxqhtrdfile(void);
static uint32_t e_rxqhtread(void);
static uint32_t e_rxqmtclose(void);
static uint32_t e_rxqmtdisc(void);
static uint32_t e_rxqmtpub(void);
static uint32_t e_rxqmtrecv(void);
static uint32_t e_rxqmtstat(void);
static uint32_t e_rxqmtsub(void);
static uint32_t e_rxopenok(void);
static uint32_t e_rxrdy(void);
static uint32_t e_rxtime(void);
static uint32_t e_timeout(void);
static uint32_t e_updcacert(void);
static uint32_t e_updclcert(void);
static uint32_t e_updclkey(void);
static uint32_t e_updfver(void);
static uint32_t e_updmver(void);
static uint32_t e_updsconfig(void);



static void a_accumjson(void);
static void a_ansbadcfg(void);
static void a_ansconnected(void);

static void a_ansinprogress(void);
static void a_ansneedping(void);
static void a_ansnopower(void);
static void a_anspowered(void);
static void a_buildpost(void);
static void a_clrupdca(void);
static void a_clrupdcc(void);
static void a_clrupdck(void);
static void a_clrupdfv(void);
static void a_clrupdmv(void);
static void a_clrupdsc(void);
static void a_flushrx(void);
static void a_gotcpinr(void);
static void a_gotrdy(void);
static void a_increcnum(void);
static void a_initjson(void);
static void a_initrecnum(void);
static void a_initvars(void);
static void a_nop(void);
static void a_powerdn(void);
static void a_powerup(void);
static void a_processjson(void);
static void a_processping(void);
static void a_processsc(void);
static void a_rcvdata(void);
static void a_rcvinitmsgs(void);
static void a_rcvqmt(void);
static void a_reset(void);
static void a_saveccid(void);
static void a_savegmr(void);
static void a_saveimei(void);
static void a_saveimsi(void);
static void a_savetime(void);
static void a_trapn(void);
static void a_trate0(void);
static void a_trcimi(void);
static void a_trcsq(void);
static void a_trcgreg(void);
static void a_trcreg(void);
static void a_trgmr(void);
static void a_trgsn(void);
static void a_trhtcfgc(void);
static void a_trhtcfgrqh(void);
static void a_trhtcfgrqh2(void);
static void a_trhtget(void);
static void a_trhtpost1(void);
static void a_trhtpost2(void);
static void a_trhtrdfile(void);
static void a_trhtrdfile2(void);
static void a_trhtrdfile3(void);
static void a_trhtread(void);

static void a_trhturl2(void);
static void a_trhturlca(void);
static void a_trhturlcc(void);
static void a_trhturlck(void);
static void a_trhturlp(void);
static void a_trhturlsc(void);
static void a_trmsg1(void);
static void a_trqccid(void);
static void a_trqcfg(void);
static void a_trqiact(void);
static void a_trqmtcfg1(void);
static void a_trqmtcfg2(void);
static void a_trqmtclose(void);
static void a_trqmtconn(void);
static void a_trqmtdisc(void);
static void a_trqmtopen(void);
static void a_trqmtpub(void);
static void a_trqmtpub2(void);
static void a_trqmtpub3(void);
static void a_trqmtsub(void);
static void a_trqntp(void);
static void a_trqssl1(void);
static void a_trqssl2(void);
static void a_trqssl3(void);
static void a_trqssl4(void);
static void a_trqssl5(void);
static void a_trqssl6(void);

static void a_txnul(void);
static void a_txrecord(void);
static void a_txsub(void);
static void a_txtwin(void);
static void a_txz(void);


static void a_trqurccfg(void);
static void a_tmr10(void);


static uint32_t compare(uint32_t, uint8_t *, uint32_t);
static void gsm_rcv_ih(uint8_t, uint32_t);
static void state_trace(uint32_t);

/***************************************************************************
 *                            GLOBAL VARIABLES
 **************************************************************************/


SM_STRUC gsm_stmachine;

/***************************************************************************
 *                             LOCAL VARIABLES
 **************************************************************************/

static const S_TABLE const st_init[] = {{&e_reqpower, &a_initvars, &a_ansinprogress, S_INIT2},
                                        {&e_reqnone, NULL, &a_nop, S_INIT},
                                        {&e_reqpwrdown, &a_ansnopower, NULL, S_INIT},
                                        {&e_always, &a_ansnopower, NULL, S_INIT}};


static const S_TABLE st_init2[] = {{&e_always, &a_powerup, &a_rcvdata, S_INIT3WT}};

static const S_TABLE st_init3wt[] = {{&e_rcvok, &a_rcvdata, &a_nop, S_INIT3WT},
                                     {&e_rcvtimout, &a_trate0, &a_nop, S_WTAT},
                                     {&e_rcverr, &a_powerdn, &a_ansnopower, S_INIT},
                                     {&e_always, NULL, &a_nop, S_INIT3WT}};



static const S_TABLE st_wtat[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSP1},
                                  {&e_rcverr, &a_powerdn, &a_ansnopower, S_INIT},
                                  {&e_always, NULL, &a_nop, S_WTAT}};

static const S_TABLE st_chkrsp1[] = {{&e_rxok, &a_flushrx, &a_trcsq, S_WTCSQ},
                                     {&e_always, &a_flushrx, &a_trmsg1, S_WTAT2}};

static const S_TABLE st_wtat2[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSP1A},
                                   {&e_rcverr, &a_reset, &a_nop, S_INIT2},
                                   {&e_always, NULL, &a_nop, S_WTAT2}};

static const S_TABLE st_chkrsp1a[] = {{&e_rxok, &a_trcsq, &a_nop, S_WTCSQ},
                                      {&e_always, &a_reset, &a_nop, S_INIT2}};


static const S_TABLE st_wtcsq[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPCSQ},
                                   {&e_rcverr, &a_reset, &a_nop, S_INIT2},
                                   {&e_always, NULL, &a_nop, S_WTCSQ}};

static const S_TABLE st_chkrspcsq[] = {{&e_rxcsq, &a_rcvdata, &a_nop, S_WTCSQ},
                                       {&e_rxok, &a_nop, &a_trapn, S_WTAPN},
                                       {&e_always, &a_reset, &a_nop, S_INIT2}};


static const S_TABLE st_wtapn[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPAPN},
                                   {&e_rcverr, &a_reset, &a_nop, S_INIT2},
                                   {&e_always, NULL, &a_nop, S_WTAPN}};

static const S_TABLE st_chkrspapn[] = {{&e_rxcr, &a_rcvdata, &a_nop, S_WTOKAPN},
                                       {&e_rxok, &a_trqurccfg, &a_nop, S_WTQURCCFG},
                                       {&e_always, &a_reset, &a_nop, S_INIT2}};

static const S_TABLE st_wtokapn[] = {{&e_rcvok, NULL, &a_nop, S_CHKOKAPN},
                                     {&e_rcverr, &a_reset, &a_nop, S_INIT2},
                                     {&e_always, NULL, &a_nop, S_WTOKAPN}};

static const S_TABLE st_chkokapn[] = {{&e_rxok,  &a_trqurccfg, &a_nop, S_WTQURCCFG},
                                      {&e_always, &a_reset, &a_nop, S_INIT2}};


static const S_TABLE st_wtqurccfg[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPQURCCFG},
                                       {&e_rcverr, &a_reset, &a_nop, S_INIT2},
                                       {&e_always, NULL, &a_nop, S_WTQURCCFG}};

static const S_TABLE st_chkrspqurccfg[] = {{&e_rxok, &a_trqcfg, &a_nop, S_WTQCFG},
                                           {&e_always, &a_reset, &a_nop, S_INIT2}};


static const S_TABLE st_wtqcfg[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPQCFG},
                                    {&e_rcvovflow, &a_nop, &a_nop, S_CHKRSPQCFG},
                                    {&e_rcverr, &a_reset, &a_nop, S_INIT},
                                    {&e_always, NULL, &a_nop, S_WTQCFG}};

static const S_TABLE st_chkrspqcfg[] = {{&e_rxqcfg, &a_rcvdata, &a_nop, S_WTQCFG},
                                        {&e_rxok, &a_trcreg, &a_nop, S_WTCREG},
                                        {&e_always, &a_reset, &a_nop, S_INIT2}};



static const S_TABLE st_wtcreg[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPCREG},
                                    {&e_rcverr, &a_reset, &a_nop, S_INIT2},
                                    {&e_always, NULL, &a_nop, S_WTCREG}};

static const S_TABLE st_chkrspcreg[] = {{&e_rxok, &a_trcgreg, &a_nop, S_WTCGREG},
                                        {&e_always, &a_reset, &a_nop, S_INIT2}};


static const S_TABLE st_wtcgreg[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPCGREG},
                                     {&e_rcverr, &a_reset, &a_nop, S_INIT2},
                                     {&e_always, NULL, &a_nop, S_WTCGREG}};

static const S_TABLE st_chkrspcgreg[] = {{&e_rxok, &a_trgsn, &a_nop, S_WTGSN},
                                        {&e_always, &a_reset, &a_nop, S_INIT2}};




static const S_TABLE st_wtgsn[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPGSN},
                                   {&e_rcverr, &a_reset, &a_nop, S_INIT2},
                                   {&e_always, NULL, &a_nop, S_WTGSN}};

static const S_TABLE st_chkrspgsn[] = {{&e_rximei, &a_saveimei, &a_rcvdata, S_WTOKGSN},
                                       {&e_always, &a_reset, &a_nop, S_INIT2}};

static const S_TABLE st_wtokgsn[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKOKGSN},
                                     {&e_rcverr, &a_reset, &a_nop, S_INIT2},
                                     {&e_always, NULL, &a_nop, S_WTOKGSN}};

static const S_TABLE st_chkokgsn[] = {{&e_rxok,  &a_trcimi, &a_nop, S_WTCIMI},
                                      {&e_always, &a_reset, &a_nop, S_INIT2}};




static const S_TABLE st_wtcimi[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPCIMI},
                                    {&e_rcverr, &a_reset, &a_nop, S_INIT2},
                                    {&e_always, NULL, &a_nop, S_WTCIMI}};

static const S_TABLE st_chkrspcimi[] = {{&e_rximsi, &a_saveimsi, &a_rcvdata, S_WTOKCIMI},
                                        {&e_always, &a_reset, &a_nop, S_INIT2}};

static const S_TABLE st_wtokcimi[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKOKCIMI},
                                      {&e_rcverr, &a_trqccid, &a_nop, S_WTQCCID},
                                      {&e_always, NULL, &a_nop, S_WTOKCIMI}};

static const S_TABLE st_chkokcimi[] = {{&e_rxok, &a_trqccid, &a_nop, S_WTQCCID},
                                        {&e_always, &a_reset, &a_nop, S_INIT2}};




static const S_TABLE st_wtqccid[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPQCCID},
                                     {&e_rcverr, &a_reset, &a_nop, S_INIT2},
                                     {&e_always, NULL, &a_nop, S_WTQCCID}};

static const S_TABLE st_chkrspqccid[] = {{&e_rxccid, &a_saveccid, &a_rcvdata, S_WTOKQCCID},
                                         {&e_always, &a_flushrx, &a_nop, S_IDLE}};

static const S_TABLE st_wtokqccid[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKOKQCCID},
                                      {&e_rcverr, &a_trgmr, &a_nop, S_WTGMR},
                                      {&e_always, NULL, &a_nop, S_WTOKQCCID}};

static const S_TABLE st_chkokqccid[] = {{&e_rxok, &a_trgmr, &a_nop, S_WTGMR},
                                        {&e_always, &a_reset, &a_nop, S_INIT}};


static const S_TABLE st_wtgmr[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPGMR},
                                   {&e_rcverr, &a_reset, &a_nop, S_INIT},
                                   {&e_always, NULL, &a_nop, S_WTGMR}};

static const S_TABLE st_chkrspgmr[] = {{&e_always, &a_savegmr, &a_rcvdata, S_WTOKGMR}};

static const S_TABLE st_wtokgmr[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKOKGMR},
                                      {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                      {&e_always, NULL, &a_nop, S_WTOKGMR}};

static const S_TABLE st_chkokgmr[] = {{&e_rxok, &a_trqiact, &a_nop, S_WTQIACT},
                                        {&e_always, &a_reset, &a_nop, S_INIT2}};

static const S_TABLE st_wtqiact[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPQIACT},
                                     {&e_reqpwrdown, &a_powerdn, &a_nop, S_INIT},
                                     {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                     {&e_always, NULL, &a_nop, S_WTQIACT}};

static const S_TABLE st_chkrspqiact[] = {{&e_rxok, &a_anspowered, &a_nop, S_IDLE},
                                         {&e_always, &a_rcvdata, &a_nop, S_WTQIACT}};




static const S_TABLE st_idle[] = {{&e_reqpower, &a_anspowered, NULL, S_IDLE},
                                  {&e_reqpwrdown, &a_powerdn, &a_nop, S_INIT},
                                  {&e_reqconnect, &a_trqmtcfg1, &a_nop, S_WTQMTCFG1},
                                  {&e_reqclock, &a_nop, &a_nop, S_CLOCK},
                                  {&e_reqping, &a_nop, &a_nop, S_PING},
                                  {&e_requpdcfg, &a_nop, &a_nop, S_UPDATECFG},
                                  {&e_reqdisconn, &a_anspowered, NULL, S_IDLE},
                                  {&e_always, NULL, &a_nop, S_IDLE}};








static const S_TABLE st_wtqmtcfg1[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPQMTCFG1},
                                       {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                       {&e_always, NULL, &a_nop, S_WTQMTCFG1}};

static const S_TABLE st_chkrspqmtcfg1[] = {{&e_rxok, &a_trqmtcfg2, &a_nop, S_WTQMTCFG2},
                                           {&e_always, &a_flushrx, &a_anspowered, S_IDLE}};

static const S_TABLE st_wtqmtcfg2[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPQMTCFG2},
                                       {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                       {&e_always, NULL, &a_nop, S_WTQMTCFG2}};

static const S_TABLE st_chkrspqmtcfg2[] = {{&e_rxok, &a_trqssl1, &a_nop, S_WTQSSL1},
                                           {&e_always, &a_flushrx, &a_anspowered, S_IDLE}};

static const S_TABLE st_wtqssl1[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPQSSL1},
                                     {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                     {&e_always, NULL, &a_nop, S_WTQSSL1}};

static const S_TABLE st_chkrspqssl1[] = {{&e_rxok, &a_trqssl2, &a_nop, S_WTQSSL2},
                                           {&e_always, &a_flushrx, &a_anspowered, S_IDLE}};

static const S_TABLE st_wtqssl2[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPQSSL2},
                                     {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                     {&e_always, NULL, &a_nop, S_WTQSSL2}};

static const S_TABLE st_chkrspqssl2[] = {{&e_rxok, &a_trqssl3, &a_nop, S_WTQSSL3},
                                         {&e_always, &a_flushrx, &a_anspowered, S_IDLE}};

static const S_TABLE st_wtqssl3[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPQSSL3},
                                     {&e_rcverr, &a_reset, &a_nop, S_IDLE},
                                     {&e_always, NULL, &a_nop, S_WTQSSL3}};

static const S_TABLE st_chkrspqssl3[] = {{&e_rxok, &a_trqssl4, &a_nop, S_WTQSSL4},
                                         {&e_always, &a_flushrx, &a_anspowered, S_IDLE}};

static const S_TABLE st_wtqssl4[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPQSSL4},
                                     {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                     {&e_always, NULL, &a_nop, S_WTQSSL4}};

static const S_TABLE st_chkrspqssl4[] = {{&e_rxok, &a_trqssl5, &a_nop, S_WTQSSL5},
                                         {&e_always, &a_flushrx, &a_anspowered, S_IDLE}};

static const S_TABLE st_wtqssl5[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPQSSL5},
                                     {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                     {&e_always, NULL, &a_nop, S_WTQSSL5}};

static const S_TABLE st_chkrspqssl5[] = {{&e_rxok, &a_trqssl6, &a_nop, S_WTQSSL6},
                                         {&e_always, &a_flushrx, &a_nop, S_IDLE}};

static const S_TABLE st_wtqssl6[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPQSSL6},
                                     {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                     {&e_always, NULL, &a_nop, S_WTQSSL6}};

static const S_TABLE st_chkrspqssl6[] = {{&e_rxok, &a_trqmtopen, &a_nop, S_WTOKQMTOPEN},
                                         {&e_always, &a_flushrx, &a_anspowered, S_IDLE}};



static const S_TABLE st_wtokqmtopen[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKOKQMTOPEN},
                                         {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                         {&e_always, NULL, &a_nop, S_WTOKQMTOPEN}};

static const S_TABLE st_chkokqmtopen[] = {{&e_rxok, &a_rcvqmt, &a_nop, S_WTQMTOPEN},
                                           {&e_always, &a_flushrx, &a_anspowered, S_IDLE}};

static const S_TABLE st_wtqmtopen[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPQMTOPEN},
                                       {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                       {&e_always, NULL, &a_nop, S_WTQMTOPEN}};

static const S_TABLE st_chkrspqmtopen[] = {{&e_rxopenok, &a_trqmtconn, &a_nop, S_WTOKQMTCON},
                                           {&e_always, &a_flushrx, &a_anspowered, S_IDLE}};



static const S_TABLE st_wtokqmtcon[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKOKQMTCON},
                                        {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                        {&e_always, NULL, &a_nop, S_WTOKQMTCON}};

static const S_TABLE st_chkokqmtcon[] = {{&e_rxok, &a_rcvqmt, &a_nop, S_WTQMTCON},
                                         {&e_always, &a_flushrx, &a_anspowered, S_IDLE}};

static const S_TABLE st_wtqmtcon[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPQMTCON},
                                      {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                      {&e_always, NULL, &a_nop, S_WTQMTCON}};

static const S_TABLE st_chkrspqmtcon[] = {{&e_rxconnok, &a_ansconnected, &a_nop, S_CONNECTED},
                                         {&e_always, &a_flushrx, &a_anspowered, S_IDLE}};


static const S_TABLE st_connected[] = {{&e_reqpower, NULL, &a_nop, S_CONNECTED},
                                       {&e_reqconnect, NULL, &a_nop, S_CONNECTED},
                                       {&e_reqrecords, &a_initrecnum, &a_nop, S_SENDRECS},
                                       {&e_reqrdtwin, &a_nop, &a_nop, S_VERIFYCV},
                                       {&e_requpdtwin, &a_nop, &a_nop, S_UPDTWIN},
                                       {&e_reqdisconn, &a_nop, &a_nop, S_MQTDISC},
                                       {&e_always, NULL, &a_nop, S_CONNECTED}};



static const S_TABLE st_clock[] = {{&e_always, &a_trqntp, &a_nop, S_CLOCKWT}};

static const S_TABLE st_clockwt[] = {{&e_rcvok, &a_nop, &a_nop, S_CLOCKRSP},
                                     {&e_rcvtimout, &a_nop, &a_nop, S_CLOCKRSP},
                                     {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                     {&e_always, NULL, &a_nop, S_CLOCKWT}};

static const S_TABLE st_clockrsp[] = {{&e_rxok, &a_rcvdata, &a_nop, S_CLOCKWT},
                                      {&e_rxtime, &a_savetime, &a_anspowered, S_IDLE},
                                      {&e_always, &a_flushrx, &a_tmr10, S_CLOCKDELAY}};

static const S_TABLE st_clockdelay[] = {{&e_reqclock, &a_ansinprogress, &a_nop, S_CLOCKDELAY},
                                        {&e_reqpower, &a_anspowered, &a_nop, S_IDLE},
                                        {&e_timeout, &a_flushrx, &a_nop, S_CLOCK},
                                        {&e_always, NULL, &a_nop, S_CLOCKDELAY}};



static const S_TABLE st_verifycv[] = {{&e_always, &a_initjson, &a_trqmtsub, S_VCSUBWT}};

static const S_TABLE st_vcsubwt[] = {{&e_rcvok, &a_nop, &a_nop, S_VCSUBRSP},
                                     {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                     {&e_always, NULL, &a_nop, S_VCSUBWT}};

static const S_TABLE st_vcsubrsp[] = {{&e_rxok, &a_rcvdata, &a_nop, S_VCSUBWT},
                                      {&e_rxqmtsub, &a_trqmtpub, &a_nop, S_VCPUBWT},
                                      {&e_always, &a_flushrx, &a_nop, S_VCSUBRSP}};

static const S_TABLE st_vcpubwt[] = {{&e_rcvok, &a_nop, &a_nop, S_VCPUBRSP},
                                     {&e_rcvtimout, &a_nop, &a_nop, S_VCPUBTMOUT},
                                     {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                     {&e_always, NULL, &a_nop, S_VCPUBWT}};

static const S_TABLE st_vcpubtmout[] = {{&e_rxgt, &a_txnul, &a_nop, S_VCPUBWTOUT},
                                        {&e_always, &a_rcvdata, &a_nop, S_VCPUBWT}};

static const S_TABLE st_vcpubrsp[] = {{&e_rxok, &a_rcvdata, &a_nop, S_VCPUBWT},
                                      {&e_rxqmtpub, &a_nop, &a_nop, S_VCPUBWT},
                                      {&e_rxgt, &a_txnul, &a_nop, S_VCPUBWTOUT},
                                      {&e_always, &a_flushrx, &a_nop, S_VCPUBRSP}};

static const S_TABLE st_vcpubwtout[] = {{&e_outidle, &a_txsub, &a_rcvdata, S_VCPUBDATAWT},
                                        {&e_always, NULL, &a_nop, S_VCPUBWTOUT}};

static const S_TABLE st_vcpubdatawt[] = {{&e_rcvok, &a_nop, NULL, S_VCPUBDATA1},
                                         {&e_rcverr, &a_rcvdata, NULL, S_VCPUBDATAWT},
                                         {&e_always, NULL, &a_nop, S_VCPUBDATAWT}};

static const S_TABLE st_vcpubdata1[] = {{&e_rxok, &a_rcvdata, &a_nop, S_VCPUBDATAWT},
                                        {&e_rxqmtpub, &a_rcvdata, &a_nop, S_VCPUBDATAWT},
                                        {&e_rxqmtrecv, &a_initjson, &a_rcvdata, S_VCJSONWT},
                                        {&e_always, &a_rcvdata, &a_nop, S_VCPUBDATAWT}};

static const S_TABLE st_vcjsonwt[] = {{&e_rcvok, &a_rcvdata, NULL, S_VCJSONRSP},
                                      {&e_rcverr, &a_rcvdata, NULL, S_VCJSONWT},
                                      {&e_always, NULL, &a_nop, S_VCJSONWT}};

static const S_TABLE st_vcjsonrsp[] = {{&e_rxjsonend, &a_accumjson, &a_processjson, S_VCJSON},
                                        {&e_always, &a_accumjson, &a_rcvdata, S_VCJSONWT}};

static const S_TABLE st_vcjson[] = {{&e_equalcvs, &a_ansconnected, &a_nop, S_CONNECTED},
                                    {&e_always, &a_ansbadcfg, &a_nop, S_CONNECTED}};







static const S_TABLE st_ping[] = {{&e_always, &a_trhtcfgc, &a_nop, S_PHTCFGCWT}};

static const S_TABLE st_phtcfgcwt[] = {{&e_rcvok, &a_nop, &a_nop, S_PHTCFGCRSP},
                                       {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                       {&e_always, NULL, &a_nop, S_PHTCFGCWT}};

static const S_TABLE st_phtcfgcrsp[] = {{&e_rxok, &a_trhtcfgrqh, &a_nop, S_PHTCFGRQHWT},
                                         {&e_always, &a_anspowered, &a_nop, S_IDLE}};

static const S_TABLE st_phtcfgrqhwt[] = {{&e_rcvok, &a_nop, &a_nop, S_PHTCFGRQHRSP},
                                         {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                         {&e_always, NULL, &a_nop, S_PHTCFGRQHWT}};

static const S_TABLE st_phtcfgrqhrsp[] = {{&e_rxok, &a_trhturlp, &a_nop, S_PHTURL1WT},
                                           {&e_always, &a_anspowered, &a_nop, S_IDLE}};

static const S_TABLE st_phturl1wt[] = {{&e_rcvok, &a_nop, &a_nop, S_PHTURL1RSP},
                                         {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                         {&e_always, NULL, &a_nop, S_PHTURL1WT}};

static const S_TABLE st_phturl1rsp[] = {{&e_rxconnect, &a_trhturl2, &a_nop, S_PHTURL2WT},
                                        {&e_always, &a_anspowered, &a_nop, S_IDLE}};

static const S_TABLE st_phturl2wt[] = {{&e_rcvok, &a_nop, &a_nop, S_PHTURL2RSP},
                                       {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                       {&e_always, NULL, &a_nop, S_PHTURL2WT}};

static const S_TABLE st_phturl2rsp[] = {{&e_rxok, &a_buildpost, &a_trhtpost1, S_PHTPOST1WT},
                                        {&e_always, &a_anspowered, &a_nop, S_IDLE}};

static const S_TABLE st_phtpost1wt[] = {{&e_rcvok, &a_nop, &a_nop, S_PHTPOST1RSP},
                                       {&e_rcverr, &a_rcvdata, NULL, S_PHTPOST1WT},
                                       {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                       {&e_always, NULL, &a_nop, S_PHTPOST1WT}};

static const S_TABLE st_phtpost1rsp[] = {{&e_rxconnect, &a_trhtpost2, &a_nop, S_PHTPOST2WT},
                                        {&e_always, &a_rcvdata, &a_nop, S_PHTPOST1WT}};

static const S_TABLE st_phtpost2wt[] = {{&e_rcvok, &a_nop, &a_nop, S_PHTPOST2RSP},
                                        {&e_rcvtimout, &a_rcvdata, &a_nop, S_PHTPOST2WT},
                                        {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                        {&e_always, NULL, &a_nop, S_PHTPOST2WT}};

static const S_TABLE st_phtpost2rsp[] = {{&e_rxqhtpost, &a_trhtread, &a_nop, S_PHTREADWT},
                                         {&e_rxok, &a_rcvdata, &a_nop, S_PHTPOST2WT},
                                        {&e_always, &a_anspowered, &a_nop, S_IDLE}};

static const S_TABLE st_phtreadwt[] = {{&e_rcvok, &a_nop, &a_nop, S_PHTREADRSP},
                                       {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                       {&e_always, NULL, &a_nop, S_PHTREADWT}};

static const S_TABLE st_phtreadrsp[] = {{&e_rxconnect, &a_initjson, &a_rcvdata, S_PHTREADWT},
                                        {&e_rxok, &a_processping, &a_nop, S_PINGCHK},
                                        {&e_always, &a_accumjson, &a_rcvdata, S_PHTREADWT}};

static const S_TABLE st_pingchk[] = {{&e_pingok, &a_anspowered, &a_nop, S_IDLE},
                                     {&e_always, &a_ansbadcfg, &a_nop, S_IDLE}};






static const S_TABLE st_sendrecs[] = {{&e_eof, &a_ansconnected, &a_nop, S_CONNECTED},
                                      {&e_always, &a_trqmtpub2, &a_initrecnum, S_SRPUBRECWT}};

static const S_TABLE st_srsubwt[] = {{&e_rcvok, &a_nop, &a_nop, S_SRSUBRSP},
                                     {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                     {&e_always, NULL, &a_nop, S_SRSUBWT}};

static const S_TABLE st_srsubrsp[] = {{&e_rxok, &a_rcvdata, &a_nop, S_SRSUBWT},
                                      {&e_rxqmtsub, &a_trqmtpub, &a_nop, S_SRPUBWT},
                                      {&e_always, &a_flushrx, &a_nop, S_SRSUBRSP}};

static const S_TABLE st_srpubwt[] = {{&e_rcvok, &a_nop, &a_nop, S_SRPUBRSP},
                                     {&e_rcvtimout, &a_nop, &a_nop, S_SRPUBTMOUT},
                                     {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                     {&e_always, NULL, &a_nop, S_SRPUBWT}};

static const S_TABLE st_srpubtmout[] = {{&e_rxgt, &a_txnul, &a_nop, S_SRPUBWTOUT},
                                        {&e_always, &a_rcvdata, &a_nop, S_SRPUBWT}};

static const S_TABLE st_srpubrsp[] = {{&e_rxok, &a_rcvdata, &a_nop, S_SRPUBWT},
                                      {&e_rxqmtpub, &a_nop, &a_nop, S_SRPUBWT},
                                      {&e_rxgt, &a_txnul, &a_nop, S_SRPUBWTOUT},
                                      {&e_always, &a_flushrx, &a_nop, S_SRPUBRSP}};

static const S_TABLE st_srpubwtout[] = {{&e_outidle, &a_txsub, &a_rcvdata, S_SRPUBDATAWT},
                                        {&e_always, NULL, &a_nop, S_SRPUBWTOUT}};

static const S_TABLE st_srpubdatawt[] = {{&e_rcvok, &a_nop, NULL, S_SRPUBDATA1},
                                         {&e_rcverr, &a_rcvdata, NULL, S_SRPUBDATAWT},
                                         {&e_always, NULL, &a_nop, S_SRPUBDATAWT}};

static const S_TABLE st_srpubdata1[] = {{&e_rxok, &a_rcvdata, &a_nop, S_SRPUBDATAWT},
                                        {&e_rxqmtpub, &a_rcvdata, &a_nop, S_SRPUBDATAWT},
                                        {&e_rxqmtrecv, &a_initjson, &a_rcvdata, S_SRJSONWT},
                                        {&e_always, &a_rcvdata, &a_nop, S_SRPUBDATAWT}};

static const S_TABLE st_srjsonwt[] = {{&e_rcvok, &a_nop, &a_nop, S_SRPUBDATA2},
                                      {&e_rcverr, &a_rcvdata, NULL, S_SRJSONWT},
                                      {&e_always, NULL, &a_nop, S_SRJSONWT}};

static const S_TABLE st_srpubdata2[] = {{&e_rxjsonend, &a_accumjson, &a_processjson, S_SRJSON},
                                        {&e_always, &a_accumjson, &a_rcvdata, S_SRJSONWT}};





static const S_TABLE st_srjson[] = {{&e_eof, &a_nop, &a_ansconnected, S_CONNECTED},
                                    {&e_always, &a_trqmtpub2, &a_nop, S_SRPUBRECWT}};

static const S_TABLE st_srpubrecwt[] = {{&e_rcvok, &a_nop, &a_nop, S_SRPUBRECRSP},
                                        {&e_rcvtimout, &a_nop, NULL, S_SRPUBRECRSP},
                                        {&e_rcverr, &a_rcvdata, NULL, S_SRPUBRECWT},
                                        {&e_always, NULL, &a_nop, S_SRPUBRECWT}};

static const S_TABLE st_srpubrecrsp[] = {{&e_rxgt, &a_txrecord, &a_nop, S_SRRECOUTWT},
                                         {&e_rxqmtpub, &a_rcvdata, &a_nop, S_SRPUBDATAWT},
                                         {&e_rxqmtrecv, &a_initjson, &a_rcvdata, S_SRJSONWT},
                                         {&e_always, &a_rcvdata, &a_nop, S_SRPUBRECWT}};

static const S_TABLE st_srrecoutwt[] = {{&e_outidle, &a_txsub, &a_rcvdata, S_SRRECSUBWT},
                                        {&e_always, NULL, &a_nop, S_SRRECOUTWT}};

static const S_TABLE st_srrecsubwt[] = {{&e_rcvok, &a_nop, &a_nop, S_SRRECRSP},
                                     {&e_rcverr, &a_rcvdata, NULL, S_SRRECSUBWT},
                                     {&e_always, NULL, &a_nop, S_SRRECSUBWT}};

static const S_TABLE st_srrecrsp[] = {{&e_rxqmtpub, &a_increcnum, &a_nop, S_SRJSON},
                                      {&e_rxok, &a_rcvdata, &a_nop, S_SRRECSUBWT},
                                      {&e_always, &a_rcvdata, &a_nop, S_SRRECSUBWT}};




static const S_TABLE st_mqtdisc[] = {{&e_always, &a_trqmtdisc, &a_nop, S_MQTDISCWT}};

static const S_TABLE st_mqtdiscwt[] = {{&e_rcvok, &a_nop, &a_nop, S_MQTDISCRSP},
                                        {&e_rcverr, &a_trqmtclose, &a_nop, S_MQTCLOSEWT},
                                        {&e_always, NULL, &a_nop, S_MQTDISCWT}};

static const S_TABLE st_mqtdiscrsp[] = {{&e_rxok, &a_rcvdata, &a_nop, S_MQTDISCWT},
                                        {&e_rxqmtdisc, &a_anspowered, NULL, S_IDLE},  /* zzz */
                                        {&e_rxqmtdisc, &a_trqmtclose, &a_nop, S_MQTCLOSEWT},
                                        {&e_always, &a_rcvdata, &a_nop, S_MQTDISCWT}};

static const S_TABLE st_mqtclosewt[] = {{&e_rcvok, &a_nop, &a_nop, S_MQTDISCRSP},
                                        {&e_rcvtimout, &a_nop, &a_nop, S_SRPUBTMOUT},
                                        {&e_rcverr, &a_trqmtdisc, &a_nop, S_MQTDISCWT},
                                        {&e_always, NULL, &a_nop, S_MQTCLOSEWT}};

static const S_TABLE st_mqtclosersp[] = {{&e_rxok, &a_rcvdata, &a_nop, S_MQTCLOSEWT},
                                         {&e_rxqmtclose, &a_anspowered, &a_nop, S_IDLE},
                                         {&e_always, &a_flushrx, &a_nop, S_SRSUBRSP}};




static const S_TABLE st_updatecfg[] = {{&e_updcacert, &a_trhtcfgc, &a_nop, S_CAHTCFGCWT},
                                    {&e_updclcert, &a_trhtcfgc, &a_nop, S_CCHTCFGCWT},
                                    {&e_updclkey, &a_trhtcfgc, &a_nop, S_CKHTCFGCWT},
                                    {&e_updsconfig, &a_trhtcfgc, &a_nop, S_SCHTCFGCWT},
                                    {&e_updfver, &a_clrupdfv, &a_nop, S_UPDATECFG},
                                    {&e_updmver, &a_clrupdmv, &a_nop, S_UPDATECFG},
                                    {&e_always, &a_anspowered, &a_nop, S_IDLE}};






static const S_TABLE st_cahtcfgcwt[] = {{&e_rcvok, &a_nop, &a_nop, S_CAHTCFGCRSP},
                                       {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                       {&e_always, NULL, &a_nop, S_CAHTCFGCWT}};

static const S_TABLE st_cahtcfgcrsp[] = {{&e_rxok, &a_trhtcfgrqh2, &a_nop, S_CAHTCFGRQHWT},
                                         {&e_always, &a_anspowered, &a_nop, S_IDLE}};

static const S_TABLE st_cahtcfgrqhwt[] = {{&e_rcvok, &a_nop, &a_nop, S_CAHTCFGRQHRSP},
                                         {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                         {&e_always, NULL, &a_nop, S_CAHTCFGRQHWT}};

static const S_TABLE st_cahtcfgrqhrsp[] = {{&e_rxok, &a_trhturlca, &a_nop, S_CAHTURL1WT},
                                           {&e_always, &a_anspowered, &a_nop, S_IDLE}};

static const S_TABLE st_cahturl1wt[] = {{&e_rcvok, &a_nop, &a_nop, S_CAHTURL1RSP},
                                         {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                         {&e_always, NULL, &a_nop, S_CAHTURL1WT}};

static const S_TABLE st_cahturl1rsp[] = {{&e_rxconnect, &a_trhturl2, &a_nop, S_CAHTURL2WT},
                                        {&e_always, &a_anspowered, &a_nop, S_IDLE}};

static const S_TABLE st_cahturl2wt[] = {{&e_rcvok, &a_nop, &a_nop, S_CAHTURL2RSP},
                                       {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                       {&e_always, NULL, &a_nop, S_CAHTURL2WT}};

static const S_TABLE st_cahturl2rsp[] = {{&e_rxok, &a_trhtget, &a_nop, S_CAHTGETWT},
                                        {&e_always, &a_anspowered, &a_nop, S_IDLE}};


static const S_TABLE st_cahtgetwt[] = {{&e_rcvok, &a_nop, &a_nop, S_CAHTGETRSP},
                                       {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                       {&e_always, NULL, &a_nop, S_CAHTGETWT}};

static const S_TABLE st_cahtgetrsp[] = {{&e_rxqhtget, &a_trhtrdfile, &a_nop, S_CAHTRDFILEWT},
                                        {&e_rxok, &a_rcvdata, &a_nop, S_CAHTGETWT},
                                        {&e_always, &a_nop, &a_rcvdata, S_CAHTGETWT}};

static const S_TABLE st_cahtrdfilewt[] = {{&e_rcvok, &a_nop, &a_nop, S_CAHTRDFILERSP},
                                           {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                           {&e_always, NULL, &a_nop, S_CAHTRDFILEWT}};


static const S_TABLE st_cahtrdfilersp[] = {{&e_rxqhtrdfile, &a_clrupdca, &a_nop, S_UPDATECFG},
                                          {&e_rxok, &a_rcvdata, &a_nop, S_CAHTRDFILEWT},
                                          {&e_always, &a_nop, &a_rcvdata, S_CAHTRDFILEWT}};





static const S_TABLE st_cchtcfgcwt[] = {{&e_rcvok, &a_nop, &a_nop, S_CCHTCFGCRSP},
                                       {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                       {&e_always, NULL, &a_nop, S_CCHTCFGCWT}};

static const S_TABLE st_cchtcfgcrsp[] = {{&e_rxok, &a_trhtcfgrqh2, &a_nop, S_CCHTCFGRQHWT},
                                         {&e_always, &a_anspowered, &a_nop, S_IDLE}};

static const S_TABLE st_cchtcfgrqhwt[] = {{&e_rcvok, &a_nop, &a_nop, S_CCHTCFGRQHRSP},
                                         {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                         {&e_always, NULL, &a_nop, S_CCHTCFGRQHWT}};

static const S_TABLE st_cchtcfgrqhrsp[] = {{&e_rxok, &a_trhturlcc, &a_nop, S_CCHTURL1WT},
                                           {&e_always, &a_anspowered, &a_nop, S_IDLE}};

static const S_TABLE st_cchturl1wt[] = {{&e_rcvok, &a_nop, &a_nop, S_CCHTURL1RSP},
                                         {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                         {&e_always, NULL, &a_nop, S_CCHTURL1WT}};

static const S_TABLE st_cchturl1rsp[] = {{&e_rxconnect, &a_trhturl2, &a_nop, S_CCHTURL2WT},
                                        {&e_always, &a_anspowered, &a_nop, S_IDLE}};

static const S_TABLE st_cchturl2wt[] = {{&e_rcvok, &a_nop, &a_nop, S_CCHTURL2RSP},
                                       {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                       {&e_always, NULL, &a_nop, S_CCHTURL2WT}};

static const S_TABLE st_cchturl2rsp[] = {{&e_rxok, &a_trhtget, &a_nop, S_CCHTGETWT},
                                        {&e_always, &a_anspowered, &a_nop, S_IDLE}};


static const S_TABLE st_cchtgetwt[] = {{&e_rcvok, &a_nop, &a_nop, S_CCHTGETRSP},
                                       {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                       {&e_always, NULL, &a_nop, S_CCHTGETWT}};

static const S_TABLE st_cchtgetrsp[] = {{&e_rxqhtget, &a_trhtrdfile2, &a_nop, S_CCHTRDFILEWT},
                                        {&e_rxok, &a_rcvdata, &a_nop, S_CCHTGETWT},
                                        {&e_always, &a_nop, &a_rcvdata, S_CCHTGETWT}};

static const S_TABLE st_cchtrdfilewt[] = {{&e_rcvok, &a_nop, &a_nop, S_CCHTRDFILERSP},
                                           {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                           {&e_always, NULL, &a_nop, S_CCHTRDFILEWT}};


static const S_TABLE st_cchtrdfilersp[] = {{&e_rxqhtrdfile, &a_clrupdcc, &a_nop, S_UPDATECFG},
                                          {&e_rxok, &a_rcvdata, &a_nop, S_CCHTRDFILEWT},
                                          {&e_always, &a_nop, &a_rcvdata, S_CCHTRDFILEWT}};






static const S_TABLE st_ckhtcfgcwt[] = {{&e_rcvok, &a_nop, &a_nop, S_CKHTCFGCRSP},
                                       {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                       {&e_always, NULL, &a_nop, S_CKHTCFGCWT}};

static const S_TABLE st_ckhtcfgcrsp[] = {{&e_rxok, &a_trhtcfgrqh2, &a_nop, S_CKHTCFGRQHWT},
                                         {&e_always, &a_anspowered, &a_nop, S_IDLE}};

static const S_TABLE st_ckhtcfgrqhwt[] = {{&e_rcvok, &a_nop, &a_nop, S_CKHTCFGRQHRSP},
                                         {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                         {&e_always, NULL, &a_nop, S_CKHTCFGRQHWT}};

static const S_TABLE st_ckhtcfgrqhrsp[] = {{&e_rxok, &a_trhturlck, &a_nop, S_CKHTURL1WT},
                                           {&e_always, &a_anspowered, &a_nop, S_IDLE}};

static const S_TABLE st_ckhturl1wt[] = {{&e_rcvok, &a_nop, &a_nop, S_CKHTURL1RSP},
                                         {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                         {&e_always, NULL, &a_nop, S_CKHTURL1WT}};

static const S_TABLE st_ckhturl1rsp[] = {{&e_rxconnect, &a_trhturl2, &a_nop, S_CKHTURL2WT},
                                        {&e_always, &a_anspowered, &a_nop, S_IDLE}};

static const S_TABLE st_ckhturl2wt[] = {{&e_rcvok, &a_nop, &a_nop, S_CKHTURL2RSP},
                                       {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                       {&e_always, NULL, &a_nop, S_CKHTURL2WT}};

static const S_TABLE st_ckhturl2rsp[] = {{&e_rxok, &a_trhtget, &a_nop, S_CKHTGETWT},
                                        {&e_always, &a_anspowered, &a_nop, S_IDLE}};


static const S_TABLE st_ckhtgetwt[] = {{&e_rcvok, &a_nop, &a_nop, S_CKHTGETRSP},
                                       {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                       {&e_always, NULL, &a_nop, S_CKHTGETWT}};

static const S_TABLE st_ckhtgetrsp[] = {{&e_rxqhtget, &a_trhtrdfile3, &a_nop, S_CKHTRDFILEWT},
                                        {&e_rxok, &a_rcvdata, &a_nop, S_CKHTGETWT},
                                        {&e_always, &a_nop, &a_rcvdata, S_CKHTGETWT}};

static const S_TABLE st_ckhtrdfilewt[] = {{&e_rcvok, &a_nop, &a_nop, S_CKHTRDFILERSP},
                                           {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                           {&e_always, NULL, &a_nop, S_CKHTRDFILEWT}};


static const S_TABLE st_ckhtrdfilersp[] = {{&e_rxqhtrdfile, &a_clrupdck, &a_nop, S_UPDATECFG},
                                          {&e_rxok, &a_rcvdata, &a_nop, S_CKHTRDFILEWT},
                                          {&e_always, &a_nop, &a_rcvdata, S_CKHTRDFILEWT}};



static const S_TABLE st_schtcfgcwt[] = {{&e_rcvok, &a_nop, &a_nop, S_SCHTCFGCRSP},
                                       {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                       {&e_always, NULL, &a_nop, S_SCHTCFGCWT}};

static const S_TABLE st_schtcfgcrsp[] = {{&e_rxok, &a_trhtcfgrqh2, &a_nop, S_SCHTCFGRQHWT},
                                         {&e_always, &a_anspowered, &a_nop, S_IDLE}};

static const S_TABLE st_schtcfgrqhwt[] = {{&e_rcvok, &a_nop, &a_nop, S_SCHTCFGRQHRSP},
                                         {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                         {&e_always, NULL, &a_nop, S_SCHTCFGRQHWT}};

static const S_TABLE st_schtcfgrqhrsp[] = {{&e_rxok, &a_trhturlsc, &a_nop, S_SCHTURL1WT},
                                           {&e_always, &a_anspowered, &a_nop, S_IDLE}};

static const S_TABLE st_schturl1wt[] = {{&e_rcvok, &a_nop, &a_nop, S_SCHTURL1RSP},
                                         {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                         {&e_always, NULL, &a_nop, S_SCHTURL1WT}};

static const S_TABLE st_schturl1rsp[] = {{&e_rxconnect, &a_trhturl2, &a_nop, S_SCHTURL2WT},
                                        {&e_always, &a_anspowered, &a_nop, S_IDLE}};

static const S_TABLE st_schturl2wt[] = {{&e_rcvok, &a_nop, &a_nop, S_SCHTURL2RSP},
                                       {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                       {&e_always, NULL, &a_nop, S_SCHTURL2WT}};

static const S_TABLE st_schturl2rsp[] = {{&e_rxok, &a_trhtget, &a_nop, S_SCHTGETWT},
                                        {&e_always, &a_anspowered, &a_nop, S_IDLE}};


static const S_TABLE st_schtgetwt[] = {{&e_rcvok, &a_nop, &a_nop, S_SCHTGETRSP},
                                       {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                       {&e_always, NULL, &a_nop, S_SCHTGETWT}};

static const S_TABLE st_schtgetrsp[] = {{&e_rxqhtget, &a_trhtread, &a_nop, S_SCHTREADWT},
                                        {&e_rxok, &a_rcvdata, &a_nop, S_SCHTGETWT},
                                        {&e_always, &a_nop, &a_rcvdata, S_SCHTGETWT}};

static const S_TABLE st_schtreadwt[] = {{&e_rcvok, &a_nop, &a_nop, S_SCHTREADRSP},
                                           {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                           {&e_always, NULL, &a_nop, S_SCHTREADWT}};


static const S_TABLE st_schtreadrsp[] = {{&e_rxconnect, &a_initjson, &a_rcvdata, S_SCHTREADWT},
                                          {&e_rxok, &a_processsc, &a_clrupdsc, S_UPDATECFG},
                                          {&e_always, &a_accumjson, &a_rcvdata, S_SCHTREADWT}};



static const S_TABLE st_updtwin[] =  {{&e_always, &a_trqmtpub3, &a_nop, S_UTPUBWT}};

static const S_TABLE st_utpubwt[] = {{&e_rcvok, &a_nop, &a_nop, S_UTPUBRSP},
                                     {&e_rcvtimout, &a_nop, &a_nop, S_UTPUBTMOUT},
                                     {&e_rcverr, &a_anspowered, &a_nop, S_CONNECTED},
                                     {&e_always, NULL, &a_nop, S_UTPUBWT}};

static const S_TABLE st_utpubtmout[] = {{&e_rxgt, &a_txtwin, &a_nop, S_UTPUBWTOUT},
                                        {&e_always, &a_rcvdata, &a_nop, S_UTPUBWT}};

static const S_TABLE st_utpubwtout[] = {{&e_outidle, &a_txsub, &a_rcvdata, S_UTPUBDATAWT},
                                        {&e_always, NULL, &a_nop, S_UTPUBWTOUT}};

static const S_TABLE st_utpubdatawt[] = {{&e_rcvok, &a_rcvdata, &a_nop, S_UTPUBRSP},
                                         {&e_rcverr, &a_ansconnected, &a_nop, S_CONNECTED},
                                         {&e_always, NULL, &a_nop, S_UTPUBDATAWT}};

static const S_TABLE st_utpubrsp[] = {{&e_rxok, &a_rcvdata, &a_nop, S_UTPUBDATAWT},
                                      {&e_rxqmtpub, &a_ansconnected, &a_nop, S_CONNECTED},
                                      {&e_always, &a_rcvdata, &a_nop, S_UTPUBDATAWT}};





static const S_TABLE * const state_table[] =
{
    st_init,
    st_wtat,
    st_chkrsp1,
    st_idle,
    st_wtat2,
    st_chkrsp1a,
    st_wtcsq,
    st_chkrspcsq,
    st_init2,
    st_wtokapn,
    st_init3wt,
    st_chkokapn,
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
    st_chkrspqccid,
    st_wtcreg,
    st_chkrspcreg,
    st_wtcgreg,
    st_chkrspcgreg,
    st_chkokgsn,
    st_wtokgsn,
    st_wtokcimi,
    st_chkokcimi,
    st_wtokqccid,
    st_chkokqccid,
    st_wtgmr,
    st_chkrspgmr,
    st_wtokgmr,
    st_chkokgmr,
    0,
    st_wtqiact,
    st_chkrspqiact,
    st_wtqmtcfg1,
    st_chkrspqmtcfg1,
    st_wtqmtcfg2,
    st_chkrspqmtcfg2,
    st_wtqssl1,
    st_chkrspqssl1,
    st_wtqssl2,
    st_chkrspqssl2,
    st_wtqssl3,
    st_chkrspqssl3,
    st_wtqssl4,
    st_chkrspqssl4,
    st_wtqssl5,
    st_chkrspqssl5,
    st_wtqssl6,
    st_chkrspqssl6,
    st_wtqmtcon,
    st_chkrspqmtcon,
    st_wtqmtopen,
    st_chkrspqmtopen,
    st_connected,
    st_wtokqmtopen,
    st_chkokqmtopen,
    st_wtokqmtcon,
    st_chkokqmtcon,
    st_clock,
    st_clockwt,
    st_clockrsp,
    st_clockdelay,
    st_sendrecs,
    st_vcsubwt,
    st_vcsubrsp,
    st_vcpubwt,
    st_vcpubrsp,
    st_vcpubwtout,
    st_vcpubdatawt,
    st_vcpubtmout,
    st_verifycv,
    st_vcpubdata1,
    st_vcjson,
    st_vcjsonwt,
    st_vcjsonrsp,
    st_ping,
    st_phtcfgcwt,
    st_phtcfgcrsp,
    st_phtcfgrqhwt,
    st_phtcfgrqhrsp,
    st_phturl1wt,
    st_phturl1rsp,
    st_phturl2wt,
    st_phturl2rsp,
    st_phtpost1wt,
    st_phtpost1rsp,
    st_phtpost2wt,
    st_phtpost2rsp,
    st_phtreadwt,
    st_phtreadrsp,
    st_pingchk,
    st_srsubwt,
    st_srsubrsp,
    st_srpubwt,
    st_srpubtmout,
    st_srpubrsp,
    st_srpubwtout,
    st_srpubdatawt,
    st_srpubdata1,
    st_srjson,
    st_srjsonwt,
    st_srpubdata2,
    st_srpubrecwt,
    st_srpubrecrsp,
    st_srrecoutwt,
    st_srrecsubwt,
    st_srrecrsp,
    st_mqtdisc,
    st_mqtdiscwt,
    st_mqtdiscrsp,
    st_mqtclosewt,
    st_mqtclosersp,

    st_updatecfg,

    st_cahtcfgcwt,
    st_cahtcfgcrsp,
    st_cahtcfgrqhwt,
    st_cahtcfgrqhrsp,
    st_cahturl1wt,
    st_cahturl1rsp,
    st_cahturl2wt,
    st_cahturl2rsp,
    st_cahtgetwt,
    st_cahtgetrsp,
    st_cahtrdfilewt,
    st_cahtrdfilersp,

    0,

    st_cchtcfgcwt,
    st_cchtcfgcrsp,
    st_cchtcfgrqhwt,
    st_cchtcfgrqhrsp,
    st_cchturl1wt,
    st_cchturl1rsp,
    st_cchturl2wt,
    st_cchturl2rsp,
    st_cchtgetwt,
    st_cchtgetrsp,
    st_cchtrdfilewt,
    st_cchtrdfilersp,

    st_ckhtcfgcwt,
    st_ckhtcfgcrsp,
    st_ckhtcfgrqhwt,
    st_ckhtcfgrqhrsp,
    st_ckhturl1wt,
    st_ckhturl1rsp,
    st_ckhturl2wt,
    st_ckhturl2rsp,
    st_ckhtgetwt,
    st_ckhtgetrsp,
    st_ckhtrdfilewt,
    st_ckhtrdfilersp,
    st_updtwin,
    st_utpubwt,
    st_utpubtmout,
    st_utpubwtout,
    st_utpubdatawt,
    st_utpubrsp,

	st_schtcfgcwt,
	st_schtcfgcrsp,
    st_schtcfgrqhwt,
    st_schtcfgrqhrsp,
    st_schturl1wt,
    st_schturl1rsp,
    st_schturl2wt,
    st_schturl2rsp,
    st_schtgetwt,
    st_schtgetrsp,
    st_schtreadwt,
    st_schtreadrsp

};
















static uint8_t rcv_status;
static uint8_t callanswer;
static uint8_t callrequest;
static uint8_t pingstat;

static uint8_t gotrdyflag;
static uint8_t gotcpinrflag;

static uint32_t myiccidlen;
static uint32_t rcv_count;
static uint32_t postbuffer_count;
static uint32_t timer;
static uint32_t oldstate;
static uint32_t recnum;
static uint32_t urlplen;
static uint8_t rcv_buffer[MAX_RECEIVE_LEN + 1];



static uint8_t tx_buffer[TXBUFLEN];
static uint8_t post_buffer[POSTBUFLEN];



static const uint8_t apnmsg[] = {"AT+QICSGP=1,1,\"data641003\",\"\",\"\",1\r\n"};
static const uint8_t qcfg1msg[] = {"AT+QCFG=\"nwscanseq\"\r\n"};

static const uint8_t qurccfgmsg[] = {"AT+QURCCFG=\"urcport\",\"uart1\"\r\n"};

static const uint8_t qmtcfg1msg[] = {"AT+QMTCFG=\"SSL\",0,1,2\r\n"};
static const uint8_t qmtcfg2msg[] = {"AT+QMTCFG=\"version\",0,4\r\n"};

static const uint8_t qssl1msg[] = {"AT+QSSLCFG=\"seclevel\",2,2\r\n"};
static const uint8_t qssl2msg[] = {"AT+QSSLCFG=\"sslversion\",2,4\r\n"};
static const uint8_t qssl3msg[] = {"AT+QSSLCFG=\"ciphersuite\",2,0xFFFF\r\n"};
static const uint8_t qssl4msg[] = {"AT+QSSLCFG=\"cacert\",2,\"security/CaCert.crt\"\r\n"};
static const uint8_t qssl5msg[] = {"AT+QSSLCFG=\"clientcert\",2,\"security/Client.crt\"\r\n"};
static const uint8_t qssl6msg[] = {"AT+QSSLCFG=\"clientkey\",2,\"security/key.pem\"\r\n"};
static const uint8_t nullmsg = 0x00;
static const uint8_t submsg = 0x1A;


static const uint8_t postmsg[] = {"Host: shark.carematix.com\r\n"
                                  "Content-Type: application/json\r\n"
                                  "Content-Length: 160\r\n"
                                  "Authorization: Basic Y2FyZW1hdGl4OnBhc3N3b3Jk\r\n\r\n"};



/***************************************************************************
 *                             GLOBAL FUNCTIONS
 **************************************************************************/

/***************************************************************************
 *                         gsm_answer
 *                         -----------
 *
 * Initializes the led pins and handler.
 *
 * param[in] - none
 *
 * return - none
 */

uint8_t gsm_answer(void)
{
    return callanswer;
}



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
    callanswer = CA_NONE;
    callrequest = CR_NONE;
    return;
}

/***************************************************************************
 *                         gsm_request
 *                         --------------
 *
 * Initializes the led pins and handler.
 *
 * param[in] - none
 *
 * return - none
 */

void gsm_request(uint8_t req)
{
    callrequest = req;
    callanswer = CA_INPROGRESS;
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
 *                            e_eof
 *                            --------
 *
 */

static uint32_t e_eof(void)
{
    uint32_t stat;

    if (mem_read_address >= mem_write_address)
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
 *                            e_equalcvs
 *                            --------
 *
 */

static uint32_t e_equalcvs(void)
{
    return desired_reported_cv_matched_flag != 0;
}



/**************************************************************************
 *                            e_outidle
 *                            --------
 *
 */

static uint32_t e_outidle(void)
{
    uint32_t stat;

    stat = quec_outcompl();
    return stat;
}

/**************************************************************************
 *                            e_pingok
 *                            --------
 *
 */

static uint32_t e_pingok(void)
{
    uint32_t stat;

    stat = ca_certificate_flag |
           client_certificate_flag |
           client_key_flag |
           configuration_service_flag |
           update_module_firmware_flag |
           update_device_firmware_flag;

    return stat == 0;
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
       debug_printf(DBGLVL_MAX, (uint8_t *)"GSM RCV STAT: %d\r\n", rcv_status);
       return 1;
   }

   return 0;
}

/**************************************************************************
 *                            e_rcvovflow
 *                            -----------
 *
 */

static uint32_t e_rcvovflow(void)
{
   return rcv_status == QS_OVERFLOW;
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
 *                            e_reqclock
 *                            -----------
 *
 */

static uint32_t e_reqclock(void)
{
    uint32_t stat;

    if (callrequest == CR_CLOCK)
    {
        callrequest = CR_NONE;
        stat = 1;
    }

    else
    {
        stat = 0;
    }

    return stat;
}


/**************************************************************************
 *                            e_reqconnect
 *                            -----------
 *
 */

static uint32_t e_reqconnect(void)
{

    uint32_t stat;

    if (callrequest == CR_CONNECT)
    {
        callrequest = CR_NONE;
        stat = 1;
    }

    else
    {
        stat = 0;
    }

    return stat;
}

/**************************************************************************
 *                            e_reqdisconn
 *                            -----------
 *
 */

static uint32_t e_reqdisconn(void)
{

    uint32_t stat;

    if (callrequest == CR_DISCONNECT)
    {
        callrequest = CR_NONE;
        stat = 1;
    }

    else
    {
        stat = 0;
    }

    return stat;
}


/**************************************************************************
 *                            e_reqnone
 *                            -----------
 *
 */

static uint32_t e_reqnone(void)
{
    return callrequest == CR_NONE;
}



/**************************************************************************
 *                            e_reqping
 *                            -----------
 *
 */

static uint32_t e_reqping(void)
{
    uint32_t stat;

    if (callrequest == CR_PING)
    {
        callrequest = CR_NONE;
        stat = 1;
    }

    else
    {
        stat = 0;
    }

    return stat;
}


/**************************************************************************
 *                            e_reqpower
 *                            -----------
 *
 */

static uint32_t e_reqpower(void)
{
    uint32_t stat;

    if (callrequest == CR_POWERUP)
    {
        callrequest = CR_NONE;
        stat = 1;
    }

    else
    {
        stat = 0;
    }

    return stat;
}



/**************************************************************************
 *                            e_reqpwrdown
 *                            -----------
 *
 */

static uint32_t e_reqpwrdown(void)
{
    uint32_t stat;

    if (callrequest == CR_POWERDOWN)
    {
        callrequest = CR_NONE;
        stat = 1;
    }

    else
    {
        stat = 0;
    }

    return stat;
}



/**************************************************************************
 *                            e_reqrdtwin
 *                            -----------
 *
 */

static uint32_t e_reqrdtwin(void)
{
    uint32_t stat;

    if (callrequest == CR_RDTWIN)
    {
        callrequest = CR_NONE;
        stat = 1;
    }

    else
    {
        stat = 0;
    }

    return stat;
}





/**************************************************************************
 *                            e_reqrecords
 *                            -----------
 *
 */

static uint32_t e_reqrecords(void)
{
    uint32_t stat;

    if (callrequest == CR_SENDRECORDS)
    {
        callrequest = CR_NONE;
        stat = 1;
    }

    else
    {
        stat = 0;
    }

    return stat;
}

/**************************************************************************
 *                            e_requpdcfg
 *                            -----------
 *
 */

static uint32_t e_requpdcfg(void)
{
    uint32_t stat;

    if (callrequest == CR_UPDCONFIG)
    {
        callrequest = CR_NONE;
        stat = 1;
    }

    else
    {
        stat = 0;
    }

    return stat;
}

/**************************************************************************
 *                            e_requpdtwin
 *                            -----------
 *
 */

static uint32_t e_requpdtwin(void)
{
    uint32_t stat;

    if (callrequest == CR_UPDTWIN)
    {
        callrequest = CR_NONE;
        stat = 1;
    }

    else
    {
        stat = 0;
    }

    return stat;
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

    stat = compare(0, (uint8_t *)"+QCCID:", 7);
    return stat;
}

/**************************************************************************
 *                            e_rxconnect
 *                            --------
 *
 */

static uint32_t e_rxconnect(void)
{
    uint32_t stat;

    stat = compare(0, (uint8_t *)"CONNECT", 7);
    return stat;
}



/**************************************************************************
 *                            e_rxconnok
 *                            ---------
 *
 */

static uint32_t e_rxconnok(void)
{
    uint32_t stat;

    stat = compare(0, (uint8_t *)"+QMTCONN: 0,0,0", 15);
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

    stat = compare(0, (uint8_t *)"+CPIN: READY", 12);
    return stat;
}


/**************************************************************************
 *                            e_rxcr
 *                            ------
 *
 */

static uint32_t e_rxcr(void)
{
    return rcv_buffer[0] == 0x0D;
}

/**************************************************************************
 *                            e_rxcsq
 *                            ------
 *
 */

static uint32_t e_rxcsq(void)
{
    uint32_t stat;

    stat = compare(0, (uint8_t *)"+CSQ:", 5);
    return stat;
}


/**************************************************************************
 *                            e_rxgt
 *                            ------
 *
 */

static uint32_t e_rxgt(void)
{

    if ((rcv_count > 0) && (rcv_buffer[0] == '>'))
    {
        return 1;
    }

    return 0;
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
 *                            e_rxjsonend
 *                            --------
 *
 */

static uint32_t e_rxjsonend(void)
{
    uint32_t stat;
    uint32_t i;

    stat = 0;

    for (i = 0; i < (rcv_count - 1); i++)
    {

       if ((rcv_buffer[i] == '}') && (rcv_buffer[i + 1] == '\"'))
       {
           stat = 1;
           break;
       }

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
 *                            e_rxopenok
 *                            ---------
 *
 */

static uint32_t e_rxopenok(void)
{
    uint32_t stat;

    stat = compare(0, (uint8_t *)"+QMTOPEN: 0,0", 12);
    return stat;
}

/**************************************************************************
 *                            e_rxqcfg
 *                            ------
 *
 */

static uint32_t e_rxqcfg(void)
{
    uint32_t stat;

    stat = compare(0, (uint8_t *)"+QCFG:", 6);
    return stat;
}

/**************************************************************************
 *                            e_rxqhtget
 *                            ------
 *
 */

static uint32_t e_rxqhtget(void)
{
    uint32_t stat;

    stat = compare(0, (uint8_t *)"+QHTTPGET: 0,", 13);
    return stat;
}


/**************************************************************************
 *                            e_rxqhtpost
 *                            ------
 *
 */

static uint32_t e_rxqhtpost(void)
{
    uint32_t stat;

    stat = compare(0, (uint8_t *)"+QHTTPPOST: 0,", 14);
    return stat;
}

/**************************************************************************
 *                            e_rxqhtrdfile
 *                            ------
 *
 */

static uint32_t e_rxqhtrdfile(void)
{
    uint32_t stat;

    stat = compare(0, (uint8_t *)"+QHTTPREADFILE: 0", 17);
    return stat;
}


/**************************************************************************
 *                            e_rxqhtread
 *                            ------
 *
 */

static uint32_t e_rxqhtread(void)
{
    uint32_t stat;

    stat = compare(0, (uint8_t *)"+QHTTPREAD: 0", 13);
    return stat;
}

/**************************************************************************
 *                            e_rxqmtclose
 *                            ------
 *
 */

static uint32_t e_rxqmtclose(void)
{
    uint32_t stat;

    stat = compare(0, (uint8_t *)"+QMTCLOSE:", 10);
    return stat;
}

/**************************************************************************
 *                            e_rxqmtdisc
 *                            ------
 *
 */

static uint32_t e_rxqmtdisc(void)
{
    uint32_t stat;

    stat = compare(0, (uint8_t *)"+QMTDISC:", 9);
    return stat;
}

/**************************************************************************
 *                            e_rxqmtpub
 *                            ------
 *
 */

static uint32_t e_rxqmtpub(void)
{
    uint32_t stat;

    stat = compare(0, (uint8_t *)"+QMTPUB:", 8);
    return stat;
}




/**************************************************************************
 *                            e_rxqmtrecv
 *                            ------
 *
 */

static uint32_t e_rxqmtrecv(void)
{
    uint32_t stat;

    stat = compare(0, (uint8_t *)"+QMTRECV: ", 10);
    return stat;
}

/**************************************************************************
 *                            e_rxqmtstat
 *                            ---------
 *
 */

static uint32_t e_rxqmtstat(void)
{
    uint32_t stat;

    stat = compare(0, (uint8_t *)"+QMTSTAT:", 9);
    return stat;
}

/**************************************************************************
 *                            e_rxqmtsub
 *                            ------
 *
 */

static uint32_t e_rxqmtsub(void)
{
    uint32_t stat;

    stat = compare(0, (uint8_t *)"+QMTSUB:", 8);
    return stat;
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
 *                            e_rxtime
 *                            -------
 *
 */

static uint32_t e_rxtime(void)
{
    uint32_t stat;

    stat = 0;

    if (rcv_count >= 29)
    {
        stat = compare(0, (uint8_t *)"+QNTP: 0,", 9);
    }

    return stat;
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



/**************************************************************************
 *                            e_updcacert
 *                            ---------
 *
 */

static uint32_t e_updcacert(void)
{
    return ca_certificate_flag != 0;
}


/**************************************************************************
 *                            e_updclcert
 *                            ---------
 *
 */

static uint32_t e_updclcert(void)
{
    return client_certificate_flag != 0;
}

/**************************************************************************
 *                            e_updclkey
 *                            ---------
 *
 */

static uint32_t e_updclkey(void)
{
    return client_key_flag != 0;
}

/**************************************************************************
 *                            e_updfver
 *                            ---------
 *
 */

static uint32_t e_updfver(void)
{
    return update_device_firmware_flag != 0;
}


/**************************************************************************
 *                            e_updmver
 *                            ---------
 *
 */

static uint32_t e_updmver(void)
{
    return update_module_firmware_flag != 0;
}



/**************************************************************************
 *                            e_updsconfig
 *                            ---------
 *
 */

static uint32_t e_updsconfig(void)
{
    return configuration_service_flag != 0;
}



/***************************************************************************
 *                               ACTIONS
 **************************************************************************/

/**************************************************************************
 *                               a_accumjson
 *                               ---------
 *
 */

static void a_accumjson(void)
{
    uint32_t i;

    i = 0;

    if (json_count == 0)
    {

        for (i = 0; i < (rcv_count - 1); i++)
        {

            if ((rcv_buffer[i] == '{') || (rcv_buffer[i] == 0x0D))
            {
                break;
            }

        }

    }

    for (; i < (rcv_count - 1); i++)
    {
        json_response[json_count] = rcv_buffer[i];
        json_count++;
    }

    return;
}

/**************************************************************************
 *                               a_ansbadcfg
 *                               ---------
 *
 */

static void a_ansbadcfg(void)
{
    callanswer = CA_BADCONFIG;
    return;
}


/**************************************************************************
 *                               a_ansconnected
 *                               ---------
 *
 */

static void a_ansconnected(void)
{
    callanswer = CA_CONNECTED;
    return;
}



/**************************************************************************
 *                               a_ansinprogress
 *                               ---------
 *
 */

static void a_ansinprogress(void)
{
    callanswer = CA_INPROGRESS;
    return;
}

/**************************************************************************
 *                               a_ansncvmatch
 *                               ---------
 *
 */

static void a_ansneedping(void)
{

    return;
}


/**************************************************************************
 *                               a_ansnopower
 *                               ---------
 *
 */

static void a_ansnopower(void)
{
    callanswer = CA_NOTPOWERED;
    return;
}

/**************************************************************************
 *                               a_anspowered
 *                               ---------
 *
 */

static void a_anspowered(void)
{
    callanswer = CA_POWERED;
    return;
}


/**************************************************************************
 *                               a_buildpost
 *                               ---------
 *
 */

static void a_buildpost(void)
{
    int32_t txbuflen;

    txbuflen = sprintf(post_buffer, "POST /cs/%s HTTP/1.1\r\n", device_id);
    sprintf(&post_buffer[txbuflen], "%s", postmsg);
    postbuffer_count = txbuflen + (sizeof(postmsg) - 1);

	txbuflen = sprintf(&post_buffer[postbuffer_count],
	                   "{\"model\":\"%s\",\"sim\":\"%s\",\"imei\":\"%s\"",
					   blipgo_model,
					   ccid,
					   imei);
    postbuffer_count += txbuflen;

	txbuflen = sprintf(&post_buffer[postbuffer_count],
	                   ",\"imsi\":\"%s\",\"cv\":%s,\"fv\":\"%s\",\"pv\":\"%s\",\"mv\":\"%s\"}\r\n",
					   imsi,
					   configuration_version,
					   firmware_version,
					   protocol_version,
					   module_firmware_version);
    postbuffer_count += txbuflen;

    return;
}


/**************************************************************************
 *                               a_clrupdca
 *                               ---------
 *
 */

static void a_clrupdca(void)
{
    ca_certificate_flag = 0;
    return;
}


/**************************************************************************
 *                               a_clrupdcc
 *                               ---------
 *
 */

static void a_clrupdcc(void)
{
    client_certificate_flag = 0;
    return;
}


/**************************************************************************
 *                               a_clrupdck
 *                               ---------
 *
 */

static void a_clrupdck(void)
{
    client_key_flag = 0;
    return;
}

/**************************************************************************
 *                               a_clrupdfv
 *                               ---------
 *
 */

static void a_clrupdfv(void)
{
    update_device_firmware_flag = 0;
    return;
}

/**************************************************************************
 *                               a_clrupdmv
 *                               ---------
 *
 */

static void a_clrupdmv(void)
{
    update_module_firmware_flag = 0;
    return;
}



/**************************************************************************
 *                               a_clrupdsc
 *                               ---------
 *
 */

static void a_clrupdsc(void)
{
    configuration_service_flag = 0;
    return;
}


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
 *                               a_increcnum
 *                               ----------
 *
 */

static void a_increcnum(void)
{
//    mem_read_address += 17;
    recnum++;
    return;
}

/**************************************************************************
 *                               a_initjson
 *                               ---------
 *
 */

static void a_initjson(void)
{

    for (json_count = 0; json_count < JRBUF_LEN; json_count++)
    {
        json_response[json_count] = 0x00;
    }

    json_count = 0;
    return;
}


/**************************************************************************
 *                               a_initrecnum
 *                               ----------
 *
 */

static void a_initrecnum(void)
{
//    mem_read_address += 17;
    recnum = 0;
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
 *                               a_powerdn
 *                               -----
 *
 */

static void a_powerdn(void)
{
    HAL_GPIO_WritePin(GSM_RESET_PORT, GSM_RESET_PIN, GPIO_PIN_SET);

#if OLD_BOARD == 0
    quec_3v8enbl(0);
#endif

    callanswer = CA_NOTPOWERED;
    return;
}


/**************************************************************************
 *                               a_powerup
 *                               -----
 *
 */

static void a_powerup(void)
{

#if OLD_BOARD == 0
    quec_3v8enbl(1);
#endif

    HAL_GPIO_WritePin(GSM_RESET_PORT, GSM_RESET_PIN, GPIO_PIN_SET);
    HAL_Delay(2500);
    HAL_GPIO_WritePin(GSM_RESET_PORT, GSM_RESET_PIN, GPIO_PIN_RESET);
    callanswer = CA_INPROGRESS;
    return;
}

/**************************************************************************
 *                               a_processjson
 *                               ---------
 *
 */

static void a_processjson(void)
{
    compare_cvs();
    return;
}

/**************************************************************************
 *                               a_processping
 *                               ---------
 *
 */

static void a_processping(void)
{
    pingstat = ping_response();
    return;
}

/**************************************************************************
 *                               a_processsc
 *                               ---------
 *
 */

static void a_processsc(void)
{
    configuration_service();
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
    rcv_status = quec_receive(rcv_buffer, MAX_RECEIVE_LEN, 2000, &gsm_rcv_ih);
    return;
}

/**************************************************************************
 *                               a_rcvqmt
 *                               ---------
 *
 */

static void a_rcvqmt(void)
{
    rcv_status = quec_receive(rcv_buffer, MAX_RECEIVE_LEN, 30000, &gsm_rcv_ih);
    return;
}


/**************************************************************************
 *                               a_reset
 *                               -------
 *
 */

static void a_reset(void)
{
    HAL_GPIO_WritePin(GSM_RESET_PORT, GSM_RESET_PIN, GPIO_PIN_SET);
    quec_rxflush();
    HAL_Delay(500);
    HAL_GPIO_WritePin(GSM_RESET_PORT, GSM_RESET_PIN, GPIO_PIN_RESET);
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

        ccid[i] = rcv_buffer[i + 8];
    }

    myiccidlen = i;
    return;
}

/**************************************************************************
 *                               a_savegmr
 *                               ----------
 *
 */

static void a_savegmr(void)
{
    uint32_t i;

    for (i = 0; i < MFVLEN; i++)
    {
        module_firmware_version[i] = 0;
    }

    for (i = 0; i < MFVLEN; i++)
    {


        if (rcv_buffer[i] == 0x0D)
        {
            break;
        }

        module_firmware_version[i] = rcv_buffer[i];
    }

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
        imei[i] = rcv_buffer[i];
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
        imsi[i] = rcv_buffer[i];
    }

    return;
}

/**************************************************************************
 *                               a_savetime
 *                               ----------
 *
 */

static void a_savetime(void)
{
    server_clock = convert_to_epoch(&rcv_buffer[8], &rcv_buffer[19]);
    return;
}


/**************************************************************************
 *                               a_tmr10
 *                               ---------
 *
 */

static void a_tmr10(void)
{
    __disable_irq();
    timer = 10000;
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
    rcv_status = quec_receive(rcv_buffer, 16, 30000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit((uint8_t *)apnmsg, sizeof(apnmsg) - 1);
    }

    return;
}


/**************************************************************************
 *                               a_trate0
 *                               --------
 *
 */

static void a_trate0(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 16, 5000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit((uint8_t *)"ATE0\r\n", 6);
    }

    return;
}


/**************************************************************************
 *                               a_trcgreg
 *                               --------
 *
 */

static void a_trcgreg(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 16, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit((uint8_t *)"AT+CGREG=0\r\n", 12);
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
 *                               a_trcreg
 *                               --------
 *
 */

static void a_trcreg(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 16, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit((uint8_t *)"AT+CREG=0\r\n", 11);
    }

    return;
}


/**************************************************************************
 *                               a_trcsq
 *                               --------
 *
 */

static void a_trcsq(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 16, 5000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit((uint8_t *)"AT+CSQ\r\n", 8);
    }

    return;
}

/**************************************************************************
 *                               a_trgmr
 *                               -------
 *
 */

static void a_trgmr(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 32, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit((uint8_t *)"AT+QGMR\r\n", 9);
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
 *                               a_trhtcfgc
 *                               -------
 *
 */

static void a_trhtcfgc(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 10, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit((uint8_t *)"AT+QHTTPCFG=\"contextid\",1\r\n", 26);
    }

    return;
}

/**************************************************************************
 *                               a_trhtcfgrqh
 *                               -------
 *
 */

static void a_trhtcfgrqh(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 10, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit((uint8_t *)"AT+QHTTPCFG=\"requestheader\",1\r\n", 31);
    }

    return;
}

/**************************************************************************
 *                               a_trhtcfgrqh2
 *                               -------
 *
 */

static void a_trhtcfgrqh2(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 10, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit((uint8_t *)"AT+QHTTPCFG=\"requestheader\",0\r\n", 31);
    }

    return;
}


/**************************************************************************
 *                               a_trhtget
 *                               -------
 *
 */

static void a_trhtget(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 16, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit((uint8_t *)"AT+QHTTPGET=80\r\n", 16);
    }

    return;
}

/**************************************************************************
 *                               a_trhtpost1
 *                               -------
 *
 */

static void a_trhtpost1(void)
{
    uint32_t txbuflen;

    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 16, 125000, &gsm_rcv_ih);
    txbuflen = sprintf(tx_buffer, "AT+QHTTPPOST=%u,80,80\r\n", postbuffer_count);

    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit(tx_buffer, txbuflen);
    }

    return;
}

/**************************************************************************
 *                               a_trhtpost2
 *                               -------
 *
 */

static void a_trhtpost2(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 16, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit(post_buffer, postbuffer_count);
    }

    return;
}


/**************************************************************************
 *                               a_trhtrdfile
 *                               -------
 *
 */

static void a_trhtrdfile(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 16, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit((uint8_t *)"AT+QHTTPREADFILE=\"UFS:security/CaCert1.crt\",80\r\n", 48);
    }

    return;
}

/**************************************************************************
 *                               a_trhtrdfile2
 *                               -------
 *
 */

static void a_trhtrdfile2(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 16, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit((uint8_t *)"AT+QHTTPREADFILE=\"UFS:security/Client.crt\",80\r\n", 47);
    }

    return;
}

/**************************************************************************
 *                               a_trhtrdfile3
 *                               -------
 *
 */

static void a_trhtrdfile3(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 16, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit((uint8_t *)"AT+QHTTPREADFILE=\"UFS:security/key.pem\",80\r\n", 44);
    }

    return;
}

/**************************************************************************
 *                               a_trhtread
 *                               -------
 *
 */

static void a_trhtread(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 16, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit((uint8_t *)"AT+QHTTPREAD=80\r\n", 17);
    }

    return;
}



/**************************************************************************
 *                               a_trhturl2
 *                               -------
 *
 */

static void a_trhturl2(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 10, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit(post_buffer, urlplen);
    }

    return;
}


/**************************************************************************
 *                               a_trhturlca
 *                               -------
 *
 */

static void a_trhturlca(void)
{
    uint32_t txlen;

    urlplen = sprintf(post_buffer, "https://shark.carematix.com/cs/%s/ca?token=%s", device_id, http_token);
    txlen = sprintf(tx_buffer, "AT+QHTTPURL=%u,80\r\n", urlplen);

    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 10, 2000, &gsm_rcv_ih);


    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit(tx_buffer, txlen);
    }

    return;
}

/**************************************************************************
 *                               a_trhturlcc
 *                               -------
 *
 */

static void a_trhturlcc(void)
{
    uint32_t txlen;

    urlplen = sprintf(post_buffer, "https://shark.carematix.com/cs/%s/cc?token=%s", device_id, http_token);
    txlen = sprintf(tx_buffer, "AT+QHTTPURL=%u,80\r\n", urlplen);

    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 10, 2000, &gsm_rcv_ih);


    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit(tx_buffer, txlen);
    }

    return;
}

/**************************************************************************
 *                               a_trhturlck
 *                               -------
 *
 */

static void a_trhturlck(void)
{
    uint32_t txlen;

    urlplen = sprintf(post_buffer, "https://shark.carematix.com/cs/%s/ck?token=%s", device_id, http_token);
    txlen = sprintf(tx_buffer, "AT+QHTTPURL=%u,80\r\n", urlplen);

    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 10, 2000, &gsm_rcv_ih);


    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit(tx_buffer, txlen);
    }

    return;
}



/**************************************************************************
 *                               a_trhturlp
 *                               -------
 *
 */

static void a_trhturlp(void)
{
    uint32_t txlen;

    urlplen = sprintf(post_buffer, "https://shark.carematix.com");
    txlen = sprintf(tx_buffer, "AT+QHTTPURL=%u,80\r\n", urlplen);

    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 10, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit(tx_buffer, txlen);
    }

    return;
}


/**************************************************************************
 *                               a_trhturlsc
 *                               -------
 *
 */

static void a_trhturlsc(void)
{
    uint32_t txlen;

    urlplen = sprintf(post_buffer, "https://shark.carematix.com/cs/%s/sc?token=%s", device_id, http_token);
    txlen = sprintf(tx_buffer, "AT+QHTTPURL=%u,80\r\n", urlplen);

    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 10, 2000, &gsm_rcv_ih);


    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit(tx_buffer, txlen);
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


/**************************************************************************
 *                               a_trqiact
 *                               ---------
 *
 */

static void a_trqiact(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 16, 160000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit((uint8_t *)"AT+QIACT=1\r\n", 12);
    }

    return;
}



/**************************************************************************
 *                               a_trqmtcfg1
 *                               -----------
 *
 */

static void a_trqmtcfg1(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 16, 5000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {

        quec_transmit((uint8_t *)qmtcfg1msg, sizeof(qmtcfg1msg) - 1);
    }

    return;
}

/**************************************************************************
 *                               a_trqmtcfg2
 *                               -----------
 *
 */

static void a_trqmtcfg2(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 16, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {

        quec_transmit((uint8_t *)qmtcfg2msg, sizeof(qmtcfg2msg) - 1);
    }

    return;
}

/**************************************************************************
 *                               a_trqmtclose
 *                               ---------
 *
 */

static void a_trqmtclose(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 16, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {

        quec_transmit((uint8_t *)"AT+QMTCLOSE=0\r\n", 15);
    }

    return;
}

/**************************************************************************
 *                               a_trqmtconn
 *                               -----------
 *
 */

static void a_trqmtconn(void)
{

    int32_t txlen;

	txlen = sprintf(tx_buffer, "AT+QMTCONN=0,\"%s\",\"%s\",\"%s\"\r\n", mqtt_client_id, mqtt_username, mqtt_password);

    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 16, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {

        quec_transmit(tx_buffer, txlen);
    }

    return;
}

/**************************************************************************
 *                               a_trqmtdisc
 *                               ---------
 *
 */

static void a_trqmtdisc(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 16, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {

        quec_transmit((uint8_t *)"AT+QMTDISC=0\r\n", 14);
    }

    return;
}

/**************************************************************************
 *                               a_trqmtopen
 *                               -----------
 *
 */

static void a_trqmtopen(void)
{
    int32_t txlen;

    txlen = sprintf(tx_buffer, "AT+QMTOPEN=0,\"%s\",%s\r\n\0", mqtt_server, mqtt_port);

    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 16, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {

        quec_transmit(tx_buffer, txlen);
    }

    return;
}

/**************************************************************************
 *                               a_trqmtpub
 *                               -----------
 *
 */

static void a_trqmtpub(void)
{
    int32_t txlen;

    txlen = sprintf(tx_buffer, "AT+QMTPUB=0,1,1,0,\"%s\"\r\n", twin_pb);

    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 20, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {

        quec_transmit(tx_buffer, txlen);
    }

    return;
}

/**************************************************************************
 *                               a_trqmtpub2
 *                               -----------
 *
 */

static void a_trqmtpub2(void)
{
    int32_t txlen;

    txlen = sprintf(tx_buffer, "AT+QMTPUB=0,1,1,0,\"devices/%s/messages/events/\"\r\n", device_id);

    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 20, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {

        quec_transmit(tx_buffer, txlen);
    }

    return;
}


/**************************************************************************
 *                               a_trqmtpub3
 *                               -----------
 *
 */

static void a_trqmtpub3(void)
{
    int32_t txlen;

    txlen = sprintf(tx_buffer, "AT+QMTPUB=0,1,1,0,\"%s\"\r\n", twin_rp);

    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 20, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {

        quec_transmit(tx_buffer, txlen);
    }

    return;
}



/**************************************************************************
 *                               a_trqmtsub
 *                               -----------
 *
 */

static void a_trqmtsub(void)
{
    int32_t txlen;

    txlen = sprintf(tx_buffer, "AT+QMTSUB=0,1,\"%s\",0\r\n", twin_sb);

    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 20, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {

        quec_transmit(tx_buffer, txlen);
    }

    return;
}

/**************************************************************************
 *                               a_trqntp
 *                               -----------
 *
 */

static void a_trqntp(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 64, 5000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {

        quec_transmit((uint8_t *)"AT+QNTP=1,\"0.us.pool.ntp.org\"\r\n", 31);
    }

    return;
}

/**************************************************************************
 *                               a_trqssl1
 *                               ---------
 *
 */

static void a_trqssl1(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 16, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {

        quec_transmit((uint8_t *)qssl1msg, sizeof(qssl1msg) - 1);
    }

    return;
}

/**************************************************************************
 *                               a_trqssl2
 *                               ---------
 *
 */

static void a_trqssl2(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 16, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {

        quec_transmit((uint8_t *)qssl2msg, sizeof(qssl2msg) - 1);
    }

    return;
}


/**************************************************************************
 *                               a_trqssl3
 *                               ---------
 *
 */

static void a_trqssl3(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 16, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {

        quec_transmit((uint8_t *)qssl3msg, sizeof(qssl3msg) - 1);
    }

    return;
}

/**************************************************************************
 *                               a_trqssl4
 *                               ---------
 *
 */

static void a_trqssl4(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 16, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {

        quec_transmit((uint8_t *)qssl4msg, sizeof(qssl4msg) - 1);
    }

    return;
}

/**************************************************************************
 *                               a_trqssl5
 *                               ---------
 *
 */

static void a_trqssl5(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 16, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {

        quec_transmit((uint8_t *)qssl5msg, sizeof(qssl5msg) - 1);
    }

    return;
}

/**************************************************************************
 *                               a_trqssl6
 *                               ---------
 *
 */

static void a_trqssl6(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 16, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {

        quec_transmit((uint8_t *)qssl6msg, sizeof(qssl6msg) - 1);
    }

    return;
}



/**************************************************************************
 *                               a_trqurccfg
 *                               -----------
 *
 */

static void a_trqurccfg(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 16, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {

        quec_transmit((uint8_t *)qurccfgmsg, sizeof(qurccfgmsg) - 1);
    }

    return;
}

/**************************************************************************
 *                               a_txnul
 *                               -------
 *
 */

static void a_txnul(void)
{
    quec_transmit((uint8_t *)&nullmsg, 1);
    return;
}

/**************************************************************************
 *                               a_txrecord
 *                               -----------
 *
 */

static void a_txrecord(void)
{
    uint16_t txlen;

    txlen = publish_meter_readings(post_buffer);
    quec_transmit(post_buffer, txlen);
    return;
}

/**************************************************************************
 *                               a_txsub
 *                               -------
 *
 */

static void a_txsub(void)
{
    HAL_Delay(2);
    quec_transmit((uint8_t *)&submsg, 1);

    return;
}


/**************************************************************************
 *                               a_txtwin
 *                               -------
 *
 */

static void a_txtwin(void)
{
    uint16_t txlen;

    txlen = sprintf((char*)post_buffer, "{\"fv\":\"%s\",\"pv\":\"%s\",\"cv\":\"%s\",\"mv\":\"%s\",\"sim\":\"%s\",\"imei\":\"%s\",\"imsi\":\"%s\"}\r\n",
                       firmware_version,
                       protocol_version,
                       configuration_version,
                       module_firmware_version,
                       ccid,
                       imei,
                       imsi);
    quec_transmit(post_buffer, txlen);
    return;
}




/**************************************************************************
 *                               a_txz
 *                               -------
 *
 */

static void a_txz(void)
{
    HAL_Delay(2);
    quec_transmit((uint8_t *)"XXX", 3);

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

static uint32_t compare(uint32_t index, uint8_t *response, uint32_t count)
{
    uint32_t i;
    uint32_t stat;

    stat = 1;

    for (i = 0; i < count; i++)
    {

        if (response[i] != rcv_buffer[i + index])
        {
            stat = 0;
            break;
        }

    }

    return stat;
}





/***************************************************************************
 *                         gsm_rcv_ih
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

/***************************************************************************
 *                         state_trace
 *                         ------------
 *
 * Set yellow led on/off as directed
 *
 * param[in] - ledstate
 *
 * return - none
 */

static void state_trace(uint32_t evnum)
{

    if (gsm_stmachine.sms_curstate != oldstate)
    {
        debug_printf(DBGLVL_MAX, (uint8_t *)"GSM STATE: %u -> %u, EVENT %u\r\n", oldstate, gsm_stmachine.sms_curstate, evnum);
    }

    oldstate = gsm_stmachine.sms_curstate;
    return;
}
/*
 * End of module.
 */

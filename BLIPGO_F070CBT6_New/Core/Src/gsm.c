
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
 *
 * This module contains the high level BG95 handler.  It contains the
 * logic for managing connections with the Carematix server.  The actual
 * data comm is handled by a lower level driver in module quectel.com.
 * Communication with the BG95 is done using Hayes modem protocol
 * type messages over a serial port.  The logic in this module is
 * implemented using a finite state machine.  The actions of this module
 * are contrtoled by the module master.c.  master.c controls our actions
 * by sending requests.  We process the request then provide an answer
 * back to master.c when complete or if we were unable to comply.  The
 * following requests can be made:
 *
 * None - This is the power up state.  It is the same as Not Powered.
 *
 * Not Powered - The power to the BG95 is disabled.  From this state
 *               we can only transition to Powered.
 *
 * Powered - In this state the BG95 will be powered and able to
 *           communicate with the network.  When transitioning from
 *           Not Powered to Powered we will fetch IMEI, CCID, RSSI, send
 *           our API, and register with the network.
 *
 * Clock - In this state we contact the time of day server and update
 *         our internal time of day.
 *
 * Connect - In this state we are connected to the MQTT server.  From
 *           this state we can transition as follows:
 *
 * Send Records - In this state we send the records stored in serial flash
 *                to the Carematix server.
 *
 * Read Twin - In this state we contact the Carematix server and read
 *             our twin information.
 *
 * Update Twin - In this state we are sending our configuration information
 *               to the server in order to make sure the twin is equal to us.
 *
 * Ping - In this state we ping the server.  The server will tell us which
 *        of our configuration items are out of date.
 *
 * Update Config - In this state we are getting the latest configuration
 *                 items from the server as indicated from the ping.
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
 **************************************************************************
 *
 * These #define-s define the states.  The value is the index into
 * state_table where the address of the state table entry can be found that
 * corresponds to that state.
 */

#define S_INIT 0
#define S_WTAT 1
#define S_IDLE 2
#define S_XATE0 3
#define S_WTATE0 4
#define S_WTCSQ 5
#define S_CHKRSPCSQ 6
#define S_INIT2 7
#define S_WTOKAPN 8
#define S_INIT3WT 9
#define S_CHKOKAPN 10
#define S_WTAPN 11
#define S_CHKRSPAPN 12
#define S_XCMEE 13
#define S_WTCMEE 14
#define S_WTQCFG 15
#define S_CHKRSPQCFG 16
#define S_CHKRSSI 17
#define S_WAITRSSI 18
#define S_XAT 19
#define S_WTAT 20
#define S_RSPAT 21
#define S_WTGSN 22
#define S_CHKRSPGSN 23
#define S_WTCIMI 24
#define S_CHKRSPCIMI 25
#define S_WTQCCID 26
#define S_CHKRSPQCCID 27
#define S_XGMR 28
#define S_XQCFG 29
#define S_XCFUN0 30
#define S_XCFUN1 31
#define S_CHKOKGSN 32
#define S_WTOKGSN 33
#define S_XCIMI 34
#define S_XCGREGQ 35
#define S_WTCFUN0 36
#define S_RSPCFUN0 37
#define S_WTGMR 38
#define S_CHKRSPGMR 39
#define S_WTOKGMR 40
#define S_CHKOKGMR 41
#define S_WTCFUN1 42
#define S_RSPCFUN1 43
#define S_WTQMTCFG1 44
#define S_CHKRSPQMTCFG1 45
#define S_WTQMTCFG2 46
#define S_CHKRSPQMTCFG2 47
#define S_WTQSSL1 48
#define S_CHKRSPQSSL1 49
#define S_WTQSSL2 50
#define S_CHKRSPQSSL2 51
#define S_WTQSSL3 52
#define S_CHKRSPQSSL3 53
#define S_WTQSSL4 54
#define S_CHKRSPQSSL4 55
#define S_WTQSSL5 56
#define S_CHKRSPQSSL5 57
#define S_WTQSSL6 58
#define S_CHKRSPQSSL6 59
#define S_WTQMTCON 60
#define S_CHKRSPQMTCON 61
#define S_WTQMTOPEN 62
#define S_CHKRSPQMTOPEN 63
#define S_CONNECTED 64
#define S_WTOKQMTOPEN 65
#define S_CHKOKQMTOPEN 66
#define S_WTOKQMTCON 67
#define S_CHKOKQMTCON 68
#define S_CLOCK 69
#define S_CLOCKWT 70
#define S_CLOCKRSP 71
#define S_CLOCKDELAY 72
#define S_SENDRECS 73
#define S_VCSUBWT 74
#define S_VCSUBRSP 75
#define S_VCPUBWT 76
#define S_VCPUBRSP 77
#define S_VCPUBWTOUT 78
#define S_VCPUBDATAWT 79
#define S_VCPUBTMOUT 80
#define S_VERIFYCV 81
#define S_VCPUBDATA1 82
#define S_VCJSON 83
#define S_VCJSONWT 84
#define S_VCJSONRSP 85
#define S_PING 86
#define S_PHTCFGCWT 87
#define S_PHTCFGCRSP 88
#define S_PHTCFGRQHWT 89
#define S_PHTCFGRQHRSP 90
#define S_PHTURL1WT 91
#define S_PHTURL1RSP 92
#define S_PHTURL2WT 93
#define S_PHTURL2RSP 94
#define S_PHTPOST1WT 95
#define S_PHTPOST1RSP 96
#define S_PHTPOST2WT 97
#define S_PHTPOST2RSP 98
#define S_PHTREADWT 99
#define S_PHTREADRSP 100
#define S_PINGCHK 101
#define S_SRSUBWT 102
#define S_SRSUBRSP 103
#define S_SRPUBWT 104
#define S_SRPUBTMOUT 105
#define S_SRPUBRSP 106
#define S_SRPUBWTOUT 107
#define S_SRPUBDATAWT 108
#define S_SRPUBDATA1 109
#define S_SRJSON 110
#define S_SRJSONWT 111
#define S_SRPUBDATA2 112
#define S_SRPUBRECWT 113
#define S_SRPUBRECRSP 114
#define S_SRRECOUTWT 115
#define S_SRRECSUBWT 116
#define S_SRRECRSP 117
#define S_MQTDISC 118
#define S_MQTDISCWT 119
#define S_MQTDISCRSP 120
#define S_XAPN 121
#define S_XGSN 122
#define S_UPDATECFG 123
#define S_CAHTCFGCWT 124
#define S_CAHTCFGCRSP 125
#define S_CAHTCFGRQHWT 126
#define S_CAHTCFGRQHRSP 127
#define S_CAHTURL1WT 128
#define S_CAHTURL1RSP 129
#define S_CAHTURL2WT 130
#define S_CAHTURL2RSP 131
#define S_CAHTGETWT 132
#define S_CAHTGETRSP 133
#define S_CAHTRDFILEWT 134
#define S_CAHTRDFILERSP 135
#define S_CCHTCFGCWT 136
#define S_CCHTCFGCRSP 137
#define S_CCHTCFGRQHWT 138
#define S_CCHTCFGRQHRSP 139
#define S_CCHTURL1WT 140
#define S_CCHTURL1RSP 141
#define S_CCHTURL2WT 142
#define S_CCHTURL2RSP 143
#define S_CCHTGETWT 144
#define S_CCHTGETRSP 145
#define S_CCHTRDFILEWT 146
#define S_CCHTRDFILERSP 147
#define S_CKHTCFGCWT 148
#define S_CKHTCFGCRSP 149
#define S_CKHTCFGRQHWT 150
#define S_CKHTCFGRQHRSP 151
#define S_CKHTURL1WT 152
#define S_CKHTURL1RSP 153
#define S_CKHTURL2WT 154
#define S_CKHTURL2RSP 155
#define S_CKHTGETWT 156
#define S_CKHTGETRSP 157
#define S_CKHTRDFILEWT 158
#define S_CKHTRDFILERSP 159
#define S_UPDTWIN 160
#define S_UTPUBWT 161
#define S_UTPUBTMOUT 162
#define S_UTPUBWTOUT 163
#define S_UTPUBDATAWT 164
#define S_UTPUBRSP 165
#define S_SCHTCFGCWT 166
#define S_SCHTCFGCRSP 167
#define S_SCHTCFGRQHWT 168
#define S_SCHTCFGRQHRSP 169
#define S_SCHTURL1WT 170
#define S_SCHTURL1RSP 171
#define S_SCHTURL2WT 172
#define S_SCHTURL2RSP 173
#define S_SCHTGETWT 174
#define S_SCHTGETRSP 175
#define S_SCHTREADWT 176
#define S_SCHTREADRSP 177
#define S_WTCGREGQ 178
#define S_CHKRSPCGREGQ 179
#define S_CHECKREGD 180
#define S_WTCEREGQ 181
#define S_CHKRSPCEREGQ 182
#define S_NOTREGD 183
#define S_WTCOPSQ 184
#define S_CHECKREGD2 185
#define S_POWERDN 186
#define S_QPDOWNWT 187
#define S_INIT2WT 188
#define S_INITWT 189
#define S_XCPINQ 190
#define S_WTCPINQ 191
#define S_RSPCPINQ 192
#define S_XQCCID 193
#define S_XCSQ 194




#define POSTBUFLEN 400
#define TXBUFLEN 150
#define MAX_RECEIVE_LEN 400

/***************************************************************************
 *                               TYPEDEFS
 **************************************************************************/

/***************************************************************************
 *                         LOCAL FUNCTION PROTOTYPES
 **************************************************************************/

static void a_accumjson(void);
static void a_ansbadcfg(void);
static void a_ansconnected(void);
static void a_ansinprogress(void);
static void a_ansneedping(void);
static void a_ansnopower(void);
static void a_anspowered(void);
static void a_buildpost(void);
static void a_clrpingfailed(void);
static void a_clrupdca(void);
static void a_clrupdcc(void);
static void a_clrupdck(void);
static void a_clrupdfv(void);
static void a_clrupdmv(void);
static void a_clrupdsc(void);
static void a_flushrx(void);

static void a_increcnum(void);
static void a_initjson(void);
static void a_initrecnum(void);
static void a_initvars(void);
static void a_nop(void);
static void a_powerup1(void);
static void a_powerup2(void);
static void a_processjson(void);
static void a_processping(void);
static void a_processsc(void);
static void a_rcvdata(void);
static void a_rcvinitmsgs(void);
static void a_rcvqmt(void);
static void a_saveccid(void);
static void a_savecsq(void);
static void a_savegmr(void);
static void a_saveimei(void);
static void a_saveimsi(void);
static void a_saveregq(void);
static void a_savetime(void);
static void a_setpingfailed(void);
static void a_t2mr5min(void);
static void a_t2mr60(void);
static void a_tmr1(void);
static void a_tmr10(void);
static void a_trapn(void);
static void a_trate0(void);
static void a_trcimi(void);
static void a_trceregq(void);
static void a_trcfun0(void);
static void a_trcfun1(void);
static void a_trcgreg(void);
static void a_trcgregq(void);
static void a_trcmee(void);
static void a_trcopsq(void);
static void a_trcpinq(void);
static void a_trcreg(void);

static void a_trcsq(void);
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
static void a_trqcfg2(void);
static void a_trqiact(void);
static void a_trqlts(void);
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
static void a_trqpowd(void);
static void a_trqssl1(void);
static void a_trqssl2(void);
static void a_trqssl3(void);
static void a_trqssl4(void);
static void a_trqssl5(void);
static void a_trqssl6(void);
static void a_trqurccfg(void);
static void a_txnul(void);
static void a_txrecord(void);
static void a_txsub(void);
static void a_txtwin(void);
static void a_updatecv(void);

static uint32_t e_always(void);
static uint32_t e_eof(void);
static uint32_t e_equalcvs(void);
static uint32_t e_outidle(void);
static uint32_t e_pingok(void);
static uint32_t e_rcverr(void);
static uint32_t e_rcvok(void);
static uint32_t e_rcvovflow(void);
static uint32_t e_rcvtimout(void);
static uint32_t e_registered(void);
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
static uint32_t e_rssiok(void);
static uint32_t e_rxat(void);
static uint32_t e_rxccid(void);
static uint32_t e_rxconnect(void);
static uint32_t e_rxconnok(void);
static uint32_t e_rxcopsq(void);
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
static uint32_t e_rxregq(void);
static uint32_t e_rxtime(void);
static uint32_t e_timeout(void);
static uint32_t e_timeout2(void);
static uint32_t e_updcacert(void);
static uint32_t e_updclcert(void);
static uint32_t e_updclkey(void);
static uint32_t e_updcv(void);
static uint32_t e_updfver(void);
static uint32_t e_updmver(void);
static uint32_t e_updsconfig(void);

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


static uint8_t callanswer;
static uint8_t callrequest;
static uint8_t gotcpinrflag;
static uint8_t gotrdyflag;
static const uint8_t nullmsg = 0x00;
static uint8_t pingstat;
static uint8_t rcv_status;
static uint8_t registered;
static const uint8_t submsg = 0x1A;

static uint32_t myiccidlen;
static uint32_t oldstate;
static uint32_t postbuffer_count;
static uint32_t rcv_count;
static uint32_t recnum;
static uint32_t timer;
static uint32_t timer2;
static uint32_t urlplen;

static uint8_t post_buffer[POSTBUFLEN];
static uint8_t rcv_buffer[MAX_RECEIVE_LEN + 1];
static uint8_t tx_buffer[TXBUFLEN];

static const uint8_t apnmsg[] = {"AT+QICSGP=1,1,\"data641003\",\"\",\"\",1\r\n"};
static const uint8_t qcfg1msg[] = {"AT+QCFG=\"nwscanseq\"\r\n"};
static const uint8_t qcfg2msg[] = {"AT+QCFG=\"iotopmode\",0,1\r\n"};
static const uint8_t qurccfgmsg[] = {"AT+QURCCFG=\"urcport\",\"uart1\"\r\n"};
static const uint8_t qmtcfg1msg[] = {"AT+QMTCFG=\"SSL\",0,1,2\r\n"};
static const uint8_t qmtcfg2msg[] = {"AT+QMTCFG=\"version\",0,4\r\n"};
static const uint8_t qssl1msg[] = {"AT+QSSLCFG=\"seclevel\",2,2\r\n"};
static const uint8_t qssl2msg[] = {"AT+QSSLCFG=\"sslversion\",2,4\r\n"};
static const uint8_t qssl3msg[] = {"AT+QSSLCFG=\"ciphersuite\",2,0xFFFF\r\n"};
static const uint8_t qssl4msg[] = {"AT+QSSLCFG=\"cacert\",2,\"security/CaCert.crt\"\r\n"};
static const uint8_t qssl5msg[] = {"AT+QSSLCFG=\"clientcert\",2,\"security/Client.crt\"\r\n"};
static const uint8_t qssl6msg[] = {"AT+QSSLCFG=\"clientkey\",2,\"security/key.pem\"\r\n"};
static const uint8_t postmsg[] = {"Host: shark.carematix.com\r\n"
                                  "Content-Type: application/json\r\n"
                                  "Content-Length: 160\r\n"
                                  "Authorization: Basic Y2FyZW1hdGl4OnBhc3N3b3Jk\r\n\r\n"};

/***************************************************************************
 *                             STATE TABLES
 **************************************************************************
 *
 * This state is executed after a power up reset.  We also come to
 * this state in response to a Not Powered request from master.  The
 * current request will be None if this is a power up reset.  The only
 * valid request is Power.
 */

static const S_TABLE st_init[] = {{&e_reqpower, &a_initvars, &a_ansinprogress, S_INIT2},
                                  {&e_reqnone, &a_rcvinitmsgs, &a_nop, S_INITWT},
                                  {&e_reqpwrdown, &a_nop, NULL, S_INIT},
                                  {&e_always, &a_nop, NULL, S_INIT}};

static const S_TABLE st_initwt[] = {{&e_rcvok, &a_rcvinitmsgs, &a_nop, S_INITWT},
                                    {&e_rcverr, &a_nop, &a_nop, S_POWERDN},
                                    {&e_always, NULL, &a_nop, S_INITWT}};

/*
 * If the request is Power we come here.  Enable power to the BG95.  Send
 * requests to get information from the chip, tell it what the api is and
 * register with the cellular network.  When the BG95 powers up it will send
 * a bunch of messages.  We need to flush those and put the chip into
 * ATE0 mode.
 */

static const S_TABLE st_init2[] = {{&e_always, &a_powerup1, &a_tmr1, S_INIT2WT}};

static const S_TABLE st_init2wt[] = {{&e_timeout, &a_powerup2, &a_rcvinitmsgs, S_INIT3WT},
                                     {&e_always, NULL, &a_nop, S_INIT2WT}};

static const S_TABLE st_init3wt[] = {{&e_rcvok, &a_rcvinitmsgs, &a_nop, S_INIT3WT},
                                     {&e_rcvtimout, &a_nop, &a_nop, S_XATE0},
                                     {&e_rcverr, &a_nop, &a_nop, S_POWERDN},
                                     {&e_always, NULL, &a_nop, S_INIT3WT}};

static const S_TABLE st_xate0[] = {{&e_always, &a_trate0, &a_nop, S_WTATE0}};

static const S_TABLE st_wtate0[] = {{&e_rcvok, &a_rcvdata, &a_nop, S_WTATE0},
                                    {&e_rcvtimout, &a_nop, &a_nop, S_XCMEE},
                                    {&e_rcverr, &a_nop, &a_nop, S_POWERDN},
                                    {&e_always, NULL, &a_nop, S_WTATE0}};

static const S_TABLE st_xcmee[] = {{&e_always, &a_trcmee, &a_nop, S_WTCMEE}};

static const S_TABLE st_wtcmee[] = {{&e_rcvok, &a_rcvdata, &a_nop, S_WTCMEE},
                                    {&e_rcvtimout, &a_nop, &a_nop, S_XAT},
                                    {&e_rcverr, &a_nop, &a_nop, S_POWERDN},
                                    {&e_always, NULL, &a_nop, S_WTCMEE}};

static const S_TABLE st_xat[] = {{&e_always, &a_trmsg1, &a_nop, S_WTAT}};

static const S_TABLE st_wtat[] = {{&e_rcvok, &a_nop, &a_nop, S_RSPAT},
                                  {&e_rcverr, &a_nop, &a_nop, S_POWERDN},
                                  {&e_always, NULL, &a_nop, S_WTAT}};

static const S_TABLE st_rspat[] = {{&e_rxok, &a_nop, &a_nop, S_XGMR},
                                   {&e_always, &a_nop, &a_nop, S_POWERDN}};

static const S_TABLE st_xgmr[] = {{&e_always, &a_trgmr, &a_nop, S_WTGMR}};

static const S_TABLE st_wtgmr[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPGMR},
                                   {&e_rcverr, &a_nop, &a_nop, S_POWERDN},
                                   {&e_always, NULL, &a_nop, S_WTGMR}};

static const S_TABLE st_chkrspgmr[] = {{&e_always, &a_savegmr, &a_rcvdata, S_WTOKGMR}};

static const S_TABLE st_wtokgmr[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKOKGMR},
                                     {&e_rcverr, &a_nop, &a_nop, S_POWERDN},
                                     {&e_always, NULL, &a_nop, S_WTOKGMR}};

static const S_TABLE st_chkokgmr[] = {{&e_rxok, &a_nop, &a_nop, S_XQCFG},
                                      {&e_always, &a_nop, &a_nop, S_POWERDN}};

static const S_TABLE st_xqcfg[] = {{&e_always, &a_trqcfg2, &a_nop, S_WTQCFG}};

static const S_TABLE st_wtqcfg[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPQCFG},
                                    {&e_rcvovflow, &a_nop, &a_nop, S_CHKRSPQCFG},
                                    {&e_rcverr, &a_nop, &a_nop, S_POWERDN},
                                    {&e_always, NULL, &a_nop, S_WTQCFG}};

static const S_TABLE st_chkrspqcfg[] = {{&e_rxqcfg, &a_rcvdata, &a_nop, S_WTQCFG},
                                        {&e_rxok, &a_nop, &a_nop, S_XCFUN0},
                                        {&e_always, &a_nop, &a_nop, S_POWERDN}};

static const S_TABLE st_xcfun0[] = {{&e_always, &a_trcfun0, &a_nop, S_WTCFUN0}};

static const S_TABLE st_wtcfun0[] = {{&e_rcvok, &a_nop, &a_nop, S_RSPCFUN0},
                                     {&e_rcverr, &a_nop, &a_nop, S_POWERDN},
                                     {&e_always, NULL, &a_nop, S_WTCFUN0}};

static const S_TABLE st_rspcfun0[] = {{&e_rxok, &a_nop, &a_nop, S_XCFUN1},
                                         {&e_always, &a_nop, &a_nop, S_POWERDN}};

static const S_TABLE st_xcfun1[] = {{&e_always, &a_trcfun1, &a_nop, S_WTCFUN1}};

static const S_TABLE st_wtcfun1[] = {{&e_rcvok, &a_nop, &a_nop, S_RSPCFUN1},
                                     {&e_rcvtimout, &a_nop, &a_nop, S_XCPINQ},
                                     {&e_rcverr, &a_nop, &a_nop, S_POWERDN},
                                     {&e_always, NULL, &a_nop, S_WTCFUN1}};

static const S_TABLE st_rspcfun1[] = {{&e_always, &a_rcvdata, &a_nop, S_WTCFUN1}};

static const S_TABLE st_xcpinq[] = {{&e_always, &a_trcpinq, &a_nop, S_WTCPINQ}};

static const S_TABLE st_wtcpinq[] = {{&e_rcvok, &a_nop, &a_nop, S_RSPCPINQ},
                                     {&e_rcverr, &a_nop, &a_nop, S_POWERDN},
                                     {&e_always, NULL, &a_nop, S_WTCPINQ}};

static const S_TABLE st_rspcpinq[] = {{&e_rxok, &a_nop, &a_nop, S_XAPN},
                                      {&e_rxcpinr, &a_rcvdata, &a_nop, S_WTCPINQ},
                                      {&e_always, &a_nop, &a_nop, S_POWERDN}};

static const S_TABLE st_xapn[] = {{&e_always, &a_trapn, &a_nop, S_WTAPN}};


static const S_TABLE st_wtapn[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPAPN},
                                   {&e_rcverr, &a_nop, &a_nop, S_POWERDN},
                                   {&e_always, NULL, &a_nop, S_WTAPN}};

static const S_TABLE st_chkrspapn[] = {{&e_rxcr, &a_rcvdata, &a_nop, S_WTOKAPN},
                                       {&e_rxok, &a_t2mr5min, &a_nop, S_XGSN},
                                       {&e_always, &a_nop, &a_nop, S_POWERDN}};

static const S_TABLE st_wtokapn[] = {{&e_rcvok, NULL, &a_nop, S_CHKOKAPN},
                                     {&e_rcverr, &a_nop, &a_nop, S_POWERDN},
                                     {&e_always, NULL, &a_nop, S_WTOKAPN}};

static const S_TABLE st_chkokapn[] = {{&e_rxok, &a_t2mr5min, &a_nop, S_XGSN},
                                      {&e_always, &a_nop, &a_nop, S_POWERDN}};




static const S_TABLE st_xgsn[] = {{&e_always, &a_trgsn, &a_nop, S_WTGSN}};

static const S_TABLE st_wtgsn[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPGSN},
                                   {&e_rcverr, &a_nop, &a_nop, S_POWERDN},
                                   {&e_always, NULL, &a_nop, S_WTGSN}};

static const S_TABLE st_chkrspgsn[] = {{&e_rximei, &a_saveimei, &a_rcvdata, S_WTOKGSN},
                                       {&e_always, &a_nop, &a_nop, S_POWERDN}};

static const S_TABLE st_wtokgsn[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKOKGSN},
                                     {&e_rcverr, &a_nop, &a_nop, S_POWERDN},
                                     {&e_always, NULL, &a_nop, S_WTOKGSN}};

static const S_TABLE st_chkokgsn[] = {{&e_rxok,  &a_nop, &a_nop, S_XCIMI},
                                      {&e_always, &a_nop, &a_nop, S_POWERDN}};




static const S_TABLE st_xcimi[] = {{&e_always, &a_trcimi, &a_nop, S_WTCIMI}};

static const S_TABLE st_wtcimi[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPCIMI},
                                    {&e_rcverr, &a_nop, &a_nop, S_POWERDN},
                                    {&e_always, NULL, &a_nop, S_WTCIMI}};

static const S_TABLE st_chkrspcimi[] = {{&e_rximsi, &a_saveimsi, &a_rcvdata, S_WTCIMI},
                                        {&e_rxok, &a_nop, &a_nop, S_XQCCID},
                                        {&e_always, &a_nop, &a_nop, S_POWERDN}};



static const S_TABLE st_xqccid[] = {{&e_always, &a_trqccid, &a_nop, S_WTQCCID}};

static const S_TABLE st_wtqccid[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPQCCID},
                                     {&e_rcverr, &a_nop, &a_nop, S_POWERDN},
                                     {&e_always, NULL, &a_nop, S_WTQCCID}};

static const S_TABLE st_chkrspqccid[] = {{&e_rxccid, &a_saveccid, &a_rcvdata, S_WTQCCID},
                                         {&e_rxok, &a_t2mr60, &a_nop, S_XCSQ},
                                         {&e_always, &a_flushrx, &a_nop, S_POWERDN}};

/*
 * If the RSSI is too small we loop, periodically checking the RSSI for
 * 1 min.  If the RSSI isnt good after a minute we abort the power on request.
 */

static const S_TABLE st_xcsq[] = {{&e_always, &a_trcsq, &a_nop, S_WTCSQ}};

static const S_TABLE st_wtcsq[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPCSQ},
                                   {&e_rcverr, &a_nop, &a_nop, S_POWERDN},
                                   {&e_always, NULL, &a_nop, S_WTCSQ}};

static const S_TABLE st_chkrspcsq[] = {{&e_rxcsq, &a_savecsq, &a_rcvdata, S_WTCSQ},
                                       {&e_rxok, &a_nop, &a_nop, S_CHKRSSI},
                                       {&e_always, &a_nop, &a_nop, S_POWERDN}};

static const S_TABLE st_chkrssi[] = {{&e_rssiok, &a_t2mr5min, &a_nop, S_XCGREGQ},
                                     {&e_always, &a_tmr10, &a_nop, S_WAITRSSI}};

static const S_TABLE st_waitrssi[] = {{&e_timeout2, &a_nop, &a_nop, S_POWERDN},
                                      {&e_timeout, &a_nop, &a_nop, S_XCSQ},
                                      {&e_always, NULL, &a_nop, S_WAITRSSI}};













/*
 * See if we are registered.  Check the EGPRS network.  If we are not registered
 * there check the EPS network.  If neither do a 10 sec delay and check again.
 * We sit it this loop for 5 mins.  If we are not registered by then we
 * revert to not powered state.
 */

static const S_TABLE st_xcgregq[] = {{&e_always, &a_trcgregq, &a_nop, S_WTCGREGQ}};


static const S_TABLE st_wtcgregq[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPCGREGQ},
                                      {&e_rcverr, &a_nop, &a_nop, S_POWERDN},
                                      {&e_always, NULL, &a_nop, S_WTCGREGQ}};

static const S_TABLE st_chkrspcgregq[] = {{&e_rxok, &a_nop, &a_nop, S_CHECKREGD},
                                          {&e_rxregq, &a_saveregq, &a_rcvdata, S_WTCGREGQ},
                                          {&e_always, &a_nop, &a_nop, S_POWERDN}};

static const S_TABLE st_checkregd[] = {{&e_registered, &a_trcopsq, &a_nop, S_WTCOPSQ},
                                       {&e_always, &a_trceregq, &a_nop, S_WTCEREGQ}};

static const S_TABLE st_wtceregq[] = {{&e_rcvok, &a_nop, &a_nop, S_CHKRSPCEREGQ},
                                      {&e_rcverr, &a_nop, &a_nop, S_POWERDN},
                                      {&e_always, NULL, &a_nop, S_WTCEREGQ}};

static const S_TABLE st_chkrspceregq[] = {{&e_rxok, &a_nop, &a_nop, S_CHECKREGD2},
                                          {&e_rxregq, &a_saveregq, &a_rcvdata, S_WTCEREGQ},
                                          {&e_always, &a_nop, &a_nop, S_POWERDN}};

static const S_TABLE st_checkregd2[] = {{&e_registered, &a_trcopsq, &a_nop, S_WTCOPSQ},
                                        {&e_timeout2, &a_nop, &a_nop, S_POWERDN},
                                        {&e_always, &a_tmr10, &a_nop, S_NOTREGD}};

static const S_TABLE st_notregd[] = {{&e_timeout, &a_nop, &a_nop, S_XCGREGQ},
                                     {&e_always, NULL, &a_nop, S_NOTREGD}};

/*
 * If we are here we are registered somewhere.  Find out where.
 */

static const S_TABLE st_wtcopsq[] = {{&e_rcvok, &a_rcvdata, &a_nop, S_WTCOPSQ},
                                     {&e_rcvtimout, &a_anspowered, &a_nop, S_IDLE},
                                     {&e_rcverr, &a_nop, &a_nop, S_POWERDN},
                                     {&e_always, NULL, &a_nop, S_WTCOPSQ}};


/*
 * At this point we are powered up.  Will can now answer to many other
 * requests.
 */

static const S_TABLE st_idle[] = {{&e_reqpower, &a_anspowered, NULL, S_IDLE},
                                  {&e_reqpwrdown, &a_nop, &a_nop, S_POWERDN},
                                  {&e_reqconnect, &a_trqmtcfg1, &a_nop, S_WTQMTCFG1},
                                  {&e_reqclock, &a_nop, &a_nop, S_CLOCK},
                                  {&e_reqping, &a_nop, &a_nop, S_PING},
                                  {&e_requpdcfg, &a_nop, &a_nop, S_UPDATECFG},
                                  {&e_reqnone, NULL, &a_nop, S_IDLE},
                                  {&e_always, &a_anspowered, NULL, S_IDLE}};

/*
 * Handle the Connect request.  We will try to connect to the MQTT server.
 * We will send all the security keys needed.  We will open the MQTT handler
 * in the BG95 then request a connect.  If we finish successfully we answer
 * Connect.  If we fail we answer Power.
 */

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
                                     {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
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

/*
 * If we are here we are connected to the MQTT server.
 */

static const S_TABLE st_connected[] = {{&e_reqpower, &a_nop, &a_nop, S_MQTDISC},
                                       {&e_reqconnect, NULL, &a_nop, S_CONNECTED},
                                       {&e_reqrecords, &a_initrecnum, &a_nop, S_SENDRECS},
                                       {&e_reqrdtwin, &a_nop, &a_nop, S_VERIFYCV},
                                       {&e_requpdtwin, &a_nop, &a_nop, S_UPDTWIN},
                                       {&e_reqnone, NULL, &a_nop, S_CONNECTED},
                                       {&e_always, &a_ansconnected, &a_nop, S_CONNECTED}};

/*
 * Come here to respond to the Clock request.  What we do here is get the time & date
 * from the internet and update the system clock.  The response is In Progress until
 * we are done then we revert to tbhe Power state.  The answer is the same whether or
 * not we succeed.
 */

static const S_TABLE st_clock[] = {{&e_always, &a_trqlts, &a_nop, S_CLOCKWT}};

static const S_TABLE st_clockwt[] = {{&e_rcvok, &a_nop, &a_nop, S_CLOCKRSP},
                                     {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                     {&e_always, NULL, &a_nop, S_CLOCKWT}};

static const S_TABLE st_clockrsp[] = {{&e_rxok, &a_anspowered, &a_nop, S_IDLE},
                                      {&e_rxtime, &a_savetime, &a_rcvdata, S_CLOCKWT},
                                      {&e_always, &a_flushrx, &a_anspowered, S_IDLE}};




/*
 * When here it is because we got a Read Twin request.  Send all the commands
 * necessary to to so.  We then process the configuration data from the twin.  We
 * can give the following answers:
 *
 * Powered - There was a comm failure of some kind.  The MQTT connection is dropped.
 *
 * Connected - The twin was successfully read and it matches our config.
 *
 * Bad Configuration - The configuration data the twin sent does not match what
 *                     we have.
 *
 * In Progress - The read of the twin is still in progress.
 *
 */

static const S_TABLE st_verifycv[] = {{&e_always, &a_initjson, &a_trqmtsub, S_VCSUBWT}};

static const S_TABLE st_vcsubwt[] = {{&e_rcvok, &a_nop, &a_nop, S_VCSUBRSP},
                                     {&e_rcverr, &a_nop, &a_nop, S_MQTDISC},
                                     {&e_always, NULL, &a_nop, S_VCSUBWT}};

static const S_TABLE st_vcsubrsp[] = {{&e_rxok, &a_rcvdata, &a_nop, S_VCSUBWT},
                                      {&e_rxqmtsub, &a_trqmtpub, &a_nop, S_VCPUBWT},
                                      {&e_always, &a_nop, &a_nop, S_MQTDISC}};

static const S_TABLE st_vcpubwt[] = {{&e_rcvok, &a_nop, &a_nop, S_VCPUBRSP},
                                     {&e_rcverr, &a_nop, &a_nop, S_MQTDISC},
                                     {&e_always, NULL, &a_nop, S_VCPUBWT}};

static const S_TABLE st_vcpubrsp[] = {{&e_rxok, &a_rcvdata, &a_nop, S_VCPUBWT},
                                      {&e_rxqmtpub, &a_nop, &a_nop, S_VCPUBWT},
                                      {&e_rxgt, &a_txnul, &a_nop, S_VCPUBWTOUT},
                                      {&e_always, &a_nop, &a_nop, S_MQTDISC}};

static const S_TABLE st_vcpubwtout[] = {{&e_outidle, &a_txsub, &a_rcvdata, S_VCPUBDATAWT},
                                        {&e_always, NULL, &a_nop, S_VCPUBWTOUT}};

static const S_TABLE st_vcpubdatawt[] = {{&e_rcvok, &a_nop, NULL, S_VCPUBDATA1},
                                         {&e_rcverr, &a_nop, NULL, S_MQTDISC},
                                         {&e_always, NULL, &a_nop, S_VCPUBDATAWT}};

static const S_TABLE st_vcpubdata1[] = {{&e_rxok, &a_rcvdata, &a_nop, S_VCPUBDATAWT},
                                        {&e_rxqmtpub, &a_rcvdata, &a_nop, S_VCPUBDATAWT},
                                        {&e_rxqmtrecv, &a_initjson, &a_rcvdata, S_VCJSONWT},
                                        {&e_always, &a_nop, &a_nop, S_MQTDISC}};

static const S_TABLE st_vcjsonwt[] = {{&e_rcvok, &a_nop, &a_nop, S_VCJSONRSP},
                                      {&e_rcverr, &a_nop, &a_nop, S_MQTDISC},
                                      {&e_always, NULL, &a_nop, S_VCJSONWT}};

static const S_TABLE st_vcjsonrsp[] = {{&e_rxjsonend, &a_accumjson, &a_processjson, S_VCJSON},
                                       {&e_always, &a_accumjson, &a_rcvdata, S_VCJSONWT}};

static const S_TABLE st_vcjson[] = {{&e_equalcvs, &a_ansconnected, NULL, S_CONNECTED},
                                    {&e_always, &a_ansbadcfg, NULL, S_CONNECTED}};

/*
 * Handle the Ping request.  The ping request will ask the server to send back a
 * list of configuration items that we are not current on.  When this request
 * requests it may set the following answers:
 *
 * In Progress - Ping has not completed
 *
 * Powered - The ping completed.  What was found is determined the following flags:
 *
 *           ca_certificate_flag
 *           client_certificate_flag
 *           client_key_flag
 *           configuration_service_flag
 *           update_module_firmware_flag
 *           update_device_firmware_flag
 *           ping_failed_flag
 *
 *           If any flag is set besides ping failed, then the configuration is not
 *           current and it must be updated.  If ping failed is set then there was a comm
 *           error and the ping did not complette.  In this case the other flags
 *           are meaningless.
 */

static const S_TABLE st_ping[] = {{&e_always, &a_trhtcfgc, &a_clrpingfailed, S_PHTCFGCWT}};

static const S_TABLE st_phtcfgcwt[] = {{&e_rcvok, &a_nop, &a_nop, S_PHTCFGCRSP},
                                       {&e_rcverr, &a_anspowered, &a_setpingfailed, S_IDLE},
                                       {&e_always, NULL, &a_nop, S_PHTCFGCWT}};

static const S_TABLE st_phtcfgcrsp[] = {{&e_rxok, &a_trhtcfgrqh, &a_nop, S_PHTCFGRQHWT},
                                        {&e_always, &a_anspowered, &a_setpingfailed, S_IDLE}};

static const S_TABLE st_phtcfgrqhwt[] = {{&e_rcvok, &a_nop, &a_nop, S_PHTCFGRQHRSP},
                                         {&e_rcverr, &a_anspowered, &a_setpingfailed, S_IDLE},
                                         {&e_always, NULL, &a_nop, S_PHTCFGRQHWT}};

static const S_TABLE st_phtcfgrqhrsp[] = {{&e_rxok, &a_trhturlp, &a_nop, S_PHTURL1WT},
                                          {&e_always, &a_anspowered, &a_setpingfailed, S_IDLE}};

static const S_TABLE st_phturl1wt[] = {{&e_rcvok, &a_nop, &a_nop, S_PHTURL1RSP},
                                       {&e_rcverr, &a_anspowered, &a_setpingfailed, S_IDLE},
                                       {&e_always, NULL, &a_nop, S_PHTURL1WT}};

static const S_TABLE st_phturl1rsp[] = {{&e_rxconnect, &a_trhturl2, &a_nop, S_PHTURL2WT},
                                        {&e_always, &a_anspowered, &a_setpingfailed, S_IDLE}};

static const S_TABLE st_phturl2wt[] = {{&e_rcvok, &a_nop, &a_nop, S_PHTURL2RSP},
                                       {&e_rcverr, &a_anspowered, &a_setpingfailed, S_IDLE},
                                       {&e_always, NULL, &a_nop, S_PHTURL2WT}};

static const S_TABLE st_phturl2rsp[] = {{&e_rxok, &a_buildpost, &a_trhtpost1, S_PHTPOST1WT},
                                        {&e_always, &a_anspowered, &a_setpingfailed, S_IDLE}};

static const S_TABLE st_phtpost1wt[] = {{&e_rcvok, &a_nop, &a_nop, S_PHTPOST1RSP},
                                        {&e_rcverr, &a_rcvdata, NULL, S_PHTPOST1WT},
                                        {&e_rcverr, &a_anspowered, &a_setpingfailed, S_IDLE},
                                        {&e_always, NULL, &a_nop, S_PHTPOST1WT}};

static const S_TABLE st_phtpost1rsp[] = {{&e_rxconnect, &a_trhtpost2, &a_nop, S_PHTPOST2WT},
                                         {&e_always, &a_rcvdata, &a_nop, S_PHTPOST1WT}};

static const S_TABLE st_phtpost2wt[] = {{&e_rcvok, &a_nop, &a_nop, S_PHTPOST2RSP},
                                        {&e_rcvtimout, &a_rcvdata, &a_nop, S_PHTPOST2WT},
                                        {&e_rcverr, &a_anspowered, &a_setpingfailed, S_IDLE},
                                        {&e_always, NULL, &a_nop, S_PHTPOST2WT}};

static const S_TABLE st_phtpost2rsp[] = {{&e_rxqhtpost, &a_trhtread, &a_nop, S_PHTREADWT},
                                         {&e_rxok, &a_rcvdata, &a_nop, S_PHTPOST2WT},
                                         {&e_always, &a_anspowered, &a_setpingfailed, S_IDLE}};

static const S_TABLE st_phtreadwt[] = {{&e_rcvok, &a_nop, &a_nop, S_PHTREADRSP},
                                       {&e_rcverr, &a_anspowered, &a_setpingfailed, S_IDLE},
                                       {&e_always, NULL, &a_nop, S_PHTREADWT}};

static const S_TABLE st_phtreadrsp[] = {{&e_rxconnect, &a_initjson, &a_rcvdata, S_PHTREADWT},
                                        {&e_rxok, &a_processping, &a_nop, S_PINGCHK},
                                        {&e_always, &a_accumjson, &a_rcvdata, S_PHTREADWT}};

static const S_TABLE st_pingchk[] = {{&e_pingok, &a_anspowered, &a_nop, S_IDLE},
                                     {&e_always, &a_anspowered, &a_nop, S_IDLE}};

/*
 * Handle the send records request here.  At this point the hub is expected to
 * be connected to the MQTT server.  We will send all records that are in the
 * serial flash that have not been sent previously. We return one of the
 * following answers:
 *
 * Connected - All records were successfully sent.
 *
 * Powered - There was a comm failure of some kind.  Not all records
 *           were sent, if any.  The NQTT connection is dropped in this case.
 *
 * In progress - We are still sending records.
 */

static const S_TABLE st_sendrecs[] = {{&e_eof, &a_ansconnected, &a_nop, S_CONNECTED},
                                      {&e_always, &a_trqmtpub2, &a_initrecnum, S_SRPUBRECWT}};

static const S_TABLE st_srsubwt[] = {{&e_rcvok, &a_nop, &a_nop, S_SRSUBRSP},
                                     {&e_rcverr, &a_nop, &a_nop, S_MQTDISC},
                                     {&e_always, NULL, &a_nop, S_SRSUBWT}};

static const S_TABLE st_srsubrsp[] = {{&e_rxok, &a_rcvdata, &a_nop, S_SRSUBWT},
                                      {&e_rxqmtsub, &a_trqmtpub, &a_nop, S_SRPUBWT},
                                      {&e_always, &a_rcvdata, &a_nop, S_SRSUBWT}};

static const S_TABLE st_srpubwt[] = {{&e_rcvok, &a_nop, &a_nop, S_SRPUBRSP},
                                     {&e_rcverr, &a_nop, &a_nop, S_MQTDISC},
                                     {&e_always, NULL, &a_nop, S_SRPUBWT}};

static const S_TABLE st_srpubrsp[] = {{&e_rxok, &a_rcvdata, &a_nop, S_SRPUBWT},
                                      {&e_rxqmtpub, &a_rcvdata, &a_nop, S_SRPUBWT},
                                      {&e_rxgt, &a_txnul, &a_nop, S_SRPUBWTOUT},
                                      {&e_always, &a_rcvdata, &a_nop, S_SRPUBWT}};

static const S_TABLE st_srpubwtout[] = {{&e_outidle, &a_txsub, &a_rcvdata, S_SRPUBDATAWT},
                                        {&e_always, NULL, &a_nop, S_SRPUBWTOUT}};

static const S_TABLE st_srpubdatawt[] = {{&e_rcvok, &a_nop, NULL, S_SRPUBDATA1},
                                         {&e_rcverr, &a_nop, &a_nop, S_MQTDISC},
                                         {&e_always, NULL, &a_nop, S_SRPUBDATAWT}};

static const S_TABLE st_srpubdata1[] = {{&e_rxok, &a_rcvdata, &a_nop, S_SRPUBDATAWT},
                                        {&e_rxqmtpub, &a_rcvdata, &a_nop, S_SRPUBDATAWT},
                                        {&e_rxqmtrecv, &a_initjson, &a_rcvdata, S_SRJSONWT},
                                        {&e_always, &a_rcvdata, &a_nop, S_SRPUBDATAWT}};

static const S_TABLE st_srjsonwt[] = {{&e_rcvok, &a_nop, &a_nop, S_SRPUBDATA2},
                                      {&e_rcverr, &a_nop, &a_nop, S_MQTDISC},
                                      {&e_always, NULL, &a_nop, S_SRJSONWT}};

static const S_TABLE st_srpubdata2[] = {{&e_rxjsonend, &a_accumjson, &a_processjson, S_SRJSON},
                                        {&e_always, &a_accumjson, &a_rcvdata, S_SRJSONWT}};

static const S_TABLE st_srjson[] = {{&e_eof, &a_nop, &a_ansconnected, S_CONNECTED},
                                    {&e_always, &a_trqmtpub2, &a_nop, S_SRPUBRECWT}};

static const S_TABLE st_srpubrecwt[] = {{&e_rcvok, &a_nop, &a_nop, S_SRPUBRECRSP},
                                        {&e_rcverr, &a_nop, NULL, S_MQTDISC},
                                        {&e_always, NULL, &a_nop, S_SRPUBRECWT}};

static const S_TABLE st_srpubrecrsp[] = {{&e_rxgt, &a_txrecord, &a_nop, S_SRRECOUTWT},
                                         {&e_rxqmtpub, &a_rcvdata, &a_nop, S_SRPUBDATAWT},
                                         {&e_rxqmtrecv, &a_initjson, &a_rcvdata, S_SRJSONWT},
                                         {&e_always, &a_rcvdata, &a_nop, S_SRPUBRECWT}};

static const S_TABLE st_srrecoutwt[] = {{&e_outidle, &a_txsub, &a_rcvdata, S_SRRECSUBWT},
                                        {&e_always, NULL, &a_nop, S_SRRECOUTWT}};

static const S_TABLE st_srrecsubwt[] = {{&e_rcvok, &a_nop, &a_nop, S_SRRECRSP},
                                        {&e_rcverr, &a_nop, &a_nop, S_MQTDISC},
                                        {&e_always, NULL, &a_nop, S_SRRECSUBWT}};

static const S_TABLE st_srrecrsp[] = {{&e_rxqmtpub, &a_increcnum, &a_nop, S_SRJSON},
                                      {&e_rxok, &a_rcvdata, &a_nop, S_SRRECSUBWT},
                                      {&e_always, &a_rcvdata, &a_nop, S_SRRECSUBWT}};

/*
 * If we are connected to the MQTT server and we want to revert back to the Powered
 * state because of some comm failure come here to try a MQTT disconnect before
 * answering powered.
 */

static const S_TABLE st_mqtdisc[] = {{&e_always, &a_trqmtdisc, &a_nop, S_MQTDISCWT}};

static const S_TABLE st_mqtdiscwt[] = {{&e_rcvok, &a_nop, &a_nop, S_MQTDISCRSP},
                                       {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                       {&e_always, NULL, &a_nop, S_MQTDISCWT}};

static const S_TABLE st_mqtdiscrsp[] = {{&e_rxok, &a_rcvdata, &a_nop, S_MQTDISCWT},
                                        {&e_rxqmtdisc, &a_anspowered, &a_nop, S_IDLE},
                                        {&e_always, &a_anspowered, &a_nop, S_IDLE}};

/*
 * Come here to handle the Update Config request.  Each configurable item has a flag
 * byte which will be set if it needs to be updated.  These flags are set during a
 * ping.  For each set flag request that item to be updated then clear its flag.  When
 * all flags are clear we are done.  We answer as follows:
 *
 * Powered - The update is complete or has failed.  If all update flags are clear
 *           then it was successful.
 *
 * In Progress - The updating is still ongoing.
 */

static const S_TABLE st_updatecfg[] = {{&e_updcacert, &a_trhtcfgc, &a_nop, S_CAHTCFGCWT},
                                       {&e_updclcert, &a_trhtcfgc, &a_nop, S_CCHTCFGCWT},
                                       {&e_updclkey, &a_trhtcfgc, &a_nop, S_CKHTCFGCWT},
                                       {&e_updsconfig, &a_trhtcfgc, &a_nop, S_SCHTCFGCWT},
                                       {&e_updfver, &a_clrupdfv, &a_nop, S_UPDATECFG},
                                       {&e_updmver, &a_clrupdmv, &a_nop, S_UPDATECFG},
                                       {&e_updcv, &a_updatecv, &a_nop, S_UPDATECFG},
                                       {&e_always, &a_anspowered, &a_nop, S_IDLE}};

/*
 * Update the CA certificate.
 */

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
                                           {&e_always, &a_rcvdata, &a_nop, S_CAHTRDFILEWT}};

/*
 * Update the client certificate.
 */

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

/*
 * Update the client key
 */

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
                                           {&e_always, &a_rcvdata, &a_nop, S_CKHTRDFILEWT}};

/*
 * Update the SC data.
 */

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
                                        {&e_always, &a_rcvdata, &a_nop, S_SCHTGETWT}};

static const S_TABLE st_schtreadwt[] = {{&e_rcvok, &a_nop, &a_nop, S_SCHTREADRSP},
                                        {&e_rcverr, &a_anspowered, &a_nop, S_IDLE},
                                        {&e_always, NULL, &a_nop, S_SCHTREADWT}};

static const S_TABLE st_schtreadrsp[] = {{&e_rxconnect, &a_initjson, &a_rcvdata, S_SCHTREADWT},
                                         {&e_rxok, &a_processsc, &a_clrupdsc, S_UPDATECFG},
                                         {&e_always, &a_accumjson, &a_rcvdata, S_SCHTREADWT}};

/*
 * Handle the Update Twin request here.  Send our current configuration to the server.
 * We can answer as follows:
 *
 * In Progress - Update is not complete yet.
 *
 * Connected - Updating is complete and successful.
 *
 * Powered - There was a comm fail in the update.  The MQTT connection is dropped
 *           and we revert to the powered state.
 */

static const S_TABLE st_updtwin[] =  {{&e_always, &a_trqmtpub3, &a_nop, S_UTPUBWT}};

static const S_TABLE st_utpubwt[] = {{&e_rcvok, &a_nop, &a_nop, S_UTPUBRSP},
                                     {&e_rcverr, &a_nop, &a_nop, S_MQTDISC},
                                     {&e_always, NULL, &a_nop, S_UTPUBWT}};

static const S_TABLE st_utpubwtout[] = {{&e_outidle, &a_txsub, &a_rcvdata, S_UTPUBDATAWT},
                                        {&e_always, NULL, &a_nop, S_UTPUBWTOUT}};

static const S_TABLE st_utpubdatawt[] = {{&e_rcvok, &a_rcvdata, &a_nop, S_UTPUBRSP},
                                         {&e_rcverr, &a_nop, &a_nop, S_MQTDISC},
                                         {&e_always, NULL, &a_nop, S_UTPUBDATAWT}};

static const S_TABLE st_utpubrsp[] = {{&e_rxok, &a_rcvdata, &a_nop, S_UTPUBDATAWT},
                                      {&e_rxgt, &a_txtwin, &a_nop, S_UTPUBWTOUT},
                                      {&e_rxqmtpub, &a_ansconnected, &a_nop, S_CONNECTED},
                                      {&e_always, &a_rcvdata, &a_nop, S_UTPUBDATAWT}};

/*
 * Come here to go to the power down state.  Send a AT+QPDOWN command to the
 * BG.  When all the responses come back answer powered down.
 */

static const S_TABLE st_powerdn[] =  {{&e_always, &a_trqpowd, &a_nop, S_QPDOWNWT}};

static const S_TABLE st_qpdownwt[] = {{&e_rcvok, &a_rcvdata, &a_nop, S_QPDOWNWT},
                                      {&e_rcverr, &a_ansnopower, &a_nop, S_INIT},
                                      {&e_always, NULL, &a_nop, S_QPDOWNWT}};


/*
 * Each state defined above must have an entry in this table.  There are corresponding
 * #define S_XXXX NN above.  These #defines must equal the index into this table of
 * the corresponding state table entry.
 */

static const S_TABLE * const state_table[] =
{
    st_init,
    st_wtat,
    st_idle,
    st_xate0,
    st_wtate0,
    st_wtcsq,
    st_chkrspcsq,
    st_init2,
    st_wtokapn,
    st_init3wt,
    st_chkokapn,
    st_wtapn,
    st_chkrspapn,
    st_xcmee,
    st_wtcmee,
    st_wtqcfg,
    st_chkrspqcfg,
    st_chkrssi,
    st_waitrssi,
    st_xat,
    st_wtat,
    st_rspat,
    st_wtgsn,
    st_chkrspgsn,
    st_wtcimi,
    st_chkrspcimi,
    st_wtqccid,
    st_chkrspqccid,
    st_xgmr,
    st_xqcfg,
    st_xcfun0,
    st_xcfun1,
    st_chkokgsn,
    st_wtokgsn,
    st_xcimi,
    st_xcgregq,
    st_wtcfun0,
    st_rspcfun0,
    st_wtgmr,
    st_chkrspgmr,
    st_wtokgmr,
    st_chkokgmr,
    st_wtcfun1,
    st_rspcfun1,
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
    0,
    st_sendrecs,
    st_vcsubwt,
    st_vcsubrsp,
    st_vcpubwt,
    st_vcpubrsp,
    st_vcpubwtout,
    st_vcpubdatawt,
    0,
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
    0,
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
    st_xapn,
    st_xgsn,
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
    0,
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
    st_schtreadrsp,
    st_wtcgregq,
    st_chkrspcgregq,
    st_checkregd,
    st_wtceregq,
    st_chkrspceregq,
    st_notregd,
    st_wtcopsq,
    st_checkregd2,
    st_powerdn,
    st_qpdownwt,
    st_init2wt,
    st_initwt,
    st_xcpinq,
    st_wtcpinq,
    st_rspcpinq,
    st_xqccid,
    st_xcsq

};

/***************************************************************************
 *                             GLOBAL FUNCTIONS
 **************************************************************************/

/***************************************************************************
 *                         gsm_answer
 *                         ----------
 *
 * This subroutine returns the current answer variable.  The answer variable
 * contains the response to a request (see gsm_request).  If the answer
 * equals CA_INPROGRESS then the last request has not completed.
 *
 * \param[in] - none
 *
 * \return - current answer value
 */

uint8_t gsm_answer(void)
{
    return callanswer;
}



/***************************************************************************
 *                         gsm_init
 *                         --------
 *
 * Called during power up init to initialize the state machine and variables.
 *
 * \param[in] - none
 *
 * \return - none
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
 *                         -----------
 *
 * This subroutine is called by master.c to request that the state
 * machine perform some function.  We set the answer to CA_INPROGRESS.  It
 * will be set to a valid answer depending on how the request is processed.
 *
 * \param[in] - req - request type
 *
 * \return - none
 */

void gsm_request(uint8_t req)
{
    callrequest = req;
    callanswer = CA_INPROGRESS;
    return;
}

/***************************************************************************
 *                         gsm_timer_ih
 *                         ------------
 *
 * This subroutine is called by the timer interrupt handler every 1 ms.
 * We decrement the local timer variable until it is 0.  The timer
 * can be used by the state machine.
 *
 * param[in] - none
 *
 * return - none
 */

void gsm_timer_ih()
{

    if (timer)
    {
        timer--;
    }

    if (timer2)
    {
        timer2--;
    }

    return;
}

/***************************************************************************
 *                             LOCAL FUNCTIONS
 **************************************************************************/

/***************************************************************************
 *                               EVENTS
 **************************************************************************/

/**************************************************************************
 *                            e_always
 *                            --------
 *
 * This event always returns true.  Each state must have an always event
 * or the state machine may fall through the state into the next state's
 * table.
 *
 */

static uint32_t e_always(void)
{
    return 1;
}

/**************************************************************************
 *                            e_eof
 *                            -----
 *
 * Returns true if all records in serial flash that havent been sent yet
 * have been sent.
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
 *                            ----------
 *
 * Used adter reading the twin.  Returns true if the config in the twin
 * is the same as what we have.
 */

static uint32_t e_equalcvs(void)
{
    return desired_reported_cv_matched_flag != 0;
}



/**************************************************************************
 *                            e_outidle
 *                            ---------
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
 * Returns true if there are no configuration items that need to
 * be updated.
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
    uint32_t stat;

    if (rcv_status == QS_OK)
    {
        stat = 1;
    }

    else if ((rcv_status == QS_TIMEOUT) && (rcv_count > 0))
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
 *                            e_rcvtimout
 *                            -----------
 *
 */

static uint32_t e_rcvtimout(void)
{
    return rcv_status == QS_TIMEOUT;
}

/**************************************************************************
 *                            e_registered
 *                            ------------
 *
 */

static uint32_t e_registered(void)
{
    return registered != 0;
}

/**************************************************************************
 *                            e_reqclock
 *                            ----------
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
 *                            ------------
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
 *                            ------------
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
 *                            ---------
 *
 */

static uint32_t e_reqnone(void)
{
    return callrequest == CR_NONE;
}

/**************************************************************************
 *                            e_reqping
 *                            ---------
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
 *                            ----------
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
 *                            ------------
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
 *                            ------------
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
 *                            ------------
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
 *                            e_rssiok
 *                            --------
 *
 */

static uint32_t e_rssiok(void)
{
    return bg95_rssi < 99;
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
 *                            -----------
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
 *                            ----------
 *
 */

static uint32_t e_rxconnok(void)
{
    uint32_t stat;

    stat = compare(0, (uint8_t *)"+QMTCONN: 0,0,0", 15);
    return stat;
}

/**************************************************************************
 *                            e_rxcopsq
 *                            ---------
 *
 */

static uint32_t e_rxcopsq(void)
{
    uint32_t stat;

    stat = compare(0, (uint8_t *)"+COPS:", 6);
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
 *                            -------
 *
 */

static uint32_t e_rxcsq(void)
{
    uint32_t stat;

    stat = compare(0, (uint8_t *)"+QCSQ:", 6);
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
 *                            --------
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
 *                            --------
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
 *                            -----------
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
 *                            ----------
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
 *                            --------
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
 *                            ----------
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
 *                            -----------
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
 *                            -------------
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
 *                            -----------
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
 *                            ------------
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
 *                            -----------
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
 *                            ----------
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
 *                            -----------
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
 *                            -----------
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
 *                            ----------
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
 *                            e_rxregq
 *                            --------
 *
 */

static uint32_t e_rxregq(void)
{
    uint32_t stat;

    stat = compare(3, (uint8_t *)"REG:", 4);
    return stat;
}

/**************************************************************************
 *                            e_rxtime
 *                            --------
 *
 */

static uint32_t e_rxtime(void)
{
    uint32_t stat;

    stat = 0;

    if (rcv_count >= 29)
    {
        stat = compare(0, (uint8_t *)"+QLTS: ", 6);
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
 *                            e_timeou2
 *                            ----------
 *
 */

static uint32_t e_timeout2(void)
{

    if (timer2 == 0)
    {
        return 1;
    }

    return 0;
}

/**************************************************************************
 *                            e_updcacert
 *                            -----------
 *
 */

static uint32_t e_updcacert(void)
{
    return ca_certificate_flag != 0;
}

/**************************************************************************
 *                            e_updclcert
 *                            -----------
 *
 */

static uint32_t e_updclcert(void)
{
    return client_certificate_flag != 0;
}

/**************************************************************************
 *                            e_updclkey
 *                            ----------
 *
 */

static uint32_t e_updclkey(void)
{
    return client_key_flag != 0;
}

/**************************************************************************
 *                            e_updcv
 *                            -------
 *
 */

static uint32_t e_updcv(void)
{
    return update_device_twin_flag != 0;
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
 *                            ------------
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
 *                               -----------
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
 *                               -----------
 *
 */

static void a_ansbadcfg(void)
{
    callanswer = CA_BADCONFIG;
    return;
}

/**************************************************************************
 *                               a_ansconnected
 *                               --------------
 *
 */

static void a_ansconnected(void)
{
    callanswer = CA_CONNECTED;
    return;
}

/**************************************************************************
 *                               a_ansinprogress
 *                               ---------------
 *
 */

static void a_ansinprogress(void)
{
    callanswer = CA_INPROGRESS;
    return;
}

/**************************************************************************
 *                               a_ansncvmatch
 *                               -------------
 *
 */

static void a_ansneedping(void)
{

    return;
}

/**************************************************************************
 *                               a_ansnopower
 *                               ------------
 *
 */

static void a_ansnopower(void)
{
    callanswer = CA_NOTPOWERED;
    return;
}

/**************************************************************************
 *                               a_anspowered
 *                               ------------
 *
 */

static void a_anspowered(void)
{
    callanswer = CA_POWERED;
    return;
}

/**************************************************************************
 *                               a_buildpost
 *                               -----------
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
 *                               a_clrpingfailed
 *                               ---------------
 *
 */

static void a_clrpingfailed(void)
{
    ping_failed_flag = 0;
    return;
}

/**************************************************************************
 *                               a_clrupdca
 *                               ----------
 *
 */

static void a_clrupdca(void)
{
    ca_certificate_flag = 0;
    return;
}

/**************************************************************************
 *                               a_clrupdcc
 *                               ----------
 *
 */

static void a_clrupdcc(void)
{
    client_certificate_flag = 0;
    return;
}

/**************************************************************************
 *                               a_clrupdck
 *                               ----------
 *
 */

static void a_clrupdck(void)
{
    client_key_flag = 0;
    return;
}

/**************************************************************************
 *                               a_clrupdfv
 *                               ----------
 *
 */

static void a_clrupdfv(void)
{
    update_device_firmware_flag = 0;
    return;
}

/**************************************************************************
 *                               a_clrupdmv
 *                               ----------
 *
 */

static void a_clrupdmv(void)
{
    update_module_firmware_flag = 0;
    return;
}

/**************************************************************************
 *                               a_clrupdsc
 *                               ----------
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
 *                               a_increcnum
 *                               -----------
 *
 */

static void a_increcnum(void)
{
    recnum++;
    return;
}

/**************************************************************************
 *                               a_initjson
 *                               ----------
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
 *                               a_powerup1
 *                               ----------
 *
 */

static void a_powerup1(void)
{
    HAL_GPIO_WritePin(GSM_PWR_PORT, GSM_PWR_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GSM_RESET_PORT, GSM_RESET_PIN, GPIO_PIN_SET);
    return;
}

/**************************************************************************
 *                               a_powerup2
 *                               ----------
 *
 */

static void a_powerup2(void)
{
    HAL_GPIO_WritePin(GSM_PWR_PORT, GSM_PWR_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GSM_RESET_PORT, GSM_RESET_PIN, GPIO_PIN_RESET);
    return;
}

/**************************************************************************
 *                               a_processjson
 *                               -------------
 *
 */

static void a_processjson(void)
{
    compare_cvs();
    return;
}

/**************************************************************************
 *                               a_processping
 *                               -------------
 *
 */

static void a_processping(void)
{
    pingstat = ping_response();
    return;
}

/**************************************************************************
 *                               a_processsc
 *                               -----------
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
    rcv_count = 0;
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
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, MAX_RECEIVE_LEN, 10000, &gsm_rcv_ih);
    return;
}

/**************************************************************************
 *                               a_rcvqmt
 *                               --------
 *
 */

static void a_rcvqmt(void)
{
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, MAX_RECEIVE_LEN, 30000, &gsm_rcv_ih);
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
 *                               a_savecsq
 *                               ---------
 *
 */

static void a_savecsq(void)
{
    uint32_t cstat;
    uint32_t rssidex;

    bg95_rssi = 99;
    rssidex = 0;
    cstat = compare(7, (uint8_t *)"\"eMTC\"", 6);

    if (cstat)
    {
        rssidex = 15;
    }

    else
    {
        cstat = compare(7, (uint8_t *)"\"GSM\"", 5);

        if (cstat)
        {
            rssidex = 14;
        }

    }

    if (rssidex)
    {
        bg95_rssi = rcv_buffer[rssidex] - '0';
        rssidex++;

        if (rcv_buffer[rssidex] != ',')
        {
            bg95_rssi *= 10;
            bg95_rssi += rcv_buffer[rssidex] - '0';
            rssidex++;

            if (rcv_buffer[rssidex] != ',')
            {
                bg95_rssi *= 10;
                bg95_rssi += rcv_buffer[rssidex] - '0';
            }

        }

    }

    return;
}

/**************************************************************************
 *                               a_savegmr
 *                               ---------
 *
 */

static void a_savegmr(void)
{
    uint32_t i;

    for (i = 0; i < MFV_LEN; i++)
    {
        module_firmware_version[i] = 0;
    }

    for (i = 0; i < MFV_LEN; i++)
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
 *                               a_saveregq
 *                               ----------
 *
 */

static void a_saveregq(void)
{

    if ((rcv_buffer[10] == '1') || (rcv_buffer[10] == '5'))
    {
        registered = 1;
    }

    else
    {
        registered = 0;
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
 *                               a_setpingfailed
 *                               ---------------
 *
 */

static void a_setpingfailed(void)
{
    ping_failed_flag = 1;
    return;
}

/**************************************************************************
 *                               a_t2mr5min
 *                               ----------
 *--
 */

static void a_t2mr5min(void)
{
    __disable_irq();
    timer2 = 60000 * 5;
    __enable_irq();
    return;
}

/**************************************************************************
 *                               a_t2mr60
 *                               --------
 *
 */

static void a_t2mr60(void)
{
    __disable_irq();
    timer2 = 60000;
    __enable_irq();
    return;
}

/**************************************************************************
 *                               a_tmr1
 *                               ------
 *
 */

static void a_tmr1(void)
{
    __disable_irq();
    timer = 1000;
    __enable_irq();
    return;
}

/**************************************************************************
 *                               a_tmr10
 *                               -------
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
        quec_transmit((uint8_t *)"A", 1);
        HAL_Delay(5);
        quec_transmit((uint8_t *)"T", 1);
        HAL_Delay(5);
        quec_transmit((uint8_t *)"E", 1);
        HAL_Delay(5);
        quec_transmit((uint8_t *)"0", 1);
        HAL_Delay(5);
        quec_transmit((uint8_t *)"\r", 1);
        HAL_Delay(5);
        quec_transmit((uint8_t *)"\n", 1);
    }

    return;
}

/**************************************************************************
 *                               a_trceregq
 *                               ----------
 *
 */

static void a_trceregq(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 16, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit((uint8_t *)"AT+CEREG?\r\n", 11);
    }

    return;
}

/**************************************************************************
 *                               a_trcfun0
 *                               ---------
 *
 */

static void a_trcfun0(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 16, 18000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit((uint8_t *)"AT+CFUN=0\r\n", 11);
    }

    return;
}

/**************************************************************************
 *                               a_trcfun1
 *                               ---------
 *
 */

static void a_trcfun1(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 16, 18000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit((uint8_t *)"AT+CFUN=1\r\n", 11);
    }

    return;
}

/**************************************************************************
 *                               a_trcgreg
 *                               ---------
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
 *                               a_trcgregq
 *                               ----------
 *
 */

static void a_trcgregq(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 16, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit((uint8_t *)"AT+CGREG?\r\n", 11);
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
 *                               a_trcmee
 *                               --------
 *
 */

static void a_trcmee(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 16, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit((uint8_t *)"AT+CMEE=2\r\n", 11);
    }

    return;
}

/**************************************************************************
 *                               a_trcopsq
 *                               ---------
 *
 */

static void a_trcopsq(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 64, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit((uint8_t *)"AT+COPS?\r\n", 10);
    }

    return;
}

/**************************************************************************
 *                               a_trcpinq
 *                               ---------
 *
 */

static void a_trcpinq(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 16, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit((uint8_t *)"AT+CPIN?\r\n", 10);
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
 *                               -------
 *
 */

static void a_trcsq(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 32, 5000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit((uint8_t *)"AT+QCSQ\r\n", 9);
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
 *                               ----------
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
 *                               ------------
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
 *                               -------------
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
 *                               ---------
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
 *                               -----------
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
 *                               -----------
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
 *                               ------------
 *
 */

static void a_trhtrdfile(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 16, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit((uint8_t *)"AT+QHTTPREADFILE=\"UFS:security/CaCert.crt\",80\r\n", 48);
    }

    return;
}

/**************************************************************************
 *                               a_trhtrdfile2
 *                               -------------
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
 *                               -------------
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
 *                               ----------
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
 *                               ----------
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
 *                               -----------
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
 *                               -----------
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
 *                               -----------
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
 *                               ----------
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
 *                               -----------
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
        quec_transmit((uint8_t *)qcfg2msg, sizeof(qcfg2msg) - 1);
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
 *                               a_trqlts
 *                               --------
 *
 */

static void a_trqlts(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 40, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {
        quec_transmit((uint8_t *)"AT+QLTS=1\r\n", 11);
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
 *                               ------------
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
 *                               -----------
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
 *                               ----------
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
 *                               --------
 *
 */

static void a_trqntp(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 64, 130000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {

        quec_transmit((uint8_t *)"AT+QNTP=1,\"0.us.pool.ntp.org\"\r\n", 31);
    }

    return;
}

/**************************************************************************
 *                               a_trqpowd
 *                               ---------
 *
 */

static void a_trqpowd(void)
{
    quec_rxflush();
    rcv_count = 0;
    rcv_status = quec_receive(rcv_buffer, 16, 2000, &gsm_rcv_ih);

    if (rcv_status == QS_INPROGRESS)
    {

        quec_transmit((uint8_t *)"AT+QPOWD\r\n", 10);
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
 *                               ----------
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
 *                               a_updatecv
 *                               ----------
 *
 */

static void a_updatecv(void)
{
    uint32_t i;

    for (i = 0; i < CV_LEN; i++)
    {
        configuration_version[i] = new_configuration_version[i];
    }

    store_settings_to_flash(DEVICE_SETTINGS_ADDRESS);
    store_settings_to_flash(DEVICE_SETTINGS_BACKUP_ADDRESS);
    update_device_twin_flag = 0;
    return;
}

/***************************************************************************
 *                             LOCAL FUNCTIONS
 **************************************************************************/

/***************************************************************************
 *                         compare
 *                         -------
 *
 * The purpose of this subroutine is to compare a substring of the
 * receive buffer with a fixed string to see if the receive buffer contains
 * the expected input.
 *
 * \param[in] - index - the index into rcv_buffer where the compare will start
 * \param[in] - response - the address of the compare buffer
 * \param[in] - count - the number of characters to compare
 *
 * return - 0 if the compare fails, not 0 if the compare is true
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
 * This is the receive complete interrupt handler.  This is not really
 * interrupt code.  It is called by the receive task in quectel.c when
 * an input completes.  Input must be started by a call to quec_receive.
 * When the quec_receive is fulfilled or there is an error this subroutine
 * is called.  The inputed data will be in rcv_buffer if there is any.
 *
 * \param[in] - stat - the receive status
 * \param[in] - count - the number of characters stored in rcv_buffer
 *
 * \return - none
 */

static void gsm_rcv_ih(uint8_t stat, uint32_t count)
{
    rcv_status = stat;
    rcv_count = count;
    return;
}

/***************************************************************************
 *                         state_trace
 *                         -----------
 *
 * This subroutine prints a debug message whenever the state changes.
 * If the state transitions to the sasme state as before then no message
 * is printed.  This address of this subroutine is in the state struct.  The
 * state machine handler executes this subroutine when an event subroutine
 * returns true.  The index of the true event relative to the start of the
 * state table entry is passed.
 *
 * \param[in] - evnum - index of the true event.
 *
 * \return - none
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

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
 * Module: config.h
 * Author: J Davis
 * Date: November 11, 2021
 *
 ***************************************************************************
 */

/***************************************************************************
 * This module contains global definitions which define the configuration
 * of software and hardware features. Most of this code came from main.c.
 */

/***************************************************************************
 *                               DEFINES
 **************************************************************************/

 ///////////////////// SELECT MODULE ///////////////
#define BG96 0
#define BG95 1
//////////////////////

//////////////////////  SETTINGS FOR INDIA/USA
#define INDIA 0
#define USA 1

/////////////////////

/////////////////////  ERASE FLASH ON STARTUP
#define ERASE_FLASH 0


///////////////    TO ENABLE OR DISABLE DEBUG USB CODE
#define USB_ENABLED 1

#define AP_MODE_TIMEOUT_DURATION 300


#define DEVICE_ID_ADDRESS 0
#define DEVICE_SETTINGS_ADDRESS 4096
#define DEVICE_SETTINGS_BACKUP_ADDRESS 8192
#define RUNTIME_PARAMETERS_ADDRESS 12288
#define RUNTIME_PARAMETERS_BACKUP_ADDRESS 16384
#define LOGS_START_ADDRESS 20480
#define FLASH_LAST_ADDRESS 2097341

/***************************************************************************
 *                               TYPEDEFS
 **************************************************************************/

/***************************************************************************
 *                            GLOBAL VARIABLE EXTERNS
 **************************************************************************/

/***************************************************************************
 *                         GLOBAL FUNCTION PROTOTYPES
 **************************************************************************/

/*
 * End of module.
 */

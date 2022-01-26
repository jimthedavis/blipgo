
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
 * Module: winbond_mem.c
 * Author: India
 * Date: 2021
 *
 ***************************************************************************
 */

 /*
  * This module contains the led handler.
  */

/***************************************************************************
 *                              INCLUDES
 **************************************************************************/

#include "main.h"
#include "stm32f0xx_hal.h"
#include "winbond_mem.h"

extern SPI_HandleTypeDef hspi1;

//////////////////////   Memory Function     /////////////////////

//total 65536 pages, 0 - 65535
// each page 256 bytes
// write 1 - 256 bytes in a page
//pages can be erased in a group of 16 (1 sector, 4KB) , 0 - 15
//total 4096 sectors

void sFLASH_CS_LOW(void)
{

    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
//    HAL_Delay(1);
}

void sFLASH_CS_HIGH(void)
{
    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET);
//    HAL_Delay(1);
}

/***************************************************************************
 *                         sFLASH_Init
 *                         -----------
 *
 * Initializes the led pins and handler.
 *
 * param[in] - none
 *
 * return - none
 */


void sFLASH_Init(void)
{
    GPIO_InitTypeDef initstruc;

    initstruc.Pin = FLASH_CS_Pin;
    initstruc.Mode = GPIO_MODE_OUTPUT_PP;
    initstruc.Pull = GPIO_NOPULL;
    initstruc.Speed = GPIO_SPEED_FREQ_LOW;
    initstruc.Alternate = 0;
    HAL_GPIO_Init(FLASH_CS_GPIO_Port, &initstruc);
    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET);
    HAL_Delay(2);

  sFLASH_CS_LOW();
    sFLASH_SendByte(0x06);
    sFLASH_CS_HIGH();

    sFLASH_CS_LOW();
    sFLASH_SendByte(0x98);
    sFLASH_CS_HIGH();
}

//void sFLASH_EraseSector(uint32_t SectorAddr, _Bool page_erase)
void sFLASH_EraseSector(uint32_t SectorAddr)
{
  /* Enable the write access to the FLASH */
  sFLASH_WriteEnable();

  /* Sector Erase */
  /* Select the FLASH: Chip Select low */
  sFLASH_CS_LOW();
  /* Send Sector Erase instruction */
//    if(page_erase)
        sFLASH_SendByte(sFLASH_CMD_SE); //4KB sector erase
//    else
//        sFLASH_SendByte(sFLASH_CMD_QE);
  /* Send SectorAddr high nibble address byte */
  sFLASH_SendByte((SectorAddr & 0xFF0000) >> 16);
  /* Send SectorAddr medium nibble address byte */
  sFLASH_SendByte((SectorAddr & 0xFF00) >> 8);
  /* Send SectorAddr low nibble address byte */
  sFLASH_SendByte(SectorAddr & 0xFF);
  /* Deselect the FLASH: Chip Select high */
  sFLASH_CS_HIGH();

  /* Wait till the end of Flash writing */
  sFLASH_WaitForWriteEnd();
}

void sFLASH_EraseBulk(uint32_t addrs)
{
    //total 512 erasable sectors, 512*4096 = 2097152 = last sector address
    for(addrs = addrs; addrs < 2097341; addrs+=4096)// addrs < LAST_FLASH_ADDR  copy value as per flash size , start from logs_start_address
    sFLASH_EraseSector(addrs);

//  /* Enable the write access to the FLASH */
//  sFLASH_WriteEnable();

//  /* Bulk Erase */
//  /* Select the FLASH: Chip Select low */
//  sFLASH_CS_LOW();
//  /* Send Bulk Erase instruction  */
//  sFLASH_SendByte(sFLASH_CMD_BE);
//  /* Deselect the FLASH: Chip Select high */
//  sFLASH_CS_HIGH();

//  /* Wait till the end of Flash writing */
//  sFLASH_WaitForWriteEnd();
}

//write_eeprom(30, '1');

//void write_eeprom(uint32_t WriteAddr, uint8_t byte)
//{
//    sFLASH_WriteByte(WriteAddr, byte);
//}

void sFLASH_WriteByte(uint32_t WriteAddr, uint8_t data_byte)
{
  /* Enable the write access to the FLASH */
  sFLASH_WriteEnable();
  /* Select the FLASH: Chip Select low */
  sFLASH_CS_LOW();
  /* Send "Byte Program" instruction */
  sFLASH_SendByte(sFLASH_CMD_WRITE);
  /* Send WriteAddr high nibble address byte to write to */
  sFLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
  /* Send WriteAddr medium nibble address byte to write to */
  sFLASH_SendByte((WriteAddr & 0xFF00) >> 8);
  /* Send WriteAddr low nibble address byte to write to */
  sFLASH_SendByte(WriteAddr & 0xFF);
  /* Send the byte */
  sFLASH_SendByte(data_byte);
  /* Deselect the FLASH: Chip Select high */
  sFLASH_CS_HIGH();
  /* Wait till the end of Flash writing */
  sFLASH_WaitForWriteEnd();
}

//void sFLASH_WriteBytes(uint8_t *pBuffer, uint32_t WriteAddr, uint32_t NumByteToWrite)
//{
//  /* Enable the write access to the FLASH */
//  sFLASH_WriteEnable();

//  /* Select the FLASH: Chip Select low */
//  sFLASH_CS_LOW();
//  /* Send "Auto Address Increment Word-Program" instruction */
//  sFLASH_SendByte(sFLASH_CMD_AAIP);
//  /* Send WriteAddr high nibble address byte to write to */
//  sFLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
//  /* Send WriteAddr medium nibble address byte to write to */
//  sFLASH_SendByte((WriteAddr & 0xFF00) >> 8);
//  /* Send WriteAddr low nibble address byte to write to */
//  sFLASH_SendByte(WriteAddr & 0xFF);
//  /* Send the first byte */
//  sFLASH_SendByte(*pBuffer++);
//  /* Send the second byte */
//  sFLASH_SendByte(*pBuffer++);
//  /* Update NumByteToWrite */
//  NumByteToWrite -= 2;
//  /* Deselect the FLASH: Chip Select high */
//  sFLASH_CS_HIGH();
//  /* Wait till the end of Flash writing */
//  sFLASH_WaitForWriteEnd();

//  /* while there is data to be written on the FLASH */
//  while (NumByteToWrite)
//  {
//    /* Select the FLASH: Chip Select low */
//    sFLASH_CS_LOW();
//    /* Send "Auto Address Increment Word-Program" instruction */
//    sFLASH_SendByte(sFLASH_CMD_AAIP);
//    /* Send the next byte and point on the next byte */
//    sFLASH_SendByte(*pBuffer++);
//    /* Send the next byte and point on the next byte */
//    sFLASH_SendByte(*pBuffer++);
//    /* Update NumByteToWrite */
//    NumByteToWrite -= 2;
//    /* Deselect the FLASH: Chip Select high */
//    sFLASH_CS_HIGH();
//    /* Wait till the end of Flash writing */
//    sFLASH_WaitForWriteEnd();
//  }

//  /* Deselect the FLASH: Chip Select high */
//  sFLASH_CS_HIGH();

//  /* Disable the write access to the FLASH */
//  //sFLASH_WriteDisable();
//}


//void sFLASH_WriteBuffer(uint8_t *pBuffer, uint32_t WriteAddr, uint32_t NumByteToWrite)
//{
//  uint32_t evenBytes;

//  /* If write starts at an odd address, need to use single byte write
//   * to write the first address. */
//  if ((WriteAddr & 0x1) == 0x1)
//  {
//    sFLASH_WriteByte(WriteAddr, *pBuffer++);
//    ++WriteAddr;
//    --NumByteToWrite;
//  }

//  /* Write bulk of bytes using auto increment write, with restriction
//   * that address must always be even and two bytes are written at a time. */
//  evenBytes = NumByteToWrite & ~0x1;
//  if (evenBytes)
//  {
//    sFLASH_WriteBytes(pBuffer, WriteAddr, evenBytes);
//    NumByteToWrite -= evenBytes;
//  }

//  /* If number of bytes to write is odd, need to use a single byte write
//   * to write the last address. */
//  if (NumByteToWrite)
//  {
//    pBuffer += evenBytes;
//    WriteAddr += evenBytes;
//    sFLASH_WriteByte(WriteAddr, *pBuffer++);
//  }
//}

//void sFLASH_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead)
//{
//  sFLASH_CS_LOW();
//  sFLASH_SendByte(sFLASH_CMD_READ);

//  sFLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
//  sFLASH_SendByte((ReadAddr& 0xFF00) >> 8);
//  sFLASH_SendByte(ReadAddr & 0xFF);

//  while (NumByteToRead) /* while there is data to be read */
//  {
//    *pBuffer++ = sFLASH_ReadByte();
//    NumByteToRead--;
//  }

//  sFLASH_CS_HIGH();
//}

uint8_t sFLASH_ReadByte1(uint32_t ReadAddr)
{
  uint8_t aaa=0;
    sFLASH_CS_LOW();

  sFLASH_SendByte(sFLASH_CMD_READ);

  sFLASH_SendByte((ReadAddr & 0xFF0000) >> 16);

  sFLASH_SendByte((ReadAddr& 0xFF00) >> 8);

  sFLASH_SendByte(ReadAddr & 0xFF);


    aaa = sFLASH_ReadByte();

  sFLASH_CS_HIGH();

    return aaa;

}

/*uint32_t sFLASH_ReadID(void)
{
  uint8_t byte[3];

  sFLASH_CS_LOW();

  sFLASH_SendByte(sFLASH_CMD_RDID);

  byte[0] = sFLASH_SendByte(sFLASH_DUMMY_BYTE);

  byte[1] = sFLASH_SendByte(sFLASH_DUMMY_BYTE);

  byte[2] = sFLASH_SendByte(sFLASH_DUMMY_BYTE);

  sFLASH_CS_HIGH();

  return (byte[0] << 16) | (byte[1] << 8) | byte[2];
}*/

void sFLASH_SendByte(uint8_t data_byte)
{
  HAL_SPI_Transmit(&hspi1, &data_byte, 1, 10);
}

uint8_t sFLASH_ReadByte(void)
{
  uint8_t mem_var=0;
    HAL_SPI_Receive(&hspi1, &mem_var, 1, 10);
    return mem_var;
}

void sFLASH_WriteEnable(void)
{
  sFLASH_CS_LOW();
  sFLASH_SendByte(sFLASH_CMD_WREN);
  sFLASH_CS_HIGH();
}

/*void sFLASH_WriteDisable(void)
{
  sFLASH_CS_LOW();

  sFLASH_SendByte(sFLASH_CMD_WRDI);

  sFLASH_CS_HIGH();
}*/


void sFLASH_WaitForWriteEnd(void)
{
  uint8_t flashstatus = 0;

    sFLASH_CS_LOW();

  sFLASH_SendByte(sFLASH_CMD_RDSR1);

    flashstatus = sFLASH_ReadByte();

  do
  {
    flashstatus = sFLASH_ReadByte();
  }
  while ((flashstatus & sFLASH_WIP_FLAG) == SET); /* Write in progress */

    sFLASH_CS_HIGH();
}

//void sFLASH_PageWriteEnable(uint32_t WriteAddr, uint8_t *byte)
//{
//  sFLASH_EraseSector(WriteAddr);
//
//    sFLASH_WriteEnable();
//    sFLASH_CS_LOW();
//    sFLASH_SendByte(sFLASH_PAGEEnable);
//
//    sFLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
//  sFLASH_SendByte((WriteAddr & 0xFF00) >> 8);
//  sFLASH_SendByte(WriteAddr & 0xFF);
//
//    for(uint8_t i=0; i<250; i++)
//    {
//        sFLASH_SendByte(byte[i]);
//        if(byte[i] == 0 || byte[i] == ';')
//            break;
//    }
//
//  sFLASH_CS_HIGH();
//  sFLASH_WaitForWriteEnd();
//}

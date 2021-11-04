#include <stdint.h>
#include <main.h>

#define sFLASH_MAX_SIZE					1599999	  /* Size in bytes */

#define sFLASH_CMD_WREN					0x06			/* Write Enable */
#define sFLASH_CMD_WRDI					0x04			/* Write Disable */
#define sFLASH_CMD_READ					0x03			/* Read Data Bytes */
#define sFLASH_CMD_FREAD				0x0B			/* Fast Read Data Bytes */
#define sFLASH_CMD_RDSR1				0x05			/* Read Status Register */
#define sFLASH_CMD_RDSR2				0x35			/* Read Status Register */
#define sFLASH_CMD_RDSR3				0x15			/* Read Status Register */
#define sFLASH_CMD_WRSR1				0x01			/* Write Status Register */
#define sFLASH_CMD_WRSR2				0x31			/* Write Status Register */
#define sFLASH_CMD_WRSR3				0x11			/* Write Status Register */
#define sFLASH_CMD_EWSR					0x50			/* Write Enable Status */
#define sFLASH_CMD_WRITE 				0x02			/* Byte Program */
#define sFLASH_CMD_AAIP         0xAD			/* Auto Address Increment */
#define sFLASH_CMD_SE           0x20			/* 4KB Sector Erase instruction */
#define sFLASH_CMD_RE           0x52			/* 32KB Sector Erase instruction */
#define sFLASH_CMD_QE           0xD8			/* 64KB Sector Erase instruction */
#define sFLASH_CMD_BE           0xC7			/* Bulk Chip Erase instruction */
#define sFLASH_CMD_RDID         0x9F			/* JEDEC ID Read */
#define sFLASH_CMD_EBSY         0x70			/* Enable SO RY/BY# Status */
#define sFLASH_CMD_DBSY         0x80			/* Disable SO RY/BY# Status */
#define sFLASH_WIP_FLAG         0x01			/* Write In Progress (WIP) flag */
#define sFLASH_DUMMY_BYTE       0xFF

#define sFLASH_PAGESIZE					0x1000		/* 4096 bytes */
#define sFLASH_PAGEEnable				0x02
#define sFLASH_CMD_PWRDN				0xB9			/* Power Down */
#define sFLASH_CMD_ERST					0x66			/* Enable Reset */
#define sFLASH_CMD_RST					0x99			/* Reset CMD */
#define sFLASH_CMD_BUNL					0x98			/* Block unlock */

void sFLASH_CS_LOW(void);
void sFLASH_CS_HIGH(void);
void sFLASH_Init(void);
//void sFLASH_EraseSector(uint32_t SectorAddr, _Bool page_erase)
void sFLASH_EraseSector(uint32_t SectorAddr);
void sFLASH_EraseBulk(uint32_t addrs);
void sFLASH_WriteByte(uint32_t WriteAddr, uint8_t byte);
void sFLASH_WriteBytes(uint8_t *pBuffer, uint32_t WriteAddr, uint32_t NumByteToWrite);
void sFLASH_WriteBuffer(uint8_t *pBuffer, uint32_t WriteAddr, uint32_t NumByteToWrite);
uint32_t sFLASH_ReadID(void);
void sFLASH_SendByte(uint8_t byte);
void sFLASH_WriteEnable(void);
void sFLASH_WriteDisable(void);
void sFLASH_WaitForWriteEnd(void);
void adc_restart_handler(void);
void write_test(void);
uint8_t sFLASH_ReadByte1(uint32_t ReadAddr);
void sFLASH_PageWriteEnable(uint32_t WriteAddr, uint8_t *byte);
uint8_t sFLASH_ReadByte(void);


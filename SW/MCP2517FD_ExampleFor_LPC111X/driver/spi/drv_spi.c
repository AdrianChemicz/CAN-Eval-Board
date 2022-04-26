/*******************************************************************************
  SPI Driver:  Implementation

  Company:
    Microchip Technology Inc.

  File Name:
    drv_spi.c

  Summary:
    Implementation of MCU specific SPI functions.

  Description:
    .
 *******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2018 Microchip Technology Inc. and its subsidiaries.

Subject to your compliance with these terms, you may use Microchip software and 
any derivatives exclusively with Microchip products. It is your responsibility 
to comply with third party license terms applicable to your use of third party 
software (including open source software) that may accompany Microchip software.

THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER EXPRESS, 
IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES 
OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.

IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER 
RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF 
THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE FULLEST EXTENT ALLOWED 
BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO 
THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY, THAT YOU HAVE PAID 
DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *******************************************************************************/
//DOM-IGNORE-END

/*******************************************************************************
 * This code operate only with single MCP2517FD device on SPI port. Code don't
 * support multiply MCP2517FD chip on single SPI. Implementation is possible
 * but wasn't present. Microchip API support multiply device but my implementation
 * for NXP microcontroller don't support it.
 *******************************************************************************/

// Include files
#include "drv_spi.h"
#include "SPI_Driver.h"
#include "GPIO_Driver.h"

#define MPC2517_CHIP_CONTROL_LINE_PORT		2
#define MPC2517_CHIP_CONTROL_LINE_PIN		11

#define MPC2517_CHIP_SPI_PORT_NUMBER		0

/* Local function prototypes */
inline void spi_master_init(void);
inline int8_t spi_master_transfer(uint8_t *SpiTxData, uint8_t *SpiRxData, uint16_t spiTransferSize);

void DRV_SPI_Initialize(void)
{
	spi_master_init();
}

int8_t DRV_SPI_TransferData(uint8_t spiSlaveDeviceIndex, uint8_t *SpiTxData, uint8_t *SpiRxData, uint16_t spiTransferSize)
{
	return spi_master_transfer(SpiTxData, SpiRxData, spiTransferSize);
}

void spi_master_init(void)
{
	GPIO_Init();
	GPIO_Direction(MPC2517_CHIP_CONTROL_LINE_PORT, MPC2517_CHIP_CONTROL_LINE_PIN, GPIO_DIR_OUTPUT);
	GPIO_SetState(MPC2517_CHIP_CONTROL_LINE_PORT, MPC2517_CHIP_CONTROL_LINE_PIN, true);

	SPI_DriverInit(MPC2517_CHIP_SPI_PORT_NUMBER, SPI_CLK_IDLE_LOW, SPI_CLK_LEADING);
}

int8_t spi_master_transfer(uint8_t *SpiTxData, uint8_t *SpiRxData, uint16_t spiTransferSize)
{
	uint16_t pos = 0;

	GPIO_SetState(MPC2517_CHIP_CONTROL_LINE_PORT, MPC2517_CHIP_CONTROL_LINE_PIN, false);

	while(pos < spiTransferSize)
	{
		uint16_t i = 0;

		for (i = 0; ((pos + i) < spiTransferSize) && i < SPI_BUFFER_SIZE; i++)
		{
			// Transmit
			SPI_PutByteToTransmitter(MPC2517_CHIP_SPI_PORT_NUMBER, SpiTxData[pos + i]);
		}

		for (; SPI_CheckBusyFlag(MPC2517_CHIP_SPI_PORT_NUMBER);){}

		for (i = 0; ((pos + i) < spiTransferSize) && i < SPI_BUFFER_SIZE; i++)
		{
			// Receive
			SpiRxData[pos + i] = SPI_ReadByteFromTrasmitter(MPC2517_CHIP_SPI_PORT_NUMBER);
		}

		pos+=i;
	}/* while(pos < spiTransferSize) */

	GPIO_SetState(MPC2517_CHIP_CONTROL_LINE_PORT, MPC2517_CHIP_CONTROL_LINE_PIN, true);

	Nop();
	Nop();

	return 0;
}/* int8_t spi_master_transfer(uint8_t *SpiTxData, uint8_t *SpiRxData, uint16_t spiTransferSize) */


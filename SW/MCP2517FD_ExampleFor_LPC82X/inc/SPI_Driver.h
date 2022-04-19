/*
* Copyright (c) 2022, Adrian Chemicz
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*    1. Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*    2. Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*    3. Neither the name of contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _SPI_DRIVER_H_
#define _SPI_DRIVER_H_

/*
* This module handle SPI of LPC82X microcontroler. Most of function directly operate
* on microcontroler register. Additional logic exist only in SPI_DriverInit fuction.
* SPI_DriverInit function contain additional logic like configure SPI pins(MISO, MOSI,
* SCK, CS) configure clock of SPI ports and this section should be configure by
* user(set prescaler registers). Value for prescaler isn't set as parameter in
* SPI_DriverInit function beacuse all hardware functions is common for microcontroller
* and emulated SPI on FTDI chip.
*
* Simple example code to send and receive data via SPI module:
*
*	SPI_DriverInit(0, SPI_CLK_IDLE_HIGH, SPI_CLK_TRAILING);
*	SPI_Status spiStatus;
*	volatile uint8_t dataFromSpi = 0;
*
*	SPI_PutByteToTransmitter(0, 0x12, SPI_CHIP_TXSSEL0_N, true, false);
*
*	spiStatus = SPI_ReturnStatusRegister(0);
*
*	for (;spiStatus.RXRDY == 0;)
*	{
*		spiStatus = SPI_ReturnStatusRegister(0);
*	}
*
*	dataFromSpi = SPI_ReadByteFromTrasmitter(0);
*
*/

#include <stdint.h>
#include <stdbool.h>

#ifdef MICROCONTROLLER

#ifdef __LPC82X__	//macro for LPC82X family
#include "chip.h"
#endif

#ifdef __LPC11XX__	//macro for LPC111X family
#include "LPC11xx.h"
#endif

#ifdef __LPC11UXX__ //macro for LPC11U2X family
#include "chip.h"
#endif

#ifdef __LPC11E6X__ //macro for LPC11E6X family
#include "chip.h"
#endif

#else
#include <windows.h>
#include "ftd2xx.h"
#include "LibFT4222.h"
#include "FT_Devices.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MICROCONTROLLER

#define SPI_MASTER_MODE 		(1<<2)
#define SPI_ENABLE 				1
#define SPI_EIGHT_BYTE_LENGTH 	(7<<24)
#define SPI_END_OF_FRAME		(1<<21)

#else
	typedef struct
	{
		CRITICAL_SECTION criticalSectionWrite;
		CRITICAL_SECTION criticalSectionRead;

		uint8_t writeBuffer[16];
		uint8_t readBuffer[16];

		uint8_t writeBufferPosition;
		uint8_t readBufferPosition;

		FT_HANDLE ftHandleSPI;
	}SPI_Device_Instance;
#endif

	typedef struct SPI_Status
	{
		uint32_t RXRDY : 	1; /* Receiver Ready flag. */
		uint32_t TXRDY : 	1; /* Transmitter Ready flag. */
		uint32_t RXOV : 	1; /* Receiver Overrun interrupt flag. */
		uint32_t TXUR : 	1; /* Transmitter Underrun interrupt flag. */
		uint32_t SSA : 		1; /* Slave Select Assert. */
		uint32_t SSD : 		1; /* Slave Select Deassert. */
		uint32_t STALLED : 	1; /* Stalled status flag. */
		uint32_t ENDTRANSFER : 1; /* End Transfer control bit. */
		uint32_t MSTIDLE : 	1; /* Master idle status flag. */
		uint32_t RESERVED : 23;
	}SPI_Status;

	typedef enum SPI_CLK_POL
	{
		SPI_CLK_IDLE_LOW = 0,
		SPI_CLK_IDLE_HIGH = 1
	}SPI_CLK_POL;

	typedef enum SPI_CLK_PHASE
	{
		SPI_CLK_LEADING = 0,
		SPI_CLK_TRAILING = 1
	}SPI_CLK_PHASE;

	typedef enum SPI_CHIP_SELECT
	{
		SPI_CHIP_TXSSEL0_N = 1,
		SPI_CHIP_TXSSEL1_N = 2,
		SPI_CHIP_TXSSEL2_N = 4,
		SPI_CHIP_TXSSEL3_N = 8
	}SPI_CHIP_SELECT;

	void SPI_DriverInit(uint8_t portNumber, SPI_CLK_POL polarity, SPI_CLK_PHASE phase);

	void SPI_PutByteToTransmitter(uint8_t portNumber, uint8_t byte, SPI_CHIP_SELECT chipSelectNumber, bool endOfTransfer, bool readIgnore);

	uint8_t SPI_ReadByteFromTrasmitter(uint8_t portNumber);

	SPI_Status SPI_ReturnStatusRegister(uint8_t portNumber);

#ifdef __cplusplus
}
#endif

#endif  /* _SPI_DRIVER_H_ */

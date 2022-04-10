/*
* Copyright (c) 2018 - 2022, Adrian Chemicz
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

#ifndef _UART_DRIVER_H_
#define _UART_DRIVER_H_

/*
* This module handle UART of LPC82X microcontroler. Most of function directly operate
* on microcontroler register. Additional logic exist only in UART_DriverInit fuction.
* UART_DriverInit function contain additional logic like configure UART pins(RX, TX)
* configure clock of UART and this section should be modify by user(set prescaler
* registers and fractional divider for appropriate baudrate). UART_DriverInit function
* should be modify by user microcontroller frequency will be changed or if new baudrate
* value will be necessary, then new value should be calculated and parameter assign to
* appropriate field.
*
* Simple example code to send and receive data via UART module:
*
*	UART_DriverInit(0, 9600, L8_BIT, ONE_BIT, NONE_PARITY);
*	UART_Status uartStatus;
*	volatile uint8_t dataFromUart = 0;
*
*	for (;;)
*	{
*		uartStatus = UART_ReturnStatusRegister(0);
*
*		if(uartStatus.TXRDY == 1)
*		{
*			UART_PutByteToTransmitter(0, 0xAA);
*		}
*
*		dataFromUart = 0;
*
*		if(uartStatus.RXRDY == 1)
*		{
*			dataFromUart = UART_ReadByteFromTrasmitter(0);
*		}
*	}
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

#define UART_BUFFER_SIZE 1
#define UART_DEVICES 3

	typedef enum WORD_LENGTH
	{
		L7_BIT = 0,
		L8_BIT = 1,
		L9_BIT = 2
	}WORD_LENGTH;

	typedef enum STOP_BITS
	{
		ONE_BIT = 0,
		TWO_BITS = 1
	}STOP_BITS;

	typedef enum PARITY
	{
		NONE_PARITY = 0,
		RESERVED_PARITY = 1,
		EVEN_PARITY = 2,
		ODD_PARITY = 3
	}PARITY;

	typedef struct
	{
		uint32_t RXRDY : 	1;
		uint32_t RXIDLE : 	1;
		uint32_t TXRDY : 	1;
		uint32_t TXIDLE : 	1;
		uint32_t CTS : 		1;
		uint32_t DELTACTS : 1;
		uint32_t TXDISSTAT : 	1;
		uint32_t RESERVED_0 : 	1;
		uint32_t OVERRUNINT : 	1;
		uint32_t RESERVED_1 : 	1;
		uint32_t RXBRK : 		1;
		uint32_t DELTARXBRK : 	1;
		uint32_t START : 		1;
		uint32_t FRAMERRINT : 	1;
		uint32_t PARITYERRINT : 1;
		uint32_t RXNOISEINT : 	1;
		uint32_t ABERR : 		1;
		uint32_t RESERVED_2 : 	15;
	}UART_Status;

	typedef struct
	{
		uint8_t PE : 1;
		uint8_t FE : 1;
		uint8_t BI : 1;
		uint8_t Error : 1;
		uint8_t Reserved : 4;
	}ReadByteErrors;

#ifdef MICROCONTROLLER
#define FRACTIONAL_DIVIDER_EQUAL_256 0xFF
#else
	typedef struct
	{
		CRITICAL_SECTION criticalSectionWrite;
		CRITICAL_SECTION criticalSectionRead;
		CRITICAL_SECTION criticalSectionStatus;

		uint8_t writeBuffer[32];
		uint8_t readBuffer[32];
		ReadByteErrors readBufferByteError[32];
		bool overrunError;

		uint8_t writeBufferPosition;
		uint8_t readBufferPosition;

		FT_HANDLE ftHandle;
	}UART_DeviceInstance;
#endif

	void UART_DriverInit(uint8_t portNumber, uint32_t baudrate, WORD_LENGTH length, STOP_BITS stopBits, PARITY parity);
	void UART_PutByteToTransmitter(uint8_t portNumber, uint8_t byte);
	uint8_t UART_ReadByteFromTrasmitter(uint8_t portNumber);
	UART_Status UART_ReturnStatusRegister(uint8_t portNumber);

#ifdef __cplusplus
}
#endif

#endif /* _UART_DRIVER_H_ */

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

#include "SPI_Driver.h"

static uint8_t ClockPrescalerRegister;

static uint32_t* SPI_GetBaseAddress(uint8_t portNumber)
{
	uint32_t* basePointer = 0;
	switch (portNumber)
	{
	case 0:
		basePointer = (uint32_t*)LPC_SPI0_BASE;
		break;
	case 1:
		basePointer = (uint32_t*)LPC_SPI1_BASE;
		break;
	default:
		basePointer = 0;
		break;
	}

	return basePointer;
}

static void SPI_SetClockPrescalers(uint8_t portNumber)
{
	/*must be defined by user
	equation to SPI bit frequency
	PCLK = MAIN CLOCK / AhbClkDivider

	SPI bit frequency = PCLK/(ClockPrescalerRegister x (SerialClockRate + 1) ) */
	switch (portNumber)
	{
	case 0:
		ClockPrescalerRegister = 6;
		break;
	case 1:
		ClockPrescalerRegister = 6;
		break;
	default:
		ClockPrescalerRegister = 6;
		break;
	}
}

void SPI_DriverInit(uint8_t portNumber, SPI_CLK_POL polarity, SPI_CLK_PHASE phase)
{
	LPC_SPI_T *SPI_Port = (LPC_SPI_T*)SPI_GetBaseAddress(portNumber);

	SPI_SetClockPrescalers(portNumber);

	switch (portNumber)
	{
	case 0:
		NVIC_DisableIRQ(SPI0_IRQn);

		//connect SPI0 to AHB bus
		LPC_SYSCTL->SYSAHBCLKCTRL |= (1<<11);

		//SSP0 reset
		LPC_SYSCTL->PRESETCTRL = LPC_SYSCON->PRESETCTRL & ~1;
		LPC_SYSCTL->PRESETCTRL |= 1;
		break;

	case 1:
		NVIC_DisableIRQ(SPI1_IRQn);

		//connect SPI1 to AHB bus
		LPC_SYSCTL->SYSAHBCLKCTRL |= (1<<12);

		//SSP1 reset
		LPC_SYSCTL->PRESETCTRL = LPC_SYSCON->PRESETCTRL & ~(1<<1);
		LPC_SYSCTL->PRESETCTRL |= (1<<1);
		break;

	default:

		break;
	}

	//configure SPI registers
	SPI_Port->CFG |= (SPI_MASTER_MODE|(phase<<4)|(polarity<<5));
	SPI_Port->DIV = ClockPrescalerRegister;
	SPI_Port->CFG |= SPI_ENABLE;

	//enable interrupts
	switch (portNumber)
	{
	case 0:
		NVIC_EnableIRQ(SPI0_IRQn);
		break;

	case 1:
		NVIC_EnableIRQ(SPI1_IRQn);
		break;

	default:

		break;

	}
}

void SPI_PutByteToTransmitter(uint8_t portNumber, uint8_t byte, SPI_CHIP_SELECT chipSelectNumber, bool endOfTransfer, bool readIgnore)
{
	LPC_SPI_T *SPI_Port = (LPC_SPI_T*)SPI_GetBaseAddress(portNumber);
	SPI_Port->TXDATCTL = byte|((15 - chipSelectNumber)<<16)|(endOfTransfer<<20)|SPI_END_OF_FRAME|(readIgnore<<22)|SPI_EIGHT_BYTE_LENGTH;
}

uint8_t SPI_ReadByteFromTrasmitter(uint8_t portNumber)
{
	LPC_SPI_T *SPI_Port = (LPC_SPI_T*)SPI_GetBaseAddress(portNumber);
	return SPI_Port->RXDAT;
}

SPI_Status SPI_ReturnStatusRegister(uint8_t portNumber)
{
	LPC_SPI_T *SPI_Port = (LPC_SPI_T*)SPI_GetBaseAddress(portNumber);
	SPI_Status status = *((SPI_Status*)&(SPI_Port->STAT));
	return status;
}

/*
* Copyright (c) 2018, 2019, Adrian Chemicz
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

#define SPI_HW_CS_ENABLE 0

static uint8_t AhbClkDivider;
static uint8_t ClockPrescalerRegister;
static uint8_t SerialClockRate;

static uint32_t* SPI_GetBaseAddress(uint8_t portNumber)
{
	uint32_t* basePointer = 0;
	switch(portNumber)
	{
	case 0:
		basePointer = (uint32_t*)LPC_SSP0;
		break;
	case 1:
		basePointer = (uint32_t*)LPC_SSP1;
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
	switch(portNumber)
	{
	case 0:
		AhbClkDivider = 4;
		ClockPrescalerRegister = 6;
		SerialClockRate = 0;
		break;
	case 1:
		AhbClkDivider = 4;
		ClockPrescalerRegister = 6;
		SerialClockRate = 0;
		break;
	default:
		AhbClkDivider = 4;
		ClockPrescalerRegister = 6;
		SerialClockRate = 0;
		break;
	}
}

void SPI_DriverInit(uint8_t portNumber, SPI_CLK_POL polarity, SPI_CLK_PHASE phase)
{
	LPC_SSP_TypeDef *SPI_Port = (LPC_SSP_TypeDef*)SPI_GetBaseAddress(portNumber);

	//turn on options responsible for change GPIO purpose like UART, ADC or SPI
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<16);

	SPI_SetClockPrescalers(portNumber);

	if(portNumber == 0)
	{
		//SSP0 reset
		LPC_SYSCON->PRESETCTRL = LPC_SYSCON->PRESETCTRL & ~1;
		LPC_SYSCON->PRESETCTRL |= 1;

		//connect SSP0 to AHB bus
		LPC_SYSCON->SYSAHBCLKCTRL |= (1<<11);

		LPC_SYSCON->SSP0CLKDIV = AhbClkDivider;

		//configure GPIO pin which is used by SSP0
		//MISO_0
		LPC_IOCON->PIO0_8 |= 1;
		//MOSI_0
		LPC_IOCON->PIO0_9 |= 1;

		//Choose location for SCK0
		//0x0 - SWCLK/PIO0_10/SCK0/CT16B0_MAT2; 0x1 - PIO2_11/SCK0; 0x2 - PIO0_6/SCK0
		LPC_IOCON->SCK_LOC = 2;
		//SCK0
		LPC_IOCON->PIO0_6 |= 2;
#if SPI_HW_CS_ENABLE
		//SSEL0
		LPC_IOCON->PIO0_2 |= 1;
#endif
	}

	if(portNumber == 1)
	{
		//SSP1 reset
		LPC_SYSCON->PRESETCTRL = LPC_SYSCON->PRESETCTRL & ~4;
		LPC_SYSCON->PRESETCTRL |= 4;

		//connect SSP0 to AHB bus
		LPC_SYSCON->SYSAHBCLKCTRL |= (1<<18);

		LPC_SYSCON->SSP1CLKDIV = AhbClkDivider;

		//configure GPIO pin which is used by SSP1
		//MISO_1
		LPC_IOCON->PIO2_2 |= 2;
		//M0SI_1
		LPC_IOCON->PIO2_3 |= 2;
		//SCK1
		LPC_IOCON->PIO2_1 |= 2;
#if SPI_HW_CS_ENABLE
		//SSEL1
		LPC_IOCON->PIO2_0 |= 2;
#endif
	}

	//clear SPI read buffer
	for(uint32_t i=0, data=0; i < 8 ;i++)
	{
		data = SPI_Port->DR;
	}

	//configure SPI register
	//SPI_Port->CR0 = (polarity<<6)|(phase<<7)|STANDARD_FRAME_LENGTH;

	SPI_Port->CR0 = (SerialClockRate<<8)|(phase<<7)|(polarity<<6)|STANDARD_FRAME_LENGTH;
	SPI_Port->CPSR = ClockPrescalerRegister;
	SPI_Port->CR1 = SPI_ENABLE;
}

void SPI_PutByteToTransmitter(uint8_t portNumber, uint8_t byte)
{
	LPC_SSP_TypeDef *SPI_Port = (LPC_SSP_TypeDef*)SPI_GetBaseAddress(portNumber);
	SPI_Port->DR = byte;
}

uint8_t SPI_ReadByteFromTrasmitter(uint8_t portNumber)
{
	LPC_SSP_TypeDef *SPI_Port = (LPC_SSP_TypeDef*)SPI_GetBaseAddress(portNumber);
	return SPI_Port->DR;
}

bool SPI_CheckTxFifoEmpty(uint8_t portNumber)
{
	LPC_SSP_TypeDef *SPI_Port = (LPC_SSP_TypeDef*)SPI_GetBaseAddress(portNumber);
	SPI_Status status = *((SPI_Status*)&(SPI_Port->SR));
	return status.TFE;
}

bool SPI_CheckTxFifoNotFull(uint8_t portNumber)
{
	LPC_SSP_TypeDef *SPI_Port = (LPC_SSP_TypeDef*)SPI_GetBaseAddress(portNumber);
	SPI_Status status = *((SPI_Status*)&(SPI_Port->SR));
	return status.TNF;
}

bool SPI_CheckRxFifoNotEmpty(uint8_t portNumber)
{
	LPC_SSP_TypeDef *SPI_Port = (LPC_SSP_TypeDef*)SPI_GetBaseAddress(portNumber);
	SPI_Status status = *((SPI_Status*)&(SPI_Port->SR));
	return status.RNE;
}

bool SPI_CheckRxFifoFull(uint8_t portNumber)
{
	LPC_SSP_TypeDef *SPI_Port = (LPC_SSP_TypeDef*)SPI_GetBaseAddress(portNumber);
	SPI_Status status = *((SPI_Status*)&(SPI_Port->SR));
	return status.RFF;
}

bool SPI_CheckBusyFlag(uint8_t portNumber)
{
	LPC_SSP_TypeDef *SPI_Port = (LPC_SSP_TypeDef*)SPI_GetBaseAddress(portNumber);
	SPI_Status status = *((SPI_Status*)&(SPI_Port->SR));
	return status.BSY;
}

SPI_Status SPI_ReturnStatusRegister(uint8_t portNumber)
{
	LPC_SSP_TypeDef *SPI_Port = (LPC_SSP_TypeDef*)SPI_GetBaseAddress(portNumber);
	SPI_Status status = *((SPI_Status*)&(SPI_Port->SR));
	return status;
}



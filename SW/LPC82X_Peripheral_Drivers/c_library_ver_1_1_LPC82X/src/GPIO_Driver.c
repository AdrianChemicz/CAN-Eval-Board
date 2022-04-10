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

#include "GPIO_Driver.h"

#ifdef __LPC11XX__

static uint32_t* GPIO_GetBaseAddress(uint8_t portNumber)
{
	uint32_t* basePointer = 0;

	switch(portNumber)
	{
	case 0:
		basePointer = (uint32_t*)LPC_GPIO0;
		break;
	case 1:
		basePointer = (uint32_t*)LPC_GPIO1;
		break;
	case 2:
		basePointer = (uint32_t*)LPC_GPIO2;
		break;
	case 3:
		basePointer = (uint32_t*)LPC_GPIO3;
		break;
	default:
		basePointer = 0;
		break;
	}

	return basePointer;
}

#endif

void GPIO_Init(void)
{
	//Defined by user - change GPIO purpose like UART, ADC or SPI

	//enable clock for switch matrix
	LPC_SYSCTL->SYSAHBCLKCTRL |= (1<<7);

	/* configure pin PIO0_0 to work as USART RXD
	 * and configure pin PIO0_4 to work as USART TXD
	 * (bits 7:0 U0_TXD_O, 15:8 U0_RXD_I) */
	LPC_SWM->PINASSIGN[0] = (0x4)|(0<<8)|(0xFF<<16)|(0xFF<<24);

	/* configure pin PIO0_23 to work as SPI0_SCK_IO
	 * (bits 31:24 SPI0_SCK_IO)*/
	LPC_SWM->PINASSIGN[3] = (LPC_SWM->PINASSIGN[3]&0xFFFFFF)|(23<<24);

	/* configure pin PIO0_12 to work as SPI0_MOSI_IO
	 * configure pin PIO0_13 to work as SPI0_MISO_IO
	 * configure pin PIO0_17 to work as SPI0_SSEL0_IO
	 * (bits 7:0 SPI0_MOSI_IO, 15:8 SPI0_MISO_IO, 23:16 SPI0_SSEL0_IO)*/
	LPC_SWM->PINASSIGN[4] = (12)|(13<<8)|(17<<16)|(0xFF<<24);
}

void GPIO_Direction(uint8_t port, uint8_t pin, GPIO_DIRECTION dir)
{
#ifdef __LPC11XX__
	uint32_t tmpDir = LPC_GPIO0->DIR;
	uint32_t dirMask = ~(1<<pin);
	LPC_GPIO_TypeDef *GPIO_Port = (LPC_GPIO_TypeDef*)GPIO_GetBaseAddress(port);

	GPIO_Port->DIR = (tmpDir & dirMask)|(dir<<pin);
#endif

#if defined(__LPC11UXX__) || defined(__LPC11E6X__)
	uint32_t tmpDir = LPC_GPIO->DIR[port];
	uint32_t dirMask = ~(1<<pin);

	LPC_GPIO->DIR[port] = (tmpDir & dirMask)|(dir<<pin);
#endif

#if defined(__LPC82X__)
	uint32_t tmpDir = LPC_GPIO_PORT->DIR[port];
	uint32_t dirMask = ~(1<<pin);

	LPC_GPIO_PORT->DIR[port] = (tmpDir & dirMask)|(dir<<pin);
#endif
}


void GPIO_SetState(uint8_t port, uint8_t pin, bool state)
{
#ifdef __LPC11XX__
	LPC_GPIO_TypeDef *GPIO_Port = (LPC_GPIO_TypeDef*)GPIO_GetBaseAddress(port);
	GPIO_Port->MASKED_ACCESS[1<<pin] = (state<<pin);
#endif

#if defined(__LPC11UXX__) || defined(__LPC11E6X__)
	LPC_GPIO->B[port][pin] = state;
#endif

#if defined(__LPC82X__)
	LPC_GPIO_PORT->B[port][pin] = state;
#endif
}

bool GPIO_GetState(uint8_t port, uint8_t pin)
{	
#ifdef __LPC11XX__
	LPC_GPIO_TypeDef *GPIO_Port = (LPC_GPIO_TypeDef*)GPIO_GetBaseAddress(port);
	return ((GPIO_Port->MASKED_ACCESS[1<<pin])>>pin);
#endif

#if defined(__LPC11UXX__) || defined(__LPC11E6X__)
	return LPC_GPIO->B[port][pin];
#endif

#if defined(__LPC82X__)
	return LPC_GPIO_PORT->B[port][pin];
#endif
}


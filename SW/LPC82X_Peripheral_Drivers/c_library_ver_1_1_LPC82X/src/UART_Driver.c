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

#include "UART_Driver.h"

/*must be defined by user */
static uint8_t FractionalGaneratorMainClkDiv = 1;
static uint8_t FractionalGaneratorFractionalMultiplier = 0x0;

static uint8_t UartFieldBRGVAL;

/*
 * Equation how baudrate is calculated:
 *
 * Input clock for fraction clock divider:
 * Main clock/(UARTCLKDIV) = Fraction divider clock input
 * UARTCLKDIV - must be grater or equal 1. 0 mean disable
 *
 * Output clock from fractional divider:
 * Fraction divider clock input/(1 + MULT/DIV) = Fractional output
 * MULT - stored in register UARTFRGMULT and is equal exact content of register. MULT
 * equal 0 mean that Fractional output will be equal Fraction divider clock input.
 * DIV - stored in register UARTFRGDIV and is equal content of register + 1, Register
 * only accept 0xFF which mean tha only one parameter is supported - 256.
 *
 * Uart final clock used to generate baudrate:
 * Fractional output/(BRGVAL + 1) = clock used to generate baudrate
 * BRGVAL - value stored in BRG register(All UART port contain own value).
 * BRGVAL can be set as zero then Fractional output will be divide by 1.
 *
 * Baudrate in asynchronous mode:
 * clock used to generate baudrate/16 = Baudrate
 */

static uint32_t* UART_GetBaseAddress(uint8_t portNumber)
{
	uint32_t* basePointer = 0;
	switch(portNumber)
	{
	case 0:
		basePointer = (uint32_t*)LPC_USART0;
		break;

	case 1:
		basePointer = (uint32_t*)LPC_USART1;
		break;

	case 2:
		basePointer = (uint32_t*)LPC_USART2;
		break;

	default:
		basePointer = 0;
		break;
	}

	return basePointer;
}

static void UART_SetClockPrescalers(uint32_t baudrate)
{
	if(LPC_SYSCON->UARTCLKDIV == 0)
	{
		//UART fractional baud rate generator reset
		LPC_SYSCON->PRESETCTRL = LPC_SYSCON->PRESETCTRL & ~(1<<2);
		LPC_SYSCON->PRESETCTRL |= (1<<2);

		//configure fractional divider located in LPC_SYSCON
		LPC_SYSCON->UARTCLKDIV = FractionalGaneratorMainClkDiv;
		LPC_SYSCON->UARTFRGDIV = FRACTIONAL_DIVIDER_EQUAL_256;
		LPC_SYSCON->UARTFRGMULT = FractionalGaneratorFractionalMultiplier;
	}

	/*must be defined by user */
	switch(baudrate)
	{
	case 9600:
		UartFieldBRGVAL = 20;//wasn't calculated, probably this value not work - user must recalculate
		break;
	case 115200:
		UartFieldBRGVAL = 10;//wasn't calculated, probably this value not work - user must recalculate
		break;
	case 3000000:
		UartFieldBRGVAL = 0;//tested and work
	default:
		break;
	}
}

void UART_DriverInit(uint8_t portNumber, uint32_t baudrate, WORD_LENGTH length, STOP_BITS stopBits, PARITY parity)
{
	LPC_USART_T *UART_Port = (LPC_USART_T*)UART_GetBaseAddress(portNumber);

	UART_SetClockPrescalers(baudrate);

	switch(portNumber)
	{
	case 0:
		NVIC_DisableIRQ(UART0_IRQn);

		//connect USART0 to AHB bus
		LPC_SYSCON->SYSAHBCLKCTRL |= (1<<14);

		//UART0 reset
		LPC_SYSCON->PRESETCTRL = LPC_SYSCON->PRESETCTRL & ~(1<<3);
		LPC_SYSCON->PRESETCTRL |= (1<<3);

		break;

	case 1:
		NVIC_DisableIRQ(UART1_IRQn);

		//connect USART1 to AHB bus
		LPC_SYSCON->SYSAHBCLKCTRL |= (1<<15);

		//UART1 reset
		LPC_SYSCON->PRESETCTRL = LPC_SYSCON->PRESETCTRL & ~(1<<4);
		LPC_SYSCON->PRESETCTRL |= (1<<4);

		break;

	case 2:
		NVIC_DisableIRQ(UART2_IRQn);

		//connect USART2 to AHB bus
		LPC_SYSCON->SYSAHBCLKCTRL |= (1<<16);

		//UART2 reset
		LPC_SYSCON->PRESETCTRL = LPC_SYSCON->PRESETCTRL & ~(1<<5);
		LPC_SYSCON->PRESETCTRL |= (1<<5);

		break;
	}/* switch(portNumber) */

	//configure UART parameters like word length and parity
	UART_Port->CFG |= (length<<2)|(parity<<4)|(stopBits<<5);

	//set UART prescaller register value
	UART_Port->BRG = UartFieldBRGVAL;

	//UART enable
	UART_Port->CFG |= 1;

	switch(portNumber)
	{
	case 0:
		NVIC_EnableIRQ(UART0_IRQn);
		break;

	case 1:
		NVIC_EnableIRQ(UART1_IRQn);
		break;

	case 2:
		NVIC_EnableIRQ(UART2_IRQn);
		break;
	}
}

void UART_PutByteToTransmitter(uint8_t portNumber, uint8_t byte)
{
	LPC_USART_T *UART_Port = (LPC_USART_T*)UART_GetBaseAddress(portNumber);
	UART_Port->TXDATA = byte;
}

uint8_t UART_ReadByteFromTrasmitter(uint8_t portNumber)
{
	LPC_USART_T *UART_Port = (LPC_USART_T*)UART_GetBaseAddress(portNumber);
	return UART_Port->RXDATA;
}

UART_Status UART_ReturnStatusRegister(uint8_t portNumber)
{
	LPC_USART_T *UART_Port = (LPC_USART_T*)UART_GetBaseAddress(portNumber);
	UART_Status status = *((UART_Status*)&(UART_Port->STAT));
	return status;
}

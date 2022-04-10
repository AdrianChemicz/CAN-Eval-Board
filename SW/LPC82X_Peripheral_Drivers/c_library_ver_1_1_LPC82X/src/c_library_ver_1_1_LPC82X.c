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

#include <cr_section_macros.h>
#include <stdbool.h>
#include <stdint.h>

#include "GPIO_Driver.h"
#include "UART_Driver.h"
#include "SPI_Driver.h"
#include "I2C_Driver.h"

/*
* Purpose of this application is to test all peripheral supported by this library like
* GPIO, UART, SPI and I2C. In this example clock source can be changed from IRC to PLL
* which operate on external quartz. During test tested board was used with 12MHz quartz
* and core is clocked with 24MHz but UART was cloced with 48MHz which provide 3MBaud.
*/

void ClockSleep(uint32_t time)
{
#if 1 //good value for 12MHz // for 1ms wait is equal 1,89ms tested under logic analyzer
	for (volatile uint32_t count = 0; count < (uint32_t)(time<<11);)
#endif
#if 0 //good value for 48MHz
	for (volatile uint32_t count = 0; count < (uint32_t)(time<<13);)
#endif
	{
		 count++;
	}
}

void ClockChangeFrequency(void)
{
	// Set system main clock divider
	LPC_SYSCON->SYSAHBCLKDIV  = 2;

#if 1 //Code for XTAL oscilator
	// Enable IOCON block
	LPC_SYSCTL->SYSAHBCLKCTRL |= (1<<18);

	/* Disable pullup/pulldown resistor for pin PIO0_8/XTALIN
	 * In IOCON->PIO0 registers pin number can't be use as index
	 * because table index not mean pin number. Element 0 is PIO0_17,
	 * element 1 is PIO0_13, element 2 is PIO0_12. */
	LPC_IOCON->PIO0[14] &= ~PIN_MODE_MASK;

	/* Disable pullup/pulldown resistor for pin PIO0_9/XTALOUT
	 * In IOCON->PIO0 registers pin number can't be use as index
	 * because table index not mean pin number. Element 0 is PIO0_17,
	 * element 1 is PIO0_13, element 2 is PIO0_12. */
	LPC_IOCON->PIO0[13] &= ~PIN_MODE_MASK;

	// Enable clock for switch matrix
	LPC_SYSCTL->SYSAHBCLKCTRL |= (1<<7);

	// Activate XTALIN function for PIO0_8/XTALIN pin
	LPC_SWM->PINENABLE0 &= ~(1<<6);

	// Activate XTALOUT function for PIO0_9/XTALOUT pin
	LPC_SWM->PINENABLE0 &= ~(1<<7);

	//Power-up System Osc
	LPC_SYSCON->PDRUNCFG &= ~(1<<5);

	//Configure system oscilator to work standard quartz and with quart with frequency range 1 MHz-20 MHz
	LPC_SYSCON->SYSOSCCTRL = 0;

	ClockSleep(1);

	/* Select PLL Input      1=System oscillator. Crystal Oscillator (SYSOSC)
	   0x0 IRC */
	LPC_SYSCON->SYSPLLCLKSEL = 1;
#else
	/* Select PLL Input      1=System oscillator. Crystal Oscillator (SYSOSC)
	   0x0 IRC */
	LPC_SYSCON->SYSPLLCLKSEL = 0;
#endif

	LPC_SYSCON->SYSPLLCLKUEN  = 0x01;               /* Update Clock Source      */
	LPC_SYSCON->SYSPLLCLKUEN  = 0x00;               /* Toggle Update Register   */
	LPC_SYSCON->SYSPLLCLKUEN  = 0x01;

	// Wait Until Updated
	while (!(LPC_SYSCON->SYSPLLCLKUEN & 0x01));

	// Power-up SYSPLL
	LPC_SYSCON->SYSPLLCTRL = 0x23;

	// Power-up PLL
	LPC_SYSCON->PDRUNCFG &= ~(1<<7);

	// Wait Until PLL Locked
	while (!(LPC_SYSCON->SYSPLLSTAT & 0x01));

	LPC_SYSCON->MAINCLKSEL    = 3;     /* Select PLL Clock Output  */
	LPC_SYSCON->MAINCLKUEN    = 0x01;  /* Update MCLK Clock Source */
	LPC_SYSCON->MAINCLKUEN    = 0x00;  /* Toggle Update Register   */
	LPC_SYSCON->MAINCLKUEN    = 0x01;

	// Wait Until Updated
	while (!(LPC_SYSCON->MAINCLKUEN & 0x01));
}

int main(void) {

	GPIO_Init();

#if 1
	//check work microcontroller with active PLL
	{
		volatile bool changeFrequency = false;

		if (changeFrequency)
		{
			ClockChangeFrequency();
		}
	}
#endif

#if 1
	//GPIO test
	{
		volatile bool changeGpio = false;

		if (changeGpio)
		{
			GPIO_Direction(0, 0, GPIO_DIR_OUTPUT);
			GPIO_SetState(0, 0, false);
			GPIO_SetState(0, 0, true);
			GPIO_SetState(0, 0, false);
			GPIO_SetState(0, 0, true);

			for (bool stateTmp = false; ; )
			{
				stateTmp = GPIO_GetState(0, 14);
				GPIO_SetState(0, 0, stateTmp);
			}
		}
	}
#endif

#if 0
	//UART test
	{
		volatile uint8_t dataFromUart = 0;

		UART_DriverInit(0, 3000000, L8_BIT, ONE_BIT, NONE_PARITY);
		UART_Status uartStatus;

		for (;;)
		{
			uartStatus = UART_ReturnStatusRegister(0);

			if(uartStatus.TXRDY == 1)
			{
				UART_PutByteToTransmitter(0, 0xAA);
			}

			dataFromUart = 0;

			if(uartStatus.RXRDY == 1)
			{
				dataFromUart = UART_ReadByteFromTrasmitter(0);
			}
		}
	}
#endif

#if 0
	//SPI test
	{
		volatile uint8_t dataFromSpi = 0;
		volatile uint16_t counter = 0;

		SPI_DriverInit(0, SPI_CLK_IDLE_HIGH, SPI_CLK_TRAILING);
		SPI_Status spiStatus;

		for (;;)
		{
			SPI_PutByteToTransmitter(0, 0x20, SPI_CHIP_TXSSEL0_N, false, true);

			spiStatus = SPI_ReturnStatusRegister(0);
			for (;spiStatus.TXRDY == 0;)
			{
				spiStatus = SPI_ReturnStatusRegister(0);
				counter++;
			}

			SPI_PutByteToTransmitter(0, 0x21, SPI_CHIP_TXSSEL0_N, false, true);

			spiStatus = SPI_ReturnStatusRegister(0);
			for (;spiStatus.TXRDY == 0;)
			{
				spiStatus = SPI_ReturnStatusRegister(0);
				counter++;
			}

			SPI_PutByteToTransmitter(0, 0x22, SPI_CHIP_TXSSEL0_N, true, true);

			spiStatus = SPI_ReturnStatusRegister(0);
			for (;spiStatus.TXRDY == 0;)
			{
				spiStatus = SPI_ReturnStatusRegister(0);
				dataFromSpi = SPI_ReadByteFromTrasmitter(0);
				counter++;
			}

			ClockSleep(1);
		}
	}
#endif

#if 1
	//I2C test
	{
		uint8_t sendData[] = { 0x8F };
		uint8_t *dataBuffer = 0;

		I2C_DriverInit(0);

		ClockSleep(1);

		I2C_SendReadData(0, 0xD2, sendData, 1, 2);

		for(I2C_Status finishOperationFlag = I2C_SENDING;
				finishOperationFlag == I2C_WAITING_FOR_SEND
				|| finishOperationFlag == I2C_SENDING;)
		{
	    	finishOperationFlag = I2C_CheckStatus(0);
		}

	    dataBuffer = I2C_PointerToInternalReadBuffer(0);

	    ClockSleep(1);
	}
#endif
    // Force the counter to be placed into memory
    volatile static int i = 0 ;

    // Enter an infinite loop, just incrementing a counter
    while(1) {
        i++ ;
    }
    return 0 ;
}

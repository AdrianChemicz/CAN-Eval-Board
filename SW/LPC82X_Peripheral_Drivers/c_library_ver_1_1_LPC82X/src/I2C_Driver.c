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

#include "I2C_Driver.h"

static uint16_t I2C_CLKDIV;
static uint8_t MSTSCLLOW;
static uint8_t MSTSCLHIGH;

I2C_Device_Instance I2C_Device_Instance_Table[NUMBER_OF_I2C_PERIPHERALS];

static uint32_t* I2C_GetBaseAddress(uint8_t portNumber)
{
	uint32_t* basePointer = 0;
	switch(portNumber)
	{
	case 0:
		basePointer = (uint32_t*)LPC_I2C0;
		break;

	case 1:
		basePointer = (uint32_t*)LPC_I2C1;
		break;

	case 2:
		basePointer = (uint32_t*)LPC_I2C2;
		break;

	case 3:
		basePointer = (uint32_t*)LPC_I2C3;
		break;

	default:
		break;
	}

	return basePointer;
}

static void I2C_SetClockPrescalers(uint8_t portNumber)
{
	/*must be defined by user*/
	switch(portNumber)
	{
	case 0:
		I2C_CLKDIV = 0x2080;
		MSTSCLLOW = 0x5;
		MSTSCLHIGH = 0x5;
		break;

	case 1:
		I2C_CLKDIV = 0x2080;
		MSTSCLLOW = 0x5;
		MSTSCLHIGH = 0x5;
		break;

	case 2:
		I2C_CLKDIV = 0x2080;
		MSTSCLLOW = 0x5;
		MSTSCLHIGH = 0x5;
		break;

	case 3:
		I2C_CLKDIV = 0x2080;
		MSTSCLLOW = 0x5;
		MSTSCLHIGH = 0x5;
		break;

	default:
		I2C_CLKDIV = 0x2080;
		MSTSCLLOW = 0x5;
		MSTSCLHIGH = 0x5;
		break;
	}
}

static void I2C_InterruptEnable(uint8_t portNumber)
{
	//enable interrupt for appropriate port
	switch(portNumber)
	{
	case 0:
		//enable I2C0 interrupt
		NVIC_EnableIRQ(I2C0_IRQn);
		break;

	case 1:
		//enable I2C1 interrupt
		NVIC_EnableIRQ(I2C1_IRQn);
		break;

	case 2:
		//enable I2C2 interrupt
		NVIC_EnableIRQ(I2C2_IRQn);
		break;

	case 3:
		//enable I2C3 interrupt
		NVIC_EnableIRQ(I2C3_IRQn);
		break;

	default:
		break;
	}
}

static void I2C_InterruptDisable(uint8_t portNumber)
{
	//enable interrupt for appropriate port
	switch(portNumber)
	{
	case 0:
		//enable I2C0 interrupt
		NVIC_DisableIRQ(I2C0_IRQn);
		break;

	case 1:
		//enable I2C1 interrupt
		NVIC_DisableIRQ(I2C1_IRQn);
		break;

	case 2:
		//enable I2C2 interrupt
		NVIC_DisableIRQ(I2C2_IRQn);
		break;

	case 3:
		//enable I2C3 interrupt
		NVIC_DisableIRQ(I2C3_IRQn);
		break;

	default:
		break;
	}
}

void I2C_InterruptProccess(uint8_t portNumber)
{
	LPC_I2C_T *I2C_Port = (LPC_I2C_T*)I2C_GetBaseAddress(portNumber);
	uint32_t busStatus = I2C_Port->STAT;

	if (busStatus&I2C_STAT_MSTARBLOSS)
	{
		I2C_Port->STAT = I2C_STAT_MSTARBLOSS;

		//disable interrupt in moment when error occur
		I2C_InterruptDisable(portNumber);

		//change device status
		I2C_Device_Instance_Table[portNumber].deviceStatus = I2C_WAITING_FOR_DATA;
	}
	else if (busStatus&I2C_STAT_MSTSTSTPERR)
	{
		I2C_Port->STAT = I2C_STAT_MSTSTSTPERR;

		//disable interrupt in moment when error occur
		I2C_InterruptDisable(portNumber);

		//change device status
		I2C_Device_Instance_Table[portNumber].deviceStatus = I2C_WAITING_FOR_DATA;
	}
	else if (busStatus&I2C_STAT_MSTPENDING)
	{
		uint8_t masterStatus = (busStatus>>1)&7;

		switch (masterStatus)
		{
		case I2C_MSTAT_IDLE:
			//disable interrupt in moment when operaion was finished to prevent enter to idle state
			I2C_InterruptDisable(portNumber);

			//change device status
			I2C_Device_Instance_Table[portNumber].deviceStatus = I2C_WAITING_FOR_DATA;
			break;

		case I2C_MSTAT_RECEIVE_READY:
			I2C_Device_Instance_Table[portNumber].readBuffer[I2C_Device_Instance_Table[portNumber].dataToReadStatus] = I2C_Port->MSTDAT;

			//check how much read data is needed. If one then clear ACK bit
			if((I2C_Device_Instance_Table[portNumber].dataToRead - I2C_Device_Instance_Table[portNumber].dataToReadStatus) == 1)
			{
				//in master control register set stop bit (MSTSTOP)
				I2C_Port->MSTCTL = I2C_MSTCTL_MSTSTOP;
			}
			else//then more than one then set ACK bit
			{
				//in master control register set stop bit (MSTCONTINUE)
				I2C_Port->MSTCTL = I2C_MSTCTL_MSTCONTINUE;
			}

			I2C_Device_Instance_Table[portNumber].dataToReadStatus++;
			break;

		case I2C_MSTAT_NACK_ADDRESS:
		case I2C_MSTAT_TRANSMIT_READY:
			if(I2C_Device_Instance_Table[portNumber].typeOfOperation == I2C_WRITE_OPERATION)
			{
				//change device status
				I2C_Device_Instance_Table[portNumber].deviceStatus = I2C_SENDING;

				//execute when all data was send
				if(I2C_Device_Instance_Table[portNumber].dataToSendStatus
						== I2C_Device_Instance_Table[portNumber].dataToSend)
				{
					//check read buffer
					if(I2C_Device_Instance_Table[portNumber].dataToRead > 0)
					{
						//change write operation to read operation
						I2C_Device_Instance_Table[portNumber].typeOfOperation = I2C_READ_OPERATION;

						//send device address with read bit
						I2C_Port->MSTDAT = I2C_Device_Instance_Table[portNumber].deviceAddress|1;

						//in master control register set start bit(MSTSTART)
						I2C_Port->MSTCTL = I2C_MSTCTL_MSTSTART;
					}
					else//execute when read data isn't necessary
					{
						//in master control register set stop bit (MSTSTOP)
						I2C_Port->MSTCTL = I2C_MSTCTL_MSTSTOP;
					}
				}
				else //send data
				{
					//copy data to I2C buffer
					I2C_Port->MSTDAT = I2C_Device_Instance_Table[portNumber].sendBuffer[I2C_Device_Instance_Table[portNumber].dataToSendStatus];

					//in master control register set continue bit(MSTCONTINUE)
					I2C_Port->MSTCTL = I2C_MSTCTL_MSTCONTINUE;

					I2C_Device_Instance_Table[portNumber].dataToSendStatus++;
				}
			}
			else if(I2C_Device_Instance_Table[portNumber].typeOfOperation == I2C_READ_OPERATION)
			{
				//in master control register set stop bit (MSTSTOP)
				I2C_Port->MSTCTL = I2C_MSTCTL_MSTSTOP;
			}
			break;

		case I2C_MSTAT_NACK_DATA:
			//in master control register set stop bit (MSTSTOP)
			I2C_Port->MSTCTL = I2C_MSTCTL_MSTSTOP;
			break;

		default://this state shouldn't occur
			//in master control register set stop bit (MSTSTOP)
			I2C_Port->MSTCTL = I2C_MSTCTL_MSTSTOP;
			break;
		}
	}
	else//This case shouldn't occure but after startup device to this case very often device enter
	{
		if(I2C_Device_Instance_Table[portNumber].unspecifiedErrorStateCounter >= I2C_MAX_UNSPECIFIED_ERROR)
		{
			//disable interrupt in moment when strange error occur
			I2C_InterruptDisable(portNumber);

			//change device status
			I2C_Device_Instance_Table[portNumber].deviceStatus = I2C_WAITING_FOR_DATA;
		}
		else
		{
			I2C_Device_Instance_Table[portNumber].unspecifiedErrorStateCounter++;
		}
	}
}

void I2C0_IRQHandler()
{
	I2C_InterruptProccess((uint8_t)0);
}

void I2C1_IRQHandler()
{
	I2C_InterruptProccess((uint8_t)1);
}

void I2C2_IRQHandler()
{
	I2C_InterruptProccess((uint8_t)2);
}

void I2C3_IRQHandler()
{
	I2C_InterruptProccess((uint8_t)3);
}

void I2C_DriverInit(uint8_t portNumber)
{
	LPC_I2C_T *I2C_Port = (LPC_I2C_T*)I2C_GetBaseAddress(portNumber);
	I2C_SetClockPrescalers(portNumber);

	I2C_Device_Instance_Table[portNumber].deviceStatus = I2C_WAITING_FOR_DATA;

	//reset and conect appropriate I2C port
	switch(portNumber)
	{
	case 0:
		//I2C0 reset
		LPC_SYSCON->PRESETCTRL = LPC_SYSCON->PRESETCTRL & ~(1<<6);
		LPC_SYSCON->PRESETCTRL |= (1<<6);

		//enable I2C0_SDA function on pin PIO0_11
		LPC_SWM->PINENABLE0 &= ~(1<<11);

		//enable I2C0_SCL function on pin PIO0_10
		LPC_SWM->PINENABLE0 &= ~(1<<12);

		//connect I2C0 to AHB bus
		LPC_SYSCTL->SYSAHBCLKCTRL |= (1<<5);
		break;

	case 1:
		//I2C1 reset
		LPC_SYSCON->PRESETCTRL = LPC_SYSCON->PRESETCTRL & ~(1<<14);
		LPC_SYSCON->PRESETCTRL |= (1<<14);

		//connect I2C1 to AHB bus
		LPC_SYSCTL->SYSAHBCLKCTRL |= (1<<21);
		break;

	case 2:
		//I2C2 reset
		LPC_SYSCON->PRESETCTRL = LPC_SYSCON->PRESETCTRL & ~(1<<15);
		LPC_SYSCON->PRESETCTRL |= (1<<15);

		//connect I2C2 to AHB bus
		LPC_SYSCTL->SYSAHBCLKCTRL |= (1<<22);
		break;

	case 3:
		//I2C3 reset
		LPC_SYSCON->PRESETCTRL = LPC_SYSCON->PRESETCTRL & ~(1<<16);
		LPC_SYSCON->PRESETCTRL |= (1<<16);

		//connect I2C3 to AHB bus
		LPC_SYSCTL->SYSAHBCLKCTRL |= (1<<23);
		break;

	default:
		break;
	}

	//Set I2C prescaller
	I2C_Port->CLKDIV = I2C_CLKDIV;

	//Set timming parameters for low state and hight state
	I2C_Port->MSTTIME = (MSTSCLHIGH<<4)|MSTSCLLOW;

	//enable I2C
	I2C_Port->CFG |= I2C_CFG_MASTER_ENABLE;

	//enable interrupts from master
	I2C_Port->INTENSET |= I2C_INT_MSTPENDINGEN|I2C_INT_MSTARBLOSSEN|I2C_INT_MSTSTSTPERREN;
}

void I2C_SendReadData(uint8_t portNumber, uint8_t deviceAddress, uint8_t *dataToSend, uint8_t sizeOfSendData, uint8_t sizeOfReadData)
{
	LPC_I2C_T *I2C_Port = (LPC_I2C_T*)I2C_GetBaseAddress(portNumber);

	//initialize data
	I2C_Device_Instance_Table[portNumber].deviceAddress = deviceAddress;
	if(dataToSend != 0)
		memcpy(I2C_Device_Instance_Table[portNumber].sendBuffer, dataToSend, sizeOfSendData);

	I2C_Device_Instance_Table[portNumber].dataToSend = sizeOfSendData;
	I2C_Device_Instance_Table[portNumber].dataToRead = sizeOfReadData;

	//change device status after initialize structure
	I2C_Device_Instance_Table[portNumber].deviceStatus = I2C_WAITING_FOR_SEND;

	//clear error counter
	I2C_Device_Instance_Table[portNumber].unspecifiedErrorStateCounter = 0;

	if(I2C_Device_Instance_Table[portNumber].dataToSend > 0)
	{
		I2C_Device_Instance_Table[portNumber].dataToSendStatus = 0;
		I2C_Device_Instance_Table[portNumber].typeOfOperation = I2C_WRITE_OPERATION;
		I2C_Port->MSTDAT = I2C_Device_Instance_Table[portNumber].deviceAddress;
	}
	else if(I2C_Device_Instance_Table[portNumber].dataToRead > 0)
	{
		I2C_Device_Instance_Table[portNumber].dataToReadStatus = 0;
		I2C_Device_Instance_Table[portNumber].typeOfOperation = I2C_READ_OPERATION;
		I2C_Port->MSTDAT = (I2C_Device_Instance_Table[portNumber].deviceAddress|1U);
	}

	//clear errors from master arbitration loss and master start/stop error
	I2C_Port->STAT = (I2C_STAT_MSTARBLOSS|I2C_STAT_MSTSTSTPERR);

	//send start bit
	I2C_Port->MSTCTL = I2C_MSTCTL_MSTSTART;

	I2C_InterruptEnable(portNumber);
}

I2C_Status I2C_CheckStatus(uint8_t portNumber)
{
	return I2C_Device_Instance_Table[portNumber].deviceStatus;
}

uint8_t* I2C_PointerToInternalReadBuffer(uint8_t portNumber)
{
	return I2C_Device_Instance_Table[portNumber].readBuffer;
}

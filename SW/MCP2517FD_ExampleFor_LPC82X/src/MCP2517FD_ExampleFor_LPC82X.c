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

/*****************************************************************************************
 * Author of this source file: Adrian Chemicz
 * Code base on microchip example for MCP2517FD chip. Only few part of code is very
 * similar but I'm not a license expert so I copied Microchip header license from original
 * file. When this topic will be clarified file will be updated. If it will be possible
 * license will be changed to 3-Clause BSD License.
 *
 * This example present how initialize, send CAN data and receive CAN data via MCP2517FD
 * chip. Also was added information how debug simple problem with SPI connection.
 * Code use as systic timer so it should be very easy portable to other ARM microcontroller.
 *****************************************************************************************/

#include "../driver/canfdspi/drv_canfdspi_api.h"
#include "../driver/spi/drv_spi.h"
#include "chip.h"
#include "GPIO_Driver.h"

/*****************************************************************************************
 * Structures used to configure MCP2517FD
 *****************************************************************************************/
// Transmit FIFO channel
#define CAN_TX_FIFO CAN_FIFO_CH2

// Receive FIFO channel
#define CAN_RX_FIFO CAN_FIFO_CH1

// Maximal amount of test that TX fifo isn't full
#define MAX_TXQUEUE_ATTEMPTS 20

// CAN configuration object
CAN_CONFIG canConfig;

// Transmit objects
CAN_TX_FIFO_CONFIG canTxConfig;
CAN_TX_MSGOBJ canTxObj;

// Receive objects
CAN_RX_FIFO_CONFIG canRxConfig;
REG_CiFLTOBJ canFifoFilterObj;
REG_CiMASK canFifoMaskObj;

CAN_RX_MSGOBJ canRxMsgObj;
uint8_t canRxMsgPayload[MAX_DATA_BYTES];

// Comunication status flags and error counters which is get from CiTREC register
CAN_ERROR_STATE canErrorFlags;
uint8_t canTrasmitErrorCounter;
uint8_t canReceiveErrorCounter;

/*****************************************************************************************
 * Application variables
 *****************************************************************************************/
uint32_t canRxMessageCounter;
uint32_t interruptCounter;

/*****************************************************************************************
* InitCanFdChip() - initialize MCP2517FD chip to work with appropriate baudrate and mode.
* During initialization is also correctly configured RX and TX FIFO.
*
*****************************************************************************************/
void InitCanFdChip(void)
{
	// Reset device
	DRV_CANFDSPI_Reset(DRV_CANFDSPI_INDEX_0);

	// Enable ECC and initialize RAM
	DRV_CANFDSPI_EccEnable(DRV_CANFDSPI_INDEX_0);

	DRV_CANFDSPI_RamInit(DRV_CANFDSPI_INDEX_0, 0xff);

	// Configure device by set CiCON register
	DRV_CANFDSPI_ConfigureObjectReset(&canConfig);
	canConfig.IsoCrcEnable = 1;
	canConfig.StoreInTEF = 0;

	DRV_CANFDSPI_Configure(DRV_CANFDSPI_INDEX_0, &canConfig);

	// Setup TX FIFO by set CiFIFOCON register
	DRV_CANFDSPI_TransmitChannelConfigureObjectReset(&canTxConfig);
	canTxConfig.FifoSize = 7;
	canTxConfig.PayLoadSize = CAN_PLSIZE_64;
	canTxConfig.TxPriority = 1;

	DRV_CANFDSPI_TransmitChannelConfigure(DRV_CANFDSPI_INDEX_0, CAN_TX_FIFO, &canTxConfig);

	// Setup RX FIFO by set CiFIFOCON register
	DRV_CANFDSPI_ReceiveChannelConfigureObjectReset(&canRxConfig);
	canRxConfig.FifoSize = 15;
	canRxConfig.PayLoadSize = CAN_PLSIZE_64;

	DRV_CANFDSPI_ReceiveChannelConfigure(DRV_CANFDSPI_INDEX_0, CAN_RX_FIFO, &canRxConfig);

	// Setup RX Filter by set CiFLTOBJ0 register
	canFifoFilterObj.word = 0;
	canFifoFilterObj.bF.SID = 0xda;
	canFifoFilterObj.bF.EXIDE = 0;
	canFifoFilterObj.bF.EID = 0x00;

	DRV_CANFDSPI_FilterObjectConfigure(DRV_CANFDSPI_INDEX_0, CAN_FILTER0, &canFifoFilterObj.bF);

	// Setup RX Mask by set CiMASK0 register
	canFifoMaskObj.word = 0;
	canFifoMaskObj.bF.MSID = 0x0;
	canFifoMaskObj.bF.MIDE = 1; // Only allow standard IDs
	canFifoMaskObj.bF.MEID = 0x0;
	DRV_CANFDSPI_FilterMaskConfigure(DRV_CANFDSPI_INDEX_0, CAN_FILTER0, &canFifoMaskObj.bF);

	// Link FIFO and Filter by set CiFLTCON0 register
	DRV_CANFDSPI_FilterToFifoLink(DRV_CANFDSPI_INDEX_0, CAN_FILTER0, CAN_RX_FIFO, true);

	// Setup Bit Time
	DRV_CANFDSPI_BitTimeConfigure(DRV_CANFDSPI_INDEX_0, CAN_500K_2M, CAN_SSP_MODE_AUTO, CAN_SYSCLK_40M);

	DRV_CANFDSPI_ReceiveChannelEventEnable(DRV_CANFDSPI_INDEX_0, CAN_RX_FIFO, CAN_RX_FIFO_NOT_EMPTY_EVENT);
	DRV_CANFDSPI_ModuleEventEnable(DRV_CANFDSPI_INDEX_0, CAN_TX_EVENT | CAN_RX_EVENT);

	// Select Normal Mode
	DRV_CANFDSPI_OperationModeSelect(DRV_CANFDSPI_INDEX_0, CAN_NORMAL_MODE);
}

/*****************************************************************************************
* TestCanChipRamAccess() - very useful function which can be used to verify that SPI
* connection work correctly. For tested microcontroller not connected SPI to MCP2517FD
* chip return 0xFF on all bytes in rxd tables after call function
* DRV_CANFDSPI_ReadByteArray.
*
* Return: true if all send data to MCP2517FD ram is the same as data read in next step.
* When some data mismatch occur then return false.
*****************************************************************************************/
bool TestCanChipRamAccess(void)
{
	uint8_t txd[MAX_DATA_BYTES];
	uint8_t rxd[MAX_DATA_BYTES];

	// Verify read/write with different access length
	// Note: RAM can only be accessed in multiples of 4 bytes
	for (uint8_t length = 4; length <= MAX_DATA_BYTES; length += 4)
	{
		for (uint32_t i = 0; i < length; i++)
		{
			txd[i] = rand() & 0xff;
			rxd[i] = 0xff;
		}

		// Write data to RAM
		DRV_CANFDSPI_WriteByteArray(DRV_CANFDSPI_INDEX_0, cRAMADDR_START, txd, length);

		// Read data back from RAM
		DRV_CANFDSPI_ReadByteArray(DRV_CANFDSPI_INDEX_0, cRAMADDR_START, rxd, length);

		// Verify value which was send to RAM with value which was read
		for (uint32_t i = 0; i < length; i++)
		{
			if (txd[i] != rxd[i])
			{
				// Data mismatch
				return false;
			}
		}
	}/* for (length = 4; length <= MAX_DATA_BYTES; length += 4) */

	return true;
}/* bool TestCanChipRamAccess(void) */

/*****************************************************************************************
* ReceiveCanMessage() - receive single message from FIFO buffer if isn't empty.
*
*****************************************************************************************/
void ReceiveCanMessage(void)
{
	CAN_RX_FIFO_EVENT canRxFlags;

	DRV_CANFDSPI_ReceiveChannelEventGet(DRV_CANFDSPI_INDEX_0, CAN_RX_FIFO, &canRxFlags);

	if (canRxFlags & CAN_RX_FIFO_NOT_EMPTY_EVENT)
	{
		// Get CAN RX message and move to global variable
		DRV_CANFDSPI_ReceiveMessageGet(DRV_CANFDSPI_INDEX_0, CAN_RX_FIFO, &canRxMsgObj,
			canRxMsgPayload, MAX_DATA_BYTES);

		// User can add here own code to process payload of received message


		canRxMessageCounter++;
	}
}/* void ReceiveCanMessage(void) */

/*****************************************************************************************
* TransmitCanMessage() - send multiply frame with similar payload when FIFO buffer is
* empty or send single message when FIFO buffer isn't full. Function also assign information
* to appropriate gobal variable when send isn't possible.
*
*****************************************************************************************/
void TransmitCanMessage(void)
{
	uint8_t dlcToByteSize;
	uint8_t txd[MAX_DATA_BYTES];
	CAN_TX_FIFO_EVENT canTxFlags;

	// Initialize CAN structure with information about CAN ID, length and flags
	canTxObj.bF.id.SID = 0x100;//CAN ID message

	canTxObj.bF.ctrl.DLC = 15;
	canTxObj.bF.ctrl.IDE = 0;
	canTxObj.bF.ctrl.BRS = 1;
	canTxObj.bF.ctrl.FDF = 1;

	dlcToByteSize = DRV_CANFDSPI_DlcToDataBytes((CAN_DLC) 15);

	// Initialize CAN payload by random data
	for (int i = 0; i < dlcToByteSize; i++)
	{
		txd[i] = rand() & 0xff;
	}

	{
		uint8_t attempts = MAX_TXQUEUE_ATTEMPTS;

		// Check if FIFO is not full
		do
		{
			// Get transmission status flags for coresponding FIFO buffer
			DRV_CANFDSPI_TransmitChannelEventGet(DRV_CANFDSPI_INDEX_0, CAN_TX_FIFO, &canTxFlags);

			// When send isn't possible then check device status
			if (attempts == 0)
			{
				Nop();
				DRV_CANFDSPI_ErrorCountStateGet(DRV_CANFDSPI_INDEX_0, &canTrasmitErrorCounter,
						&canReceiveErrorCounter, &canErrorFlags);
				return;
			}

			attempts--;
		}
		while (!(canTxFlags & CAN_TX_FIFO_NOT_FULL_EVENT));

		// Check that buffer is empty and then send many data via buffer
		if (canTxFlags & CAN_TX_FIFO_EMPTY_EVENT)
		{
			for (int i = 0; i < 4; i++)
			{
				txd[0] = i;

				// Transmit CAN message
				DRV_CANFDSPI_TransmitChannelLoad(DRV_CANFDSPI_INDEX_0, CAN_TX_FIFO, &canTxObj, txd, dlcToByteSize, true);
			}
		}
		else// Buffer is not full and isn't empty so then send single CAN message
		{
			// Transmit CAN message
			DRV_CANFDSPI_TransmitChannelLoad(DRV_CANFDSPI_INDEX_0, CAN_TX_FIFO, &canTxObj, txd, dlcToByteSize, true);
		}
	}
}/* void TransmitCanMessage(void) */

void SysTick_Handler(void)
{
	if(interruptCounter >= 5)
	{
		ReceiveCanMessage();

		TransmitCanMessage();

		interruptCounter = 0;
	}

	interruptCounter++;
}

int main(void)
{
	// Variable which can be used to confirm that access via SPI is performed correctly
	volatile bool ramTestStatus = false;

	DRV_SPI_Initialize();

	InitCanFdChip();

	ramTestStatus = TestCanChipRamAccess();

#if 0 //SPI protocol debug code
	{
		uint8_t testedWritePayload[8] = {46, 5, 124, 119, 122, 9, 87, 234};
		uint8_t readPayload[8] = {0, 0, 0, 0, 0, 0, 0, 0};
		// Write data to RAM
		DRV_CANFDSPI_WriteByteArray(DRV_CANFDSPI_INDEX_0, cRAMADDR_START, testedWritePayload, 8);

		// Read data back from RAM
		DRV_CANFDSPI_ReadByteArray(DRV_CANFDSPI_INDEX_0, cRAMADDR_START, readPayload, 8);

		Nop();
	}
#endif
	TransmitCanMessage();

	/***********************************************************************
	 * configure systick timer
	 **********************************************************************/
	// Clear SYST_CVR register
	SysTick->VAL = 0;

	// Set counted value to SYST_RVR register
	SysTick->LOAD = (1<<23);

	// Set bit 0(ENABLE) and 1(TICKINT) in SYST_CSR register
	SysTick->CTRL |= 3;

	// Force the counter to be placed into memory
	volatile static int i = 0 ;
	// Enter an infinite loop, just incrementing a counter
	while(1) {
		i++ ;
	}
	return 0 ;
}/* int main(void) */

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

#ifndef _GPIO_DRIVER_H_
#define _GPIO_DRIVER_H_

/*
* This module handle GPIO of LPC11E6x, LPC111x, LPC11U2X and LPC82X microcontrolers.
* Selection between microntroler family is selected via defines(set 1 in appropriate
* define USE_LPC111x_SERIES or USE_LPC11E6x_SERIES). Functions in this module offer
* necessary functionality like set GPIO direction, set state and get state. Function
* GPIO_Init is place where user can put own code which change function of GPIO pins
* which on default is use as other fuction like Reserved or SWD debug.
*
* Simple example code to use GPIO module:
*
* 	GPIO_Init();
*
*	bool state1 = GPIO_GetState(1, 0);
*
* 	GPIO_Direction(1, 0, GPIO_DIR_OUTPUT);
*
*  	GPIO_SetState(1, 0, true);
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

#include "ftd2xx.h"
#include "LibFT4222.h"
#include "FT_Devices.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MICROCONTROLLER
	typedef struct
	{
		uint8_t port;
		uint8_t pin;
		uint8_t ftPin;
		FT_HANDLE ftGPIO;
		GPIO_Dir *pointerToGPIOTable;
	}GPIO_PinStruct;

#define GPIO_TABLE_SIZE 30
#endif

	typedef enum GPIO_DIRECTION
	{
		GPIO_DIR_INPUT = 0,
		GPIO_DIR_OUTPUT = 1
	}GPIO_DIRECTION;

	void GPIO_Init();
	void GPIO_Direction(uint8_t port, uint8_t pin, GPIO_DIRECTION dir);
	void GPIO_SetState(uint8_t port, uint8_t pin, bool state);
	bool GPIO_GetState(uint8_t port, uint8_t pin);

#ifdef __cplusplus
}
#endif

#endif  /* _GPIO_DRIVER_H_ */

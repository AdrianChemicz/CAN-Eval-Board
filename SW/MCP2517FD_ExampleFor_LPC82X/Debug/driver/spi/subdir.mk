################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../driver/spi/drv_spi.c 

OBJS += \
./driver/spi/drv_spi.o 

C_DEPS += \
./driver/spi/drv_spi.d 


# Each subdirectory must supply rules for building sources it contributes
driver/spi/%.o: ../driver/spi/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -DDEBUG -DMICROCONTROLLER -D__CODE_RED -DCORE_M0PLUS -D__MTB_BUFFER_SIZE=256 -D__USE_ROMDIVIDE -D__USE_LPCOPEN -DNO_BOARD_LIB -D__LPC82X__ -D__REDLIB__ -I"D:\NXP_workspace\MCP2517FD_ExampleFor_LPC82X\inc" -I"D:\NXP_workspace\lpc_chip_82x\inc" -Os -fno-common -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m0 -mthumb -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



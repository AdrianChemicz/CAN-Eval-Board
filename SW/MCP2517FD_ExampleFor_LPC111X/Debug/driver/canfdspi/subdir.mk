################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../driver/canfdspi/drv_canfdspi_api.c 

OBJS += \
./driver/canfdspi/drv_canfdspi_api.o 

C_DEPS += \
./driver/canfdspi/drv_canfdspi_api.d 


# Each subdirectory must supply rules for building sources it contributes
driver/canfdspi/%.o: ../driver/canfdspi/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DDEBUG -DMICROCONTROLLER -D__CODE_RED -DCORE_M0 -D__USE_CMSIS=CMSISv2p00_LPC11xx -D__LPC11XX__ -D__REDLIB__ -I"D:\NXP_workspace\MCP2517FD_ExampleFor_LPC111X\inc" -I"D:\NXP_workspace\CMSISv2p00_LPC11xx\inc" -O0 -fno-common -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m0 -mthumb -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


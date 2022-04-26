################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/GPIO_Driver.c \
../src/I2C_Driver.c \
../src/MCP2517FD_ExampleFor_LPC111X.c \
../src/SPI_Driver.c \
../src/UART_Driver.c \
../src/cr_startup_lpc11xx.c \
../src/crp.c 

OBJS += \
./src/GPIO_Driver.o \
./src/I2C_Driver.o \
./src/MCP2517FD_ExampleFor_LPC111X.o \
./src/SPI_Driver.o \
./src/UART_Driver.o \
./src/cr_startup_lpc11xx.o \
./src/crp.o 

C_DEPS += \
./src/GPIO_Driver.d \
./src/I2C_Driver.d \
./src/MCP2517FD_ExampleFor_LPC111X.d \
./src/SPI_Driver.d \
./src/UART_Driver.d \
./src/cr_startup_lpc11xx.d \
./src/crp.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DDEBUG -DMICROCONTROLLER -D__CODE_RED -DCORE_M0 -D__USE_CMSIS=CMSISv2p00_LPC11xx -D__LPC11XX__ -D__REDLIB__ -I"D:\NXP_workspace\MCP2517FD_ExampleFor_LPC111X\inc" -I"D:\NXP_workspace\CMSISv2p00_LPC11xx\inc" -O0 -fno-common -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m0 -mthumb -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



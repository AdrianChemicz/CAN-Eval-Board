################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include driver/spi/subdir.mk
-include driver/canfdspi/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: MCP2517FD_ExampleFor_LPC11UXX.axf

# Tool invocations
MCP2517FD_ExampleFor_LPC11UXX.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -L"D:\NXP_workspace\lpc_chip_11uxx_lib\Debug" -Xlinker -Map="MCP2517FD_ExampleFor_LPC11UXX.map" -Xlinker --gc-sections -Xlinker -print-memory-usage -mcpu=cortex-m0 -mthumb -T "MCP2517FD_ExampleFor_LPC11UXX_Debug.ld" -o "MCP2517FD_ExampleFor_LPC11UXX.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) MCP2517FD_ExampleFor_LPC11UXX.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "MCP2517FD_ExampleFor_LPC11UXX.axf"; # arm-none-eabi-objcopy -v -O binary "MCP2517FD_ExampleFor_LPC11UXX.axf" "MCP2517FD_ExampleFor_LPC11UXX.bin" ; # checksum -p LPC11U24_301 -d "MCP2517FD_ExampleFor_LPC11UXX.bin";
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets

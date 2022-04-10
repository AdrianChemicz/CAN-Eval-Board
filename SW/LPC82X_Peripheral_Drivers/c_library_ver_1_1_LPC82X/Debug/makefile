################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
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
all: c_library_ver_1_1_LPC82X.axf

# Tool invocations
c_library_ver_1_1_LPC82X.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -L"D:\NXP_workspace\lpc_chip_82x\Debug" -Xlinker -Map="c_library_ver_1_1_LPC82X.map" -Xlinker --gc-sections -Xlinker -print-memory-usage -mcpu=cortex-m0 -mthumb -T "c_library_ver_1_1_LPC82X_Debug.ld" -o "c_library_ver_1_1_LPC82X.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) c_library_ver_1_1_LPC82X.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "c_library_ver_1_1_LPC82X.axf"; # arm-none-eabi-objcopy -v -O binary "c_library_ver_1_1_LPC82X.axf" "c_library_ver_1_1_LPC82X.bin" ; # checksum -p LPC822 -d "c_library_ver_1_1_LPC82X.bin";
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets

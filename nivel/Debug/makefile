################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include cunitTest/subdir.mk
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
all: nivel

dependents:
	-cd ../../so-commons-library/Debug && $(MAKE) all
	-cd ../../nivel-gui/Debug && $(MAKE) all

# Tool invocations
nivel: $(OBJS) $(USER_OBJS) ../../so-commons-library/Debug/libso-commons-library.so ../../nivel-gui/Debug/libnivel-gui.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"../../nivel-gui/Debug" -L"../../so-commons-library/Debug" -o "nivel" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) nivel
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
../../so-commons-library/Debug/libso-commons-library.so:
../../nivel-gui/Debug/libnivel-gui.so:

-include ../makefile.targets

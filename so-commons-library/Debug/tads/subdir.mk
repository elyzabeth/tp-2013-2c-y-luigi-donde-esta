################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../tads/tad_nivel.c \
../tads/tad_personaje.c 

OBJS += \
./tads/tad_nivel.o \
./tads/tad_personaje.o 

C_DEPS += \
./tads/tad_nivel.d \
./tads/tad_personaje.d 


# Each subdirectory must supply rules for building sources it contributes
tads/%.o: ../tads/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



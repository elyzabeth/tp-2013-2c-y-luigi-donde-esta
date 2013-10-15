################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../config/configNivel.c \
../config/configPersonaje.c \
../config/configPlataforma.c \
../config/funciones.c 

OBJS += \
./config/configNivel.o \
./config/configPersonaje.o \
./config/configPlataforma.o \
./config/funciones.o 

C_DEPS += \
./config/configNivel.d \
./config/configPersonaje.d \
./config/configPlataforma.d \
./config/funciones.d 


# Each subdirectory must supply rules for building sources it contributes
config/%.o: ../config/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



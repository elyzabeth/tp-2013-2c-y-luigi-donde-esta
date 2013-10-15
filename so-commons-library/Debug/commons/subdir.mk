################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../commons/bitarray.c \
../commons/comunicacion.c \
../commons/config.c \
../commons/error.c \
../commons/funciones_comunes.c \
../commons/log.c \
../commons/process.c \
../commons/string.c \
../commons/temporal.c \
../commons/txt.c 

OBJS += \
./commons/bitarray.o \
./commons/comunicacion.o \
./commons/config.o \
./commons/error.o \
./commons/funciones_comunes.o \
./commons/log.o \
./commons/process.o \
./commons/string.o \
./commons/temporal.o \
./commons/txt.o 

C_DEPS += \
./commons/bitarray.d \
./commons/comunicacion.d \
./commons/config.d \
./commons/error.d \
./commons/funciones_comunes.d \
./commons/log.d \
./commons/process.d \
./commons/string.d \
./commons/temporal.d \
./commons/txt.d 


# Each subdirectory must supply rules for building sources it contributes
commons/%.o: ../commons/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



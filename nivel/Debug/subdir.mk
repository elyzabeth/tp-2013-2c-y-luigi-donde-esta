################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../NivelMain.c \
../enemigo.c \
../funcionesNivel.c \
../interbloqueo.c 

OBJS += \
./NivelMain.o \
./enemigo.o \
./funcionesNivel.o \
./interbloqueo.o 

C_DEPS += \
./NivelMain.d \
./enemigo.d \
./funcionesNivel.d \
./interbloqueo.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"../../nivel-gui" -I"../../so-commons-library" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



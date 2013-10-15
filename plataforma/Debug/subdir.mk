################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../listas_compartidas.c \
../orquestador.c \
../planificador.c \
../plataforma.c 

OBJS += \
./listas_compartidas.o \
./orquestador.o \
./planificador.o \
./plataforma.o 

C_DEPS += \
./listas_compartidas.d \
./orquestador.d \
./planificador.d \
./plataforma.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"../../so-commons-library" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



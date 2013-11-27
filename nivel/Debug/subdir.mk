################################################################################
# Automatically-generated file. Do not edit!
################################################################################



# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../NivelMain.c \
../enemigo.c \
../funcionesNivel.c \
../interbloqueo.c \
../listasCompartidasNivel.c 

OBJS += \
./NivelMain.o \
./enemigo.o \
./funcionesNivel.o \
./interbloqueo.o \
./listasCompartidasNivel.o 

C_DEPS += \
./NivelMain.d \
./enemigo.d \
./funcionesNivel.d \
./interbloqueo.d \
./listasCompartidasNivel.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/arwen/201302/git/tp-2013-2c-y-luigi-donde-esta/nivel-gui" -I"/home/arwen/201302/git/tp-2013-2c-y-luigi-donde-esta/so-commons-library" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



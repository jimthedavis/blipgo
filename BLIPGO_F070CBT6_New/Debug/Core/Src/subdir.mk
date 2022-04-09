################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/beeper.c \
../Core/Src/cmmain.c \
../Core/Src/console.c \
../Core/Src/debug.c \
../Core/Src/espif.c \
../Core/Src/espifat.c \
../Core/Src/gsm.c \
../Core/Src/jpd500f.c \
../Core/Src/junk.c \
../Core/Src/led.c \
../Core/Src/main.c \
../Core/Src/quectel.c \
../Core/Src/stm32f0xx_hal_msp.c \
../Core/Src/stm32f0xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f0xx.c \
../Core/Src/template.c \
../Core/Src/usbcdc.c \
../Core/Src/usbmain.c \
../Core/Src/winbond_mem.c 

OBJS += \
./Core/Src/beeper.o \
./Core/Src/cmmain.o \
./Core/Src/console.o \
./Core/Src/debug.o \
./Core/Src/espif.o \
./Core/Src/espifat.o \
./Core/Src/gsm.o \
./Core/Src/jpd500f.o \
./Core/Src/junk.o \
./Core/Src/led.o \
./Core/Src/main.o \
./Core/Src/quectel.o \
./Core/Src/stm32f0xx_hal_msp.o \
./Core/Src/stm32f0xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f0xx.o \
./Core/Src/template.o \
./Core/Src/usbcdc.o \
./Core/Src/usbmain.o \
./Core/Src/winbond_mem.o 

C_DEPS += \
./Core/Src/beeper.d \
./Core/Src/cmmain.d \
./Core/Src/console.d \
./Core/Src/debug.d \
./Core/Src/espif.d \
./Core/Src/espifat.d \
./Core/Src/gsm.d \
./Core/Src/jpd500f.d \
./Core/Src/junk.d \
./Core/Src/led.d \
./Core/Src/main.d \
./Core/Src/quectel.d \
./Core/Src/stm32f0xx_hal_msp.d \
./Core/Src/stm32f0xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f0xx.d \
./Core/Src/template.d \
./Core/Src/usbcdc.d \
./Core/Src/usbmain.d \
./Core/Src/winbond_mem.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F070xB -c -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Core/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F0xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -Wno-format -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"


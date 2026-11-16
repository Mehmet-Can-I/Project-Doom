################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/app.c \
../Core/Src/keypad.c \
../Core/Src/lcd_hd44780.c \
../Core/Src/main.c \
../Core/Src/rgb_led.c \
../Core/Src/snake.c \
../Core/Src/ssd1306.c \
../Core/Src/ssd1306_fonts.c \
../Core/Src/status_led.c \
../Core/Src/stm32l4xx_hal_msp.c \
../Core/Src/stm32l4xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32l4xx.c 

OBJS += \
./Core/Src/app.o \
./Core/Src/keypad.o \
./Core/Src/lcd_hd44780.o \
./Core/Src/main.o \
./Core/Src/rgb_led.o \
./Core/Src/snake.o \
./Core/Src/ssd1306.o \
./Core/Src/ssd1306_fonts.o \
./Core/Src/status_led.o \
./Core/Src/stm32l4xx_hal_msp.o \
./Core/Src/stm32l4xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32l4xx.o 

C_DEPS += \
./Core/Src/app.d \
./Core/Src/keypad.d \
./Core/Src/lcd_hd44780.d \
./Core/Src/main.d \
./Core/Src/rgb_led.d \
./Core/Src/snake.d \
./Core/Src/ssd1306.d \
./Core/Src/ssd1306_fonts.d \
./Core/Src/status_led.d \
./Core/Src/stm32l4xx_hal_msp.d \
./Core/Src/stm32l4xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32l4xx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_NUCLEO_64 -DUSE_HAL_DRIVER -DSTM32L476xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/BSP/STM32L4xx_Nucleo -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/app.cyclo ./Core/Src/app.d ./Core/Src/app.o ./Core/Src/app.su ./Core/Src/keypad.cyclo ./Core/Src/keypad.d ./Core/Src/keypad.o ./Core/Src/keypad.su ./Core/Src/lcd_hd44780.cyclo ./Core/Src/lcd_hd44780.d ./Core/Src/lcd_hd44780.o ./Core/Src/lcd_hd44780.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/rgb_led.cyclo ./Core/Src/rgb_led.d ./Core/Src/rgb_led.o ./Core/Src/rgb_led.su ./Core/Src/snake.cyclo ./Core/Src/snake.d ./Core/Src/snake.o ./Core/Src/snake.su ./Core/Src/ssd1306.cyclo ./Core/Src/ssd1306.d ./Core/Src/ssd1306.o ./Core/Src/ssd1306.su ./Core/Src/ssd1306_fonts.cyclo ./Core/Src/ssd1306_fonts.d ./Core/Src/ssd1306_fonts.o ./Core/Src/ssd1306_fonts.su ./Core/Src/status_led.cyclo ./Core/Src/status_led.d ./Core/Src/status_led.o ./Core/Src/status_led.su ./Core/Src/stm32l4xx_hal_msp.cyclo ./Core/Src/stm32l4xx_hal_msp.d ./Core/Src/stm32l4xx_hal_msp.o ./Core/Src/stm32l4xx_hal_msp.su ./Core/Src/stm32l4xx_it.cyclo ./Core/Src/stm32l4xx_it.d ./Core/Src/stm32l4xx_it.o ./Core/Src/stm32l4xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32l4xx.cyclo ./Core/Src/system_stm32l4xx.d ./Core/Src/system_stm32l4xx.o ./Core/Src/system_stm32l4xx.su

.PHONY: clean-Core-2f-Src


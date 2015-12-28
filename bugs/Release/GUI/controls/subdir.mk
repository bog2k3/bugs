################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../GUI/controls/Button.cpp \
../GUI/controls/TextField.cpp 

OBJS += \
./GUI/controls/Button.o \
./GUI/controls/TextField.o 

CPP_DEPS += \
./GUI/controls/Button.d \
./GUI/controls/TextField.d 


# Each subdirectory must supply rules for building sources it contributes
GUI/controls/%.o: ../GUI/controls/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -I"/mnt/docs/Work/bugs/bugs" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



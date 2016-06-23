################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../input/GLFWInput.cpp 

OBJS += \
./input/GLFWInput.o 

CPP_DEPS += \
./input/GLFWInput.d 


# Each subdirectory must supply rules for building sources it contributes
input/GLFWInput.o: /mnt/docs/Work/bugs/bugs/input/GLFWInput.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -DDEBUG -I/home/bog/work/bugs/3rdparty/easyunit -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



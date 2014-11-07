################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../input/GLFWInput.cpp \
../input/OperationPan.cpp 

OBJS += \
./input/GLFWInput.o \
./input/OperationPan.o 

CPP_DEPS += \
./input/GLFWInput.d \
./input/OperationPan.d 


# Each subdirectory must supply rules for building sources it contributes
input/%.o: ../input/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -DDEBUG -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



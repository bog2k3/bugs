################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../input/operations/OperationGui.cpp \
../input/operations/OperationPan.cpp \
../input/operations/OperationSpring.cpp \
../input/operations/OperationsStack.cpp 

OBJS += \
./input/operations/OperationGui.o \
./input/operations/OperationPan.o \
./input/operations/OperationSpring.o \
./input/operations/OperationsStack.o 

CPP_DEPS += \
./input/operations/OperationGui.d \
./input/operations/OperationPan.d \
./input/operations/OperationSpring.d \
./input/operations/OperationsStack.d 


# Each subdirectory must supply rules for building sources it contributes
input/operations/%.o: ../input/operations/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++0x -DGLM_FORCE_RADIANS -I"/mnt/docs/Work/bugs/bugs" -O3 -ftest-coverage -fprofile-arcs -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


